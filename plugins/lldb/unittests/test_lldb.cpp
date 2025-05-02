/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_lldb.h"

#include "controllers/framestackmodel.h"
#include "debugsession.h"
#include "tests/debuggers-tests-config.h"
#include "tests/testhelper.h"

#include <execute/iexecuteplugin.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/variable/variablecollection.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>

#include <KConfigGroup>
#include <KIO/Global>
#include <KSharedConfig>

#include <QFileInfo>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QTemporaryFile>
#include <QUrl>
#include <QDir>

using namespace KDevelop;
using namespace KDevMI::LLDB;
using KDevMI::Testing::breakpoints;
using KDevMI::Testing::currentMiLine;
using KDevMI::Testing::findExecutable;
using KDevMI::Testing::findFile;
using KDevMI::Testing::findSourceFile;
using KDevMI::Testing::TestLaunchConfiguration;
using KDevMI::Testing::validateColumnCountsThreadCountAndStackFrameNumbers;

namespace {

class TestFrameStackModel : public LldbFrameStackModel
{
    Q_OBJECT

public:

    explicit TestFrameStackModel(DebugSession* session)
        : LldbFrameStackModel(session), fetchFramesCalled(0), fetchThreadsCalled(0) {}

    void fetchFrames(int threadNumber, int from, int to) override
    {
        fetchFramesCalled++;
        LldbFrameStackModel::fetchFrames(threadNumber, from, to);
    }

    void fetchThreads() override
    {
        fetchThreadsCalled++;
        LldbFrameStackModel::fetchThreads();
    }

    int fetchFramesCalled;
    int fetchThreadsCalled;
};

/// Check success with RETURN_IF_TEST_FAILED().
void verifyStackFrameCountNotLessThan(const QModelIndex& threadIndex, int expectedStackFrameCount)
{
    const auto* const stackModel = threadIndex.model();
    QVERIFY(stackModel);

    const auto stackFrameCount = stackModel->rowCount(threadIndex);
    qDebug() << "actual stack frame count:" << stackFrameCount;
    qDebug() << "minimum expected count:  " << expectedStackFrameCount;
    QVERIFY(stackFrameCount >= expectedStackFrameCount);

    // the optional third last frame name is something like "___lldb_unnamed_symbol3264";
    // the penultimate frame name is "__libc_start_main" or "__libc_start1";
    // the last frame name is "_start".
}

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    TestDebugSession() : DebugSession()
    {
        // explicit set formatter path to force use in-tree formatters, not the one installed in system.
        auto formatter = findFile(LLDB_SRC_DIR, "formatters/all.py");
        setFormatterPath(formatter);

        setSourceInitFile(false);
        m_frameStackModel = new TestFrameStackModel(this);

        KDevelop::ICore::self()->debugController()->addSession(this);
    }

    TestFrameStackModel* frameStackModel() const override
    {
        return m_frameStackModel;
    }

private:
    TestFrameStackModel* m_frameStackModel;
};

} // end of anonymous namespace

VariableCollection *LldbTest::variableCollection()
{
    return m_core->debugController()->variableCollection();
}

Variable *LldbTest::watchVariableAt(int i)
{
    auto watchRoot = variableCollection()->indexForItem(variableCollection()->watches(), 0);
    auto idx = variableCollection()->index(i, 0, watchRoot);
    return qobject_cast<Variable*>(variableCollection()->itemForIndex(idx));
}

QModelIndex LldbTest::localVariableIndexAt(int i, int col)
{
    auto localRoot = variableCollection()->indexForItem(variableCollection()->locals(), 0);
    return variableCollection()->index(i, col, localRoot);
}

// Called before the first testfunction is executed
void LldbTest::initTestCase()
{
    AutoTestShell::init();
    AutoTestShell::initializeNotifications();
    m_core = TestCore::initialize(Core::NoUi);

    m_iface = m_core->pluginController()
                ->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevexecute"))
                ->extension<IExecutePlugin>();
    Q_ASSERT(m_iface);

    m_debugeeFileName = findSourceFile("debugee.cpp");

    const QString lldbMiExecutable = QStandardPaths::findExecutable(QStringLiteral("lldb-mi"));
    if (lldbMiExecutable.isEmpty()) {
        QSKIP("Skipping, lldb-mi not available");
    }
}

// Called after the last testfunction was executed
void LldbTest::cleanupTestCase()
{
    TestCore::shutdown();
}

// Called before each testfunction is executed
void LldbTest::init()
{
    //remove all breakpoints - so we can set our own in the test
    KConfigGroup bpCfg = KSharedConfig::openConfig()->group("breakpoints");
    bpCfg.writeEntry("number", 0);
    bpCfg.sync();

    breakpoints()->removeRows(0, breakpoints()->rowCount());

    while (variableCollection()->watches()->childCount() > 0) {
        auto var = watchVariableAt(0);
        if (!var) break;
        var->die();
    }
}

void LldbTest::cleanup()
{
    // Called after every testfunction
}

void LldbTest::testStdout()
{
    auto *session = new TestDebugSession;

    QSignalSpy outputSpy(session, &TestDebugSession::inferiorStdoutLines);

    TestLaunchConfiguration cfg;
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    QVERIFY(outputSpy.count() > 0);

    QStringList outputLines;
    while (outputSpy.count() > 0) {
        const QList<QVariant> arguments = outputSpy.takeFirst();
        for (const auto &item : arguments) {
            outputLines.append(item.toStringList());
        }
    }
    QCOMPARE(outputLines, QStringList() << "Hello, world!"
                                        << "Hello");
}

void LldbTest::testEnvironmentSet()
{
    KDevMI::Testing::testEnvironmentSet(new TestDebugSession, QStringLiteral("LldbTestGroup"), m_iface);
}

void LldbTest::testUnsupportedUrlExpressionBreakpoints()
{
    KDevMI::Testing::testUnsupportedUrlExpressionBreakpoints(new TestDebugSession, m_iface, true);
}

void LldbTest::testBreakpoint()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 29);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);
    QCOMPARE(session->currentLine(), 29);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);
}

void LldbTest::testBreakOnStart()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(KDevMI::Config::BreakOnStartEntry, true);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    // line 28 is the start of main function in debugee.cpp
    QCOMPARE(session->currentLine(), 27); // currentLine is zero-based

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testDisableBreakpoint()
{
    //Description: We must stop only on the third breakpoint

    int firstBreakLine=28;
    int secondBreakLine=23;
    int thirdBreakLine=24;
    int fourthBreakLine=31;

    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b;

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), firstBreakLine);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);


    //this is needed to emulate debug from GUI. If we are in edit mode, the debugSession doesn't exist.
    m_core->debugController()->breakpointModel()->blockSignals(true);
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), secondBreakLine);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    //all disabled breakpoints were added

    auto *thirdBreak = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName),
                                                        thirdBreakLine);
    m_core->debugController()->breakpointModel()->blockSignals(false);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), thirdBreak->line());

    //disable existing breakpoint
    thirdBreak->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    //add another disabled breakpoint
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), fourthBreakLine);
    WAIT_FOR_A_WHILE(session, 300);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    WAIT_FOR_A_WHILE(session, 300);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testChangeLocationBreakpoint()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    auto *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 27);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 27);

    WAIT_FOR_A_WHILE(session, 100);
    b->setLine(28);
    WAIT_FOR_A_WHILE(session, 100);
    session->run();

    WAIT_FOR_A_WHILE(session, 100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 28);
    WAIT_FOR_A_WHILE(session, 500);
    breakpoints()->setData(breakpoints()->index(0, KDevelop::Breakpoint::LocationColumn), QString(m_debugeeFileName+":30"));
    QCOMPARE(b->line(), 29);
    WAIT_FOR_A_WHILE(session, 100);
    QCOMPARE(b->line(), 29);
    session->run();
    WAIT_FOR_A_WHILE(session, 100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 29);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testDeleteBreakpoint()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    QCOMPARE(breakpoints()->rowCount(), 0);
    //add breakpoint before startDebugging
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 21);
    QCOMPARE(breakpoints()->rowCount(), 1);
    QVERIFY(breakpoints()->removeRow(0));
    QCOMPARE(breakpoints()->rowCount(), 0);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 22);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QVERIFY(breakpoints()->removeRow(0));
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testPendingBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 28);

    auto* const b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("debugeeqt.cpp")), 10);
    QCOMPARE(b->state(), Breakpoint::NotStartedState);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(b->state(), Breakpoint::PendingState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakpointsOnNoOpLines()
{
    KDevMI::Testing::testBreakpointsOnNoOpLines(new TestDebugSession, m_iface);
}

void LldbTest::testUpdateBreakpoint()
{
    // Description: user might insert breakpoints using lldb console. model should
    // pick up the manually set breakpoint
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    // breakpoint 1: real line 29: foo();
    auto b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 28);
    QCOMPARE(breakpoints()->rowCount(), 1);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(currentMiLine(session), 29);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // stop after step

    QCOMPARE(session->currentLine(), 23-1); // at the beginning of foo():23: ++i;

    session->addUserCommand(QStringLiteral("break set --file %1 --line %2").arg(m_debugeeFileName).arg(33));
    WAIT_FOR_A_WHILE(session, 20);

    QCOMPARE(breakpoints()->rowCount(), 2);
    b = breakpoints()->breakpoint(1);
    QCOMPARE(b->url(), QUrl::fromLocalFile(m_debugeeFileName));
    QCOMPARE(b->line(), 33-1);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 33-1);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testIgnoreHitsBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't provide breakpoint hit count update");
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 21);
    b1->setIgnoreHits(1);

    KDevelop::Breakpoint * b2 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 22);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    //WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR(session, session->state() == DebugSession::PausedState && b2->hitCount() == 1);
    b2->setIgnoreHits(1);
    session->run();
    //WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR(session, session->state() == DebugSession::PausedState && b1->hitCount() == 1);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testConditionBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    auto b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 39);
    b->setCondition(QStringLiteral("x[0] == 'H'"));

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 23);
    b->setCondition(QStringLiteral("i==2"));

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR(session, session->state() == DebugSession::PausedState && session->currentLine() == 24);
    b->setCondition(QStringLiteral("i == 0"));
    WAIT_FOR_A_WHILE(session, 100);
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 23);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 39);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnWriteBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 24);

    breakpoints()->addWatchpoint(QStringLiteral("i"));
    WAIT_FOR_A_WHILE(session, 100);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22); // line 23: ++i; int j = i;
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnWriteWithConditionBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 24);

    KDevelop::Breakpoint *b = breakpoints()->addWatchpoint(QStringLiteral("i"));
    b->setCondition(QStringLiteral("i==2"));
    QTest::qWait(100);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22); // line 23: ++i; int j = i;
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnReadBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addReadWatchpoint(QStringLiteral("foo::i"));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    for (int fooCall = 0; fooCall < 2; ++fooCall) {
        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
        QCOMPARE(session->currentLine(), 22); // ++i;
        session->run();

        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
        QCOMPARE(session->currentLine(), 22); // int j = i;
        session->run();
    }

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnReadBreakpoint2()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 24);

    breakpoints()->addReadWatchpoint(QStringLiteral("i"));

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22); // ++i

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22); // int j = i

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnAccessBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 24);

    breakpoints()->addAccessWatchpoint(QStringLiteral("i"));

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22); // line 23: ++i (read)

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22); // line 23: ++i (write)

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22); // line 23: int j = i (read)

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakpointErrors()
{
    KDevMI::Testing::testBreakpointErrors(new TestDebugSession, m_iface, true);
}

void LldbTest::testInsertBreakpointWhileRunning()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeslow"));
    QString fileName = findSourceFile("debugeeslow.cpp");

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 2000);

    qDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 29); // ++i;
    WAIT_FOR_A_WHILE(session, 500);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 500);

    QCOMPARE(session->currentLine(), 29); // ++i;
    breakpoints()->removeBreakpoint(b);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testInsertBreakpointWhileRunningMultiple()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeslow"));
    QString fileName = findSourceFile("debugeeslow.cpp");

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 2000);

    qDebug() << "adding breakpoint";
    auto b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 29); // ++i;
    auto b2 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 30); // std::cout << i << std::endl;

    WAIT_FOR_A_WHILE(session, 500);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(session->currentLine(), 29);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(session->currentLine(), 30);
    breakpoints()->removeBreakpoint(b1);
    breakpoints()->removeBreakpoint(b2);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testInsertBreakpointFunctionName()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 27);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testManualBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 27);

    breakpoints()->removeRows(0, 1);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->addCommand(MI::NonMI, QStringLiteral("break set --file debugee.cpp --line 23"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 1);

    auto b = breakpoints()->breakpoint(0);
    QCOMPARE(b->line(), 22);

    session->addCommand(MI::NonMI, QStringLiteral("break disable 2"));
    session->addCommand(MI::NonMI, QStringLiteral("break modify -c 'i == 1' 2"));
    session->addCommand(MI::NonMI, QStringLiteral("break modify -i 1 2"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(b->enabled(), false);
    QEXPECT_FAIL("", "LLDB 4.0 does not report condition in mi response", Continue);
    QCOMPARE(b->condition(), QString("i == 1"));
    QEXPECT_FAIL("", "LLDB 4.0 does not report ignore hits in mi response", Continue);
    QCOMPARE(b->ignoreHits(), 1);

    session->addCommand(MI::NonMI, QStringLiteral("break delete 2"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QEXPECT_FAIL("", "LLDB 4.0 does not report breakpoint deletion as mi notification", Continue);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

//Bug 201771
void LldbTest::testInsertAndRemoveBreakpointWhileRunning()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeslow"));

    QString fileName = findSourceFile("debugeeslow.cpp");

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 2000);

    qDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 30); // std::cout << i << std::endl;
    breakpoints()->removeBreakpoint(b);

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testPickupManuallyInsertedBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    session->addCommand(MI::NonMI, QStringLiteral("break set --file debugee.cpp --line 32"));
    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->breakpoints().count(), 2);
    QCOMPARE(breakpoints()->rowCount(), 2);
    KDevelop::Breakpoint *b = breakpoints()->breakpoint(1);
    QVERIFY(b);
    QCOMPARE(b->line(), 31); //we start with 0, gdb with 1
    QCOMPARE(b->url().fileName(), QString("debugee.cpp"));
}

//Bug 270970
void LldbTest::testPickupManuallyInsertedBreakpointOnlyOnce()
{
    auto *session = new TestDebugSession;

    QString sourceFile = findSourceFile("debugee.cpp");
    //inject here, so it behaves similar like a command from .lldbinit
    QTemporaryFile configScript;
    configScript.open();
    configScript.write(QStringLiteral("break set --file %0 --line 32\n").arg(sourceFile).toLocal8Bit());
    configScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::LldbConfigScriptEntry, QUrl::fromLocalFile(configScript.fileName()));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(sourceFile), 31);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QCOMPARE(breakpoints()->breakpoints().count(), 1);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakpointWithSpaceInPath()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeespace"));

    KConfigGroup grp = cfg.config();
    QString fileName = findSourceFile("debugee space.cpp");

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 20);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QCOMPARE(session->currentLine(), 20);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakpointDisabledOnStart()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    auto b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 23);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 29);
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 34); // testStruct ts;
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QCOMPARE(session->currentLine(), 29);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Checked);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 35); // ts.a = 0; (LLDB automatically moves the breakpoint from its no-op line)

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testMultipleLocationsBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeemultilocbreakpoint"));

    breakpoints()->addCodeBreakpoint(QStringLiteral("aPlusB"));

    //TODO check if the additional location breakpoint is added

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 19);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 23);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testMultipleBreakpoint()
{
    auto *session = new TestDebugSession;

    //there'll be about 3-4 breakpoints, but we treat it like one.
    TestLaunchConfiguration c(QStringLiteral("debuggee_debugeemultiplebreakpoint"));
    auto b = breakpoints()->addCodeBreakpoint(QStringLiteral("debugeemultiplebreakpoint.cpp:52"));

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &c, m_iface);
    QCOMPARE(breakpoints()->breakpoints().count(), 1);

    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testRegularExpressionBreakpoint()
{
    QSKIP("Skipping... lldb has only one breakpoint for multiple locations"
          " (and lldb-mi returns the first one), not support this yet");
    auto *session = new TestDebugSession;
    TestLaunchConfiguration c(QStringLiteral("debuggee_debugeemultilocbreakpoint"));

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &c, m_iface);

    session->addCommand(MI::NonMI, QStringLiteral("break set --func-regex .*aPl.*B"));
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(breakpoints()->breakpoints().count(), 3);

    session->addCommand(MI::BreakDelete, QString());
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testChangeBreakpointWhileRunning()
{
    KDevMI::Testing::testChangeBreakpointWhileRunning(new TestDebugSession, m_iface);
}

void LldbTest::testCatchpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeexception"));

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestFrameStackModel* fsModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("debugeeexception.cpp")), 29);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QCOMPARE(fsModel->currentFrame(), 0);
    QCOMPARE(session->currentLine(), 29);

    session->addCommand(MI::NonMI, QStringLiteral("break set -E c++"));
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    const auto frames = fsModel->frames(fsModel->currentThread());
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

void LldbTest::testShowStepInSource()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    QSignalSpy showStepInSourceSpy(session, &TestDebugSession::showStepInSource);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 29);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    {
        QCOMPARE(showStepInSourceSpy.count(), 3);
        QList<QVariant> arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().toUrl(), QUrl::fromLocalFile(m_debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 29);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().toUrl(), QUrl::fromLocalFile(m_debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 22);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().toUrl(), QUrl::fromLocalFile(m_debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 23);
    }
}

void LldbTest::testStack()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 21);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QModelIndex tIdx = stackModel->index(0,0);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at foo()");
    verifyStackFrameCountNotLessThan(tIdx, 2);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "foo()");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), m_debugeeFileName+":23");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(1, 2, tIdx), m_debugeeFileName+":29");

    session->stepOut();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at main");
    verifyStackFrameCountNotLessThan(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), m_debugeeFileName+":30");

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testStackFetchMore()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeerecursion"));
    QString fileName = findSourceFile("debugeerecursion.cpp");

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->frameStackModel()->fetchFramesCalled, 1);

    QModelIndex tIdx = stackModel->index(0,0);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at foo()");
    QCOMPARE(stackModel->rowCount(tIdx), 21);
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "foo()");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), fileName+":26");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "foo()");
    COMPARE_DATA(stackModel->index(1, 2, tIdx), fileName+":24");
    COMPARE_DATA(stackModel->index(2, 1, tIdx), "foo()");
    COMPARE_DATA(stackModel->index(2, 2, tIdx), fileName+":24");

    stackModel->fetchMoreFrames();
    WAIT_FOR_A_WHILE(session, 200);
    QCOMPARE(stackModel->fetchFramesCalled, 2);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    QCOMPARE(stackModel->rowCount(tIdx), 41);

    stackModel->fetchMoreFrames();
    WAIT_FOR_A_WHILE(session, 200);
    QCOMPARE(stackModel->fetchFramesCalled, 3);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    QCOMPARE(stackModel->rowCount(tIdx), 121);

    stackModel->fetchMoreFrames();
    WAIT_FOR_A_WHILE(session, 200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    verifyStackFrameCountNotLessThan(tIdx, 299);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(stackModel->index(298, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(298, 2, tIdx), fileName+":30");

    for (int counter = 0; counter < 2; ++counter) {
        stackModel->fetchMoreFrames(); // possibly nothing more to fetch if we are at the end
        WAIT_FOR_A_WHILE(session, 200);
        qDebug() << "fetchFrames() was called" << stackModel->fetchFramesCalled << "times.";
        QVERIFY(stackModel->fetchFramesCalled >= 4);
        QVERIFY(stackModel->fetchFramesCalled <= 5);

        validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 1);
        RETURN_IF_TEST_FAILED();
        verifyStackFrameCountNotLessThan(tIdx, 299);
        RETURN_IF_TEST_FAILED();
    }

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testStackSwitchThread()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeethreads"));
    QString fileName = findSourceFile("debugeethreads.cpp");

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 43); // QThread::msleep(600);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QModelIndex tIdx = stackModel->index(0,0);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 4);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(tIdx, "#1 at main");
    verifyStackFrameCountNotLessThan(tIdx, 1);
    RETURN_IF_TEST_FAILED();
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), fileName+":44"); // QThread::msleep(600);

    tIdx = stackModel->index(1,0);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 4);
    RETURN_IF_TEST_FAILED();
    QVERIFY(stackModel->data(tIdx).toString().startsWith("#2 at "));

    stackModel->setCurrentThread(2);
    WAIT_FOR_A_WHILE(session, 200);

    validateColumnCountsThreadCountAndStackFrameNumbers(tIdx, 4);
    RETURN_IF_TEST_FAILED();
    verifyStackFrameCountNotLessThan(tIdx, 4);
    RETURN_IF_TEST_FAILED();

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testAttach()
{
    SKIP_IF_ATTACH_FORBIDDEN();

    QString fileName = findSourceFile("debugeeslow.cpp");

    KProcess debugeeProcess;
    debugeeProcess << QStringLiteral("nice") << findExecutable(QStringLiteral("debuggee_debugeeslow")).toLocalFile();
    debugeeProcess.start();
    QVERIFY(debugeeProcess.waitForStarted());
    QTest::qWait(100);

    auto *session = new TestDebugSession;
    session->attachToProcess(debugeeProcess.processId());

    WAIT_FOR_A_WHILE(session, 100);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 39); // the blank line in main()

    // lldb-mi silently stops when attaching to a process. Force it continue to run.
    session->addCommand(MI::ExecContinue, QString(), MI::CmdMaybeStartsRunning);
    WAIT_FOR_A_WHILE(session, 2000);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 40); // return 0; (LLDB automatically moves the breakpoint from its no-op line)

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testRemoteDebugging()
{
    KProcess gdbServer;
    gdbServer << QStringLiteral("lldb-server") << QStringLiteral("gdbserver") << QStringLiteral("*:1234");
    gdbServer.start();
    QVERIFY(gdbServer.waitForStarted());

    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    cfg.config().writeEntry(Config::LldbRemoteDebuggingEntry, true);
    cfg.config().writeEntry(Config::LldbRemoteServerEntry, "localhost:1234");
    cfg.config().writeEntry(Config::LldbRemotePathEntry, "/tmp");

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 34);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testCoreFile()
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
        qDebug() << "try to use coredumpctl";
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
    COMPARE_DATA(tIdx, "#1 at foo()");

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesLocals()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QCOMPARE(variableCollection()->rowCount(), 2);
    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");

    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    COMPARE_DATA(variableCollection()->index(0, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "2");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesLocalsStruct()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    WAIT_FOR_A_WHILE(session, 1000);

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
    WAIT_FOR_A_WHILE(session, 100);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 1000);
    COMPARE_DATA(variableCollection()->index(structIndex, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(structIndex, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesWatches()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    m_core->debugController()->variableCollection()->variableWidgetShown();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    WAIT_FOR_A_WHILE(session, 300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    QModelIndex ts = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "...");
    variableCollection()->expanded(ts);
    WAIT_FOR_A_WHILE(session, 100);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 100);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesWatchesQuotes()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    // the unquoted string (the actual content):               t\"t
    // quoted string (what we would write as a c string):     "t\\\"t"
    // written in source file:                             R"("t\\\"t")"
    const QString testString(QStringLiteral("t\\\"t")); // the actual content
    const QString quotedTestString(QStringLiteral(R"("t\\\"t")"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    variableCollection()->watches()->add(quotedTestString); //just a constant string
    WAIT_FOR_A_WHILE(session, 3000);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), quotedTestString);
    COMPARE_DATA(variableCollection()->index(0, 1, i), quotedTestString);

    QModelIndex testStr = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, testStr), "...");
    variableCollection()->expanded(testStr);
    WAIT_FOR_A_WHILE(session, 100);
    int len = testString.length();
    for (int ind = 0; ind < len; ind++)
    {
        COMPARE_DATA(variableCollection()->index(ind, 0, testStr), QStringLiteral("[%0]").arg(ind));
        QChar c = testString.at(ind);
        QString value = QString::number(c.toLatin1()) + " '" + c + "'";
        COMPARE_DATA(variableCollection()->index(ind, 1, testStr), value);
    }
    COMPARE_DATA(variableCollection()->index(len, 0, testStr), QStringLiteral("[%0]").arg(len));
    COMPARE_DATA(variableCollection()->index(len, 1, testStr), "0 '\\0'");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesWatchesTwoSessions()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    WAIT_FOR_A_WHILE(session, 300);

    QModelIndex ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    variableCollection()->expanded(ts);
    WAIT_FOR_A_WHILE(session, 100);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //check if variable is marked as out-of-scope
    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    auto v = qobject_cast<LldbVariable*>(watchVariableAt(0));
    QVERIFY(v);
    QVERIFY(!v->inScope());
    QCOMPARE(v->childCount(), 3);
    v = qobject_cast<LldbVariable*>(v->child(0));
    QVERIFY(!v->inScope());

    //start a second debug session
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    v = qobject_cast<LldbVariable*>(watchVariableAt(0));
    QVERIFY(v);
    QVERIFY(v->inScope());
    QCOMPARE(v->childCount(), 3);
    v = qobject_cast<LldbVariable*>(v->child(0));
    QVERIFY(v->inScope());
    COMPARE_DATA(variableCollection()->indexForItem(v, 1), QString::number(0));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //check if variable is marked as out-of-scope
    v = qobject_cast<LldbVariable*>(watchVariableAt(0));
    QVERIFY(!v->inScope());
    QVERIFY(!qobject_cast<KDevelop::Variable*>(v->child(0))->inScope());
}

void LldbTest::testVariablesStopDebugger()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesStartSecondSession()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesSwitchFrame()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "j"); // only non-static variable works
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 200);

    i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);
    COMPARE_DATA(variableCollection()->index(2, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(2, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(3, 0, i), "argv");

    breakpoints()->removeRow(0);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesQuicklySwitchFrame()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "j"); // only non-static variable works
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 300);
    stackModel->setCurrentFrame(0);
    WAIT_FOR_A_WHILE(session, 1);
    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 1);
    stackModel->setCurrentFrame(0);
    WAIT_FOR_A_WHILE(session, 1);
    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 500);

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

void LldbTest::testSwitchFrameLldbConsole()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QCOMPARE(stackModel->currentFrame(), 0);
    stackModel->setCurrentFrame(1);
    QCOMPARE(stackModel->currentFrame(), 1);
    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(stackModel->currentFrame(), 1);

    session->addUserCommand(QStringLiteral("print i"));
    WAIT_FOR_A_WHILE(session, 500);
    //currentFrame must not reset to 0; Bug 222882
    QCOMPARE(stackModel->currentFrame(), 1);
}

void LldbTest::testSegfaultDebugee()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_crash"));

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    QString fileName = findSourceFile("debugeecrash.cpp");
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 23);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
    QCOMPARE(session->currentLine(), 23);
    session->run();

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

//Bug 274390
void LldbTest::testCommandOrderFastStepping()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeqt"));

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    QVERIFY(session->startDebugging(&cfg, m_iface));
    for(int i=0; i<20; i++) {
        session->stepInto();
    }
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testRunLldbScript()
{
    auto *session = new TestDebugSession;

    QTemporaryFile runScript;
    runScript.open();

    runScript.write(QStringLiteral("break set --file %1 --line 35\n").arg(findSourceFile("debugee.cpp")).toUtf8());
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::LldbConfigScriptEntry, QUrl::fromLocalFile(runScript.fileName()));

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    QCOMPARE(session->currentLine(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBug301287()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 28);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    variableCollection()->watches()->add(QStringLiteral("argc"));
    WAIT_FOR_A_WHILE(session, 300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //start second debug session (same cfg)
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);

    i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testDebugInExternalTerminal()
{
    TestLaunchConfiguration cfg;

    const QStringList consoles { "konsole", "xterm", "xfce4-terminal", "gnome-terminal" };
    for (const QString& console : consoles) {

        if (QStandardPaths::findExecutable(console).isEmpty()) {
            continue;
        }

        auto* session = new TestDebugSession();

        cfg.config().writeEntry(IExecutePlugin::useTerminalEntry, true);
        cfg.config().writeEntry(IExecutePlugin::terminalEntry, console);

        KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 28);

        START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, m_iface);
        QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);

        session->stepInto();
        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
    }
}

void LldbTest::testSpecialPath()
{
    auto* session = new TestDebugSession;

    auto debugee = findExecutable(QStringLiteral("path with space/debuggee_spacedebugee"));
    TestLaunchConfiguration c(debugee, KIO::upUrl(debugee));
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QStringLiteral("spacedebugee.cpp:30"));
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &c, m_iface);
    QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void KDevMI::LLDB::LldbTest::testEnvironmentCd()
{
    auto *session = new TestDebugSession;

    QSignalSpy outputSpy(session, &TestDebugSession::inferiorStdoutLines);

    auto path = KIO::upUrl(findExecutable(QStringLiteral("path with space/debuggee_spacedebugee")));
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeepath"), path);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    QVERIFY(outputSpy.count() > 0);

    QStringList outputLines;
    while (outputSpy.count() > 0) {
        const QList<QVariant> arguments = outputSpy.takeFirst();
        for (const auto &item : arguments) {
            outputLines.append(item.toStringList());
        }
    }
    QCOMPARE(outputLines, QStringList() << path.toLocalFile());
}

QTEST_MAIN(KDevMI::LLDB::LldbTest)

#include "test_lldb.moc"
#include "moc_test_lldb.cpp"
