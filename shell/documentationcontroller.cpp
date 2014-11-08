/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "documentationcontroller.h"
#include "debug.h"

#include <interfaces/iplugin.h>
#include <interfaces/idocumentationprovider.h>
#include <interfaces/idocumentationproviderprovider.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <shell/core.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <interfaces/contextmenuextension.h>
#include <interfaces/idocumentcontroller.h>

#include <language/interfaces/codecontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/typeutils.h>
#include <documentation/documentationview.h>
#include <KParts/MainWindow>
#include <KActionCollection>
#include <QAction>

using namespace KDevelop;

namespace {

/**
 * Return a "more useful" declaration that documentation providers can look-up
 *
 * @code
 *   QPoint point;
 *            ^-- cursor here
 * @endcode
 *
 * In this case, this method returns a Declaration pointer to the *type*
 * instead of a pointer to the instance, which is more useful when looking for help
 *
 * @return A more appropriate Declaration pointer or the given parameter @p decl
 */
Declaration* usefulDeclaration(Declaration* decl)
{
    if (!decl)
        return nullptr;

    // First: Attempt to find the declaration of a definition
    decl = DUChainUtils::declarationForDefinition(decl);

    // Convenience feature: Retrieve the type declaration of instances,
    // it makes no sense to pass the declaration pointer of instances of types
    if (decl->kind() == Declaration::Instance) {
        AbstractType::Ptr type = TypeUtils::targetTypeKeepAliases(decl->abstractType(), decl->topContext());
        IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.data());
        Declaration* idDecl = idType ? idType->declaration(decl->topContext()) : 0;
        if (idDecl) {
            decl = idDecl;
        }
    }
    return decl;
}

}

class DocumentationViewFactory: public KDevelop::IToolViewFactory
{
    public:
        DocumentationViewFactory()
            : mProvidersModel(0)
        {}

        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new DocumentationView( parent, providers() );
        }

        virtual Qt::DockWidgetArea defaultPosition() { return Qt::RightDockWidgetArea; }
        virtual QString id() const { return "org.kdevelop.DocumentationView"; }

    private:
        ProvidersModel* providers() {
            if(!mProvidersModel)
                mProvidersModel = new ProvidersModel;

            return mProvidersModel;
        }

        ProvidersModel* mProvidersModel;
};

DocumentationController::DocumentationController(Core* core)
    : m_factory(new DocumentationViewFactory)
{
    m_showDocumentation = core->uiController()->activeMainWindow()->actionCollection()->addAction("showDocumentation");
    m_showDocumentation->setText(i18n("Show Documentation"));
    m_showDocumentation->setIcon(QIcon::fromTheme("documentation"));
    connect(m_showDocumentation, &QAction::triggered, this, &DocumentationController::doShowDocumentation);
}

void DocumentationController::initialize()
{
    if(!documentationProviders().isEmpty() && !(Core::self()->setupFlags() & Core::NoUi)) {
        Core::self()->uiController()->addToolView( i18n("Documentation"), m_factory );
    }
}


void KDevelop::DocumentationController::doShowDocumentation()
{
    KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
    if(!view)
      return;

    KDevelop::DUChainReadLocker lock( DUChain::lock() );

    Declaration* decl = usefulDeclaration(DUChainUtils::itemUnderCursor(view->document()->url(), KTextEditor::Cursor(view->cursorPosition())));
    auto documentation = documentationForDeclaration(decl);
    if (documentation) {
        showDocumentation(documentation);
    }
}

KDevelop::ContextMenuExtension KDevelop::DocumentationController::contextMenuExtension ( Context* context )
{
    ContextMenuExtension menuExt;

    DeclarationContext* ctx = dynamic_cast<DeclarationContext*>(context);
    if(ctx) {
        DUChainReadLocker lock(DUChain::lock());
        if(!ctx->declaration().data())
            return menuExt;

        auto doc = documentationForDeclaration(ctx->declaration().data());
        if (doc) {
            menuExt.addAction(ContextMenuExtension::ExtensionGroup, m_showDocumentation);;
        }
    }

    return menuExt;
}

IDocumentation::Ptr DocumentationController::documentationForDeclaration(Declaration* decl)
{
    if (!decl)
        return {};

    foreach (IDocumentationProvider* doc, documentationProviders()) {
        qCDebug(SHELL) << "Documentation provider found:" << doc;
        auto ret = doc->documentationForDeclaration(decl);

        qCDebug(SHELL) << "Documentation proposed: " << ret.data();
        if (ret) {
            return ret;
        }
    }

    return {};
}


QList< IDocumentationProvider* > DocumentationController::documentationProviders() const
{
    QList<IPlugin*> plugins=ICore::self()->pluginController()->allPluginsForExtension("org.kdevelop.IDocumentationProvider");
    QList<IPlugin*> pluginsProvider=ICore::self()->pluginController()->allPluginsForExtension("org.kdevelop.IDocumentationProviderProvider");

    QList<IDocumentationProvider*> ret;
    foreach(IPlugin* p, pluginsProvider)
    {
        IDocumentationProviderProvider *docProvider=p->extension<IDocumentationProviderProvider>();
        if (!docProvider) {
            qWarning() << "plugin" << p << "does not implement ProviderProvider extension, rerun kbuildsycoca4";
            continue;
        }
        ret.append(docProvider->providers());
    }

    foreach(IPlugin* p, plugins)
    {
        IDocumentationProvider *doc=p->extension<IDocumentationProvider>();
        if (!doc) {
            qWarning() << "plugin" << p << "does not implement Provider extension, rerun kbuildsycoca4";
            continue;
        }
        ret.append(doc);
    }

    return ret;
}

void KDevelop::DocumentationController::showDocumentation(const IDocumentation::Ptr& doc)
{
    QWidget* w = ICore::self()->uiController()->findToolView(i18n("Documentation"), m_factory, KDevelop::IUiController::CreateAndRaise);
    if(!w) {
        qWarning() << "Could not add documentation toolview";
        return;
    }

    DocumentationView* view = dynamic_cast<DocumentationView*>(w);
    if( !view ) {
        qWarning() << "Could not cast toolview" << w << "to DocumentationView class!";
        return;
    }
    view->showDocumentation(doc);
}

void DocumentationController::changedDocumentationProviders()
{
    emit providersChanged();
}

