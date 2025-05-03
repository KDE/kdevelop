/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "test_gdb.h"

#include "debugsession.h"
#include "gdbframestackmodel.h"
#include "mi/micommand.h"
#include "mi/milexer.h"
#include "mi/miparser.h"
#include "tests/debuggers-tests-config.h"
#include "tests/testhelper.h"

#include <execute/iexecuteplugin.h>
#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/framestack/framestackmodel.h>
#include <debugger/interfaces/ivariablecontroller.h>
#include <debugger/variable/variablecollection.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>
#include <shell/shellextension.h>

#include <KIO/Global>
#include <KProcess>
#include <KSharedConfig>
#include <KShell>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryFile>

using KDevelop::AutoTestShell;
using KDevMI::Testing::breakpoints;
using KDevMI::Testing::findExecutable;
using KDevMI::Testing::findFile;
using KDevMI::Testing::findSourceFile;
using KDevMI::Testing::TestLaunchConfiguration;
using KDevMI::Testing::validateColumnCountsThreadCountAndStackFrameNumbers;

namespace KDevMI { namespace GDB {

void GdbTest::initTestCase()
{
#ifdef Q_OS_WIN
    QSKIP("apparently this test is killing processes, but is leaving kdeinit5, dbus-daemon and klauncher running, "
          "breaking the Windows CI builder");
#endif
    AutoTestShell::init();
    AutoTestShell::initializeNotifications();
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    m_iface = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevexecute"))->extension<IExecutePlugin>();
    Q_ASSERT(m_iface);
}

void GdbTest::cleanupTestCase()
{
    KDevelop::TestCore::shutdown();
}

void GdbTest::init()
{
    //remove all breakpoints - so we can set our own in the test
    KConfigGroup breakpoints = KSharedConfig::openConfig()->group("breakpoints");
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

class TestFrameStackModel : public GdbFrameStackModel
{
    Q_OBJECT

public:

    explicit TestFrameStackModel(DebugSession* session)
        : GdbFrameStackModel(session), fetchFramesCalled(0), fetchThreadsCalled(0) {}

    int fetchFramesCalled;
    int fetchThreadsCalled;
    void fetchFrames(int threadNumber, int from, int to) override
    {
        fetchFramesCalled++;
        GdbFrameStackModel::fetchFrames(threadNumber, from, to);
    }

    void fetchThreads() override
    {
        fetchThreadsCalled++;
        GdbFrameStackModel::fetchThreads();
    }
};

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    TestDebugSession() : DebugSession()
    {
        setSourceInitFile(false);
        setAutoDisableASLR(false);
        m_frameStackModel = new TestFrameStackModel(this);
        KDevelop::ICore::self()->debugController()->addSession(this);
    }

    QUrl url() { return currentUrl(); }
    int line() { return currentLine(); }

    TestFrameStackModel* frameStackModel() const override
    {
        return m_frameStackModel;
    }

private:
    TestFrameStackModel* m_frameStackModel;
};

static const QString debugeeFileName = findSourceFile(QStringLiteral("debugee.cpp"));

void GdbTest::testStdOut()
{
    auto *session = new TestDebugSession;

    QSignalSpy outputSpy(session, &TestDebugSession::inferiorStdoutLines);

    TestLaunchConfiguration cfg;
    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    {
        QCOMPARE(outputSpy.count(), 1);
        QList<QVariant> arguments = outputSpy.takeFirst();
        QCOMPARE(arguments.count(), 1);
        QCOMPARE(arguments.first().toStringList(), QStringList() << "Hello, world!" << "Hello");
    }
}

void GdbTest::testEnvironmentSet()
{
    KDevMI::Testing::testEnvironmentSet(new TestDebugSession, QStringLiteral("GdbTestGroup"), m_iface);
}

void GdbTest::testUnsupportedUrlExpressionBreakpoints()
{
    // GDB/MI breaks some non-ASCII characters by replacing certain bytes, e.g. \x85 with \\205 and \x96 with \\226.
    // TODO: replace the bytes back in kdevgdb somewhere?
    const bool debuggerSupportsNonAsciiExpressions = false;
    KDevMI::Testing::testUnsupportedUrlExpressionBreakpoints(new TestDebugSession, m_iface,
                                                             debuggerSupportsNonAsciiExpressions);
}

void GdbTest::testBreakpoint()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);
}

void GdbTest::testDisableBreakpoint()
{
    //Description: We must stop only on the third breakpoint

    int firstBreakLine=28;
    int secondBreakLine=23;
    int thirdBreakLine=24;
    int fourthBreakLine=31;

    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b;

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), firstBreakLine);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);


    //this is needed to emulate debug from GUI. If we are in edit mode, the debugSession doesn't exist.
    KDevelop::ICore::self()->debugController()->breakpointModel()->blockSignals(true);
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), secondBreakLine);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    //all disabled breakpoints were added

    KDevelop::Breakpoint * thirdBreak = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), thirdBreakLine);
    KDevelop::ICore::self()->debugController()->breakpointModel()->blockSignals(false);


    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), thirdBreak->line());

    //disable existing breakpoint
    thirdBreak->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    //add another disabled breakpoint
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), fourthBreakLine);
    QTest::qWait(300);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    QTest::qWait(300);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testChangeLocationBreakpoint()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 27);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);

    QTest::qWait(100);
    b->setLine(28);
    QTest::qWait(100);
    session->run();

    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 28);
    QTest::qWait(500);
    breakpoints()->setData(breakpoints()->index(0, KDevelop::Breakpoint::LocationColumn), QString(debugeeFileName+":30"));
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
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    QCOMPARE(breakpoints()->rowCount(), 0);
    //add breakpoint before startDebugging
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    QCOMPARE(breakpoints()->rowCount(), 1);
    QVERIFY(breakpoints()->removeRow(0));
    QCOMPARE(breakpoints()->rowCount(), 0);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 22);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QVERIFY(breakpoints()->removeRow(0));
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testPendingBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile(QStringLiteral("debugeeqt.cpp"))), 10);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::PendingState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakpointsOnNoOpLines()
{
    KDevMI::Testing::testBreakpointsOnNoOpLines(new TestDebugSession, m_iface);
}

void GdbTest::testUpdateBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    // breakpoint 1: real line 29: foo();
    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);
    QCOMPARE(breakpoints()->rowCount(), 1);

    session->startDebugging(&cfg, m_iface);

    // breakpoint 2: real line 32: const char *x = "Hello";
    //insert custom command as user might do it using GDB console
    session->addCommand(std::make_unique<MI::UserCommand>(MI::NonMI, "break " + debugeeFileName + ":32"));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // stop at breakpoint 1, with custom command handled
    QCOMPARE(session->currentLine(), 28);

    // check breakpoint 2 got picked up
    QCOMPARE(breakpoints()->rowCount(), 2);
    b = breakpoints()->breakpoint(1);
    QCOMPARE(b->url(), QUrl::fromLocalFile(debugeeFileName));
    QCOMPARE(b->line(), 31);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState); // stop at breakpoint 2
    QCOMPARE(session->currentLine(), 31);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testIgnoreHitsBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    b1->setIgnoreHits(1);

    KDevelop::Breakpoint * b2 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 22);

    session->startDebugging(&cfg, m_iface);

    //WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR(session, session->state() == DebugSession::PausedState && b2->hitCount() == 1);
    b2->setIgnoreHits(1);
    session->run();
    //WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR(session, session->state() == DebugSession::PausedState && b1->hitCount() == 1);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testConditionBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 39);
    b->setCondition(QStringLiteral("x[0] == 'H'"));

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 23);
    b->setCondition(QStringLiteral("i==2"));

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR(session, session->state() == DebugSession::PausedState && session->line() == 24);
    b->setCondition(QStringLiteral("i == 0"));
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
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addWatchpoint(QStringLiteral("i"));
    QTest::qWait(100);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: ++i; int j = i;
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnWriteWithConditionBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    KDevelop::Breakpoint *b = breakpoints()->addWatchpoint(QStringLiteral("i"));
    b->setCondition(QStringLiteral("i==2"));
    QTest::qWait(100);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: ++i; int j = i;
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnReadBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addReadWatchpoint("foo::i");

    session->startDebugging(&cfg, m_iface);

    for (int fooCall = 0; fooCall < 2; ++fooCall) {
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(session->line(), 22); // ++i;
        session->run();

        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(session->line(), 22); // int j = i;
        session->run();
    }

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

// This test adds a read watchpoint during a debug session rather than before it in order to
// work around the (already fixed) GDB bug http://sourceware.org/bugzilla/show_bug.cgi?id=10136
void GdbTest::testBreakOnReadBreakpoint2()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addReadWatchpoint(QStringLiteral("i"));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // ++i

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // int j = i

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    if(session->line() == 22) { // some GDB versions break 3 times on this line
        session->run();
        WAIT_FOR_STATE(session, DebugSession::PausedState);
    }
    QCOMPARE(session->line(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnAccessBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addAccessWatchpoint(QStringLiteral("i"));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: ++i (read)

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: ++i (write)

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: int j = i (read)

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakpointErrors()
{
    KDevMI::Testing::testBreakpointErrors(new TestDebugSession, m_iface, false);
}

void GdbTest::testInsertBreakpointWhileRunning()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeslow"));
    QString fileName = findSourceFile(QStringLiteral("debugeeslow.cpp"));

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    QTest::qWait(2000);
    qDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 29); // ++i;
    QTest::qWait(500);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);
    QCOMPARE(session->line(), 29); // ++i;
    breakpoints()->removeBreakpoint(b);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointWhileRunningMultiple()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeslow"));
    QString fileName = findSourceFile(QStringLiteral("debugeeslow.cpp"));

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    QTest::qWait(2000);
    qDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 29); // ++i;
    KDevelop::Breakpoint *b2 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 30); // std::cout << i << std::endl;
    QTest::qWait(500);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);
    QCOMPARE(session->line(), 29);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);
    QCOMPARE(session->line(), 30);
    breakpoints()->removeBreakpoint(b1);
    breakpoints()->removeBreakpoint(b2);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointFunctionName()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testManualBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);

    breakpoints()->removeRows(0, 1);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->addCommand(MI::NonMI, QStringLiteral("break debugee.cpp:23"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 1);

    KDevelop::Breakpoint* b = breakpoints()->breakpoint(0);
    QCOMPARE(b->line(), 22);

    session->addCommand(MI::NonMI, QStringLiteral("disable 2"));
    session->addCommand(MI::NonMI, QStringLiteral("condition 2 i == 1"));
    session->addCommand(MI::NonMI, QStringLiteral("ignore 2 1"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(b->enabled(), false);
    QCOMPARE(b->condition(), QString("i == 1"));
    QCOMPARE(b->ignoreHits(), 1);

    session->addCommand(MI::NonMI, QStringLiteral("delete 2"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testShowStepInSource()
{
    auto *session = new TestDebugSession;

    QSignalSpy showStepInSourceSpy(session, &TestDebugSession::showStepInSource);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 29);
    session->startDebugging(&cfg, m_iface);
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
        QCOMPARE(arguments.first().toUrl(), QUrl::fromLocalFile(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 29);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().toUrl(), QUrl::fromLocalFile(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 22);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().toUrl(), QUrl::fromLocalFile(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 23);
    }
}

void GdbTest::testStack()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex tIdx = stackModel->index(0,0);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at foo");
    QCOMPARE(stackModel->rowCount(tIdx), 2);
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "foo");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), debugeeFileName+":23");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(1, 2, tIdx), debugeeFileName+":29");

    session->stepOut();
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 1);
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    // depending on the compiler and gdb version, we may either end up in
    // one line or the other
    const auto last = stackModel->index(0, 2, tIdx).data().toString();
    if (last.endsWith(":29"))
        QCOMPARE(last, debugeeFileName + ":29");
    else
        QCOMPARE(last, debugeeFileName + ":30");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testStackFetchMore()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeerecursion"));
    QString fileName = findSourceFile(QStringLiteral("debugeerecursion.cpp"));

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->frameStackModel()->fetchFramesCalled, 1);

    QModelIndex tIdx = stackModel->index(0,0);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at foo");
    QCOMPARE(stackModel->rowCount(tIdx), 21);
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "foo");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), fileName+":26");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "foo");
    COMPARE_DATA(stackModel->index(1, 2, tIdx), fileName+":24");
    COMPARE_DATA(stackModel->index(2, 1, tIdx), "foo");
    COMPARE_DATA(stackModel->index(2, 2, tIdx), fileName+":24");

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 2);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    QCOMPARE(stackModel->rowCount(tIdx), 41);

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 3);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    QCOMPARE(stackModel->rowCount(tIdx), 121);

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    QCOMPARE(stackModel->rowCount(tIdx), 299);
    COMPARE_DATA(stackModel->index(298, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(298, 2, tIdx), fileName+":30");

    stackModel->fetchMoreFrames(); //nothing to fetch, we are at the end
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    QCOMPARE(stackModel->rowCount(tIdx), 299);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testStackSwitchThread()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeethreads"));
    QString fileName = findSourceFile(QStringLiteral("debugeethreads.cpp"));

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 43); // QThread::msleep(600);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex tIdx = stackModel->index(0,0);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 4);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 1);
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), fileName+":44"); // QThread::msleep(600);

    tIdx = stackModel->index(1,0);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 4);
    RETURN_IF_TEST_FAILED();
    QVERIFY(stackModel->data(tIdx).toString().startsWith("#2 at "));

    stackModel->setCurrentThread(2);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 4);
    RETURN_IF_TEST_FAILED();
    QTRY_VERIFY(stackModel->rowCount(tIdx) > 3);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testAttach()
{
    SKIP_IF_ATTACH_FORBIDDEN();

    QString fileName = findSourceFile(QStringLiteral("debugeeslow.cpp"));

    KProcess debugeeProcess;
    debugeeProcess << QStringLiteral("nice") << findExecutable(QStringLiteral("debuggee_debugeeslow")).toLocalFile();
    debugeeProcess.start();
    QVERIFY(debugeeProcess.waitForStarted());
    QTest::qWait(100);

    auto *session = new TestDebugSession;
    session->attachToProcess(debugeeProcess.processId());
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 39); // the blank line in main()
    QTest::qWait(100);
    session->run();
    QTest::qWait(2000);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 40); // return 0; (GDB automatically moves the breakpoint from its no-op line)

    session->run();
#ifdef Q_OS_FREEBSD
    QSKIP("The test freezes while waiting for the ended state, so skip it");
#endif
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testManualAttach()
{
    SKIP_IF_ATTACH_FORBIDDEN();

    KProcess debugeeProcess;
    debugeeProcess << QStringLiteral("nice") << findExecutable(QStringLiteral("debuggee_debugeeslow")).toLocalFile();
    debugeeProcess.start();
    QVERIFY(debugeeProcess.waitForStarted());

    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(Config::RemoteGdbRunEntry,
                            QUrl::fromLocalFile(findFile(GDB_SRC_DIR,
                                                         QStringLiteral("unittests/gdb_script_empty"))));
    QVERIFY(session->startDebugging(&cfg, m_iface));

    session->addCommand(MI::NonMI, QStringLiteral("attach %0").arg(debugeeProcess.processId()));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->run();
#ifdef Q_OS_FREEBSD
    QSKIP("The test freezes while waiting for the ended state, so skip it");
#endif
    QTest::qWait(2000); // give the slow inferior some extra time to run
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testCoreFile()
{
    QFileInfo f(QStringLiteral("core"));
    f.setCaching(false); // don't cache information
    if (f.exists()) {
        QVERIFY(QFile::remove(f.canonicalFilePath()));
    }

    KProcess debugeeProcess;
    debugeeProcess.setOutputChannelMode(KProcess::MergedChannels);
    debugeeProcess << QStringLiteral("bash") << QStringLiteral("-c")
                   << "ulimit -c unlimited; "
                      + findExecutable(QStringLiteral("debuggee_crash")).toLocalFile();
    debugeeProcess.start();
    debugeeProcess.waitForFinished();
    qDebug() << "Debuggee output:\n" << debugeeProcess.readAll();

    bool coreFileFound = f.exists();
    if (!coreFileFound) {
        // Try to use coredumpctl
        auto coredumpctl = QStandardPaths::findExecutable(QStringLiteral("coredumpctl"));
        if (!coredumpctl.isEmpty()) {
            KProcess::execute(coredumpctl, {"-1", "-o", f.absoluteFilePath(), "dump", "debuggee_crash"}, 5000);
            // coredumpctl seems to create an empty file "core" even if no cores can be delivered
            // (like when run inside docker containers as on KDE CI or with kernel.core_pattern=|/dev/null)
            // so also check for size != 0
            coreFileFound = f.exists() && (f.size() > 0);
        }
    }
    if (!coreFileFound)
        QSKIP("no core dump found, check your system configuration (see /proc/sys/kernel/core_pattern).");

    auto *session = new TestDebugSession;
    session->examineCoreFile(findExecutable(QStringLiteral("debuggee_crash")),
                             QUrl::fromLocalFile(f.canonicalFilePath()));

    TestFrameStackModel *stackModel = session->frameStackModel();

    WAIT_FOR_STATE(session, DebugSession::StoppedState);

    QModelIndex tIdx = stackModel->index(0,0);
    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at foo");

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}


KDevelop::VariableCollection *variableCollection()
{
    return KDevelop::ICore::self()->debugController()->variableCollection();
}

void GdbTest::testVariablesLocals()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 22);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);

    QCOMPARE(variableCollection()->rowCount(), 2);
    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 2);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");
    // COMPARE_DATA(variableCollection()->index(1, 1, i), "1"); // j is not initialized yet
    session->run();
    QTest::qWait(1000);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(1, 1, i), "1");
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesLocalsStruct()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);

    QModelIndex i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);

    int structIndex = 0;
    for(int j=0; j<3; ++j) {
        if (variableCollection()->index(j, 0, i).data().toString() == QLatin1String("ts")) {
            structIndex = j;
        }
    }

    COMPARE_DATA(variableCollection()->index(structIndex, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(structIndex, 1, i), "{...}");
    QModelIndex ts = variableCollection()->index(structIndex, 0, i);
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
    QTest::qWait(1000);
    COMPARE_DATA(variableCollection()->index(structIndex, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(structIndex, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesWatches()
{
    auto *session = new TestDebugSession;
    KDevelop::ICore::self()->debugController()->variableCollection()->variableWidgetShown();

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("ts"));
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
    QTest::qWait(100);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesWatchesQuotes()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    TestLaunchConfiguration cfg;

    // the unquoted string (the actual content):               t\"t
    // quoted string (what we would write as a c string):     "t\\\"t"
    // written in source file:                             R"("t\\\"t")"
    const QString testString(QStringLiteral("t\\\"t")); // the actual content
    const QString quotedTestString(QStringLiteral(R"("t\\\"t")"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(quotedTestString); //just a constant string
    QTest::qWait(300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), quotedTestString);
    COMPARE_DATA(variableCollection()->index(0, 1, i), "[" + QString::number(testString.length() + 1) + "]");

    QModelIndex testStr = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, testStr), "...");
    variableCollection()->expanded(testStr);
    QTest::qWait(100);
    int len = testString.length();
    for (int ind = 0; ind < len; ind++)
    {
        COMPARE_DATA(variableCollection()->index(ind, 0, testStr), QString::number(ind));
        QChar c = testString.at(ind);
        QString value = QString::number(c.toLatin1()) + " '";
        if (c == '\\')
            value += QLatin1String("\\\\");
        else if (c == '\'')
            value += QLatin1String("\\'");
        else
            value += c;
        value += QLatin1String("'");
        COMPARE_DATA(variableCollection()->index(ind, 1, testStr), value);
    }
    COMPARE_DATA(variableCollection()->index(len, 0, testStr), QString::number(len));
    COMPARE_DATA(variableCollection()->index(len, 1, testStr), "0 '\\000'");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesWatchesTwoSessions()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    QTest::qWait(300);

    QModelIndex ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    variableCollection()->expanded(ts);
    QTest::qWait(100);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //check if variable is marked as out-of-scope
    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    auto* v = qobject_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(v);
    QVERIFY(!v->inScope());
    QCOMPARE(v->childCount(), 3);
    v = qobject_cast<KDevelop::Variable*>(v->child(0));
    QVERIFY(!v->inScope());

    //start a second debug session
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(300);

    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    v = qobject_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(v);
    QVERIFY(v->inScope());
    QCOMPARE(v->childCount(), 3);

    v = qobject_cast<KDevelop::Variable*>(v->child(0));
    QVERIFY(v->inScope());
    QCOMPARE(v->data(1, Qt::DisplayRole).toString(), QString::number(0));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //check if variable is marked as out-of-scope
    v = qobject_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(!v->inScope());
    QVERIFY(!dynamic_cast<KDevelop::Variable*>(v->child(0))->inScope());
}

void GdbTest::testVariablesStopDebugger()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->stopDebugger();
    QTest::qWait(300);
}


void GdbTest::testVariablesStartSecondSession()
{
    QPointer<TestDebugSession> session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QPointer<TestDebugSession> session2 = new TestDebugSession;
    session2->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session2->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session2, DebugSession::PausedState);

    session2->run();
    WAIT_FOR_STATE(session2, DebugSession::EndedState);
}

void GdbTest::testVariablesSwitchFrame()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 2);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");

    stackModel->setCurrentFrame(1);
    QTest::qWait(200);

    i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);
    COMPARE_DATA(variableCollection()->index(2, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(2, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(3, 0, i), "argv");

    breakpoints()->removeRow(0);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesQuicklySwitchFrame()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 2);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");

    stackModel->setCurrentFrame(1);
    QTest::qWait(300);
    stackModel->setCurrentFrame(0);
    QTest::qWait(1);
    stackModel->setCurrentFrame(1);
    QTest::qWait(1);
    stackModel->setCurrentFrame(0);
    QTest::qWait(1);
    stackModel->setCurrentFrame(1);
    QTest::qWait(500);

    i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);
    QStringList locs;
    for (int j = 0; j < variableCollection()->rowCount(i); ++j) {
        locs << variableCollection()->index(j, 0, i).data().toString();
    }
    QVERIFY(locs.contains("argc"));
    QVERIFY(locs.contains("argv"));
    QVERIFY(locs.contains("x"));

    breakpoints()->removeRow(0);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}


void GdbTest::testSegfaultDebugee()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_crash"));
    QString fileName = findSourceFile(QStringLiteral("debugeecrash.cpp"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 23);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testSwitchFrameGdbConsole()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(stackModel->currentFrame(), 0);
    stackModel->setCurrentFrame(1);
    QCOMPARE(stackModel->currentFrame(), 1);
    QTest::qWait(500);
    QCOMPARE(stackModel->currentFrame(), 1);

    session->addUserCommand(QStringLiteral("print x"));
    QTest::qWait(500);
    //currentFrame must not reset to 0; Bug 222882
    QCOMPARE(stackModel->currentFrame(), 1);

}

void GdbTest::testInsertAndRemoveBreakpointWhileRunning()
{
    KDevMI::Testing::testInsertAndRemoveBreakpointWhileRunning(new TestDebugSession, m_iface, false);
}

//Bug 274390
void GdbTest::testCommandOrderFastStepping()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeqt"));

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    QVERIFY(session->startDebugging(&cfg, m_iface));
    for(int i=0; i<20; i++) {
        session->stepInto();
    }
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testPickupManuallyInsertedBreakpoint()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    QVERIFY(session->startDebugging(&cfg, m_iface));
    session->addCommand(MI::NonMI, QStringLiteral("break debugee.cpp:32"));
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000); //wait for breakpoints update
    QCOMPARE(breakpoints()->breakpoints().count(), 2);
    QCOMPARE(breakpoints()->rowCount(), 2);
    KDevelop::Breakpoint *b = breakpoints()->breakpoint(1);
    QVERIFY(b);
    QCOMPARE(b->line(), 31); //we start with 0, gdb with 1
    QCOMPARE(b->url().fileName(), QString("debugee.cpp"));
}

//Bug 270970
void GdbTest::testPickupManuallyInsertedBreakpointOnlyOnce()
{
    auto *session = new TestDebugSession;

    //inject here, so it behaves similar like a command from .gdbinit
    QTemporaryFile configScript;
    configScript.open();
    configScript.write(QStringLiteral("file %0\n").arg(findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile()).toLocal8Bit());
    configScript.write("break debugee.cpp:32\n");
    configScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbConfigEntry, QUrl::fromLocalFile(configScript.fileName()));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("debugee.cpp")), 31);
    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->breakpoints().count(), 1);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testPickupCatchThrowOnlyOnce()
{
    QTemporaryFile configScript;
    configScript.open();
    configScript.write("catch throw\n");
    configScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbConfigEntry, QUrl::fromLocalFile(configScript.fileName()));


    for (int i = 0; i < 2; ++i) {
        auto* session = new TestDebugSession;
        QVERIFY(session->startDebugging(&cfg, m_iface));
        WAIT_FOR_STATE(session, DebugSession::EndedState);
    }

    QTRY_COMPARE(breakpoints()->rowCount(), 1); //one from kdevelop, one from runScript
}

void GdbTest::testRunGdbScript()
{
    auto *session = new TestDebugSession;

    QTemporaryFile runScript;
    runScript.open();

    runScript.write("file " + KShell::quoteArg(findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile()).toUtf8() + "\n");
    runScript.write("break main\n");
    runScript.write("run\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 27);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testRemoteDebug()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable(QStringLiteral("gdbserver"));
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available");
    }

    auto *session = new TestDebugSession;

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + "\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + "\n");
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbShellEntry, QUrl::fromLocalFile((shellScript.fileName()+"-copy")));
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QFile::remove(shellScript.fileName()+"-copy");
}

void GdbTest::testRemoteDebugInsertBreakpoint()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable(QStringLiteral("gdbserver"));
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available");
    }

    auto *session = new TestDebugSession;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 29);
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 35);

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + "\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + '\n');
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbShellEntry, QUrl::fromLocalFile(shellScript.fileName()+"-copy"));
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    QCOMPARE(breakpoints()->breakpoints().count(), 2); //one from kdevelop, one from runScript

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QFile::remove(shellScript.fileName()+"-copy");
}


void GdbTest::testRemoteDebugInsertBreakpointPickupOnlyOnce()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable(QStringLiteral("gdbserver"));
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available");
    }

    auto *session = new TestDebugSession;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 35);

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 "+findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toLatin1()+"\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file "+findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toLatin1()+"\n");
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbShellEntry, QUrl::fromLocalFile((shellScript.fileName()+"-copy")));
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    QCOMPARE(breakpoints()->breakpoints().count(), 2); //one from kdevelop, one from runScript

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //************************** second session
    session = new TestDebugSession;
    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    QCOMPARE(breakpoints()->breakpoints().count(), 2); //one from kdevelop, one from runScript

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QFile::remove(shellScript.fileName()+"-copy");
}

void GdbTest::testBreakpointWithSpaceInPath()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeespace"));
    KConfigGroup grp = cfg.config();
    QString fileName = findSourceFile(QStringLiteral("debugee space.cpp"));

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 20);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 20);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakpointDisabledOnStart()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28)
        ->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 29);
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 31);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 29);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Checked);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 31);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testCatchpoint()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeexception"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile(QStringLiteral("debugeeexception.cpp"))), 29);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);
    TestFrameStackModel* fsModel = session->frameStackModel();
    QCOMPARE(fsModel->currentFrame(), 0);
    QCOMPARE(session->line(), 29);

    session->addCommand(MI::NonMI, QStringLiteral("catch throw"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QTest::qWait(1000);

    const QVector<KDevelop::FrameStackModel::FrameItem> frames = fsModel->frames(fsModel->currentThread());
    QVERIFY(frames.size() >= 2);
    // frame 0 is somewhere inside libstdc++
    QCOMPARE(frames[1].file, QUrl::fromLocalFile(findSourceFile("debugeeexception.cpp")));
    QCOMPARE(frames[1].line, 22);

    QCOMPARE(breakpoints()->rowCount(),2);
    QVERIFY(!breakpoints()->breakpoint(0)->location().isEmpty());
    QVERIFY(!breakpoints()->breakpoint(1)->location().isEmpty());

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testThreadAndFrameInfo()
{
    // Check if --thread is added to user commands

    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeethreads"));
    QString fileName = findSourceFile(QStringLiteral("debugeethreads.cpp"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 43); // QThread::msleep(600);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QSignalSpy outputSpy(session, &TestDebugSession::debuggerUserCommandOutput);

    session->addCommand(std::make_unique<MI::UserCommand>(MI::ThreadInfo, QString()));
    session->addCommand(std::make_unique<MI::UserCommand>(MI::StackListLocals, QStringLiteral("0")));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // wait for command finish

    // outputs should be
    // 1. -thread-info
    // 2. ^done for thread-info
    // 3. -stack-list-locals
    // 4. ^done for -stack-list-locals
    QCOMPARE(outputSpy.count(), 4);
    QVERIFY(outputSpy.at(2).at(0).toString().contains(QLatin1String("--thread 1")));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::parseBug304730()
{
    MI::FileSymbol file;
    file.contents = QByteArray("^done,bkpt={"
        "number=\"1\",type=\"breakpoint\",disp=\"keep\",enabled=\"y\",addr=\"<MULTIPLE>\",times=\"0\","
        "original-location=\"/media/portable/Projects/BDSInpainting/PatchMatch/PatchMatch.hpp:231\"},"
        "{number=\"1.1\",enabled=\"y\",addr=\"0x081d84aa\","
        "func=\"PatchMatch<itk::Image<itk::CovariantVector<unsigned char, 3u>, 2u> >"
        "::Propagation<ForwardPropagationNeighbors>(ForwardPropagationNeighbors)\","
        "file=\"/media/portable/Projects/BDSInpainting/Drivers/../PatchMatch/PatchMatch.hpp\","
        "fullname=\"/media/portable/Projects/BDSInpainting/PatchMatch/PatchMatch.hpp\",line=\"231\"},"
        "{number=\"1.2\",enabled=\"y\",addr=\"0x081d8ae2\","
        "func=\"PatchMatch<itk::Image<itk::CovariantVector<unsigned char, 3u>, 2u> >"
        "::Propagation<BackwardPropagationNeighbors>(BackwardPropagationNeighbors)\","
        "file=\"/media/portable/Projects/BDSInpainting/Drivers/../PatchMatch/PatchMatch.hpp\","
        "fullname=\"/media/portable/Projects/BDSInpainting/PatchMatch/PatchMatch.hpp\",line=\"231\"},"
        "{number=\"1.3\",enabled=\"y\",addr=\"0x081d911a\","
        "func=\"PatchMatch<itk::Image<itk::CovariantVector<unsigned char, 3u>, 2u> >"
        "::Propagation<AllowedPropagationNeighbors>(AllowedPropagationNeighbors)\","
        "file=\"/media/portable/Projects/BDSInpainting/Drivers/../PatchMatch/PatchMatch.hpp\","
        "fullname=\"/media/portable/Projects/BDSInpainting/PatchMatch/PatchMatch.hpp\",line=\"231\"}");

    MI::MIParser parser;

    std::unique_ptr<MI::Record> record(parser.parse(&file));
    QVERIFY(record.get() != nullptr);
}

void GdbTest::testMultipleLocationsBreakpoint()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeemultilocbreakpoint"));

    breakpoints()->addCodeBreakpoint(QStringLiteral("aPlusB"));

    //TODO check if the additional location breakpoint is added

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 19);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBug301287()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("argc"));
    QTest::qWait(300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //start second debug session (same cfg)
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QTest::qWait(300);

    i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testMultipleBreakpoint()
{
        auto *session = new TestDebugSession;

        //there'll be about 3-4 breakpoints, but we treat it like one.
        TestLaunchConfiguration c(QStringLiteral("debuggee_debugeemultiplebreakpoint"));
        KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QStringLiteral("debugeemultiplebreakpoint.cpp:52"));
        session->startDebugging(&c, m_iface);
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(breakpoints()->breakpoints().count(), 1);

        b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testRegularExpressionBreakpoint()
{
        auto *session = new TestDebugSession;

        TestLaunchConfiguration c(QStringLiteral("debuggee_debugeemultilocbreakpoint"));
        breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
        session->startDebugging(&c, m_iface);
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        session->addCommand(MI::NonMI, QStringLiteral("rbreak .*aPl.*B"));
        QTest::qWait(100);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(breakpoints()->breakpoints().count(), 3);

        session->addCommand(MI::BreakDelete, QString());
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testChangeBreakpointWhileRunning()
{
    KDevMI::Testing::testChangeBreakpointWhileRunning(new TestDebugSession, m_iface);
}

void GdbTest::testDebugInExternalTerminal()
{
    TestLaunchConfiguration cfg;

    const QStringList consoles { "konsole", "xterm", "xfce4-terminal", "gnome-terminal" };
    for (const QString& console : consoles) {

        TestDebugSession* session = nullptr;
        if (QStandardPaths::findExecutable(console).isEmpty()) {
            continue;
        }

        session = new TestDebugSession();

        cfg.config().writeEntry(IExecutePlugin::useTerminalEntry, true);
        cfg.config().writeEntry(IExecutePlugin::terminalEntry, console);

        KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);

        session->startDebugging(&cfg, m_iface);
        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
        QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);
        session->stepInto();
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
    }
}

// see: https://bugs.kde.org/show_bug.cgi?id=339231
void GdbTest::testPathWithSpace()
{
    auto* session = new TestDebugSession;

    auto debugee = findExecutable(QStringLiteral("path with space/debuggee_spacedebugee"));
    TestLaunchConfiguration c(debugee, KIO::upUrl(debugee));
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QStringLiteral("spacedebugee.cpp:30"));
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);
    session->startDebugging(&c, m_iface);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

} // end of namespace GDB
} // end of namespace KDevMI

QTEST_MAIN(KDevMI::GDB::GdbTest)


#include "test_gdb.moc"
#include "moc_test_gdb.cpp"

