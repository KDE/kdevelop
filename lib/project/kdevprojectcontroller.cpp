/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#include "kdevprojectcontroller.h"

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

#include "kdevcore.h"
#include "kdevconfig.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevlanguagecontroller.h"
#include "kdevplugincontroller.h"
#include "kdevdocumentcontroller.h"

namespace Koncrete
{

ProjectController::ProjectController( QObject *parent )
        : QObject( parent ),
        m_localFile( KUrl() ),
        m_globalFile( KUrl() ),
        m_projectsDir( KUrl() ),
        m_lastProject( KUrl() ),
        m_isLoaded( false ),
        m_project( 0 ),
        m_projectPart( 0 ),
        m_recentAction( 0 )

{}

ProjectController::~ProjectController()
{}

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
        standard->writePathEntry( "Last Project", m_lastProject.path() );
    }
}

void ProjectController::initialize()
{
    KActionCollection * ac =
        Core::mainWindow() ->actionCollection();

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

    m_recentAction =
        KStandardAction::openRecent( this, SLOT( openProject( const KUrl& ) ),
                                ac);
    ac->addAction( "project_open_recent", m_recentAction );
    m_recentAction->setToolTip( i18n( "Open recent project" ) );
    m_recentAction->setWhatsThis(
        i18n( "<b>Open recent project</b><p>Opens recently opened project." ) );
    m_recentAction->loadEntries( config, "RecentProjects" );
}

void ProjectController::cleanup()
{
    closeProject();
}

bool ProjectController::isLoaded() const
{
    return m_isLoaded;
}

KUrl ProjectController::localFile() const
{
    return m_localFile;
}

void ProjectController::setLocalFile( const KUrl &localFile )
{
    m_localFile = localFile;
}

KUrl ProjectController::globalFile() const
{
    return m_globalFile;
}

void ProjectController::setGlobalFile( const KUrl &globalFile )
{
    m_globalFile = globalFile;
}

KUrl ProjectController::projectDirectory() const
{
    return KUrl::fromPath( m_globalFile.directory() );
}

KUrl ProjectController::projectsDirectory() const
{
    return m_projectsDir;
}

void ProjectController::setProjectsDirectory( const KUrl &projectsDir )
{
    m_projectsDir = projectsDir;
}

Project* ProjectController::activeProject() const
{
    return m_project;
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
                                       Core::mainWindow(),
                                       i18n( "Open Project" ) );
    }

    if ( !url.isValid() )
        return false;

    if ( url == m_globalFile )
    {
        if ( KMessageBox::questionYesNo( Core::mainWindow(),
                                         i18n( "Reopen the current project?" ) )
                == KMessageBox::No )
            return false;
    }

    if ( m_isLoaded )
        closeProject();

    m_globalFile = url;

    //FIXME Create the hidden directory if it doesn't exist
    m_localFile = KUrl::fromPath( m_globalFile.directory( KUrl::AppendTrailingSlash )
                                  + ".kdev4/"
                                  + m_globalFile.fileName() );

    if ( loadProjectPart() )
    {
        m_isLoaded = true;
        //The project file has been opened.
        //Now we can load settings for all of the Core objects including this one!!
        Core::loadSettings();
        PluginController::self() ->loadPlugins( PluginController::Project );
    }
    else
        return false;

    KActionCollection * ac = Core::mainWindow() ->actionCollection();
    QAction * action;

    action = ac->action( "project_close" );
    action->setEnabled( true );

    m_recentAction->addUrl( url );
    m_recentAction->saveEntries( Config::standard(), "RecentProjects" );

    Config::standard() ->sync();
    emit projectOpened();

    return true;
}

bool ProjectController::closeProject()
{
    if ( !m_isLoaded )
        return false;

    emit projectClosing();

    //The project file is being closed.
    //Now we can save settings for all of the Core objects including this one!!
    Core::saveSettings();

    Core::documentController() ->closeAllDocuments();

    // save the the project to open it automaticly on startup if needed
    m_lastProject = m_globalFile;

    m_name = QString::null;
    m_localFile.clear();
    m_globalFile.clear();
    m_projectsDir.clear();


    //The project file has been closed.
    //Now we can save settings for all of the Core objects including this one!!
    Core::saveSettings();

    KActionCollection * ac = Core::mainWindow() ->actionCollection();
    QAction * action;

    action = ac->action( "project_close" );
    action->setEnabled( false );

    emit projectClosed();
    PluginController::self() ->unloadPlugins( PluginController::Project );

    //FIXME
    //     PluginController::self() ->changeProfile( m_oldProfileName );

    m_project->close();
    m_project->deleteLater(); //be safe when deleting
    m_project = 0;
    m_recentAction->setCurrentAction( 0 );
    m_isLoaded = false;
    return true;
}

bool ProjectController::loadProjectPart()
{
    KConfig * config = Config::standard();
    config->setGroup( "General Options" );

    QString projectManager =
            config->readPathEntry( "Project Management", "ProjectManager" );

    QString constraint =
            QString::fromLatin1("[X-KDE-PluginInfo-Name] == '%1'")
            .arg(projectManager);

    KPluginInfo::List projectList = PluginController::query("KDevelop/Plugin", constraint);
    if ( projectList.isEmpty() )
    {
        KMessageBox::sorry( Core::mainWindow(),
                            i18n( "No project management plugin %1 found.",
                                  projectManager ) );
        return false;
    }

    m_project = new Project();
    if ( !m_project->open( m_globalFile ) )
    {
        delete m_project;
        m_project = 0;
        return false;
    }

    KPluginInfo* projectPluginInfo = *projectList.begin();
    PluginController *pc = PluginController::self();
    m_projectPart = pc->loadPlugin( projectPluginInfo->pluginName() );
    if ( !m_projectPart )
    {
        KMessageBox::sorry( Core::mainWindow(),
                            i18n( "Could not create project management plugin %1.",
                                  projectManager ) );
        m_project->close();
        delete m_project;
        m_project = 0;
        return false;
    }

    return true;
}

}
#include "kdevprojectcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
