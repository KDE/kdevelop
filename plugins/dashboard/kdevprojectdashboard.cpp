/* This file is part of KDevelop
  Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kdevprojectdashboard.h"

#include <QAction>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include "dashboarddocument.h"

K_PLUGIN_FACTORY(KDevProjectDashboardFactory, registerPlugin<KDevProjectDashboard>(); )

using namespace KDevelop;

class ProjectDashboardFactory : public KDevelop::IDocumentFactory
{
    public:
        /**
         * Only create a document instance in case we are selecting
         * the .kdev4 project file of an *open* project
         */
        virtual IDocument* create(const QUrl& url, ICore*)
        {
            IProject* proj = ICore::self()->projectController()->findProjectForUrl(url);
            if (!proj || proj->projectFileUrl() != url)
                return 0;

            return new DashboardDocument(proj);
        }
};

KDevProjectDashboard::KDevProjectDashboard( QObject* parent, const QVariantList& )
    : IPlugin(KDevProjectDashboardFactory::componentData(), parent )
{
    ICore::self()->documentController()->registerDocumentForMimetype("text/x-kdevelop", new ProjectDashboardFactory);
}

KDevelop::ContextMenuExtension KDevProjectDashboard::contextMenuExtension(KDevelop::Context* context)
{
    m_selectedProjects.clear();
    if( context->type() != KDevelop::Context::ProjectItemContext )
        return IPlugin::contextMenuExtension( context );
    
    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    QList<KDevelop::ProjectBaseItem*> items = ctx->items();

    foreach(KDevelop::ProjectBaseItem* item, items) {
        ProjectFolderItem* folder = item->folder();
        if(folder && folder->isProjectRoot())
            m_selectedProjects += folder->project();
    }

    ContextMenuExtension menuExt;
    if(!m_selectedProjects.isEmpty()) {
        QAction* action = new QAction(QIcon::fromTheme("dashboard-show"), i18n("Show Dashboard"), this);
        connect(action, SIGNAL(triggered()), SLOT(showDashboard()));
        menuExt.addAction(ContextMenuExtension::ProjectGroup, action);
    }
    return menuExt;
}

void KDevProjectDashboard::showDashboard()
{
    foreach(IProject* proj, m_selectedProjects) {
        DashboardDocument* doc=new DashboardDocument(proj);
        ICore::self()->documentController()->openDocument(doc);
    }
}
