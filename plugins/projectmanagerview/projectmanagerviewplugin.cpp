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
#include "idocumentcontroller.h"
#include "iprojectbuilder.h"
#include "iprojectcontroller.h"
#include "importprojectjob.h"
#include "context.h"

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
        virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
        {
            return Qt::RightDockWidgetArea;
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
    d->m_buildAll = new KAction( i18n("Build all Projects"), this );
    connect( d->m_buildAll, SIGNAL(triggered()), this, SLOT(buildAllProjects()) );
    actionCollection()->addAction( "project_buildall", d->m_buildAll );
    d->m_build = new KAction( i18n("Build"), this );
    d->m_build->setShortcut( Qt::Key_F8 );
    connect( d->m_build, SIGNAL(triggered()), this, SLOT(buildProjectItems()) );
    actionCollection()->addAction( "project_build", d->m_build );
    d->m_install = new KAction( i18n("Install"), this );
    connect( d->m_install, SIGNAL(triggered()), this, SLOT(installProjectItems()) );
    actionCollection()->addAction( "project_install", d->m_install );
    d->m_clean = new KAction( i18n("Clean"), this );
    connect( d->m_clean, SIGNAL(triggered()), this, SLOT(cleanProjectItems()) );
    actionCollection()->addAction( "project_clean", d->m_clean );
    d->m_configure = new KAction( i18n("Configure"), this );
    connect( d->m_configure, SIGNAL(triggered()), this, SLOT(configureProjectItems()) );
    actionCollection()->addAction( "project_configure", d->m_configure );
    d->m_prune = new KAction( i18n("Prune"), this );
    connect( d->m_prune, SIGNAL(triggered()), this, SLOT(pruneProjectItems()) );
    actionCollection()->addAction( "project_prune", d->m_prune );
    setXMLFile( "kdevprojectmanagerview.rc" );
    d->factory = new KDevProjectManagerViewFactory( this );
    core()->uiController()->addToolView( i18n("Project Manager"), d->factory );

}

ProjectManagerViewPlugin::~ProjectManagerViewPlugin()
{
    delete d;
}

void ProjectManagerViewPlugin::unload()
{
    core()->uiController()->removeToolView(d->factory);
}

QPair<QString, QList<QAction*> > ProjectManagerViewPlugin::requestContextMenuActions( KDevelop::Context* context )
{
    if( context->type() != KDevelop::Context::ProjectItemContext )
        return IPlugin::requestContextMenuActions( context );

    QList<QAction*> actions;
    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    QList<KDevelop::ProjectBaseItem*> items = ctx->items();

    if( items.isEmpty() )
        return IPlugin::requestContextMenuActions( context );

    bool closeProjectsAdded = false;
    bool buildItemsAdded = false;
    foreach( ProjectBaseItem* item, items )
    {
        d->ctxProjectItemList << item;
        KDevelop::ProjectFolderItem *prjitem = dynamic_cast<KDevelop::ProjectFolderItem*>(item);
        if ( !closeProjectsAdded && prjitem && prjitem->isProjectRoot() )
        {
            KAction* close = new KAction( i18n( "Close projects" ), this );
            connect( close, SIGNAL(triggered()), this, SLOT(closeProjects()) );
            actions << close;
            closeProjectsAdded = true;
        }
        if ( !buildItemsAdded && ( item->folder() || item->target() || item->folder() ) )
        {
            KAction* action = new KAction( i18n( "Build items(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(buildItemsFromContextMenu()) );
            actions << action;
            action = new KAction( i18n( "Install item(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(installItemsFromContextMenu()) );
            actions << action;
            action = new KAction( i18n( "Clean item(s)" ), this );
            connect( action, SIGNAL( triggered() ), this, SLOT(cleanItemsFromContextMenu()) );
            actions << action;
            action = new KAction( i18n( "Add item(s) to buildset" ), this );
            connect( action, SIGNAL(triggered() ), this, SLOT(addItemsFromContextMenuToBuildset() ) );
            actions << action;
            buildItemsAdded = true;
        }
    }
    return qMakePair(QString("Project Management"), actions);
}


KDevelop::IProjectBuilder* ProjectManagerViewPlugin::getProjectBuilder( KDevelop::ProjectBaseItem* item )
{
    if( !item )
        return 0;
    IProject* project = item->project();
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
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Building item:" << item->text();

    core()->documentController()->saveAllDocuments(IDocument::Silent);

    if( builder )
        builder->build( item );
}

void ProjectManagerViewPlugin::executeClean( KDevelop::ProjectBaseItem* item )
{
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Cleaning item:" << item->text();
    if( builder )
        builder->clean( item );
}

void ProjectManagerViewPlugin::executeInstall( KDevelop::ProjectBaseItem* item )
{
    IProjectBuilder* builder = getProjectBuilder( item );
    kDebug(9511) << "Installing item:" << item->text();

    core()->documentController()->saveAllDocuments(IDocument::Silent);

    if( builder )
        builder->install( item );
}


void ProjectManagerViewPlugin::executeConfigure( KDevelop::IProject* item )
{
    IProjectBuilder* builder = getProjectBuilder( item->projectItem() );
    kDebug(9511) << "Configuring item:" << item->name();

    core()->documentController()->saveAllDocuments(IDocument::Silent);

    if( builder )
        builder->configure( item );
}

void ProjectManagerViewPlugin::executePrune( KDevelop::IProject* item )
{
    IProjectBuilder* builder = getProjectBuilder( item->projectItem() );
    kDebug(9511) << "Pruning item:" << item->name();

    core()->documentController()->saveAllDocuments(IDocument::Silent);

    if( builder )
        builder->prune( item );
}


void ProjectManagerViewPlugin::closeProjects()
{
    foreach( KDevelop::ProjectBaseItem* item, d->ctxProjectItemList )
    {
        KDevelop::ProjectFolderItem *prjitem = dynamic_cast<KDevelop::ProjectFolderItem*>(item);
        if ( prjitem && prjitem->isProjectRoot() )
        {
            core()->projectController()->closeProject( prjitem->project() );
        }
    }
    d->ctxProjectItemList.clear();
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
    foreach( KDevelop::ProjectBaseItem* item, d->buildSet->items() )
    {
        executeInstall( item );
    }
}

void ProjectManagerViewPlugin::pruneProjectItems()
{
    QSet<KDevelop::IProject*> projects;
    foreach( KDevelop::ProjectBaseItem* item, d->buildSet->items() )
    {
        projects << item->project();
    }
    foreach( KDevelop::IProject* project, projects )
    {
        executePrune( project );
    }
}

void ProjectManagerViewPlugin::configureProjectItems()
{
    QSet<KDevelop::IProject*> projects;
    foreach( KDevelop::ProjectBaseItem* item, d->buildSet->items() )
    {
        projects << item->project();
    }
    foreach( KDevelop::IProject* project, projects )
    {
        executeConfigure( project );
    }
}

void ProjectManagerViewPlugin::cleanProjectItems()
{
    foreach( KDevelop::ProjectBaseItem* item, d->buildSet->items() )
    {
        executeClean( item );
    }
}

void ProjectManagerViewPlugin::buildProjectItems()
{
    foreach( KDevelop::ProjectBaseItem* item, d->buildSet->items() )
    {
        executeBuild( item );
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
    
void ProjectManagerViewPlugin::storeBuildset()
{
    KConfigGroup setgrp = KGlobal::config()->group("Buildset");
    buildSet()->saveSettings( setgrp );
    setgrp.sync();
}


#include "projectmanagerviewplugin.moc"

