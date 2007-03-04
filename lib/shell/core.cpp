/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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
#include "../kdevconfig.h"
#include "mainwindow.h"
#include "uicontroller.h"
#include "plugincontroller.h"
#include "projectcontroller.h"
#include "partcontroller.h"
#include "languagecontroller.h"

namespace KDevelop {

Core *Core::m_self = 0;

struct CorePrivate {
    CorePrivate(Core *core): m_core(core)
    {
    }
    void initialize()
    {
        config = new Config( m_core );
        uiController = new UiController(m_core);
        pluginController = new PluginController(m_core);
        partController = new PartController(m_core, uiController->defaultMainWindow());
        projectController = new ProjectController(m_core);
        languageController = new LanguageController(m_core);

        uiController->initialize();
        languageController->initialize();

        pluginController->loadPlugins( PluginController::Global );

        Sublime::Area *defaultArea = uiController->defaultArea();
        uiController->showArea(defaultArea, uiController->defaultMainWindow());

        uiController->defaultMainWindow()->show();
    }
    ~CorePrivate()
    {
        delete config;
        delete projectController;
        delete languageController;
        delete pluginController;
        delete uiController;
        delete partController;
    }

    QPointer<PluginController> pluginController;
    QPointer<UiController> uiController;
    QPointer<ProjectController> projectController;
    QPointer<LanguageController> languageController;
    QPointer<PartController> partController;
    QPointer<Config> config;

    Core *m_core;
};

void Core::initialize()
{
    if( m_self )
        return;
    m_self = new Core();
    m_self->d->initialize();
}

Core *KDevelop::Core::self()
{
    return m_self;
}

Core::Core(QObject *parent)
    :ICore(parent)
{
    d = new CorePrivate(this);
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

Core::~Core()
{
    kDebug() << k_funcinfo << endl;
    cleanup();
    delete d;
}

void Core::cleanup()
{
//     d->projectController->cleanup();
    d->pluginController->cleanup();
}

IUiController *Core::uiController()
{
    return d->uiController;
}

IPluginController* Core::pluginController()
{
    return d->pluginController;
}

IProjectController* Core::projectController()
{
    return d->projectController;
}

PartController *Core::partController()
{
    return d->partController;
}

UiController *Core::uiControllerInternal()
{
    return d->uiController;
}

ILanguageController *Core::languageController()
{
    return d->languageController;
}

}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
