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

#include <KGlobal>
#include <KSharedConfig>
#include <KDebug>

#include <shell/testcore.h>
#include <shell/shellextension.h>
#include <debugger/interfaces/stackmodel.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/breakpoint/breakpoint.h>

#include "gdbcontroller.h"
#include "debugsession.h"
#include <debugger/breakpoint/breakpoints.h>

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

    //remove all breakpoints - so we can set our own in the test
    KConfigGroup breakpoints = KGlobal::config()->group("breakpoints");
    breakpoints.writeEntry("number", 0);
    breakpoints.sync();
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
    waitForState(session, KDevelop::IDebugSession::StoppedState);

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

    KDevelop::Breakpoints* breakpoints = KDevelop::ICore::self()->debugController()
                                            ->breakpointModel()->breakpointsItem();
    breakpoints->addCodeBreakpoint(fileName, 25);
    session.startProgram(run, 0);
    waitForState(session, DebugSession::PausedState);
    session.stepInto();
    waitForState(session, DebugSession::PausedState);
    session.stepInto();
    waitForState(session, DebugSession::PausedState);
    session.run();
    waitForState(session, DebugSession::StoppedState);
}

void GdbTest::testDisableBreakpoint()
{
    GDBController controller;
    DebugSession session(&controller);

    KDevelop::IRun run;
    run.setExecutable("unittests/debugee");
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugee.cpp";

    KDevelop::Breakpoints *breakpoints = KDevelop::ICore::self()->debugController()
                                            ->breakpointModel()->breakpointsItem();
    KDevelop::Breakpoint *b;
                                            
    //add disabled breakpoint before startProgram
    b = breakpoints->addCodeBreakpoint(fileName, 26);
    b->setColumn(KDevelop::Breakpoint::EnableColumn, false);

    b = breakpoints->addCodeBreakpoint(fileName, 21);
    session.startProgram(run, 0);
    waitForState(session, DebugSession::PausedState);

    //disable existing breakpoint
    b->setColumn(KDevelop::Breakpoint::EnableColumn, false);

    //add another disabled breakpoint
    b = breakpoints->addCodeBreakpoint(fileName, 28);
    QTest::qWait(300);
    b->setColumn(KDevelop::Breakpoint::EnableColumn, false);

    QTest::qWait(300);
    session.run();
    waitForState(session, DebugSession::StoppedState);
}

void GdbTest::testChangeLocationBreakpoint()
{
    GDBController controller;
    DebugSession session(&controller);

    KDevelop::IRun run;
    run.setExecutable("unittests/debugee");
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugee.cpp";

    KDevelop::Breakpoints *breakpoints = KDevelop::ICore::self()->debugController()
                                            ->breakpointModel()->breakpointsItem();

    //add disabled breakpoint before startProgram
    KDevelop::Breakpoint *b = breakpoints->addCodeBreakpoint(fileName, 25);

    session.startProgram(run, 0);
    waitForState(session, DebugSession::PausedState);

    QTest::qWait(100);
    b->setLine(29);
    QTest::qWait(100);
    session.run();

    QTest::qWait(100);
    waitForState(session, DebugSession::PausedState);

    session.run();
    waitForState(session, DebugSession::StoppedState);
}

void GdbTest::testDeleteBreakpoint()
{
    GDBController controller;
    DebugSession session(&controller);

    KDevelop::IRun run;
    run.setExecutable("unittests/debugee");
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugee.cpp";

    KDevelop::Breakpoints *breakpoints = KDevelop::ICore::self()->debugController()
                                            ->breakpointModel()->breakpointsItem();

    QCOMPARE(KDevelop::ICore::self()->debugController()->breakpointModel()->rowCount(), 1); //one for the "insert here" entry
    //add breakpoint before startProgram
    KDevelop::Breakpoint *b = breakpoints->addCodeBreakpoint(fileName, 21);
    QCOMPARE(KDevelop::ICore::self()->debugController()->breakpointModel()->rowCount(), 2);
    breakpoints->remove(KDevelop::ICore::self()->debugController()->breakpointModel()->index(0, 0));
    QCOMPARE(KDevelop::ICore::self()->debugController()->breakpointModel()->rowCount(), 1);

    b = breakpoints->addCodeBreakpoint(fileName, 22);

    session.startProgram(run, 0);
    waitForState(session, DebugSession::PausedState);

    breakpoints->remove(KDevelop::ICore::self()->debugController()->breakpointModel()->index(0, 0));

    QTest::qWait(100);
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

    KDevelop::Breakpoints* breakpoints = KDevelop::ICore::self()->debugController()
                                            ->breakpointModel()->breakpointsItem();
    breakpoints->addCodeBreakpoint(fileName, 26);
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
        QCOMPARE(arguments.at(1).toInt(), 26);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<KUrl>(), KUrl::fromPath(fileName));
        QCOMPARE(arguments.at(1).toInt(), 21);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<KUrl>(), KUrl::fromPath(fileName));
        QCOMPARE(arguments.at(1).toInt(), 22);
    }
}

void GdbTest::testStack()
{
    GDBController controller;
    DebugSession session(&controller);

    KDevelop::IRun run;
    run.setExecutable("unittests/debugee");
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugee.cpp";

    KDevelop::Breakpoints* breakpoints = KDevelop::ICore::self()->debugController()
                                            ->breakpointModel()->breakpointsItem();
    breakpoints->addCodeBreakpoint(fileName, 21);
    session.startProgram(run, 0);
    waitForState(session, DebugSession::PausedState);

    KDevelop::StackModel *model = session.stackModel();
    model->setAutoUpdate(true);
    QTest::qWait(100);

    QCOMPARE(model->rowCount(QModelIndex()), 1);
    QCOMPARE(model->columnCount(QModelIndex()), 3);

    QCOMPARE(model->data(model->index(0,0), Qt::DisplayRole).toString(), QString("Thread 1"));
    QCOMPARE(model->data(model->index(0,1), Qt::DisplayRole).toString(), QString("foo"));
    QCOMPARE(model->data(model->index(0,2), Qt::DisplayRole).toString(), fileName+QString(":22"));

    model->expanded(model->index(0,0));
    QTest::qWait(200);
    QCOMPARE(model->rowCount(model->index(0,0)), 1);
    QCOMPARE(model->data(model->index(0,0,model->index(0,0)), Qt::DisplayRole).toString(), QString("#1"));
    QCOMPARE(model->data(model->index(0,1,model->index(0,0)), Qt::DisplayRole).toString(), QString("main"));
    QCOMPARE(model->data(model->index(0,2,model->index(0,0)), Qt::DisplayRole).toString(), fileName+QString(":27"));


    session.run();
    waitForState(session, DebugSession::PausedState);
    session.run();
    waitForState(session, DebugSession::StoppedState);

}

void GdbTest::waitForState(const GDBDebugger::DebugSession &session, DebugSession::DebuggerState state)
{
    QTime stopWatch;
    stopWatch.start();
    while (session.state() != state) {
        if (stopWatch.elapsed() > 5000) qFatal("Didn't reach state");
        QTest::qWait(20);
        kDebug() << session.state() << state;
    }
}

QTEST_MAIN( GdbTest )

#include "gdbtest.moc"
