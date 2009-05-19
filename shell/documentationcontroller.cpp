/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   
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
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <shell/core.h>
#include <sublime/view.h>

#include <KDebug>

#include "documentationview.h"

using namespace KDevelop;

class DocumentationViewFactory: public KDevelop::IToolViewFactory
{
    public:
        DocumentationViewFactory()
        {}
        
        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new DocumentationView( parent );
        }
        
        virtual Qt::DockWidgetArea defaultPosition() { return Qt::RightDockWidgetArea; }
        virtual QString id() const { return "org.kdevelop.DocumentationView"; }
        
    private:
};

DocumentationController::DocumentationController(Core* core)
    : m_factory(new DocumentationViewFactory)
{
}

void DocumentationController::initialize()
{
    if(!(Core::self()->setupFlags() & Core::NoUi)) {
        Core::self()->uiController()->addToolView( i18n("Documentation"), m_factory );
    }
}

KSharedPtr< KDevelop::IDocumentation > DocumentationController::documentationForDeclaration(Declaration* decl)
{
    QList<IPlugin*> m_plugins=ICore::self()->pluginController()->allPluginsForExtension(IDocumentationProvider_iid);
    kDebug(9529) << "All plugins for" << IDocumentationProvider_iid << m_plugins;
    KSharedPtr<KDevelop::IDocumentation> ret;
    
    foreach(IPlugin* p, m_plugins)
    {
        IDocumentationProvider *doc=dynamic_cast<IDocumentationProvider*>(p);
        if(doc)
        {
            kDebug(9529) << "Documentation provider found:" << doc;
            ret=doc->documentationForDeclaration(decl);
            
            kDebug(9529) << "Documentation proposed: " << ret;
            if(ret)
                break;
        }
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
    Q_ASSERT(view);
    view->showDocumentation(doc);
}
