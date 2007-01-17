/* This file is part of KDevelop
Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>
Copyright (C) 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
Copyright (C) 2003,2006 Hamish Rodda <rodda@kde.org>
Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
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

#include "kdevcore.h"

#include <QPointer>
#include <kcmdlineargs.h>
#include <kstaticdeleter.h>

#include "kdevconfig.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevenvironment.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevprojectcontroller.h"
#include "kdevdocumentcontroller.h"
#include "kdevlanguagecontroller.h"
#include "kdevbackgroundparser.h"

namespace Koncrete
{

class CorePrivate;
static KStaticDeleter<CorePrivate> s_deleter;

class CorePrivate
{
    friend class KStaticDeleter<CorePrivate>;
public:
    static CorePrivate *self()
    {
        if ( !s_private )
            s_deleter.setObject( s_private, new CorePrivate );
        return s_private;
    }

    static CorePrivate *s_private;
    /* We hold these as QPointers in case some errant plugin
       decides to delete one of us.  This way we'll know.
     */
    QPointer<Environment> environment;
    QPointer<MainWindow> mainWindow;
    QPointer<PartController> partController;
    QPointer<ProjectController> projectController;
    QPointer<LanguageController> languageController;
    QPointer<DocumentController> documentController;
    QPointer<BackgroundParser> backgroundParser;

private:
    CorePrivate()
    {}

    ~CorePrivate()
    {
        /* Check to make sure these objects haven't been deleted already.
           Some errant plugin might delete it or they might be deleted by
           Qt in the case of MainWindow.
        */

        /* WARNING!!! Any object that is deleted here should be cleaned up
           in MainWindow::queryClose()!  All data or objects that require
           one of these classes must be deleted or cleaned up there first.
        */
        if ( environment )
            delete environment;
        if ( projectController )
            delete projectController;
        if ( documentController )
            delete documentController;
        if ( languageController )
            delete languageController;
        if ( backgroundParser )
            delete backgroundParser;
        if ( partController )
            delete partController;
        if ( mainWindow )
            delete mainWindow; //should go last
    }
};

CorePrivate *CorePrivate::s_private = 0;

#define d (CorePrivate::self())

Environment *Core::environment()
{
    return d->environment;
}

void Core::setEnvironment( Environment *environment )
{
    d->environment = environment;
}

ProjectController *Core::projectController()
{
    return d->projectController;
}

void Core::setProjectController( ProjectController *projectController )
{
    d->projectController = projectController;
}

Project* Core::activeProject()
{
    Q_ASSERT( d->projectController );
    return d->projectController->activeProject();
}

MainWindow *Core::mainWindow()
{
    return d->mainWindow;
}

void Core::setMainWindow( MainWindow *mainWindow )
{
    d->mainWindow = mainWindow;
}

DocumentController* Core::documentController()
{
    return d->documentController;
}

void Core::setDocumentController( DocumentController* documentController )
{
    d->documentController = documentController;
}

PartController* Core::partController()
{
    return d->partController;
}

void Core::setPartController( PartController* partController )
{
    d->partController = partController;
}

LanguageController* Core::languageController()
{
    return d->languageController;
}

void Core::setLanguageController( LanguageController * langController )
{
    d->languageController = langController;
}

LanguageSupport *Core::activeLanguage()
{
    Q_ASSERT( d->languageController );
    return d->languageController->activeLanguage();
}

BackgroundParser* Core::backgroundParser()
{
    return d->backgroundParser;
}

void Core::setBackgroundParser( BackgroundParser* backgroundParser )
{
    d->backgroundParser = backgroundParser;
}

void Core::initialize()
{
    //All Core API objects can utilize resources which
    //depend upon one another.  Can not do this in the constructor
    //as they might depend upon one another.

    Q_ASSERT( d->environment );
    Q_ASSERT( d->partController );
    Q_ASSERT( d->languageController );
    Q_ASSERT( d->documentController );
    Q_ASSERT( d->projectController );
    Q_ASSERT( d->mainWindow );
    Q_ASSERT( d->backgroundParser );

    d->environment->initialize();
    d->partController->initialize();
    d->languageController->initialize();
    d->documentController->initialize();
    d->projectController->initialize();
    d->mainWindow->initialize();
    d->backgroundParser->initialize();

    bool success = false;

    KCmdLineArgs * args = KCmdLineArgs::parsedArgs();
    if ( args->isSet( "project" ) )
    {
        QString project = QString::fromLocal8Bit( args->getOption( "project" ) );
        success = d->projectController->openProject( KUrl( project ) );
    }
    else
    {
        KConfig * config = Config::standard();
        config->setGroup( "General Options" );
        QString project = config->readPathEntry( "Last Project" );
        bool readProject = config->readEntry( "Read Last Project On Startup", true );

        if ( !project.isEmpty() && readProject )
        {
            success = d->projectController->openProject( KUrl( project ) );
        }
    }

    //If the project opened successfully then projectController will call Core::loadSettings
    //once the project file has been loaded.  Else we will do it here.
    if ( !success )
        loadSettings();

    d->mainWindow->setVisible( true ); //Done initializing
}

void Core::cleanup()
{
    //All Core API objects can utilize resources which
    //depend upon one another.  Can not do this in the destructor
    //as they might depend upon one another.

    Q_ASSERT( d->environment );
    Q_ASSERT( d->partController );
    Q_ASSERT( d->languageController );
    Q_ASSERT( d->documentController );
    Q_ASSERT( d->projectController );
    Q_ASSERT( d->mainWindow );
    Q_ASSERT( d->backgroundParser );

    d->mainWindow->setVisible( false );

    //If a project is open then projectController will call Core::saveSettings
    //both before the project is closed and then once after.  Else we will do it here.
    if ( !d->projectController->closeProject() )
        saveSettings();

    d->environment->cleanup();
    d->partController->cleanup();
    d->languageController->cleanup();
    d->documentController->cleanup();
    d->projectController->cleanup();
    d->mainWindow->cleanup();
    d->backgroundParser->cleanup();
}

/* This function should be called right after initialization of the objects and a project has
   been opened, or if no project is opened it should be called before the mainWindow is shown. */
void Core::loadSettings()
{
    Q_ASSERT( d->environment );
    Q_ASSERT( d->partController );
    Q_ASSERT( d->languageController );
    Q_ASSERT( d->documentController );
    Q_ASSERT( d->projectController );
    Q_ASSERT( d->mainWindow );
    Q_ASSERT( d->backgroundParser );

    bool projectIsLoaded = Core::projectController()->isLoaded();

    d->environment->loadSettings( projectIsLoaded );
    d->partController->loadSettings( projectIsLoaded );
    d->languageController->loadSettings( projectIsLoaded );
    d->documentController->loadSettings( projectIsLoaded );
    d->projectController->loadSettings( projectIsLoaded );
    d->mainWindow->loadSettings( projectIsLoaded );
    d->backgroundParser->loadSettings( projectIsLoaded );
}

/* This function should be called right before closing of the project and right after closing the
   project, or if no project is opened it should be called right before cleanup. */
void Core::saveSettings()
{
    Q_ASSERT( d->environment );
    Q_ASSERT( d->partController );
    Q_ASSERT( d->languageController );
    Q_ASSERT( d->documentController );
    Q_ASSERT( d->projectController );
    Q_ASSERT( d->mainWindow );
    Q_ASSERT( d->backgroundParser );

    bool projectIsLoaded = Core::projectController()->isLoaded();

    d->environment->saveSettings( projectIsLoaded );
    d->partController->saveSettings( projectIsLoaded );
    d->languageController->saveSettings( projectIsLoaded );
    d->documentController->saveSettings( projectIsLoaded );
    d->projectController->saveSettings( projectIsLoaded );
    d->mainWindow->saveSettings( projectIsLoaded );
    d->backgroundParser->saveSettings( projectIsLoaded );
}

void CoreInterface::load()
{
    Q_ASSERT( Core::projectController() );
    loadSettings( Core::projectController()->isLoaded() );
}

void CoreInterface::save()
{
    Q_ASSERT( Core::projectController() );
    saveSettings( Core::projectController()->isLoaded() );
}

}
