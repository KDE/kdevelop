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

#include "projectcontroller.h"

// #include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kservice.h>
#include <kmessagebox.h>
#include <kmainwindow.h>
#include <kservicetypetrader.h>

#include "core.h"
#include "toplevel.h"
#include "kdevconfig.h"
#include "kdevproject.h"
#include "plugincontroller.h"
#include "languagecontroller.h"
#include "documentcontroller.h"

ProjectController *ProjectController::s_instance = 0;

ProjectController::ProjectController( QObject *parent )
        : QObject( parent ),
        m_name( QString::null ),
        m_localFile( KUrl() ),
        m_globalFile( KUrl() ),
        m_projectsDir( KUrl() ),
        m_isLoaded( false )
{}

ProjectController::~ProjectController()
{}

ProjectController *ProjectController::getInstance()
{
    if ( !s_instance )
        s_instance = new ProjectController;
    return s_instance;
}

void ProjectController::init()
{
    KConfig * config = KDevConfig::standard();
    config->setGroup( "General Options" );
    QString project = config->readPathEntry( "Last Project" );
    bool readProject = config->readEntry( "Read Last Project On Startup", true );
    if ( !project.isEmpty() && readProject )
    {
        openProject( KUrl( project ) );
    }
}

bool ProjectController::isLoaded() const
{
    return m_isLoaded;
}

QString ProjectController::name() const
{
    return m_name;
}

void ProjectController::setName( const QString &name )
{
    m_name = name;
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

bool ProjectController::openProject( const KUrl &KDev4ProjectFile )
{
    if ( !KDev4ProjectFile.isValid() )
        return false;

    if ( KDev4ProjectFile == m_globalFile )
    {
        if ( KMessageBox::questionYesNo( TopLevel::getInstance() ->main(),
                                         i18n( "Reopen the current project?" ) )
                == KMessageBox::No )
            return false;
    }

    if ( m_isLoaded )
        closeProject();

    m_globalFile = KDev4ProjectFile;

    //FIXME Create the hidden directory if it doesn't exist
    m_localFile = KUrl::fromPath( m_globalFile.directory( KUrl::AppendTrailingSlash )
                                  + ".kdev4/"
                                  + m_globalFile.fileName() );

    KDevConfig::standard() ->sync();

    //FIXME LAME LAME LAME LAME LAME
    legacyLoading();

    //FIXME LAME LAME LAME LAME LAME
    Core::getInstance() ->doEmitProjectOpened();

    m_isLoaded = true;
    return true;
}

bool ProjectController::closeProject()
{
    QStringList paths;
    QList<KDevDocument* > openDocs = DocumentController::getInstance() ->openDocuments();
    QList<KDevDocument* >::const_iterator it = openDocs.begin();
    for ( ; it != openDocs.end(); ++it )
    {
        paths.append( ( *it ) ->url().path() );
    }

    KConfig * local = KDevConfig::localProject();
    local->setGroup( "General Options" );
    if ( !paths.empty() )
        local->writePathEntry( "OpenDocuments", paths );
    else
        local->deleteEntry( "OpenDocuments" );

    local ->sync();

    // save the the project to open it automaticly on startup if needed
    KUrl lastProject = m_globalFile;

    m_name = QString::null;
    m_localFile.clear();
    m_globalFile.clear();
    m_projectsDir.clear();
    m_isLoaded = false;

    KConfig* standard = KDevConfig::standard();
    standard->setGroup( "General Options" );
    standard->writePathEntry( "Last Project", lastProject.path() );

    Core::getInstance() ->doEmitProjectClosed();

    PluginController::getInstance() ->unloadPlugins( ProfileEngine::Project );

    //FIXME
    //     PluginController::getInstance() ->changeProfile( m_oldProfileName );

    unloadProjectPart();

    return true;
}

void ProjectController::legacyLoading()
{
    //FIXME GET RID OF THIS!!!

    KConfig * config = KDevConfig::standard();
    config->setGroup( "General Options" );

    PluginController::getInstance() ->loadPlugins( ProfileEngine::Core );
    PluginController::getInstance() ->loadPlugins( ProfileEngine::Global );

    QString language = config->readPathEntry( "PrimaryLanguage", "C++" );
    LanguageController::getInstance() ->languageSupport( language );
    QStringList paths = config->readPathListEntry( "OpenDocuments" );
    foreach( QString doc, paths )
    {
        DocumentController::getInstance() ->editDocument(
            KUrl::fromPath( doc ),
            KTextEditor::Cursor::invalid(),
            false );
    }

    //Activate the first doc in the list
    DocumentController::getInstance() ->editDocument(
        KUrl::fromPath( paths.first() ),
        KTextEditor::Cursor::invalid(),
        true );

    QString projectManagement =
        config->readPathEntry( "Project Management", "KDevProjectManager" );
    loadProjectPart( projectManagement );
    PluginController::getInstance() ->loadPlugins( ProfileEngine::Project );
}

bool ProjectController::loadProjectPart( const QString &projectManager )
{
    KService::Ptr projectService = KService::serviceByDesktopName( projectManager );
    if ( !projectService )
    {
        KMessageBox::sorry( TopLevel::getInstance() ->main(),
                            i18n( "No project management plugin %1 found.",
                                  projectManager ) );
        return false;
    }

    KDevProject *projectPart =
        KService::createInstance< KDevProject >(
            projectService, 0,
            PluginController::argumentsFromService( projectService ) );

    if ( !projectPart )
    {
        KMessageBox::sorry( TopLevel::getInstance() ->main(),
                            i18n( "Could not create project management plugin %1.",
                                  projectManager ) );
        return false;
    }

    KDevApi::self() ->setProject( projectPart );

    projectPart->openProject( m_globalFile.directory(), m_globalFile.fileName() );

    PluginController::getInstance() ->integratePart( projectPart );

    return true;
}

void ProjectController::unloadProjectPart()
{
    KDevProject * projectPart = KDevApi::self() ->project();
    if ( projectPart )
    {
        PluginController::getInstance() ->removePart( projectPart );
        projectPart->closeProject();
        delete projectPart;
    }
    KDevApi::self() ->setProject( 0 );
}

#include "projectcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
