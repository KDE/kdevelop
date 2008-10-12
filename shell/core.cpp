/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>             *
 *   Copyright 2007 Kris Wong <kris.p.wong@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "core.h"

#include <QApplication>
#include <QPointer>

#include <kdebug.h>
#include <kglobal.h>

#include <sublime/area.h>
#include <sublime/tooldocument.h>

#include <language/editor/editorintegrator.h>
#include <language/backgroundparser/backgroundparser.h>

#include "shellextension.h"


#include "mainwindow.h"
#include "sessioncontroller.h"
#include "uicontroller.h"
#include "plugincontroller.h"
#include "projectcontroller.h"
#include "partcontroller.h"
#include "languagecontroller.h"
#include "documentcontroller.h"
#include "runcontroller.h"

namespace KDevelop {

Core *Core::m_self = 0;

struct CorePrivate {
    CorePrivate(Core *core):
        m_componentData( KAboutData( "kdevplatform", "kdevplatform", ki18n("KDevelop Platform"), "1.0", ki18n("Development Platform for IDE-like Applications"), KAboutData::License_LGPL_V2 ) ), m_core(core), m_cleanedUp(false)
    {
    }

    void initialize(Core::Setup mode)
    {
        m_mode=mode;
        sessionController = new SessionController(m_core);
        kDebug() << "Creating ui controller";
        uiController = new UiController(m_core);
        kDebug() << "Creating plugin controller";
        pluginController = new PluginController(m_core);
        if(!(mode & Core::NoUi)) partController = new PartController(m_core, uiController->defaultMainWindow());
        projectController = new ProjectController(m_core);
        languageController = new LanguageController(m_core);
        documentController = new DocumentController(m_core);
        runController = new RunController(m_core);

        kDebug() << "initializing ui controller";
        sessionController->initialize();
        if(!(mode & Core::NoUi)) uiController->initialize();
        languageController->initialize();
        projectController->initialize();
        documentController->initialize();

        /* This is somewhat messy.  We want to load the areas before
           loading the plugins, so that when each plugin is loaded we
           know if an area wants some of the tool view from that plugin.
           OTOH, loading of areas creates documents, and some documents
           might require that a plugin is already loaded.
           Probably, the best approach would be to plugins to just add
           tool views to a list of available tool view, and then grab
           those tool views when loading an area.  */

        kDebug() << "loading global plugin";
        pluginController->loadPlugins( PluginController::Global );

        if(!(mode & Core::NoUi))
        {
            /* Need to do this after everything else is loaded.  It's too
               hard to restore position of views, and toolbars, and whatever
               that are not created yet.  */
            uiController->loadAllAreas(KGlobal::config());
            uiController->defaultMainWindow()->show();
        }
    }
    ~CorePrivate()
    {
        delete projectController;
        delete languageController;
        delete pluginController;
        delete uiController;
        delete partController;
        delete documentController;
        delete runController;
        delete sessionController;
    }

    QPointer<PluginController> pluginController;
    QPointer<UiController> uiController;
    QPointer<ProjectController> projectController;
    QPointer<LanguageController> languageController;
    QPointer<PartController> partController;
    QPointer<DocumentController> documentController;
    QPointer<RunController> runController;
    QPointer<SessionController> sessionController;
 
    KComponentData m_componentData;
    Core *m_core;
    bool m_cleanedUp;
    Core::Setup m_mode;
};

void Core::initialize(Setup mode)
{
    if (m_self)
        return;

    m_self = new Core();
    m_self->d->initialize(mode);
}

Core *KDevelop::Core::self()
{
    return m_self;
}

Core::Core(QObject *parent)
    : ICore(parent)
{
    d = new CorePrivate(this);
}

Core::~Core()
{
    kDebug() ;
    //Cleanup already called before mass destruction of GUI
    delete d;
}

Core::Setup Core::setupFlags() const
{
    return d->m_mode;
}

void Core::cleanup()
{
    if (!d->m_cleanedUp) {
        d->sessionController->cleanup();
        // Save the layout of the ui here, so run it first
        d->uiController->cleanup();

        /* Must be called before projectController->cleanup(). */
        // Closes all documents (discards, as already saved if the user wished earlier)
        d->documentController->cleanup();

        d->projectController->cleanup();
        d->pluginController->cleanup();
    }

    d->m_cleanedUp = true;
}

KComponentData Core::componentData() const
{
    return d->m_componentData;
}

IUiController *Core::uiController()
{
    return d->uiController;
}

ISession* Core::activeSession()
{
    return sessionController()->activeSession();
}

SessionController *Core::sessionController()
{
    return d->sessionController;
}

UiController *Core::uiControllerInternal()
{
    return d->uiController;
}

IPluginController *Core::pluginController()
{
    return d->pluginController;
}

PluginController *Core::pluginControllerInternal()
{
    return d->pluginController;
}

IProjectController *Core::projectController()
{
    return d->projectController;
}

ProjectController *Core::projectControllerInternal()
{
    return d->projectController;
}

KParts::PartManager *Core::partManager()
{
    return d->partController;
}

PartController *Core::partManagerInternal()
{
    return d->partController;
}

ILanguageController *Core::languageController()
{
    return d->languageController;
}

LanguageController *Core::languageControllerInternal()
{
    return d->languageController;
}

IDocumentController *Core::documentController()
{
    return d->documentController;
}

DocumentController *Core::documentControllerInternal()
{
    return d->documentController;
}

IRunController *Core::runController()
{
    return d->runController;
}

RunController *Core::runControllerInternal()
{
    return d->runController;
}

}
