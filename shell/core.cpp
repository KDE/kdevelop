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

#include "shellextension.h"

#include "mainwindow.h"
#include "uicontroller.h"
#include "plugincontroller.h"
#include "projectcontroller.h"
#include "partcontroller.h"
#include "languagecontroller.h"
#include "editorintegrator.h"
#include "documentcontroller.h"
#include "backgroundparser.h"
#include "runcontroller.h"

namespace KDevelop {

Core *Core::m_self = 0;

struct CorePrivate {
    CorePrivate(Core *core): m_core(core), m_cleanedUp(false)
    {
    }
    void initialize()
    {
	kDebug(9501) << "Creating ui controller";
        uiController = new UiController(m_core);
	kDebug(9501) << "Creating plugin controller";
        pluginController = new PluginController(m_core);
        partController = new PartController(m_core, uiController->defaultMainWindow());
        projectController = new ProjectController(m_core);
        languageController = new LanguageController(m_core);
        documentController = new DocumentController(m_core);
        runController = new RunController(m_core);

	kDebug(9501) << "initializing ui controller";
        uiController->initialize();
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

	kDebug(9501) << "loading global plugin";
        pluginController->loadPlugins( PluginController::Global );

        /* Need to do this after everything else is loaded.  It's too
           hard to restore position of views, and toolbars, and whatever
           that are not created yet.  */
        uiController->loadAllAreas(KGlobal::config());
        uiController->defaultMainWindow()->show();
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
    }

    QPointer<PluginController> pluginController;
    QPointer<UiController> uiController;
    QPointer<ProjectController> projectController;
    QPointer<LanguageController> languageController;
    QPointer<PartController> partController;
    QPointer<DocumentController> documentController;
    QPointer<RunController> runController;

    Core *m_core;
    bool m_cleanedUp;
};

void Core::initialize()
{
    if (m_self)
        return;

    m_self = new Core();
    m_self->d->initialize();
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
    kDebug(9501) ;
    //Cleanup already called before mass destruction of GUI
    delete d;
}

void Core::cleanup()
{
    if (!d->m_cleanedUp) {
        /* Must be called before projectController->cleanup(). */
        d->documentController->cleanup();
        d->uiController->cleanup();
        d->projectController->cleanup();
        d->pluginController->cleanup();
    }

    d->m_cleanedUp = true;
}

IUiController *Core::uiController()
{
    return d->uiController;
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
