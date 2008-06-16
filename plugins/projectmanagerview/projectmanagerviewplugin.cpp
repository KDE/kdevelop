/* This file is part of KDevelop
   Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#include "projectmanagerviewplugin.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtCore/QSignalMapper>

#include <kservicetypetrader.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdialog.h>
#include <kaboutdata.h>
#include <kplugininfo.h>

#include <kparts/componentfactory.h>

#include "projectmodel.h"
#include "icore.h"
#include "iproject.h"
#include "iprojectfilemanager.h"
#include "ibuildsystemmanager.h"
#include "iuicontroller.h"
#include "iruncontroller.h"
#include "idocumentcontroller.h"
#include "iprojectbuilder.h"
#include "iprojectcontroller.h"
#include "importprojectjob.h"
#include "context.h"
#include <interfaces/contextmenuextension.h>

#include "projectmanagerview.h"
#include "projectbuildsetmodel.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(ProjectManagerFactory, registerPlugin<ProjectManagerViewPlugin>(); )
K_EXPORT_PLUGIN(ProjectManagerFactory("kdevprojectmanagerview"))

class KDevProjectManagerViewFactory: public KDevelop::IToolViewFactory
{
    public:
        KDevProjectManagerViewFactory( ProjectManagerViewPlugin *plugin ): mplugin( plugin )
        {}
        virtual QWidget* create( QWidget *parent = 0 )
        {
            return new ProjectManagerView( mplugin, parent );
        }
        virtual Qt::DockWidgetArea defaultPosition()
        {
            return Qt::RightDockWidgetArea;
        }
        virtual QString id() const
        {
            return "org.kdevelop.ProjectsView";
        }

    private:
        ProjectManagerViewPlugin *mplugin;
};

class ProjectManagerViewPluginPrivate
{
public:
    ProjectManagerViewPluginPrivate()
    {}
    KDevProjectManagerViewFactory *factory;
    QList<KDevelop::ProjectBaseItem*> ctxProjectItemList;
    KAction* m_buildAll;
    KAction* m_build;
    KAction* m_install;
    KAction* m_clean;
    KAction* m_configure;
    KAction* m_prune;
    ProjectBuildSetModel* buildSet;
};

ProjectManagerViewPlugin::ProjectManagerViewPlugin( QObject *parent, const QVariantList& )
        : IPlugin( ProjectManagerFactory::componentData(), parent ), d(new ProjectManagerViewPluginPrivate)
{
    d->buildSet = new ProjectBuildSetModel( this );

    connect( core()->projectController(), SIGNAL( projectOpened( KDevelop::IProject* ) ),
             d->buildSet, SLOT( loadFromProject( KDevelop::IProject* ) ) );

    connect( core()->projectController(), SIGNAL( projectClosing( KDevelop::IProject* ) ),
             d->buildSet, SLOT( saveToProject( KDevelop::IProject* ) ) );

    d->m_buildAll = new KAction( i18n("Build all Projects"), this );
    d->m_buildAll->setIcon(KIcon("run-build"));
    connect( d->m_buildAll, SIGNAL(triggered()), this, SLOT(buildAllProjects()) );
    actionCollection()->addAction( "project_buildall", d->m_buildAll );
    d->m_build = new KAction( i18n("Build"), this );
    d->m_build->setShortcut( Qt::Key_F8 );
    d->m_build->setIcon(KIcon("run-build"));
    connect( d->m_build, SIGNAL(triggered()), this, SLOT(buildProjectItems()) );
    actionCollection()->addAction( "project_build", d->m_build );
    d->m_install = new KAction( i18n("Install"), this );
    connect( d->m_install, SIGNAL(triggered()), this, SLOT(installProjectItems()) );
    actionCollection()->addAction( "project_install", d->m_install );
    d->m_clean = new KAction( i18n("Clean"), this );
    connect( d->m_clean, SIGNAL(triggered()), this, SLOT(cleanProjectItems()) );
    actionCollection()->addAction( "project_clean", d->m_clean );
    d->m_configure = new KAction( i18n("Configure"), this );
    d->m_configure->setIcon(KIcon("configure"));
    connect( d->m_configure, SIGNAL(triggered()), this, SLOT(configureProjectItems()) );
    actionCollection()->addAction( "project_configure", d->m_configure );
    d->m_prune = new KAction( i18n("Prune"), this );
    connect( d->m_prune, SIGNAL(triggered()), this, SLOT(pruneProjectItems()) );
    actionCollection()->addAction( "project_prune", d->m_prune );
    setXMLFile( "kdevprojectmanagerview.rc" );
    d->factory = new KDevProjectManagerViewFactory( this );
    core()->uiController()->addToolView( i18n("Projects"), d->factory );

}

ProjectManagerViewPlugin::~ProjectManagerViewPlugin()
{
    delete d;
}

void ProjectManagerViewPlugin::unload()
{
    kDebug() << "unloading manager view";
    core()->uiController()->removeToolView(d->factory);
}

ContextMenuExtension ProjectManagerViewPlugin::contextMenuExtension( KDevelop::Context* context )
{
    d->ctxProjectItemList.clear();
    if( context->type() != KDevelop::Context::ProjectItemContext )
        return IPlugin::contextMenuExtension( context );

    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    QList<KDevelop::ProjectBaseItem*> items = ctx->items();

    if( items.isEmpty() )
        return IPlugin::contextMenuExtension( context );

    ContextMenuExtension menuExt;
    bool closeProjectsAdded = false;
    bool buildItemsAdded = false;
    foreach( ProjectBaseItem* item, items )
    {
        d->ctxProjectItemList << item;
        KDevelop::ProjectFolderItem *prjitem = dynamic_cast<KDevelop::ProjectFolderItem*>(item);
        if ( !buildItemsAdded && ( item->folder() || item->target() || item->folder() ) )
        {
            KAction* action = new KAction( i18n( "Build items(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(buildItemsFromContextMenu()) );
            menuExt.addAction( ContextMenuExtension::ProjectGroup, action );
            action = new KAction( i18n( "Install item(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(installItemsFromContextMenu()) );
            menuExt.addAction( ContextMenuExtension::ProjectGroup, action );
            action = new KAction( i18n( "Clean item(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(cleanItemsFromContextMenu()) );
            menuExt.addAction( ContextMenuExtension::ProjectGroup, action );
            action = new KAction( i18n( "Add item(s) to buildset" ), this );
            connect( action, SIGNAL(triggered() ), this, SLOT(addItemsFromContextMenuToBuildset() ) );
            menuExt.addAction( ContextMenuExtension::ProjectGroup, action );
            buildItemsAdded = true;
        }
        if ( !closeProjectsAdded && prjitem && prjitem->isProjectRoot() )
        {
            KAction* close = new KAction( i18n( "Close projects" ), this );
            connect( close, SIGNAL(triggered()), this, SLOT(closeProjects()) );
            menuExt.addAction( ContextMenuExtension::ProjectGroup, close );
            closeProjectsAdded = true;
        }

    }
//     if( items.count() == 1 )
//     {
//         KAction* action = new KAction( i18n( "Project Configuration" ), this );
//         connect( action, SIGNAL( triggered() ), this, SLOT( projectConfiguration() ) );
//         menuExt.addAction( ContextMenuExtension::ProjectGroup, action );
//     }

    return menuExt;
}


KDevelop::IProjectBuilder* ProjectManagerViewPlugin::getProjectBuilder( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return 0;
    IProject* project = item->project();
    if (!project)
        return 0;

    ProjectFolderItem* prjitem = project->projectItem();
    IPlugin* fmgr = project->managerPlugin();
    IBuildSystemManager* mgr = fmgr->extension<IBuildSystemManager>();
    if( mgr )
    {
        return mgr->builder( prjitem );
    }
    return 0;
}

void ProjectManagerViewPlugin::executeBuild( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return;
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Building item:" << item->text();

    core()->documentController()->saveAllDocuments(IDocument::Silent);

    if( builder )
        core()->runController()->registerJob(builder->build( item ));
}

void ProjectManagerViewPlugin::executeClean( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return;
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Cleaning item:" << item->text();
    if( builder )
        core()->runController()->registerJob(builder->clean( item ));
}

void ProjectManagerViewPlugin::executeInstall( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return;
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Installing item:" << item->text();

    core()->documentController()->saveAllDocuments(IDocument::Silent);

    if( builder )
        core()->runController()->registerJob(builder->install( item ));
}


void ProjectManagerViewPlugin::executeConfigure( KDevelop::IProject* item )
{
    if( !item )
        return;
    IProjectBuilder* builder = getProjectBuilder( item->projectItem() );
    kDebug(9511) << "Configuring item:" << item->name();

    core()->documentController()->saveAllDocuments(IDocument::Silent);

    if( builder )
        core()->runController()->registerJob(builder->configure( item ));
}

void ProjectManagerViewPlugin::executePrune( KDevelop::IProject* item )
{
    if( !item )
        return;
    IProjectBuilder* builder = getProjectBuilder( item->projectItem() );
    kDebug(9511) << "Pruning item:" << item->name();

    core()->documentController()->saveAllDocuments(IDocument::Silent);

    if( builder )
        core()->runController()->registerJob(builder->prune( item ));
}


void ProjectManagerViewPlugin::closeProjects()
{
    QList<KDevelop::IProject*> projectsToClose;
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        if( !projectsToClose.contains( item->project() ) )
        {
            projectsToClose << item->project();
        }
    }
    d->ctxProjectItemList.clear();
    foreach( KDevelop::IProject* proj, projectsToClose )
    {
        core()->projectController()->closeProject( proj );
    }
}


void ProjectManagerViewPlugin::installItemsFromContextMenu()
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        executeInstall( item );
    }
    d->ctxProjectItemList.clear();
}

void ProjectManagerViewPlugin::cleanItemsFromContextMenu()
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        executeClean( item );
    }
    d->ctxProjectItemList.clear();
}

void ProjectManagerViewPlugin::buildItemsFromContextMenu()
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        executeBuild( item );
    }
    d->ctxProjectItemList.clear();
}

void ProjectManagerViewPlugin::buildAllProjects()
{
    foreach( KDevelop::IProject* project, core()->projectController()->projects() )
    {
        executeBuild( project->projectItem() );
    }
}

void ProjectManagerViewPlugin::installProjectItems()
{
    foreach( BuildItem item, d->buildSet->items() )
    {
        executeInstall( item.findItem() );
    }
}

void ProjectManagerViewPlugin::pruneProjectItems()
{
    QSet<KDevelop::IProject*> projects;
    foreach( BuildItem item, d->buildSet->items() )
    {
        if( item.findItem() )
            projects << item.findItem()->project();
    }
    foreach( KDevelop::IProject* project, projects )
    {
        executePrune( project );
    }
}

void ProjectManagerViewPlugin::configureProjectItems()
{
    QSet<KDevelop::IProject*> projects;

    foreach( BuildItem item, d->buildSet->items() )
    {
        if( item.findItem() )
            projects << item.findItem()->project();
    }
    foreach( KDevelop::IProject* project, projects )
    {
        executeConfigure( project );
    }
}

void ProjectManagerViewPlugin::cleanProjectItems()
{
    foreach( BuildItem item, d->buildSet->items() )
    {
        executeClean( item.findItem() );
    }
}

void ProjectManagerViewPlugin::buildProjectItems()
{
    foreach( BuildItem item, d->buildSet->items() )
    {
        executeBuild( item.findItem() );
    }
}

ProjectBuildSetModel* ProjectManagerViewPlugin::buildSet()
{
    return d->buildSet;
}

void ProjectManagerViewPlugin::addItemsFromContextMenuToBuildset( )
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        buildSet()->addProjectItem( item );
    }
}

void ProjectManagerViewPlugin::projectConfiguration( )
{
    if( d->ctxProjectItemList.count() > 0 )
    {
        core()->projectController()->configureProject( d->ctxProjectItemList.at( 0 )->project() );
    }
}


#include "projectmanagerviewplugin.moc"

