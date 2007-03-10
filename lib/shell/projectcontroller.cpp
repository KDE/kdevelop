/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Anreas Pakulat <apaku@gmx.de>

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

#include "projectcontroller.h"

#include <QDir>

#include <kaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kservice.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kmainwindow.h>
#include <kfiledialog.h>
#include <kactioncollection.h>
#include <kservicetypetrader.h>
#include <krecentfilesaction.h>

#include "core.h"
#include "iplugin.h"
#include "kdevconfig.h"
#include "project.h"
#include "mainwindow.h"
#include "projectmodel.h"
// #include "kdevlanguagecontroller.h"
#include "iplugincontroller.h"
#include "uicontroller.h"
// #include "kdevdocumentcontroller.h"

namespace KDevelop
{

struct ProjectControllerPrivate
{
public:

    QList<IProject*> m_projects;
    IPlugin* m_projectPart;
    KRecentFilesAction *m_recentAction;
    Core* m_core;
    IProject* m_currentProject;
    ProjectModel* model;
};

ProjectController::ProjectController( Core* core )
        : IProjectController( core ), d( new ProjectControllerPrivate )
{
    d->m_core = core;
    d->m_projectPart = 0;
    d->m_currentProject = 0;
    d->model = new ProjectModel();
    setupActions();
}

void ProjectController::setupActions()
{
    KActionCollection * ac =
        d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();

    QAction *action;

    action = ac->addAction( "project_open" );
    action->setText(i18n( "&Open Project..." ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( openProject() ) );
    action->setToolTip( i18n( "Open project" ) );
    action->setWhatsThis( i18n( "<b>Open project</b><p>Opens a KDevelop 4 project." ) );

    action = ac->addAction( "project_close" );
    action->setText( i18n( "C&lose Project" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( closeProject() ) );
    action->setToolTip( i18n( "Close project" ) );
    action->setWhatsThis( i18n( "<b>Close project</b><p>Closes the current project." ) );
    action->setEnabled( false );

    KConfig * config = Config::self()->standard();
//     KConfigGroup group = config->group( "General Options" );

    d->m_recentAction =
        KStandardAction::openRecent( this, SLOT( openProject( const KUrl& ) ),
                                ac);
    ac->addAction( "project_open_recent", d->m_recentAction );
    d->m_recentAction->setToolTip( i18n( "Open recent project" ) );
    d->m_recentAction->setWhatsThis(
        i18n( "<b>Open recent project</b><p>Opens recently opened project." ) );
    d->m_recentAction->loadEntries( KConfigGroup(config, "RecentProjects") );
}

ProjectController::~ProjectController()
{
    delete d->model;
    delete d;
}

void ProjectController::cleanup()
{
    foreach( IProject* project, d->m_projects )
        closeProject( project );
}

void ProjectController::loadSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
}

void ProjectController::saveSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
    // Do not save if a project is loaded as this doesn't make sense inside a project file...
//     if ( projects().count() == 0 )
//     {
//         KConfig* standard = Config::self()->standard();
//         KConfigGroup group = standard->group( "General Options" );
//         standard->writePathEntry( "Last Project", d->m_lastProject.path() );
//     }
}

// bool ProjectController::isLoaded() const
// {
//     return d->m_isLoaded;
// }

int ProjectController::projectCount() const
{
    return d->m_projects.count();
}

IProject* ProjectController::projectAt( int num ) const
{
    if( !d->m_projects.isEmpty() && num >= 0 && num < d->m_projects.count() )
        return d->m_projects.at( num );
    return 0;
}

QList<IProject*> ProjectController::projects() const
{
    return d->m_projects;
}

bool ProjectController::openProject( const KUrl &projectFile )
{
    KUrl url = projectFile;

    if ( url.isEmpty() )
    {
        KConfig * config = Config::self()->standard();
        KConfigGroup group = config->group( "General Options" );
        QString dir = group.readEntry( "DefaultProjectsDirectory",
                                             QDir::homePath() );

        url = KFileDialog::getOpenUrl( dir, i18n( "*.kdev4|KDevelop 4 Project Files\n" ),
                                       d->m_core->uiControllerInternal()->defaultMainWindow(),
                                       i18n( "Open Project" ) );
    }

    if ( !url.isValid() )
        return false;

    foreach( IProject* project, d->m_projects )
    {
        if ( url == project->globalFile() )
        {
            if ( KMessageBox::questionYesNo( d->m_core->uiControllerInternal()->defaultMainWindow(),
                                             i18n( "Reopen the current project?" ) )
                    == KMessageBox::No )
                return false;
        }
    }

    //FIXME Create the hidden directory if it doesn't exist

    if ( loadProjectPart() )
    {
        //The project file has been opened.
        //Now we can load settings for all of the Core objects including this one!!
//         Core::loadSettings();
        d->m_core->pluginController()->loadPlugins( IPluginController::Project );
    }
    else
        return false;


    IProject* project = new Project();
    if ( !project->open( url ) )
    {
        delete project;
        return false;
    }
    d->m_projects.append( project );


    KActionCollection * ac = d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();
    QAction * action;

    action = ac->action( "project_close" );
    action->setEnabled( true );

    d->m_recentAction->addUrl( url );
    KConfigGroup recentGroup = Config::self()->standard()->group("RecentProjects");
    d->m_recentAction->saveEntries( recentGroup );

    Config::self()->standard() ->sync();
    emit projectOpened( project );

    return true;
}

bool ProjectController::closeProject( IProject* proj )
{
    if( d->m_projects.indexOf( proj ) == -1 )
        return false;
//     if ( !d->m_isLoaded )
//         return false;

    emit projectClosing( proj );

    //The project file is being closed.
    //Now we can save settings for all of the Core objects including this one!!
//     Core::self()->saveSettings();

//     Core::self()->documentController() ->closeAllDocuments();

    // save the the project to open it automaticly on startup if needed
//     d->m_lastProject = d->m_globalFile;

//     d->m_name = QString::null;
//     d->m_localFile.clear();
//     d->m_globalFile.clear();
//     d->m_projectsDir.clear();

    if (d->m_core->uiControllerInternal()->defaultMainWindow())
    {
        KActionCollection * ac = d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();
        QAction * action;

        action = ac->action( "project_close" );
        action->setEnabled( false );
    }


    proj->close();
    proj->deleteLater(); //be safe when deleting
    d->m_projects.removeAll( proj );
    d->m_recentAction->setCurrentAction( 0 );
//     d->m_isLoaded = false;
    if( d->m_projects.isEmpty() )
        d->m_core->pluginController()->unloadPlugins( IPluginController::Project );


    emit projectClosed( proj );

    //FIXME
    //     PluginController::self() ->changeProfile( m_oldProfileName );

    return true;
}

bool ProjectController::loadProjectPart()
{
    if( !d->m_projectPart )
    {
        KConfig * config = Config::self()->standard();
        KConfigGroup group = config->group( "General Options" );

        QString projectManager =
                group.readEntry( "ProjectManagementView", "KDevProjectManagerView" );

        d->m_projectPart = d->m_core->pluginController()->loadPlugin( projectManager );
        if ( !d->m_projectPart )
        {
            KMessageBox::sorry( d->m_core->uiControllerInternal()->defaultMainWindow(),
                                i18n( "Could not load project management view plugin %1.",
                                      projectManager ) );
            return false;
        }
    }
    return true;
}

void ProjectController::changeCurrentProject( const QModelIndex& index )
{
    ProjectBaseItem* item = projectModel()->item( index );
    if( item )
        d->m_currentProject = item->project();
}

IProject* ProjectController::currentProject() const
{
    return d->m_currentProject;
}

ProjectModel* ProjectController::projectModel()
{
    return d->model;
}

}
#include "projectcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
