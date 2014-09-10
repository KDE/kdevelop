/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
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
#include "testcore.h"

#include <shell/uicontroller.h>
#include <shell/sessioncontroller.h>
#include <shell/languagecontroller.h>
#include <shell/runcontroller.h>
#include <shell/documentcontroller.h>
#include <shell/plugincontroller.h>
#include <shell/partcontroller.h>
#include <shell/projectcontroller.h>
#include <interfaces/isession.h>
#include "../shell/core_p.h"

#include <QCoreApplication>
#include <QTest>

#include <KParts/MainWindow>

namespace KDevelop
{

TestCore::TestCore()
 : Core( new CorePrivate(this) )
{
    Core::m_self = this;
}

TestCore* TestCore::initialize( Core::Setup mode, const QString& session )
{
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");

    if (!Core::m_self) {
        new TestCore;
    }

    TestCore* core = dynamic_cast<TestCore*>(Core::m_self);
    Q_ASSERT(core);
    core->initializeNonStatic(mode, session);

    if (mode == Default) {
        // we don't want the window to be visible, hide it
        // the unit tests should work anyways
        core->uiController()->activeMainWindow()->hide();
    }
    return core;
}

void TestCore::initializeNonStatic(Core::Setup mode, const QString& _session)
{
    QString session = _session;
    if (_session.isEmpty()) {
        // use a distinct session name for unit test sessions
        // they are temporary (see below) but still - we want to make sure
        session = "test-" + qAppName();
    }

    d->initialize( mode, session );

    if (_session.isEmpty()) {
        activeSession()->setTemporary(true);
    }
}

void TestCore::shutdown()
{
    if (self()) {
        // trigger eventloop to handle Queued connections
        // before entering cleanup
        // this can fix random crashes under certain conditions
        QTest::qWait(1);
        self()->shutdown();
    }
}

void TestCore::setShuttingDown(bool shuttingDown)
{
    d->m_shuttingDown = shuttingDown;
}

void TestCore::setSessionController( SessionController* ctrl )
{
    d->sessionController = ctrl;
}

void TestCore::setPluginController( PluginController* ctrl )
{
    d->pluginController = ctrl;
}

void TestCore::setRunController( RunController* ctrl )
{
    d->runController = ctrl;
}

void TestCore::setDocumentController( DocumentController* ctrl )
{
    d->documentController = ctrl;
}

void TestCore::setPartController( PartController* ctrl )
{
    d->partController = ctrl;
}

void TestCore::setProjectController( ProjectController* ctrl )
{
    d->projectController = ctrl;
}

void TestCore::setLanguageController( LanguageController* ctrl )
{
    d->languageController = ctrl;
}

void TestCore::setUiController( UiController* ctrl )
{
    d->uiController = ctrl;
}

}

