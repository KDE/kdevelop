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
#include "../kdevconfig.h"
#include "project.h"
#include "mainwindow.h"
// #include "kdevlanguagecontroller.h"
#include "iplugincontroller.h"
#include "uicontroller.h"
// #include "kdevdocumentcontroller.h"

namespace Koncrete
{

struct ProjectControllerPrivate
{
public:
    ProjectControllerPrivate()
        :m_isLoaded(false)
    {
    }

    QString m_name;
    KUrl m_localFile;
    KUrl m_globalFile;
    KUrl m_projectsDir;
    KUrl m_lastProject;
    bool m_isLoaded;
    IProject* m_project;
    IPlugin* m_projectPart;
    KRecentFilesAction *m_recentAction;
    Core* m_core;
};

ProjectController::ProjectController( Core* core )
        : IProjectController( core ), d( new ProjectControllerPrivate )
{
    d->m_core = core;
    d->m_project = 0;
    d->m_projectPart = 0;

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

    KConfig * config = Config::standard();
    config->setGroup( "General Options" );

    d->m_recentAction =
        KStandardAction::openRecent( this, SLOT( openProject( const KUrl& ) ),
                                ac);
    ac->addAction( "project_open_recent", d->m_recentAction );
    d->m_recentAction->setToolTip( i18n( "Open recent project" ) );
    d->m_recentAction->setWhatsThis(
        i18n( "<b>Open recent project</b><p>Opens recently opened project." ) );
    d->m_recentAction->loadEntries( config, "RecentProjects" );
}

ProjectController::~ProjectController()
{
    delete d;
}

void ProjectController::cleanup()
{
    closeProject();
}

void ProjectController::loadSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
}

void ProjectController::saveSettings( bool projectIsLoaded )
{
    // Do not save if a project is loaded as this doesn't make sense inside a project file...
    if ( !projectIsLoaded )
    {
        KConfig* standard = Config::standard();
        standard->setGroup( "General Options" );
        standard->writePathEntry( "Last Project", d->m_lastProject.path() );
    }
}

bool ProjectController::isLoaded() const
{
    return d->m_isLoaded;
}

KUrl ProjectController::localFile() const
{
    return d->m_localFile;
}

void ProjectController::setLocalFile( const KUrl &localFile )
{
    d->m_localFile = localFile;
}

KUrl ProjectController::globalFile() const
{
    return d->m_globalFile;
}

void ProjectController::setGlobalFile( const KUrl &globalFile )
{
    d->m_globalFile = globalFile;
}

KUrl ProjectController::projectDirectory() const
{
    return KUrl::fromPath( d->m_globalFile.directory() );
}

KUrl ProjectController::projectsDirectory() const
{
    return d->m_projectsDir;
}

void ProjectController::setProjectsDirectory( const KUrl &projectsDir )
{
    d->m_projectsDir = projectsDir;
}

IProject* ProjectController::activeProject() const
{
    return d->m_project;
// return 0;
}

bool ProjectController::openProject( const KUrl &projectFile )
{
    KUrl url = projectFile;

    if ( url.isEmpty() )
    {
        KConfig * config = Config::standard();
        config->setGroup( "General Options" );
        QString dir = config->readPathEntry( "DefaultProjectsDirectory",
                                             QDir::homePath() );

        url = KFileDialog::getOpenUrl( dir, i18n( "*.kdev4|KDevelop 4 Project Files\n" ),
                                       d->m_core->uiControllerInternal()->defaultMainWindow(),
                                       i18n( "Open Project" ) );
    }

    if ( !url.isValid() )
        return false;

    if ( url == d->m_globalFile )
    {
        if ( KMessageBox::questionYesNo( d->m_core->uiControllerInternal()->defaultMainWindow(),
                                         i18n( "Reopen the current project?" ) )
                == KMessageBox::No )
            return false;
    }

    if ( d->m_isLoaded )
        closeProject();

    d->m_globalFile = url;

    //FIXME Create the hidden directory if it doesn't exist
    d->m_localFile = KUrl::fromPath( d->m_globalFile.directory( KUrl::AppendTrailingSlash )
                                  + ".kdev4/"
                                  + d->m_globalFile.fileName() );

    if ( loadProjectPart() )
    {
        d->m_isLoaded = true;
        //The project file has been opened.
        //Now we can load settings for all of the Core objects including this one!!
//         Core::loadSettings();
        d->m_core->pluginController()->loadPlugins( IPluginController::Project );
    }
    else
        return false;

    KActionCollection * ac = d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();
    QAction * action;

    action = ac->action( "project_close" );
    action->setEnabled( true );

    d->m_recentAction->addUrl( url );
    d->m_recentAction->saveEntries( Config::standard(), "RecentProjects" );

    Config::standard() ->sync();
    emit projectOpened();

    return true;
}

bool ProjectController::closeProject()
{
    if ( !d->m_isLoaded )
        return false;

    emit projectClosing();

    //The project file is being closed.
    //Now we can save settings for all of the Core objects including this one!!
//     Core::self()->saveSettings();

//     Core::self()->documentController() ->closeAllDocuments();

    // save the the project to open it automaticly on startup if needed
    d->m_lastProject = d->m_globalFile;

    d->m_name = QString::null;
    d->m_localFile.clear();
    d->m_globalFile.clear();
    d->m_projectsDir.clear();

    if (d->m_core->uiControllerInternal()->defaultMainWindow())
    {
        KActionCollection * ac = d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();
        QAction * action;

        action = ac->action( "project_close" );
        action->setEnabled( false );
    }

    emit projectClosed();
    d->m_core->pluginController()->unloadPlugins( IPluginController::Project );

    //FIXME
    //     PluginController::self() ->changeProfile( m_oldProfileName );

    d->m_project->close();
    d->m_project->deleteLater(); //be safe when deleting
    d->m_project = 0;
    d->m_recentAction->setCurrentAction( 0 );
    d->m_isLoaded = false;
    return true;
}

bool ProjectController::loadProjectPart()
{
    KConfig * config = Config::standard();
    config->setGroup( "General Options" );

    QString projectManager =
            config->readPathEntry( "Project Management", "ProjectManager" );

    d->m_project = new Project();
    if ( !d->m_project->open( d->m_globalFile ) )
    {
        delete d->m_project;
        d->m_project = 0;
        delete d->m_projectPart;
        d->m_projectPart = 0;
        return false;
    }

    d->m_projectPart = d->m_core->pluginController()->loadPlugin( projectManager );
    if ( !d->m_projectPart )
    {
        KMessageBox::sorry( d->m_core->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Could not load project management plugin %1.",
                                  projectManager ) );
        d->m_project->close();
        delete d->m_project;
        d->m_project = 0;
        return false;
    }
    return true;
}

}
#include "projectcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
