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
    KLocalizedString::setApplicationDomain("kdevelop");

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
        // TODO: see to gain full control over execution flow instead of random waiting
        QTest::qWait(1);

        // The event processing by above's QTest::qWait can potentially
        // also result in the destruction of the Core instance.
        // So we need to recheck once more next if the instance still exists.
        //
        // This can happen due to calling Core::shutdown(), e.g. via the
        // KDevelop::MainWindow destructor, which in turn could be invoked by
        // calling close() on the window instance. QWidget::close() invokes
        // deleteLater(), creating a QDeferredDeleteEvent for itself.
        // Core::shutdown() also calls deleteLater() on itself, which results
        // in a QDeferredDeleteEvent for the Core instance.
        // Now with QTest::qWait (as of Qt 6.7) the 1 ms time window passed as
        // argument (the minimal value possible) for its event processing is
        // mapped onto a single 10 ms time slot. There first
        // QCoreApplication::processEvents(QEventLoop::AllEvents, ...) and then
        // QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete)
        // is invoked, before the qWait method returns.
        // The processEvents() method would process the QDeferredDeleteEvent for
        // the KDevelop::MainWindow, and with the glib event dispatcher
        // actually trigger posting the event to the handlers. So the destructor
        // is invoked, which again yields in the end the QDeferredDeleteEvent for
        // the Core, added to the current event queue.
        // So sendPostedEvents() then picks up that new event as well for
        // delivering it to the handlers, and thus the destructor of the Core
        // instance is invoked in the same qWait time slot.
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
