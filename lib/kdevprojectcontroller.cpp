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
#include <kstdaction.h>
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

KDevProjectController::KDevProjectController( QObject *parent )
        : QObject( parent ),
        m_name( QString::null ),
        m_localFile( KUrl() ),
        m_globalFile( KUrl() ),
        m_projectsDir( KUrl() ),
        m_lastProject( KUrl() ),
        m_isLoaded( false ),
        m_project( 0 ),
        m_recentAction( 0 )
{}

KDevProjectController::~KDevProjectController()
{}

void KDevProjectController::loadSettings( bool projectIsLoaded )
{
    if ( projectIsLoaded )
    {
        KConfig * config = KDevConfig::standard();
        config->setGroup( "General Options" );

        QString projectManagement =
            config->readPathEntry( "Project Management", "KDevProjectManager" );

        loadProjectPart( projectManagement );

        KDevCore::pluginController() ->loadPlugins( ProfileEngine::Project );
    }
}

void KDevProjectController::saveSettings( bool projectIsLoaded )
{
    // Do not save if a project is loaded as this doesn't make sense inside a project file...
    if ( !projectIsLoaded )
    {
        KConfig* standard = KDevConfig::standard();
        standard->setGroup( "General Options" );
        standard->writePathEntry( "Last Project", m_lastProject.path() );
    }
}

void KDevProjectController::initialize()
{
    KActionCollection * ac =
        KDevCore::mainWindow() ->actionCollection();

    KAction *action;

    action = new KAction( i18n( "&Open Project..." ), ac, "project_open" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( openProject() ) );
    action->setToolTip( i18n( "Open project" ) );
    action->setWhatsThis( i18n( "<b>Open project</b><p>Opens a KDevelop 4 project." ) );

    action = new KAction( i18n( "C&lose Project" ), ac, "project_close" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( closeProject() ) );
    action->setToolTip( i18n( "Close project" ) );
    action->setWhatsThis( i18n( "<b>Close project</b><p>Closes the current project." ) );
    action->setEnabled( false );

    KConfig * config = KDevConfig::standard();
    config->setGroup( "General Options" );

    m_recentAction =
        KStdAction::openRecent( this, SLOT( openProject( const KUrl& ) ),
                                ac, "project_open_recent" );
    m_recentAction->setToolTip( i18n( "Open recent project" ) );
    m_recentAction->setWhatsThis(
        i18n( "<b>Open recent project</b><p>Opens recently opened project." ) );
    m_recentAction->loadEntries( config, "RecentProjects" );
}

void KDevProjectController::cleanup()
{
}

bool KDevProjectController::isLoaded() const
{
    return m_isLoaded;
}

QString KDevProjectController::name() const
{
    return m_name;
}

void KDevProjectController::setName( const QString &name )
{
    m_name = name;
}

KUrl KDevProjectController::localFile() const
{
    return m_localFile;
}

void KDevProjectController::setLocalFile( const KUrl &localFile )
{
    m_localFile = localFile;
}

KUrl KDevProjectController::globalFile() const
{
    return m_globalFile;
}

void KDevProjectController::setGlobalFile( const KUrl &globalFile )
{
    m_globalFile = globalFile;
}

KUrl KDevProjectController::projectDirectory() const
{
    return KUrl::fromPath( m_globalFile.directory() );
}

KUrl KDevProjectController::projectsDirectory() const
{
    return m_projectsDir;
}

void KDevProjectController::setProjectsDirectory( const KUrl &projectsDir )
{
    m_projectsDir = projectsDir;
}

KDevProject* KDevProjectController::activeProject() const
{
    return m_project;
}

bool KDevProjectController::openProject( const KUrl &KDev4ProjectFile )
{
    KUrl url = KDev4ProjectFile;

    if ( url.isEmpty() )
    {
        KConfig * config = KDevConfig::standard();
        config->setGroup( "General Options" );
        QString dir = config->readPathEntry( "DefaultProjectsDirectory",
                                             QDir::homePath() );

        url = KFileDialog::getOpenUrl( dir, i18n( "*.kdev4|KDevelop 4 Project Files\n" ),
                                       KDevCore::mainWindow(),
                                       i18n( "Open Project" ) );

    }

    if ( !url.isValid() )
        return false;

    if ( url == m_globalFile )
    {
        if ( KMessageBox::questionYesNo( KDevCore::mainWindow(),
                                         i18n( "Reopen the current project?" ) )
                == KMessageBox::No )
            return false;
    }

    if ( m_isLoaded )
        closeProject();

    KActionCollection * ac = KDevCore::mainWindow() ->actionCollection();
    KAction * action;

    action = ac->action( "project_close" );
    action->setEnabled( true );

    m_recentAction->addUrl( url );
    m_recentAction->saveEntries( KDevConfig::standard(), "RecentProjects" );

    m_globalFile = url;

    //FIXME Create the hidden directory if it doesn't exist
    m_localFile = KUrl::fromPath( m_globalFile.directory( KUrl::AppendTrailingSlash )
                                  + ".kdev4/"
                                  + m_globalFile.fileName() );

    KDevConfig::standard() ->sync();
    m_isLoaded = true;

    //The project file has been opened.
    //Now we can load settings for all of the KDevCore objects including this one!!
    KDevCore::loadSettings();

    emit projectOpened();

    return true;
}

bool KDevProjectController::closeProject()
{
    if ( !m_isLoaded )
        return false;

    emit projectClosing();

    //The project file is being closed.
    //Now we can save settings for all of the KDevCore objects including this one!!
    KDevCore::saveSettings();

    KDevCore::documentController() ->closeAllDocuments();

    // save the the project to open it automaticly on startup if needed
    m_lastProject = m_globalFile;

    m_name = QString::null;
    m_localFile.clear();
    m_globalFile.clear();
    m_projectsDir.clear();
    m_isLoaded = false;

    //The project file has been closed.
    //Now we can save settings for all of the KDevCore objects including this one!!
    KDevCore::saveSettings();

    KActionCollection * ac = KDevCore::mainWindow() ->actionCollection();
    KAction * action;

    action = ac->action( "project_close" );
    action->setEnabled( false );

    emit projectClosed();

    KDevCore::pluginController() ->unloadPlugins( ProfileEngine::Project );

    //FIXME
    //     KDevCore::pluginController() ->changeProfile( m_oldProfileName );

    unloadProjectPart();

    m_recentAction->setCurrentAction( 0 );

    return true;
}

bool KDevProjectController::loadProjectPart( const QString &projectManager )
{
    KService::Ptr projectService = KService::serviceByDesktopName( projectManager );
    if ( !projectService )
    {
        KMessageBox::sorry( KDevCore::mainWindow(),
                            i18n( "No project management plugin %1 found.",
                                  projectManager ) );
        return false;
    }

    m_project = KService::createInstance< KDevProject >(
                    projectService, 0,
                    KDevPluginController::argumentsFromService( projectService ) );

    if ( !m_project )
    {
        KMessageBox::sorry( KDevCore::mainWindow(),
                            i18n( "Could not create project management plugin %1.",
                                  projectManager ) );
        return false;
    }

    m_project->openProject( m_globalFile.directory(), m_globalFile.fileName() );
    KDevCore::pluginController() ->integratePart( m_project );

    KDevCore::mainWindow() ->addPlugin( m_project );

    return true;
}

void KDevProjectController::unloadProjectPart()
{
    if ( m_project )
    {
        KDevCore::mainWindow() ->removePlugin( m_project );
        KDevCore::pluginController() ->removePart( m_project );
        m_project->closeProject();
        delete m_project;
        m_project = 0;
    }
}

#include "kdevprojectcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
