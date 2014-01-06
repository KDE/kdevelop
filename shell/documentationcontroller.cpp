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
#include <interfaces/iplugin.h>
#include <interfaces/idocumentationprovider.h>
#include <interfaces/idocumentationproviderprovider.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <shell/core.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <KDebug>

#include <interfaces/contextmenuextension.h>
#include <interfaces/idocumentcontroller.h>

#include <language/interfaces/codecontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <documentation/documentationview.h>
#include <KParts/MainWindow>
#include <KActionCollection>

using namespace KDevelop;

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
    connect(m_showDocumentation, SIGNAL(triggered(bool)), SLOT(doShowDocumentation()));
}

void DocumentationController::initialize()
{
    if(!documentationProviders().isEmpty() && !(Core::self()->setupFlags() & Core::NoUi)) {
        Core::self()->uiController()->addToolView( i18n("Documentation"), m_factory );
    }
}


void KDevelop::DocumentationController::doShowDocumentation()
{
    IDocument* doc = ICore::self()->documentController()->activeDocument();
    if(!doc)
      return;
    
    KTextEditor::Document* textDoc = doc->textDocument();
    if(!textDoc)
      return;
    
    KTextEditor::View* view = textDoc->activeView();
    if(!view)
      return;
    
    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    
    Declaration *dec = DUChainUtils::declarationForDefinition( DUChainUtils::itemUnderCursor( doc->url(), SimpleCursor(view->cursorPosition()) ) );
    
    if(dec) {
        KSharedPtr< IDocumentation > documentation = documentationForDeclaration(dec);
        if(documentation) {
            showDocumentation(documentation);
        }
    }
}


Q_DECLARE_METATYPE(KSharedPtr<KDevelop::IDocumentation>)

KDevelop::ContextMenuExtension KDevelop::DocumentationController::contextMenuExtension ( Context* context )
{
    ContextMenuExtension menuExt;
    
    qRegisterMetaType<KSharedPtr<KDevelop::IDocumentation> >("KSharedPtr<KDevelop::IDocumentation>");
    
    DeclarationContext* ctx = dynamic_cast<DeclarationContext*>(context);
    if(ctx) {
        DUChainReadLocker lock(DUChain::lock());
        if(!ctx->declaration().data())
            return menuExt;
        
        KSharedPtr< IDocumentation > doc = documentationForDeclaration(ctx->declaration().data());
        if(doc) {
            menuExt.addAction(ContextMenuExtension::ExtensionGroup, m_showDocumentation);;
        }
    }
    
    return menuExt;
}

KSharedPtr< KDevelop::IDocumentation > DocumentationController::documentationForDeclaration(Declaration* decl)
{
    KSharedPtr<KDevelop::IDocumentation> ret;
    
    foreach(IDocumentationProvider* doc, documentationProviders())
    {
        kDebug(9529) << "Documentation provider found:" << doc;
        ret=doc->documentationForDeclaration(decl);
        
        kDebug(9529) << "Documentation proposed: " << ret;
        if(ret)
            break;
    }
    
    return ret;
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
            kWarning() << "plugin" << p << "does not implement ProviderProvider extension, rerun kbuildsycoca4";
            continue;
        }
        ret.append(docProvider->providers());
    }
    
    foreach(IPlugin* p, plugins)
    {
        IDocumentationProvider *doc=p->extension<IDocumentationProvider>();
        if (!doc) {
            kWarning() << "plugin" << p << "does not implement Provider extension, rerun kbuildsycoca4";
            continue;
        }
        ret.append(doc);
    }
    
    return ret;
}

void KDevelop::DocumentationController::showDocumentation(KSharedPtr< KDevelop::IDocumentation > doc)
{
    QWidget* w = ICore::self()->uiController()->findToolView(i18n("Documentation"), m_factory, KDevelop::IUiController::CreateAndRaise);
    if(!w) {
        kWarning() << "Could not add documentation toolview";
        return;
    }
    
    DocumentationView* view = dynamic_cast<DocumentationView*>(w);
    if( !view ) {
        kWarning() << "Could not cast toolview" << w << "to DocumentationView class!";
        return;
    }
    view->showDocumentation(doc);
}

void DocumentationController::changedDocumentationProviders()
{
    emit providersChanged();
}

#include "documentationcontroller.moc"
