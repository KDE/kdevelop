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
#include <qtest_kde.h>

#include <shell/testcore.h>
#include <shell/shellextension.h>
#include <debugger/interfaces/stackmodel.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/breakpoint/breakpoint.h>
#include <debugger/interfaces/ibreakpointcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <debugger/variable/variablecollection.h>
#include <debugger/interfaces/ivariablecontroller.h>
#include <tests/autotestshell.h>

#include "gdbcommand.h"
#include "gdbcontroller.h"
#include "debugsession.h"

namespace GDBDebugger {


void GdbTest::init()
{
    AutoTestShell::init();
    m_core = new KDevelop::TestCore();
    m_core->initialize(KDevelop::Core::NoUi);

    //remove all breakpoints - so we can set our own in the test
    KConfigGroup breakpoints = KGlobal::config()->group("breakpoints");
    breakpoints.writeEntry("number", 0);
    breakpoints.sync();
    
    KDevelop::BreakpointModel* m = KDevelop::ICore::self()->debugController()->breakpointModel();
    m->removeRows(0, m->rowCount());

    KDevelop::VariableCollection *vc = KDevelop::ICore::self()->debugController()->variableCollection();
    for (int i=0; i < vc->watches()->childCount(); ++i) {
        delete vc->watches()->child(i);
    }
    vc->watches()->clear();
}

void GdbTest::cleanup()
{
    m_core->cleanup();
    delete m_core;
}

class TestLaunchConfiguration : public KDevelop::ILaunchConfiguration
{
public:
    TestLaunchConfiguration(KUrl executable = KUrl(QDir::currentPath()+"/unittests/debugee") ) {
        c = new KConfig();
        cfg = c->group("launch");
        cfg.writeEntry("isExecutable", true);
        cfg.writeEntry("Executable", executable);
    }
    ~TestLaunchConfiguration() {
        delete c;
    }
    virtual const KConfigGroup config() const { return cfg; }
    virtual QString name() const { return QString("Test-Launch"); }
    virtual KDevelop::IProject* project() const { return 0; }
    virtual KDevelop::LaunchConfigurationType* type() const { return 0; }
private:
    KConfigGroup cfg;
    KConfig *c;
};

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    TestDebugSession() : DebugSession(new GDBController), m_line(0)
    {
        qRegisterMetaType<KUrl>("KUrl");
        connect(this, SIGNAL(showStepInSource(KUrl, int)), SLOT(slotShowStepInSource(KUrl, int)));
        
        KDevelop::ICore::self()->debugController()->addSession(this);
    }
    ~TestDebugSession()
    {
        delete controller();
    }
    KUrl url() { return m_url; }
    int line() { return m_line; }

private slots:
    void slotShowStepInSource(const KUrl &url, int line)
    {
        m_url = url;
        m_line = line;
    }
private:
    KUrl m_url;
    int m_line;

};


#define WAIT_FOR_STATE(session, state) \
    waitForState((session), (state), __FILE__, __LINE__)

static const QString debugeeFileName = QFileInfo(__FILE__).dir().path()+"/debugee.cpp";

KDevelop::BreakpointModel* breakpoints()
{
    return KDevelop::ICore::self()->debugController()->breakpointModel();
}

void GdbTest::testStdOut()
{
    TestDebugSession *session = new TestDebugSession;

    QSignalSpy outputSpy(session, SIGNAL(applicationStandardOutputLines(QStringList)));

    TestLaunchConfiguration cfg;
    session->startProgram(&cfg);
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    {
        QCOMPARE(outputSpy.count(), 1);
        QList<QVariant> arguments = outputSpy.takeFirst();
        QCOMPARE(arguments.count(), 1);
        QCOMPARE(arguments.first().toStringList(), QStringList() << "Hello, world!" << "Hello");
    }
}

void GdbTest::testBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(debugeeFileName, 28);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::NotStartedState);

    session->startProgram(&cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::CleanState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testDisableBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b;

    //add disabled breakpoint before startProgram
    b = breakpoints()->addCodeBreakpoint(debugeeFileName, 29);
    b->setData(KDevelop::Breakpoint::EnableColumn, false);

    b = breakpoints()->addCodeBreakpoint(debugeeFileName, 21);
    session->startProgram(&cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    //disable existing breakpoint
    b->setData(KDevelop::Breakpoint::EnableColumn, false);

    //add another disabled breakpoint
    b = breakpoints()->addCodeBreakpoint(debugeeFileName, 31);
    QTest::qWait(300);
    b->setData(KDevelop::Breakpoint::EnableColumn, false);

    QTest::qWait(300);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

}

void GdbTest::testChangeLocationBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(debugeeFileName, 27);

    session->startProgram(&cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);

    QTest::qWait(100);
    b->setLine(28);
    QTest::qWait(100);
    session->run();

    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 28);
    QTest::qWait(500);
    breakpoints()->setData(breakpoints()->index(0, KDevelop::Breakpoint::LocationColumn), debugeeFileName+":30");
    QCOMPARE(b->line(), 29);
    QTest::qWait(100);
    QCOMPARE(b->line(), 29);
    session->run();
    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 29);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testDeleteBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    QCOMPARE(KDevelop::ICore::self()->debugController()->breakpointModel()->rowCount(), 1); //one for the "insert here" entry
    //add breakpoint before startProgram
    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(debugeeFileName, 21);
    QCOMPARE(KDevelop::ICore::self()->debugController()->breakpointModel()->rowCount(), 2);
    breakpoints()->removeRow(0);
    QCOMPARE(KDevelop::ICore::self()->debugController()->breakpointModel()->rowCount(), 1);

    b = breakpoints()->addCodeBreakpoint(debugeeFileName, 22);

    session->startProgram(&cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    breakpoints()->removeRow(0);
    QTest::qWait(100);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testPendingBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 28);

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QFileInfo(__FILE__).dir().path()+"/gdbtest.cpp", 10);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::NotStartedState);

    session->startProgram(&cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::PendingState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testUpdateBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(debugeeFileName, 28);
    QCOMPARE(KDevelop::ICore::self()->debugController()->breakpointModel()->rowCount(), 2);

    session->startProgram(&cfg);

    //insert custom command as user might do it using GDB console
    session->controller()->addCommand(new UserCommand(GDBMI::NonMI, "break "+debugeeFileName+":28"));

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(100);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(KDevelop::ICore::self()->debugController()->breakpointModel()->rowCount(), 3);
    b = breakpoints()->breakpoint(1);
    QCOMPARE(b->url(), KUrl(debugeeFileName));
    QCOMPARE(b->line(), 27);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testIgnoreHitsBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(debugeeFileName, 21);
    b->setIgnoreHits(1);

    b = breakpoints()->addCodeBreakpoint(debugeeFileName, 22);

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(100);
    b->setIgnoreHits(1);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testConditionBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(debugeeFileName, 39);
    b->setCondition("x[0] == 'H'");

    b = breakpoints()->addCodeBreakpoint(debugeeFileName, 23);
    b->setCondition("i==2");

    b = breakpoints()->addCodeBreakpoint(debugeeFileName, 24);

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);
    b->setCondition("i == 0");
    QTest::qWait(100);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 39);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnWriteBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addWatchpoint("foo::i");

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnWriteWithConditionBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b = breakpoints()->addWatchpoint("foo::i");
    b->setCondition("foo::i==2");

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnReadBreakpoint()
{
    /*
    test disabled because of gdb bug: http://sourceware.org/bugzilla/show_bug.cgi?id=10136

    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b = breakpoints()->addReadWatchpoint("foo::i");

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
    */
}

void GdbTest::testBreakOnReadBreakpoint2()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 27);

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);

    breakpoints()->addReadWatchpoint("foo::i");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnAccessBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 27);

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);

    breakpoints()->addAccessWatchpoint("foo::i");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);


    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointWhileRunning()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(KUrl(QDir::currentPath()+"/unittests/debugeeslow"));
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugeeslow.cpp";

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    QTest::qWait(2000);
    kDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(fileName, 23);
    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    b->setDeleted();
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointWhileRunningMultiple()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(KUrl(QDir::currentPath()+"/unittests/debugeeslow"));
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugeeslow.cpp";

    session->startProgram(&cfg);

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    QTest::qWait(2000);
    kDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b1 = breakpoints()->addCodeBreakpoint(fileName, 22);
    KDevelop::Breakpoint *b2 = breakpoints()->addCodeBreakpoint(fileName, 23);
    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    b1->setDeleted();
    b2->setDeleted();
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointFunctionName()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint("main");

    session->startProgram(&cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testShowStepInSource()
{
    TestDebugSession *session = new TestDebugSession;

    qRegisterMetaType<KUrl>("KUrl");
    QSignalSpy showStepInSourceSpy(session, SIGNAL(showStepInSource(KUrl, int)));

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 29);
    session->startProgram(&cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    {
        QCOMPARE(showStepInSourceSpy.count(), 3);
        QList<QVariant> arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<KUrl>(), KUrl::fromPath(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 29);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<KUrl>(), KUrl::fromPath(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 22);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<KUrl>(), KUrl::fromPath(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 23);
    }
}

void GdbTest::testStack()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 21);
    QVERIFY(session->startProgram(&cfg));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    KDevelop::StackModel *model = session->stackModel();
    model->setAutoUpdate(true);
    QTest::qWait(200);

    QCOMPARE(model->rowCount(QModelIndex()), 1);
    QCOMPARE(model->columnCount(QModelIndex()), 1);

    QCOMPARE(model->data(model->index(0,0), Qt::DisplayRole).toString(), QString("#1 at foo"));

    QTest::qWait(200);
    KDevelop::FramesModel* fmodel=model->modelForThread(0);
    QCOMPARE(fmodel->rowCount(), 2);
    QCOMPARE(fmodel->columnCount(), 3);
    QCOMPARE(fmodel->framesCount(), 2);
    QCOMPARE(fmodel->data(fmodel->index(0,0), Qt::DisplayRole).toString(), QString("0"));
    QCOMPARE(fmodel->data(fmodel->index(0,1), Qt::DisplayRole).toString(), QString("foo"));
    QCOMPARE(fmodel->data(fmodel->index(0,2), Qt::DisplayRole).toString(), debugeeFileName+QString(":23"));
    QCOMPARE(fmodel->data(fmodel->index(1,0), Qt::DisplayRole).toString(), QString("1"));
    QCOMPARE(fmodel->data(fmodel->index(1,1), Qt::DisplayRole).toString(), QString("main"));
    QCOMPARE(fmodel->data(fmodel->index(1,2), Qt::DisplayRole).toString(), debugeeFileName+QString(":29"));


    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}


void GdbTest::testAttach()
{
    QString fileName = QFileInfo(__FILE__).dir().path()+"/debugeeslow.cpp";

    KProcess debugeeProcess;
    debugeeProcess << "nice" << QDir::currentPath()+"/unittests/debugeeslow";
    debugeeProcess.start();
    Q_ASSERT(debugeeProcess.waitForStarted());
    QTest::qWait(100);

    TestDebugSession *session = new TestDebugSession;
    session->attachToProcess(debugeeProcess.pid());
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    breakpoints()->addCodeBreakpoint(fileName, 32);
    QTest::qWait(100);
    session->run();
    QTest::qWait(2000);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 32);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testCoreFile()
{
    QFile f("core");
    if (f.exists()) f.remove();

    KProcess debugeeProcess;
    debugeeProcess.setOutputChannelMode(KProcess::MergedChannels);
    debugeeProcess << "bash" << "-c" << "ulimit -c unlimited; ./unittests/debugeecrash";
    debugeeProcess.start();
    debugeeProcess.waitForFinished();
    kDebug() << debugeeProcess.readAll();
    QFile f2("core");
    if (!f2.exists()) {
        QFAIL("no core dump found");
    }

    TestDebugSession *session = new TestDebugSession;
    session->examineCoreFile(KUrl(QDir::currentPath()+"/unittests/debugeecrash"), KUrl(QDir::currentPath()+"/core"));

    KDevelop::StackModel *model = session->stackModel();
    model->setAutoUpdate(true);
    QTest::qWait(500);

    QCOMPARE(model->rowCount(QModelIndex()), 1);
    QCOMPARE(model->columnCount(QModelIndex()), 1);

    QCOMPARE(model->data(model->index(0,0), Qt::DisplayRole).toString(), QString("#1 at foo"));

    session->stopDebugger();
    QTest::qWait(100);
}


KDevelop::VariableCollection *variableCollection()
{
    return KDevelop::ICore::self()->debugController()->variableCollection();
}

#define COMPARE_DATA(index, expected) \
    compareData((index), (expected), __FILE__, __LINE__)
void compareData(QModelIndex index, QString expected, const char *file, int line)
{
    QString s = index.model()->data(index, Qt::DisplayRole).toString();
    if (s != expected) {
        kFatal() << QString("'%0' didn't match expected '%1' in %2:%3").arg(s).arg(expected).arg(file).arg(line);
    }
}

void GdbTest::testVariablesLocals()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(true);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 22);
    QVERIFY(session->startProgram(&cfg));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(variableCollection()->rowCount(), 2);
    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "0");
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesLocalsStruct()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(true);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 38);
    QVERIFY(session->startProgram(&cfg));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(300);

    QModelIndex i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);
    COMPARE_DATA(variableCollection()->index(1, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(1, 1, i), "{...}");
    QModelIndex ts = variableCollection()->index(1, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "...");
    variableCollection()->expanded(ts);
    QTest::qWait(100);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(variableCollection()->index(1, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(1, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesWatches()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(true);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 38);
    QVERIFY(session->startProgram(&cfg));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    
    variableCollection()->watches()->add("ts");
    QTest::qWait(300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    QModelIndex ts = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "...");
    variableCollection()->expanded(ts);
    QTest::qWait(100);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}


void GdbTest::testVariablesWatchesTwoSessions()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(true);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeFileName, 38);
    QVERIFY(session->startProgram(&cfg));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    
    variableCollection()->watches()->add("ts");
    QTest::qWait(300);

    QModelIndex ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    variableCollection()->expanded(ts);
    QTest::qWait(100);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
    
    //check if variable is marked as out-of-scope
    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    KDevelop::Variable* v = dynamic_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(v);
    QVERIFY(!v->inScope());
    QCOMPARE(v->childCount(), 3);
    v = dynamic_cast<KDevelop::Variable*>(v->child(0));
    QVERIFY(!v->inScope());

    //start a second debug session
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(true);
    QVERIFY(session->startProgram(&cfg));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(300);

    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    v = dynamic_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(v);
    QVERIFY(v->inScope());
    QCOMPARE(v->childCount(), 3);

    v = dynamic_cast<KDevelop::Variable*>(v->child(0));
    QVERIFY(v->inScope());
    QCOMPARE(v->data(1, Qt::DisplayRole).toString(), QString::number(0));
    
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
    
    //check if variable is marked as out-of-scope
    v = dynamic_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(!v->inScope());
    QVERIFY(!dynamic_cast<KDevelop::Variable*>(v->child(0))->inScope());
}

void GdbTest::waitForState(GDBDebugger::DebugSession *session, DebugSession::DebuggerState state,
                            const char *file, int line)
{
    QPointer<GDBDebugger::DebugSession> s(session); //session can get deleted in DebugController
    kDebug() << "waiting for state" << state;
    QTime stopWatch;
    stopWatch.start();
    while (s->state() != state) {
        if (stopWatch.elapsed() > 5000) {
            kFatal() << QString("Didn't reach state in %0:%1").arg(file).arg(line);
        }
        QTest::qWait(20);
        if (!s) {
            if (state == DebugSession::EndedState) break;
            kFatal() << QString("Didn't reach state; session ended in %0:%1").arg(file).arg(line);
        }
    }
    QTest::qWait(100);
}

}

QTEST_KDEMAIN(GDBDebugger::GdbTest, GUI)


#include "gdbtest.moc"
#include "moc_gdbtest.cpp"
