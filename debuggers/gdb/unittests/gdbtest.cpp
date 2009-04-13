/*
   Copyright 2009 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "gdbtest.h"

#include <QtTest/QTest>
#include <QSignalSpy>
#include <QDebug>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

#include <shell/testcore.h>
#include <shell/shellextension.h>

#include <debugger/interfaces/stackmodel.h>
#include <debugger/interfaces/ibreakpoint.h>

#include "breakpointcontroller.h"
#include "gdbcontroller.h"
#include "debugsession.h"

using namespace GDBDebugger;

class AutoTestShell : public KDevelop::ShellExtension
{
public:
    QString xmlFile() { return QString(); }
    QString defaultProfile() { return "kdevtest"; }
    KDevelop::AreaParams defaultArea() {
        KDevelop::AreaParams params;
        params.name = "test";
        params.title = "Test";
        return params;
    }
    QString projectFileExtension() { return QString(); }
    QString projectFileDescription() { return QString(); }
    QStringList defaultPlugins() { return QStringList(); }

    static void init() { s_instance = new AutoTestShell; }
};

void GdbTest::init()
{
    AutoTestShell::init();
    m_core = new KDevelop::TestCore();
    m_core->initialize(KDevelop::Core::NoUi);

}

void GdbTest::cleanup()
{
    m_core->cleanup();
    delete m_core;
}

void GdbTest::testStdOut()
{
    GDBController controller;
    DebugSession session(&controller);

    QSignalSpy outputSpy(&session, SIGNAL(applicationStandardOutputLines(QStringList)));

    KDevelop::IRun run;
    run.setExecutable("unittests/debugee");

    session.startProgram(run, 0);
    waitForState(session, DebugSession::StoppedState);

    {
        QCOMPARE(outputSpy.count(), 1);
        QList<QVariant> arguments = outputSpy.takeFirst();
        QCOMPARE(arguments.count(), 1);
        QCOMPARE(arguments.first().toStringList(), QStringList() <<"Hello, world!");
    }
}

void GdbTest::testBreakpoint()
{
    GDBController controller;
    DebugSession session(&controller);

    KDevelop::IRun run;
    run.setExecutable("unittests/debugee");
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugee.cpp";

    BreakpointController* c = session.controller()->breakpoints();
    KDevelop::IBreakpoints* breakpoints = c->breakpointsItem();
    breakpoints->addCodeBreakpoint(fileName+":8");
    session.startProgram(run, 0);
    waitForState(session, DebugSession::PausedState);
    session.stepInto();
    waitForState(session, DebugSession::PausedState);
    session.stepInto();
    waitForState(session, DebugSession::PausedState);
    session.run();
    waitForState(session, DebugSession::StoppedState);
}

void GdbTest::testShowStepInSource()
{
    GDBController controller;
    DebugSession session(&controller);

    qRegisterMetaType<KUrl>("KUrl");
    QSignalSpy showStepInSourceSpy(&session, SIGNAL(showStepInSource(KUrl, int)));

    KDevelop::IRun run;
    run.setExecutable("unittests/debugee");
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugee.cpp";

    BreakpointController* c = session.controller()->breakpoints();
    KDevelop::IBreakpoints* breakpoints = c->breakpointsItem();
    breakpoints->addCodeBreakpoint(fileName+":8");
    session.startProgram(run, 0);
    waitForState(session, DebugSession::PausedState);
    session.stepInto();
    waitForState(session, DebugSession::PausedState);
    session.stepInto();
    waitForState(session, DebugSession::PausedState);
    session.run();
    waitForState(session, DebugSession::StoppedState);

    {
        QCOMPARE(showStepInSourceSpy.count(), 3);
        QList<QVariant> arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<KUrl>(), KUrl::fromPath(fileName));
        QCOMPARE(arguments.at(1).toInt(), 7);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<KUrl>(), KUrl::fromPath(fileName));
        QCOMPARE(arguments.at(1).toInt(), 8);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<KUrl>(), KUrl::fromPath(fileName));
        QCOMPARE(arguments.at(1).toInt(), 3);
    }
}

void GdbTest::testStack()
{
    GDBController controller;
    DebugSession session(&controller);

    KDevelop::IRun run;
    run.setExecutable("unittests/debugee");
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugee.cpp";

    BreakpointController* c = session.controller()->breakpoints();
    KDevelop::IBreakpoints* breakpoints = c->breakpointsItem();
    breakpoints->addCodeBreakpoint(fileName+":4");
    session.startProgram(run, 0);
    waitForState(session, DebugSession::PausedState);

    KDevelop::StackModel *model = session.stackModel();
    model->setAutoUpdate(true);
    QTest::qWait(100);

    QCOMPARE(model->rowCount(QModelIndex()), 1);
    QCOMPARE(model->columnCount(QModelIndex()), 3);

    QCOMPARE(model->data(model->index(0,0), Qt::DisplayRole).toString(), QString("Thread 1"));
    QCOMPARE(model->data(model->index(0,1), Qt::DisplayRole).toString(), QString("foo"));
    QCOMPARE(model->data(model->index(0,2), Qt::DisplayRole).toString(), fileName+QString(":4"));

    kDebug() << "---------------------------------------------------------------------------------";
    model->expanded(model->index(0,0));
    QTest::qWait(200);
    kDebug() << model->rowCount(QModelIndex());
    kDebug() << model->rowCount(model->index(0,0));
    kDebug() << model->rowCount(model->index(0,1));
    kDebug() << model->rowCount(model->index(0,2));
    QCOMPARE(model->rowCount(model->index(0,0)), 1);
    QCOMPARE(model->data(model->index(0,0,model->index(0,0)), Qt::DisplayRole).toString(), QString("#1"));
    QCOMPARE(model->data(model->index(0,1,model->index(0,0)), Qt::DisplayRole).toString(), QString("main"));
    QCOMPARE(model->data(model->index(0,2,model->index(0,0)), Qt::DisplayRole).toString(), fileName+QString(":9"));


    session.run();
    waitForState(session, DebugSession::StoppedState);

}

void GdbTest::waitForState(const GDBDebugger::DebugSession &session, DebugSession::DebuggerState state)
{
    QTime stopWatch;
    stopWatch.start();
    while (session.state() != state) {
        if (stopWatch.elapsed() > 5000) QFAIL("Didn't reach state");
        QTest::qWait(20);
        kDebug() << session.state();
    }
}

QTEST_MAIN( GdbTest )

#include "gdbtest.moc"
