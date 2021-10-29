/*
    SPDX-FileCopyrightText: 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
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

#include <QAction>

#include <KActionCollection>
#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <interfaces/contextmenuextension.h>
#include <interfaces/idocumentcontroller.h>

#include <language/interfaces/codecontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/typeutils.h>
#include <documentation/documentationview.h>

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
        auto* idType = dynamic_cast<IdentifiedType*>(type.data());
        Declaration* idDecl = idType ? idType->declaration(decl->topContext()) : nullptr;
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
    {}

    QWidget* create(QWidget *parent = nullptr) override
    {
        if (!m_providersModel) {
            m_providersModel.reset(new ProvidersModel);
        }
        return new DocumentationView(parent, m_providersModel.data());
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::RightDockWidgetArea;
    }
    QString id() const override { return QStringLiteral("org.kdevelop.DocumentationView"); }
    QList<QAction*> contextMenuActions(QWidget* viewWidget) const override
    {
        auto documentationViewWidget = qobject_cast<DocumentationView*>(viewWidget);
        Q_ASSERT(documentationViewWidget);
        return documentationViewWidget->contextMenuActions();
    }

private:
    QScopedPointer<ProvidersModel> m_providersModel;
};

DocumentationController::DocumentationController(Core* core)
    : m_factory(new DocumentationViewFactory)
{
    m_showDocumentation = core->uiController()->activeMainWindow()->actionCollection()->addAction(QStringLiteral("showDocumentation"));
    m_showDocumentation->setText(i18nc("@action", "Show Documentation"));
    m_showDocumentation->setIcon(QIcon::fromTheme(QStringLiteral("documentation")));
    connect(m_showDocumentation, &QAction::triggered, this, &DocumentationController::doShowDocumentation);

    // registering the tool view here so it registered before the areas are restored
    // and thus also gets treated like the ones registered from plugins
    // cmp. comment about tool views in CorePrivate::initialize
    core->uiController()->addToolView(i18nc("@title:window", "Documentation"), m_factory);
}

DocumentationController::~DocumentationController()
{
}

void DocumentationController::initialize()
{
}


void KDevelop::DocumentationController::doShowDocumentation()
{
    KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
    if(!view)
      return;

    KDevelop::DUChainReadLocker lock( DUChain::lock() );

    Declaration* decl = usefulDeclaration(DUChainUtils::itemUnderCursor(view->document()->url(), KTextEditor::Cursor(view->cursorPosition())).declaration);
    auto documentation = documentationForDeclaration(decl);
    if (documentation) {
        showDocumentation(documentation);
    }
}

KDevelop::ContextMenuExtension KDevelop::DocumentationController::contextMenuExtension(Context* context, QWidget* parent)
{
    Q_UNUSED(parent);

    ContextMenuExtension menuExt;

    auto* ctx = dynamic_cast<DeclarationContext*>(context);
    if(ctx) {
        DUChainReadLocker lock(DUChain::lock());
        if(!ctx->declaration().data())
            return menuExt;

        auto doc = documentationForDeclaration(ctx->declaration().data());
        if (doc) {
            menuExt.addAction(ContextMenuExtension::ExtensionGroup, m_showDocumentation);
        }
    }

    return menuExt;
}

IDocumentation::Ptr DocumentationController::documentationForDeclaration(Declaration* decl)
{
    if (!decl)
        return {};

    const auto documentationProviders = this->documentationProviders();
    for (IDocumentationProvider* doc : documentationProviders) {
        qCDebug(SHELL) << "Documentation provider found:" << doc;
        auto ret = doc->documentationForDeclaration(decl);

        qCDebug(SHELL) << "Documentation proposed: " << ret.data();
        if (ret) {
            return ret;
        }
    }

    return {};
}

IDocumentation::Ptr DocumentationController::documentation(const QUrl& url) const
{
    const auto providers = this->documentationProviders();
    for (const IDocumentationProvider* provider : providers) {
        IDocumentation::Ptr doc = provider->documentation(url);
        if (doc) {
            return doc;
        }
    }
    return {};
}

QList< IDocumentationProvider* > DocumentationController::documentationProviders() const
{
    const QList<IPlugin*> plugins = ICore::self()->pluginController()->allPluginsForExtension(QStringLiteral("org.kdevelop.IDocumentationProvider"));
    const QList<IPlugin*> pluginsProvider = ICore::self()->pluginController()->allPluginsForExtension(QStringLiteral("org.kdevelop.IDocumentationProviderProvider"));

    QList<IDocumentationProvider*> ret;
    for (IPlugin* p : pluginsProvider) {
        auto *docProvider=p->extension<IDocumentationProviderProvider>();
        if (!docProvider) {
            qCWarning(SHELL) << "plugin" << p << "does not implement ProviderProvider extension, rerun kbuildsycoca5";
            continue;
        }
        ret.append(docProvider->providers());
    }

    for (IPlugin* p : plugins) {
        auto *doc=p->extension<IDocumentationProvider>();
        if (!doc) {
            qCWarning(SHELL) << "plugin" << p << "does not implement Provider extension, rerun kbuildsycoca5";
            continue;
        }
        ret.append(doc);
    }

    return ret;
}

void KDevelop::DocumentationController::showDocumentation(const IDocumentation::Ptr& doc)
{
    Q_ASSERT_X(doc, Q_FUNC_INFO, "Null documentation pointer is unsupported.");
    QWidget* w = ICore::self()->uiController()->findToolView(i18nc("@title:window", "Documentation"), m_factory, KDevelop::IUiController::CreateAndRaise);
    if(!w) {
        qCWarning(SHELL) << "Could not add documentation tool view";
        return;
    }

    auto* view = dynamic_cast<DocumentationView*>(w);
    if( !view ) {
        qCWarning(SHELL) << "Could not cast tool view" << w << "to DocumentationView class!";
        return;
    }
    view->showDocumentation(doc);
}

void DocumentationController::changedDocumentationProviders()
{
    emit providersChanged();
}

