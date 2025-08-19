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

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/interfaces/ivariablecontroller.h>
#include <debugger/variable/variablecollection.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <tests/temporaryfilehelpers.h>
#include <tests/testhelpers.h>

#include <KProcess>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QList>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryFile>

using KDevMI::Testing::currentMiLine;
using KDevMI::Testing::findExecutable;
using KDevMI::Testing::findFile;
using KDevMI::Testing::findSourceFile;
using KDevMI::Testing::TestLaunchConfiguration;

namespace KDevMI { namespace GDB {

void GdbTest::startInitTestCase()
{
#ifdef Q_OS_WIN
    QSKIP("apparently this test is killing processes, but is leaving kdeinit5, dbus-daemon and klauncher running, "
          "breaking the Windows CI builder");
#endif
}

void GdbTest::finishInit()
{
    KDevelop::VariableCollection *vc = KDevelop::ICore::self()->debugController()->variableCollection();
    for (int i=0; i < vc->watches()->childCount(); ++i) {
        delete vc->watches()->child(i);
    }
    vc->watches()->clear();
}

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    using TestFrameStackModel = KDevMI::Testing::TestFrameStackModel<DebugSession, GdbFrameStackModel>;

    TestDebugSession() : DebugSession()
    {
        setSourceInitFile(false);
        setAutoDisableASLR(false);
        m_frameStackModel = new TestFrameStackModel(this);
        KDevelop::ICore::self()->debugController()->addSession(this);
    }

    int line() { return currentLine(); }

    TestFrameStackModel* frameStackModel() const override
    {
        return m_frameStackModel;
    }

private:
    TestFrameStackModel* m_frameStackModel;
};

MIDebugSession* GdbTest::createTestDebugSession()
{
    return new TestDebugSession;
}

const char* GdbTest::configScriptEntryKey() const
{
    return Config::RemoteGdbConfigEntry;
}

const char* GdbTest::runScriptEntryKey() const
{
    return Config::RemoteGdbRunEntry;
}

bool GdbTest::isLldb() const
{
    return false;
}

void GdbTest::testUpdateBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    // breakpoint 1: real line 29: foo();
    auto* b = breakpoints()->addCodeBreakpoint(debugeeUrl(), 28);
    QCOMPARE(breakpoints()->rowCount(), 1);

    START_DEBUGGING_E(session, cfg);

    // breakpoint 2: real line 32: const char *x = "Hello";
    //insert custom command as user might do it using GDB console
    session->addCommand(std::make_unique<MI::UserCommand>(MI::NonMI, "break " + debugeeLocationAt(32)));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // stop at breakpoint 1, with custom command handled
    QCOMPARE(session->currentLine(), 28);

    // check breakpoint 2 got picked up
    QCOMPARE(breakpoints()->rowCount(), 2);
    b = breakpoints()->breakpoint(1);
    QCOMPARE(b->url(), debugeeUrl());
    QCOMPARE(b->line(), 31);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState); // stop at breakpoint 2
    QCOMPARE(session->currentLine(), 31);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testManualBreakpoint()
{
    auto *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    START_DEBUGGING_E(session, cfg);
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
    WAIT_FOR_A_WHILE(session, 100);
    session->run();
    WAIT_FOR_A_WHILE(session, 2000);
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
    START_DEBUGGING_E(session, cfg);

    session->addCommand(MI::NonMI, QStringLiteral("attach %0").arg(debugeeProcess.processId()));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->run();
#ifdef Q_OS_FREEBSD
    QSKIP("The test freezes while waiting for the ended state, so skip it");
#endif
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesLocals()
{
    auto *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeUrl(), 22);
    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(variableCollection()->rowCount(), 2);
    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 2);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");
    // COMPARE_DATA(variableCollection()->index(1, 1, i), "1"); // j is not initialized yet
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(1, 1, i), "1");
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

    breakpoints()->addCodeBreakpoint(debugeeUrl(), 38);
    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(quotedTestString); //just a constant string
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), quotedTestString);
    COMPARE_DATA(variableCollection()->index(0, 1, i), "[" + QString::number(testString.length() + 1) + "]");

    QModelIndex testStr = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, testStr), "...");

    EXPAND_VARIABLE_COLLECTION(testStr);
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

    breakpoints()->addCodeBreakpoint(debugeeUrl(), 38);
    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    EXPAND_VARIABLE_COLLECTION(ts);

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
    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

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

void GdbTest::testPickupCatchThrowOnlyOnce()
{
    QTemporaryFile configScript;
    OPEN_WRITE_AND_CLOSE_TEMPORARY_FILE(configScript, u"catch throw\n");

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(Config::RemoteGdbConfigEntry, QUrl::fromLocalFile(configScript.fileName()));

    for (int i = 0; i < 2; ++i) {
        auto* session = new TestDebugSession;
        START_DEBUGGING_E(session, cfg);
        WAIT_FOR_STATE(session, DebugSession::EndedState);

        // During the second iteration there are two equivalent breakpoints:
        // 1) from KDevelop's breakpoint model (left over from the first session);
        // 2) from the config script.
        // Verify that KDevelop removes the duplicate and retains a single breakpoint.
        QCOMPARE(breakpoints()->rowCount(), 1);
    }
}

void GdbTest::testRemoteDebug_data()
{
    QTest::addColumn<QList<int>>("linesToAddBreakpointsAt");
    QTest::addColumn<bool>("startSecondSession");

    QTest::newRow("no-custom-breakpoints") << QList<int>{} << false;
    QTest::newRow("breakpoints-at-same-and-another-line") << QList<int>{30, 36} << false;
    QTest::newRow("breakpoint-at-another-line-start-second-session") << QList<int>{36} << true;
}

void GdbTest::testRemoteDebug()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable(QStringLiteral("gdbserver"));
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available");
    }

    auto *session = new TestDebugSession;

    QFETCH(const QList<int>, linesToAddBreakpointsAt);
    for (const auto line : linesToAddBreakpointsAt) {
        addDebugeeBreakpoint(line);
    }

    // In case the shell script QTemporaryFile object is alive during debugging:
    // 1) the sh process started in DebugSession::execInferior() exits immediately with the code 126
    //    (which means "A specified command_file could not be executed due to an [ENOEXEC]  error");
    // 2) the GDB command `source path/to/runscript` fails with the following error message:
    //    "/path/to/runscript:2: Error in sourced command file:
    //     could not connect: Connection timed out.";
    // 3) and the debug session is stuck in the starting state, so the test
    //    fails due to a timeout while waiting for the first paused state.
    // These errors likely occur because the QTemporaryFile object always keeps
    // the unique temporary file open internally (according to its documentation).
    // Prevent the errors by destroying the QTemporaryFile object before starting debugging.
    QString shellScriptFileName;
    {
        QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
        OPEN_WRITE_AND_CLOSE_TEMPORARY_FILE(
            shellScript,
            QLatin1String{"gdbserver localhost:2345 %1\n"}.arg(findExecutable("debuggee_debugee").toLocalFile()));
        QVERIFY(shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser));

        shellScriptFileName = shellScript.fileName();
        shellScript.setAutoRemove(false);
    }
    QVERIFY(!shellScriptFileName.isEmpty());
    const QScopeGuard shellScriptGuard([&shellScriptFileName] {
        QFile::remove(shellScriptFileName);
    });

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    const auto runScriptContents = QLatin1String{R"(file %1
target remote localhost:2345
break debugee.cpp:30
continue
)"}
                                       .arg(findExecutable("debuggee_debugee").toLocalFile());
    OPEN_WRITE_AND_CLOSE_TEMPORARY_FILE(runScript, runScriptContents);

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbShellEntry, QUrl::fromLocalFile(shellScriptFileName));
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(currentMiLine(session), 30);

    // the run script adds one breakpoint
    const auto expectedBreakpointCount = linesToAddBreakpointsAt.empty() ? 1 : 2;
    QCOMPARE(breakpoints()->breakpoints().size(), expectedBreakpointCount);

    if (!linesToAddBreakpointsAt.empty()) {
        session->run();
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(currentMiLine(session), 36);
    }

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QFETCH(const bool, startSecondSession);
    if (startSecondSession) {
        session = new TestDebugSession;
        START_DEBUGGING_E(session, cfg);
        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
        QCOMPARE(currentMiLine(session), 30);

        QCOMPARE(breakpoints()->breakpoints().size(), expectedBreakpointCount);

        if (!linesToAddBreakpointsAt.empty()) {
            session->run();
            WAIT_FOR_STATE(session, DebugSession::PausedState);
            QCOMPARE(currentMiLine(session), 36);
        }

        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
    }
}

void GdbTest::testBreakpointDisabledOnStart()
{
    auto *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(debugeeUrl(), 28)->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    breakpoints()->addCodeBreakpoint(debugeeUrl(), 29);
    auto* const b = breakpoints()->addCodeBreakpoint(debugeeUrl(), 31);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 29);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Checked);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 31);
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
    START_DEBUGGING_E(session, cfg);
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

} // end of namespace GDB
} // end of namespace KDevMI

QTEST_MAIN(KDevMI::GDB::GdbTest)


#include "test_gdb.moc"
#include "moc_test_gdb.cpp"

