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
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <tests/testhelpers.h>

#include <KConfigGroup>

#include <QString>
#include <QStringList>
#include <QTest>
#include <QUrl>
#include <QDir>

using namespace KDevelop;
using namespace KDevMI::LLDB;
using KDevMI::Testing::ActiveStateSessionSpy;
using KDevMI::Testing::currentMiLine;
using KDevMI::Testing::findExecutable;
using KDevMI::Testing::findFile;
using KDevMI::Testing::findSourceFile;
using KDevMI::Testing::TestLaunchConfiguration;

namespace {

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    using TestFrameStackModel = KDevMI::Testing::TestFrameStackModel<DebugSession, LldbFrameStackModel>;

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

KDevMI::MIDebugSession* LldbTest::createTestDebugSession()
{
    return new TestDebugSession;
}

const char* LldbTest::configScriptEntryKey() const
{
    return Config::LldbConfigScriptEntry;
}

const char* LldbTest::runScriptEntryKey() const
{
    return Config::LldbConfigScriptEntry;
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

bool LldbTest::isLldb() const
{
    return true;
}

void LldbTest::startInitTestCase()
{
    const QString lldbMiExecutable = QStandardPaths::findExecutable(QStringLiteral("lldb-mi"));
    if (lldbMiExecutable.isEmpty()) {
        QSKIP("Skipping, lldb-mi not available");
    }
}

void LldbTest::finishInit()
{
    while (variableCollection()->watches()->childCount() > 0) {
        auto var = watchVariableAt(0);
        if (!var) break;
        var->die();
    }
}

void LldbTest::testBreakOnStart()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(KDevMI::Config::BreakOnStartEntry, true);

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    // line 28 is the start of main function in debugee.cpp
    QCOMPARE(session->currentLine(), 27); // currentLine is zero-based

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testUpdateBreakpoint()
{
    // Description: user might insert breakpoints using lldb console. model should
    // pick up the manually set breakpoint
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    // breakpoint 1: real line 29: foo();
    auto* b = breakpoints()->addCodeBreakpoint(debugeeUrl(), 28);
    QCOMPARE(breakpoints()->rowCount(), 1);

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 29);

    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(session->currentLine(), 23-1); // at the beginning of foo():23: ++i;

    session->addUserCommand(QStringLiteral("break set --file %1 --line %2").arg(debugeeFilePath()).arg(33));
    QTRY_COMPARE(breakpoints()->rowCount(), 2);

    b = breakpoints()->breakpoint(1);
    QCOMPARE(b->url(), debugeeUrl());
    QCOMPARE(b->line(), 33-1);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(session->currentLine(), 33-1);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testManualBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(session->currentLine(), 27);

    breakpoints()->removeRows(0, 1);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->addCommand(MI::NonMI, QStringLiteral("break set --file debugee.cpp --line 23"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QTRY_COMPARE(breakpoints()->rowCount(), 1);

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

void LldbTest::testBreakpointDisabledOnStart()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    auto* b = breakpoints()->addCodeBreakpoint(debugeeUrl(), 23);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    breakpoints()->addCodeBreakpoint(debugeeUrl(), 29);
    b = breakpoints()->addCodeBreakpoint(debugeeUrl(), 34); // testStruct ts;
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    QCOMPARE(session->currentLine(), 29);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Checked);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(session->currentLine(), 35); // ts.a = 0; (LLDB automatically moves the breakpoint from its no-op line)

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

#ifdef Q_OS_FREEBSD
    QSKIP(
        "The test freezes while waiting for the ended state; skipping after session->run() "
        "makes the next test function testRemoteDebugging() freeze, so skip now");
#endif
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

    breakpoints()->addCodeBreakpoint(debugeeUrl(), 34);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesLocals()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(debugeeUrl(), 24);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    QCOMPARE(variableCollection()->rowCount(), 2);
    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");

    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    COMPARE_DATA(variableCollection()->index(0, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "2");

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

    breakpoints()->addCodeBreakpoint(debugeeUrl(), 38);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    variableCollection()->watches()->add(quotedTestString); //just a constant string
    WAIT_FOR_A_WHILE(session, 3000);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), quotedTestString);
    COMPARE_DATA(variableCollection()->index(0, 1, i), quotedTestString);

    QModelIndex testStr = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, testStr), "...");

    EXPAND_VARIABLE_COLLECTION(testStr);
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

    breakpoints()->addCodeBreakpoint(debugeeUrl(), 38);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    WAIT_FOR_A_WHILE(session, 300);

    QModelIndex ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    EXPAND_VARIABLE_COLLECTION(ts);

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

    const ActiveStateSessionSpy sessionSpy2(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy2);

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

QTEST_MAIN(KDevMI::LLDB::LldbTest)

#include "test_lldb.moc"
#include "moc_test_lldb.cpp"
