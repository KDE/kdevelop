/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testcore.h"

#include <shell/uicontroller.h>
#include <shell/sessioncontroller.h>
#include <shell/languagecontroller.h>
#include <shell/runcontroller.h>
#include <shell/documentcontroller.h>
#include <shell/plugincontroller.h>
#include <shell/partcontroller.h>
#include <shell/projectcontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/isession.h>
#include "../shell/core_p.h"

#include <QCoreApplication>
#include <QTest>

#include <KParts/MainWindow>

namespace KDevelop {
TestCore::TestCore()
    : Core(new CorePrivate(this))
{
    Core::m_self = this;
}

TestCore* TestCore::initialize(Core::Setup mode, const QString& session)
{
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");

    if (!Core::m_self) {
        new TestCore;
    }

    auto* core = qobject_cast<TestCore*>(Core::m_self);
    Q_ASSERT(core);
    core->initializeNonStatic(mode, session);

    if (mode == Default) {
        // we don't want the window to be visible, hide it
        // the unit tests should work anyways
        core->uiController()->activeMainWindow()->hide();
    }

    // resume the background parser when a unit test replaces the project controller
    QObject::connect(core->d->projectController.data(), &ProjectController::destroyed,
                     core, [core]() {
            core->d->languageController->backgroundParser()->resume();
        });

    return core;
}

void TestCore::initializeNonStatic(Core::Setup mode, const QString& _session)
{
    QString session = _session;
    if (_session.isEmpty()) {
        // use a distinct session name for unit test sessions
        // they are temporary (see below) but still - we want to make sure
        session = QLatin1String("test-") + QCoreApplication::applicationName();
    }

    d->initialize(mode, session);

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

        if (self()) {
            self()->shutdown();

            // wait until Core is deleted via event loop
            QTest::qWait(1);
        }
    }
}

void TestCore::setShuttingDown(bool shuttingDown)
{
    d->m_shuttingDown = shuttingDown;
}

void TestCore::setSessionController(SessionController* ctrl)
{
    d->sessionController = ctrl;
}

void TestCore::setPluginController(PluginController* ctrl)
{
    d->pluginController = ctrl;
}

void TestCore::setRunController(RunController* ctrl)
{
    d->runController = ctrl;
}

void TestCore::setDocumentController(DocumentController* ctrl)
{
    d->documentController = ctrl;
}

void TestCore::setPartController(PartController* ctrl)
{
    d->partController = ctrl;
}

void TestCore::setProjectController(ProjectController* ctrl)
{
    d->projectController = ctrl;
}

void TestCore::setLanguageController(LanguageController* ctrl)
{
    d->languageController = ctrl;
}

void TestCore::setUiController(UiController* ctrl)
{
    d->uiController = ctrl;
}
}

#include "moc_testcore.cpp"
