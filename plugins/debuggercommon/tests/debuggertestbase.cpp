/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>
    SPDX-FileCopyrightText: 2024, 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "debuggertestbase.h"

#include "midebugsession.h"
#include "testhelper.h"

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/framestack/framestackmodel.h>
#include <debugger/interfaces/ivariablecontroller.h>
#include <debugger/variable/variablecollection.h>
#include <execute/iexecuteplugin.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iframestackmodel.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/temporaryfilehelpers.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>
#include <util/environmentprofilelist.h>

#include <KConfigGroup>
#include <KIO/Global>
#include <KProcess>
#include <KSharedConfig>
#include <KShell>

#include <QDebug>
#include <QFileInfo>
#include <QMetaObject>
#include <QSignalBlocker>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QStringList>
#include <QTemporaryFile>
#include <QTest>
#include <QVariant>

#include <algorithm>
#include <array>

using namespace KDevelop;
using namespace KDevMI;
using KDevMI::Testing::ActiveStateSessionSpy;
using KDevMI::Testing::breakpointMiLine;
using KDevMI::Testing::currentMiLine;
using KDevMI::Testing::DebugeeslowOutputProcessor;
using KDevMI::Testing::findExecutable;
using KDevMI::Testing::findSourceFile;
using KDevMI::Testing::TestLaunchConfiguration;

namespace {
class WritableEnvironmentProfileList : public EnvironmentProfileList
{
public:
    explicit WritableEnvironmentProfileList(KConfig* config)
        : EnvironmentProfileList(config)
    {
    }

    using EnvironmentProfileList::removeProfile;
    using EnvironmentProfileList::saveSettings;
    using EnvironmentProfileList::variables;
};

#define VERIFY_VALID_ADDRESS(address)                                                                                  \
    do {                                                                                                               \
        QVERIFY(!address.isEmpty());                                                                                   \
        QVERIFY2(address.startsWith("0x"), qPrintable("unexpected address format: \"" + address + '"'));               \
    } while (false)

enum class AddressKind {
    Empty,
    Valid
};

void verifyCurrentLocation(const IDebugSession* session, const QUrl& url, int miLine, AddressKind address)
{
    QVERIFY(session);

    QCOMPARE(session->currentUrl(), url);
    QCOMPARE(currentMiLine(session), miLine);
    switch (address) {
    case AddressKind::Empty:
        QCOMPARE(session->currentAddr(), QString{});
        break;
    case AddressKind::Valid:
        VERIFY_VALID_ADDRESS(session->currentAddr());
        break;
    }
}

#define VERIFY_CURRENT_LOCATION(session, url, miLine, address)                                                         \
    do {                                                                                                               \
        verifyCurrentLocation(session, url, miLine, address);                                                          \
        RETURN_IF_TEST_FAILED();                                                                                       \
    } while (false)

#define VERIFY_INVALID_CURRENT_LOCATION(session) VERIFY_CURRENT_LOCATION(session, {}, 0, AddressKind::Empty);

#define VERIFY_VALID_CURRENT_LOCATION(session, url, miLine)                                                            \
    VERIFY_CURRENT_LOCATION(session, url, miLine, AddressKind::Valid);

[[nodiscard]] int fetchFramesCallCount(const IFrameStackModel* model)
{
    QVERIFY_RETURN(model, -1);
    const auto* const testModel = dynamic_cast<const Testing::ITestFrameStackModel*>(model);
    QVERIFY_RETURN(testModel, -1);
    return testModel->fetchFramesCallCount();
}

/**
 * Unlike kdevgdb, kdevlldb reports several frames under the main() frame. Possible names of the frames at
 * the bottom of the call stack in different versions of GNU/Linux and FreeBSD systems are the following:
 * - (expected and verified) "main";
 * - (optional) "__libc_start_call_main" or something like "___lldb_unnamed_symbol3264";
 * - "__libc_start_main_impl" or "__libc_start_main" or "__libc_start1";
 * - "_start".
 * Therefore, when some (usually all) of the frames under "main" are fetched,
 * test_lldb verifies the row count of the frame stack model via QCOMPARE_GE().
 */
#define COMPARE_ENTIRE_MAIN_THREAD_STACK_FRAME_COUNT(stackModel, threadIndex, expectedFrameCount)                      \
    do {                                                                                                               \
        if (isLldb())                                                                                                  \
            QCOMPARE_GE(stackModel->rowCount(threadIndex), expectedFrameCount);                                        \
        else                                                                                                           \
            QCOMPARE_EQ(stackModel->rowCount(threadIndex), expectedFrameCount);                                        \
    } while (false)

} // unnamed namespace

ICore* DebuggerTestBase::core() const
{
    return m_core;
}
IExecutePlugin* DebuggerTestBase::executePlugin() const
{
    return m_executePlugin;
}

VariableCollection* DebuggerTestBase::variableCollection() const
{
    return core()->debugController()->variableCollection();
}
BreakpointModel* DebuggerTestBase::breakpoints() const
{
    return core()->debugController()->breakpointModel();
}

QString DebuggerTestBase::debugeeFilePath() const
{
    return m_debugeeFilePath;
}
QUrl DebuggerTestBase::debugeeUrl() const
{
    return m_debugeeUrl;
}
QString DebuggerTestBase::debugeeLocationAt(int miLine) const
{
    return debugeeFilePath() + ':' + QString::number(miLine);
}

Breakpoint* DebuggerTestBase::addDebugeeBreakpoint(int miLine)
{
    return breakpoints()->addCodeBreakpoint(debugeeUrl(), miLine - 1);
}

void DebuggerTestBase::expandVariableCollection(const QModelIndex& index)
{
    auto* const collection = variableCollection();
    QSignalSpy childrenReadySpy(collection, &TreeModel::itemChildrenReady);
    collection->expanded(index);
    if (childrenReadySpy.empty()) {
        QVERIFY(childrenReadySpy.wait(2'000));
    }
}

void DebuggerTestBase::verifyInferiorStdout(TestLaunchConfiguration& launchConfiguration,
                                            const QStringList& expectedOutputLines)
{
    auto* const session = createTestDebugSession();
    const QSignalSpy outputSpy(session, &MIDebugSession::inferiorStdoutLines);

    START_DEBUGGING_E(session, launchConfiguration);
    WAIT_FOR_STATE(session, IDebugSession::EndedState);

    QCOMPARE_GT(outputSpy.count(), 0);

    QStringList outputLines;
    for (const auto& arguments : outputSpy) {
        QCOMPARE_EQ(arguments.size(), 1); // MIDebugSession::inferiorStdoutLines() has one parameter
        outputLines << arguments.constFirst().toStringList();
    }
    QCOMPARE(outputLines, expectedOutputLines);
}

void DebuggerTestBase::initTestCase()
{
    startInitTestCase();

    AutoTestShell::init();
    AutoTestShell::initializeNotifications();
    m_core = TestCore::initialize(Core::NoUi);
    QVERIFY(m_core);

    m_executePlugin = core()
                          ->pluginController()
                          ->pluginForExtension("org.kdevelop.IExecutePlugin", "kdevexecute")
                          ->extension<IExecutePlugin>();
    QVERIFY(m_executePlugin);

    m_debugeeFilePath = findSourceFile("debugee.cpp");
    m_debugeeUrl = QUrl::fromLocalFile(debugeeFilePath());
}

void DebuggerTestBase::cleanupTestCase()
{
    TestCore::shutdown();
}

void DebuggerTestBase::init()
{
    // remove all breakpoints - so we can set our own in the test
    auto breakpointConfig = KSharedConfig::openConfig()->group("breakpoints");
    breakpointConfig.writeEntry("number", 0);
    breakpointConfig.sync();

    auto* const breakpointModel = breakpoints();
    breakpointModel->removeRows(0, breakpointModel->rowCount());

    finishInit();
}

void DebuggerTestBase::testStdout()
{
    TestLaunchConfiguration cfg;
    verifyInferiorStdout(cfg, {"Hello, world!", "Hello"});
}

void DebuggerTestBase::testEnvironmentSet()
{
    TestLaunchConfiguration cfg("debuggee_debugeeechoenv");
    const QString profileName = isLldb() ? "LldbTestGroup" : "GdbTestGroup";

    cfg.config().writeEntry(IExecutePlugin::environmentProfileEntry, profileName);

    WritableEnvironmentProfileList envProfiles(cfg.rootConfig());
    envProfiles.removeProfile(profileName);
    auto& envs = envProfiles.variables(profileName);
    const QString variableA = "-A' \" complex --value";
    const QString variableB = "-B' \" complex --value";
    envs["VariableA"] = variableA;
    envs["VariableB"] = variableB;
    envProfiles.saveSettings(cfg.rootConfig());

    verifyInferiorStdout(cfg, {variableA, variableB});
}

void DebuggerTestBase::testEnvironmentCd()
{
    const auto path = KIO::upUrl(findExecutable("path with space/debuggee_spacedebugee"));
    TestLaunchConfiguration cfg("debuggee_debugeepath", path);
    verifyInferiorStdout(cfg, {path.toLocalFile()});
}

void DebuggerTestBase::testRunDebuggerScript()
{
    auto* const session = createTestDebugSession();

    QTemporaryFile runScript;

    const auto makeScriptContents = [this] {
        if (isLldb()) {
            return QLatin1String{"break set --file %1 --line 28\n"}.arg(debugeeFilePath());
        } else {
            return QLatin1String{R"(file %1
break main
run
)"}
                .arg(KShell::quoteArg(findExecutable("debuggee_debugee").toLocalFile()));
        }
    };
    OPEN_WRITE_AND_CLOSE_TEMPORARY_FILE(runScript, makeScriptContents());

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(runScriptEntryKey(), QUrl::fromLocalFile(runScript.fileName()));

    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 28);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testUnsupportedUrlExpressionBreakpoints()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    // Verify that the following tricky breakpoint expressions are not converted
    // into (URL, line) pairs during a debug session.

    // clang-format off
    constexpr std::array expressions = {
        "simple expression",
        "non-ASCII: øü¶¥¤¿жіЬ®施ą",
        "https://example.com/abc.txt:2",
        "//:1",
        "noprefix:1",
        "./dotslash:1",
        "../dotdotslash:1",
        "/Untitled:3",
        "/Untitled (123):75",
    };
    // clang-format on

    std::array<Breakpoint*, expressions.size()> bpoints;
    std::transform(expressions.cbegin(), expressions.cend(), bpoints.begin(), [this](const char* expression) {
        return breakpoints()->addCodeBreakpoint(QString::fromUtf8(expression));
    });

    const auto verifyBreakpoints = [&expressions, &bpoints](bool nonAsciiExpressionsSupported) {
        for (std::size_t i = 0; i < expressions.size(); ++i) {
            QCOMPARE(bpoints[i]->url(), {});
            QCOMPARE(bpoints[i]->line(), -1);

            if (i == 1 && !nonAsciiExpressionsSupported) {
                QEXPECT_FAIL("", "Non-ASCII breakpoint expressions are unsupported", Continue);
            }
            QCOMPARE(bpoints[i]->expression(), expressions[i]);
        }
    };

    verifyBreakpoints(true); // the debugger did not have a chance to break the non-ASCII expression yet
    RETURN_IF_TEST_FAILED();

    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE(session, IDebugSession::EndedState);

    // GDB/MI breaks some non-ASCII characters by replacing certain bytes, e.g. \x85 with \\205 and \x96 with \\226.
    // TODO: replace the bytes back in kdevgdb somewhere?
    const auto debuggerSupportsNonAsciiExpressions = isLldb();
    verifyBreakpoints(debuggerSupportsNonAsciiExpressions);
    RETURN_IF_TEST_FAILED();
}

void DebuggerTestBase::testBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    auto* const breakpoint = addDebugeeBreakpoint(30);
    QCOMPARE(breakpoint->state(), Breakpoint::NotStartedState);

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    QCOMPARE(breakpoint->state(), Breakpoint::CleanState);
    QCOMPARE(currentMiLine(session), 30);

    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
    QCOMPARE(breakpoint->state(), Breakpoint::NotStartedState);
}

void DebuggerTestBase::testDisableBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    // Description: we must stop only on the third breakpoint.
    const auto firstBreakLine = 29;
    const auto secondBreakLine = 24;
    const auto thirdBreakLine = 25;
    const auto fourthBreakLine = 32;

    Breakpoint* breakpoint;

    breakpoint = addDebugeeBreakpoint(firstBreakLine);
    breakpoint->setData(Breakpoint::EnableColumn, Qt::Unchecked);

    // This is needed to emulate debug from GUI. If we are in edit mode, the debugSession doesn't exist.
    Breakpoint* thirdBreak;
    {
        QSignalBlocker signalBlocker(breakpoints());

        breakpoint = addDebugeeBreakpoint(secondBreakLine);
        breakpoint->setData(Breakpoint::EnableColumn, Qt::Unchecked);
        // all disabled breakpoints were added

        thirdBreak = addDebugeeBreakpoint(thirdBreakLine);
    }

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(session->currentLine(), thirdBreak->line());

    // disable existing breakpoint
    thirdBreak->setData(Breakpoint::EnableColumn, Qt::Unchecked);

    // add another disabled breakpoint
    breakpoint = addDebugeeBreakpoint(fourthBreakLine);
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    breakpoint->setData(Breakpoint::EnableColumn, Qt::Unchecked);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testChangeLocationBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    auto* const breakpoint = addDebugeeBreakpoint(28);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 28);

    WAIT_FOR_A_WHILE(session, 100);
    breakpoint->setLine(28); // i.e. MI line 29
    WAIT_FOR_A_WHILE(session, 100);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 29);

    WAIT_FOR_A_WHILE(session, 500);
    breakpoints()->setData(breakpoints()->index(0, Breakpoint::LocationColumn), debugeeLocationAt(30));
    QCOMPARE(breakpointMiLine(breakpoint), 30);
    WAIT_FOR_A_WHILE(session, 100);
    QCOMPARE(breakpointMiLine(breakpoint), 30);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 30);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testDeleteBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    QCOMPARE(breakpoints()->rowCount(), 0);
    // add breakpoint before startDebugging()
    addDebugeeBreakpoint(22);
    QCOMPARE(breakpoints()->rowCount(), 1);
    QVERIFY(breakpoints()->removeRow(0));
    QCOMPARE(breakpoints()->rowCount(), 0);

    addDebugeeBreakpoint(23);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    QVERIFY(breakpoints()->removeRow(0));

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testPendingBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    addDebugeeBreakpoint(29);

    const auto* const breakpoint =
        breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("debugeeqt.cpp")), 10);
    QCOMPARE(breakpoint->state(), Breakpoint::NotStartedState);

    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    QCOMPARE(breakpoint->state(), Breakpoint::PendingState);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testBreakpointsOnNoOpLines()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    const auto* const licenseBreakpoint = addDebugeeBreakpoint(9);
    const auto* const blankLineBreakpoint = addDebugeeBreakpoint(34);
    const auto* const lastLineBreakpoint = addDebugeeBreakpoint(42);

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    const auto debuggerMovesBreakpointFromLicenseNotice = currentMiLine(session) == 20;

    if (debuggerMovesBreakpointFromLicenseNotice) {
        // The lines 9-19 consist of no-op code, so GDB versions older than 16
        // move the breakpoint from line 9 to line 20. The contents
        // of the line 20 is "void noop() {}", so GDB stops at it 4 times (4 is the number of calls to noop()).
        for (int noopCall = 0; noopCall < 4; ++noopCall) {
            QCOMPARE(currentMiLine(session), 20);
            CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
        }
    }

    // The lines 34 and 35 consist of no-op code, so a debugger moves
    // the breakpoint from line 34 to line 36 and stops at it.
    QCOMPARE(currentMiLine(session), 36);

    if (debuggerMovesBreakpointFromLicenseNotice) {
        QCOMPARE(breakpointMiLine(licenseBreakpoint), 20);
        QCOMPARE(licenseBreakpoint->state(), Breakpoint::CleanState);
    } else {
        // GDB since version 16 and LLDB do not move the breakpoint from
        // the no-op line 9 and permanently keep it in the pending state.
        QCOMPARE(breakpointMiLine(licenseBreakpoint), 9);
        QCOMPARE(licenseBreakpoint->state(), Breakpoint::PendingState);
    }

    QCOMPARE(breakpointMiLine(blankLineBreakpoint), 36);
    QCOMPARE(blankLineBreakpoint->state(), Breakpoint::CleanState);

    // A debugger does not move the breakpoint from the last no-op line 42
    // and permanently keeps it in the pending state.
    QCOMPARE(breakpointMiLine(lastLineBreakpoint), 42);
    QCOMPARE(lastLineBreakpoint->state(), Breakpoint::PendingState);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testIgnoreHitsBreakpoint()
{
    if (isLldb()) {
        QSKIP("Skipping... LLDB-MI does not provide breakpoint hit count update");
    }
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    auto* const b1 = addDebugeeBreakpoint(22);
    b1->setIgnoreHits(1);

    auto* const b2 = addDebugeeBreakpoint(23);

    START_DEBUGGING_E(session, cfg);
    WAIT_FOR(session, session->state() == IDebugSession::PausedState && b2->hitCount() == 1);

    b2->setIgnoreHits(1);

    session->run();
    WAIT_FOR(session, session->state() == IDebugSession::PausedState && b1->hitCount() == 1);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testConditionBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    auto* breakpoint = addDebugeeBreakpoint(40);
    breakpoint->setCondition("x[0] == 'H'");

    breakpoint = addDebugeeBreakpoint(24);
    breakpoint->setCondition("i==2");

    breakpoint = addDebugeeBreakpoint(25);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    breakpoint->setCondition("i == 0");
    WAIT_FOR_A_WHILE(session, 100);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 24);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 40);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testBreakOnWriteBreakpoint()
{
    if (isLldb()) {
        QSKIP("Skipping... LLDB-MI does not have proper watchpoint support");
    }
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    addDebugeeBreakpoint(25);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    breakpoints()->addWatchpoint("i");
    WAIT_FOR_A_WHILE(session, 100);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 23); // ++i; int j = i;

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testBreakOnWriteWithConditionBreakpoint()
{
    if (isLldb()) {
        QSKIP("Skipping... LLDB-MI does not have proper watchpoint support");
    }
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugee4foo");

    breakpoints()->addCodeBreakpoint("debugee4foo.cpp:25");
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    auto* const breakpoint = breakpoints()->addWatchpoint("i");
    breakpoint->setCondition("i==3");
    // The state is already paused. Wait for the debugger to become idle again
    // so that the breakpoint condition is set before the debugging continues.
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 23); // ++i; int j = i;

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testBreakOnReadBreakpoint()
{
    if (isLldb()) {
        QSKIP("Skipping... LLDB-MI does not have proper watchpoint support");
    }
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    breakpoints()->addReadWatchpoint("foo::i");

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_E(session, cfg);

    for (int fooCall = 0; fooCall < 2; ++fooCall) {
        WAIT_FOR_PAUSED_STATE(session, sessionSpy);
        QCOMPARE(currentMiLine(session), 23); // ++i;

        CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
        QCOMPARE(currentMiLine(session), 23); // int j = i;

        session->run();
    }
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

// This test adds a read watchpoint during a debug session rather than before it in order to
// work around the (already fixed) GDB bug http://sourceware.org/bugzilla/show_bug.cgi?id=10136
void DebuggerTestBase::testBreakOnReadBreakpoint2()
{
    if (isLldb()) {
        QSKIP("Skipping... LLDB-MI does not have proper watchpoint support");
    }
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    addDebugeeBreakpoint(25);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    breakpoints()->addReadWatchpoint("i");

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 23); // ++i

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 23); // int j = i

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testBreakOnAccessBreakpoint()
{
    if (isLldb()) {
        QSKIP("Skipping... LLDB-MI does not have proper watchpoint support");
    }
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    addDebugeeBreakpoint(25);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    breakpoints()->addAccessWatchpoint("i");

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 23); // ++i (read)

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 23); // ++i (write)

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 23); // int j = i (read)

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testBreakpointErrors()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    // The following breakpoint data makes GDB/MI (but not lldb-mi) report breakpoint errors.
    // Verify that a debug session works correctly despite such uncommon error reports.

    // clang-format off
    constexpr std::array expressions = {
        ":resourcepath",
        ":resourcepath:1",
        ":/colonslash:1",
        ":./colondotslash",
    };
    constexpr std::array urls = {
        "file::resourcepath",
        "file::/colonslash",
        "file::../colondotdotslash",
    };
    constexpr std::array conditions = {
        "not_exist_var > 3",
    };
    // clang-format on

    for (const auto expression : expressions) {
        breakpoints()->addCodeBreakpoint(QString::fromUtf8(expression));
    }
    for (const auto url : urls) {
        breakpoints()->addCodeBreakpoint(QUrl{QString::fromUtf8(url)}, 1);
    }
    for (const auto condition : conditions) {
        auto* const breakpoint = addDebugeeBreakpoint(29);
        breakpoint->setCondition(QString::fromUtf8(condition));
    }

    addDebugeeBreakpoint(30);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    const auto debuggerStopsOnInvalidCondition = isLldb();
    if (debuggerStopsOnInvalidCondition) {
        QCOMPARE(currentMiLine(session), 29);
        CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    }

    QCOMPARE(currentMiLine(session), 30);
    session->run();

    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testInsertBreakpointWhileRunning()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeeslow");
    const auto fileName = findSourceFile("debugeeslow.cpp");

    START_DEBUGGING_E(session, cfg);

    WAIT_FOR_STATE(session, IDebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 2000);

    qDebug() << "adding breakpoint";
    auto* const breakpoint = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 29); // ++i;
    WAIT_FOR_A_WHILE(session, 500);

    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(session->currentLine(), 29); // ++i;

    breakpoints()->removeBreakpoint(breakpoint);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testInsertBreakpointWhileRunningMultiple()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeeslow");
    const auto fileName = findSourceFile("debugeeslow.cpp");

    START_DEBUGGING_E(session, cfg);

    WAIT_FOR_STATE(session, IDebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 2000);

    qDebug() << "adding breakpoints";
    auto* const b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 29); // ++i;
    auto* const b2 =
        breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 30); // std::cout << i << std::endl;
    WAIT_FOR_A_WHILE(session, 500);

    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(session->currentLine(), 29);

    const ActiveStateSessionSpy sessionSpy(session);
    session->run();
    WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(session->currentLine(), 30);

    breakpoints()->removeBreakpoint(b1);
    breakpoints()->removeBreakpoint(b2);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testInsertBreakpointFunctionName()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint("main");
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 28);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

// Bug 201771
void DebuggerTestBase::testInsertAndRemoveBreakpointWhileRunning()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeeslow");
    const auto fileName = findSourceFile("debugeeslow.cpp");

    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE(session, IDebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 2000);

    // The idea of this test function is to add and quickly remove a breakpoint while the debugee is running.
    // The Bug 201771 that inspired this test function reported a segfault in GDBDebugger.
    qDebug() << "adding breakpoint";
    auto* const breakpoint =
        breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 30); // std::cout << i << std::endl;
    breakpoints()->removeBreakpoint(breakpoint);

    QCOMPARE(session->state(), IDebugSession::ActiveState);

    QObject testFunctionScopedObject;
    if (isLldb()) {
        // The paused state is entered only rarely (about 1% of test_lldb runs) on GNU/Linux.
        // On FreeBSD, however, LLDB-MI always manages to stop at the breakpoint soon after it is added
        // and before it is removed, no matter how long this test function waits before adding the breakpoint.
        // Entering the paused state should not cause a test failure, so detect it and continue debugging.

        // Set a function-scoped object as the context to break the connection when this
        // test function returns, and so avoid interfering with the next test function.
        connect(
            session, &IDebugSession::stateChanged, &testFunctionScopedObject,
            [session, pauseCount = 0](IDebugSession::DebuggerState state) mutable {
                if (state != IDebugSession::PausedState) {
                    return; // nothing to do
                }
                // Calling session->run() directly happens to work too. But synchronously
                // continuing debugging in a function directly connected to the stateChanged()
                // signal is not reasonable and does not need to be supported.
                QMetaObject::invokeMethod(session, &IDebugSession::run, Qt::QueuedConnection);

                if (++pauseCount == 1) {
                    qWarning()
                        << "debug session reached paused state because LLDB-MI stopped at the transitory breakpoint";
                } else {
                    QFAIL("debug session reached paused state more than once, did removing the breakpoint fail?");
                }
            });
    }

    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testChangeBreakpointWhileRunning()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeeslow");
    auto* const breakpoint = breakpoints()->addCodeBreakpoint("debugeeslow.cpp:30"); // ++i;

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 30);

    QSignalSpy outputSpy(session, &MIDebugSession::inferiorStdoutLines);
    DebugeeslowOutputProcessor outputProcessor(outputSpy);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::ActiveState);

    qDebug() << "Disabling breakpoint";
    breakpoint->setData(Breakpoint::EnableColumn, Qt::Unchecked);

    // Wait for two lines of output "1" and "2" to verify that the debugger does not stop at the disabled breakpoint.
    while (true) {
        outputProcessor.processOutput();
        RETURN_IF_TEST_FAILED();
        if (outputProcessor.processedLineCount() >= 2) {
            break;
        }

        qDebug() << "Waiting for debuggee output...";
        const auto outputArrived = outputSpy.wait(20'000);
        // Verify that the debugger did not stop at the disabled breakpoint while we waited for the debuggee output.
        QCOMPARE(session->state(), IDebugSession::ActiveState);

        QVERIFY(outputArrived);
        QCOMPARE_GT(outputSpy.size(), 0);
    }

    // If the debuggee has already produced all 3 lines of output,
    // the paused state waited for below will never be reached!
    QCOMPARE_EQ(outputProcessor.processedLineCount(), 2);

    qDebug() << "Enabling breakpoint";
    breakpoint->setData(Breakpoint::EnableColumn, Qt::Checked);
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);

    outputProcessor.processOutput();
    RETURN_IF_TEST_FAILED();
    QCOMPARE_EQ(outputProcessor.processedLineCount(), 3);
}

void DebuggerTestBase::testPickupManuallyInsertedBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint("main");
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    session->addCommand(MI::NonMI, isLldb() ? "break set --file debugee.cpp --line 32" : "break debugee.cpp:32");
    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    QCOMPARE(breakpoints()->breakpoints().size(), 2);
    QCOMPARE(breakpoints()->rowCount(), 2);
    const auto* const breakpoint = breakpoints()->breakpoint(1);
    QVERIFY(breakpoint);
    QCOMPARE(breakpointMiLine(breakpoint), 32);
    QCOMPARE(breakpoint->url(), debugeeUrl());
}

// Bug 270970
void DebuggerTestBase::testPickupManuallyInsertedBreakpointOnlyOnce()
{
    auto* const session = createTestDebugSession();

    // inject here so that it behaves like a command from .gdbinit or .lldbinit
    QTemporaryFile configScript;

    const auto makeScriptContents = [this] {
        if (isLldb()) {
            return QLatin1String{"break set --file %1 --line 32\n"}.arg(debugeeFilePath());
        } else {
            return QLatin1String{R"(file %1
break debugee.cpp:32
)"}
                .arg(findExecutable("debuggee_debugee").toLocalFile());
        }
    };
    OPEN_WRITE_AND_CLOSE_TEMPORARY_FILE(configScript, makeScriptContents());

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(configScriptEntryKey(), QUrl::fromLocalFile(configScript.fileName()));

    addDebugeeBreakpoint(32);

    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    QCOMPARE(breakpoints()->breakpoints().size(), 1);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testBreakpointWithSpaceInPath()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeespace");
    const auto fileName = findSourceFile("debugee space.cpp");

    const auto* const breakpoint = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 20);
    QCOMPARE(breakpoint->state(), Breakpoint::NotStartedState);

    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(session->currentLine(), 20);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

// see https://bugs.kde.org/show_bug.cgi?id=339231
void DebuggerTestBase::testPathWithSpace()
{
    auto* const session = createTestDebugSession();
    const auto debugee = findExecutable("path with space/debuggee_spacedebugee");
    TestLaunchConfiguration cfg(debugee, KIO::upUrl(debugee));

    const auto* const breakpoint = breakpoints()->addCodeBreakpoint("spacedebugee.cpp:30");
    QCOMPARE(breakpoint->state(), Breakpoint::NotStartedState);

    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(breakpoint->state(), Breakpoint::CleanState);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testBreakpointInSharedLibrary()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("multifile/debuggee_debugeemultifile");
    breakpoints()->addCodeBreakpoint("multifile_main.cpp:14"); // return 0;
    breakpoints()->addCodeBreakpoint("multifile_shared.cpp:11"); // return n * n;

    VERIFY_INVALID_CURRENT_LOCATION(session);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    VERIFY_VALID_CURRENT_LOCATION(session, QUrl::fromLocalFile(findSourceFile("multifile/multifile_shared.cpp")), 11);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    VERIFY_VALID_CURRENT_LOCATION(session, QUrl::fromLocalFile(findSourceFile("multifile/multifile_main.cpp")), 14);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testMultipleLocationsBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeemultilocbreakpoint");

    breakpoints()->addCodeBreakpoint("aPlusB");

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 20);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 24);

    const auto breakpointList = breakpoints()->breakpoints();
    QCOMPARE(breakpointList.size(), 1);
    if (isLldb()) {
        // FIXME: the peculiar behavior (bug?) of LLDB-MI tricks KDevelop into wrongly replacing a multiple-location
        //        breakpoint with a single-location breakpoint (debugeemultilocbreakpoint.cpp:24 in this case).
        QEXPECT_FAIL("", "Unlike GDB/MI, LLDB-MI reports only one of the locations of a multiple-location breakpoint",
                     Continue);
    }
    // GDB/MI reports the individual locations of a multiple-location breakpoint, but KDevelop
    // displays only the original location string in the UI. There is room for improvement here.
    QCOMPARE(breakpointList.front()->location(), "aPlusB");

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testMultipleBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeemultiplebreakpoint");

    // there will be about 3-4 breakpoints, but we treat it like one
    auto* const breakpoint = breakpoints()->addCodeBreakpoint("debugeemultiplebreakpoint.cpp:52");

    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(breakpoints()->breakpoints().size(), 1);

    breakpoint->setData(Breakpoint::EnableColumn, Qt::Unchecked);
    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testRegularExpressionBreakpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeemultilocbreakpoint");

    breakpoints()->addCodeBreakpoint("main");
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    session->addCommand(MI::NonMI, isLldb() ? "break set --func-regex .*aPl.*B" : "rbreak .*aPl.*B");

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 20);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 24);

    if (isLldb()) {
        QCOMPARE_GE(breakpoints()->breakpoints().size(), 2);
        QCOMPARE_LE(breakpoints()->breakpoints().size(), 3);

        // FIXME: the peculiar behavior (bug?) of LLDB-MI tricks KDevelop into wrongly replacing a multiple-location
        //        breakpoint with a single-location breakpoint (debugeemultilocbreakpoint.cpp:24 in this case).
        QEXPECT_FAIL("",
                     "Unlike GDB, LLDB creates a single breakpoint for multiple locations matched "
                     "by a regular expression, and LLDB-MI reports only one of the locations",
                     Continue);
    }
    // GDB/MI creates two separate breakpoints for the two locations matched by the regular expression.
    QCOMPARE(breakpoints()->breakpoints().size(), 3);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testCatchpoint()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeeexception");
    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    const auto* const stackModel = qobject_cast<FrameStackModel*>(session->frameStackModel());
    QVERIFY(stackModel);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("debugeeexception.cpp")), 29);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(stackModel->currentFrame(), 0);
    QCOMPARE(session->currentLine(), 29);

    session->addCommand(MI::NonMI, isLldb() ? "break set -E c++" : "catch throw");
    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    const auto frames = stackModel->frames(stackModel->currentThread());
    QCOMPARE_GE(frames.size(), 2);
    // frame 0 is somewhere inside libstdc++
    QCOMPARE(frames[1].file, QUrl::fromLocalFile(findSourceFile("debugeeexception.cpp")));
    QCOMPARE(frames[1].line, 22);

    QCOMPARE(breakpoints()->rowCount(), 2);
    QVERIFY(!breakpoints()->breakpoint(0)->location().isEmpty());
    QVERIFY(!breakpoints()->breakpoint(1)->location().isEmpty());

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testShowStepInSource()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    const QSignalSpy showStepInSourceSpy(session, &IDebugSession::showStepInSource);

    constexpr auto breakpointLine = 30;
    addDebugeeBreakpoint(breakpointLine);

    VERIFY_INVALID_CURRENT_LOCATION(session);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    VERIFY_VALID_CURRENT_LOCATION(session, debugeeUrl(), breakpointLine);

    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    constexpr auto firstStepIntoLine = 23;
    VERIFY_VALID_CURRENT_LOCATION(session, debugeeUrl(), firstStepIntoLine);

    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    constexpr auto secondStepIntoLine = 24;
    VERIFY_VALID_CURRENT_LOCATION(session, debugeeUrl(), secondStepIntoLine);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);

    const auto verifyArgumentsOfShowStepInSource = [this](const QVariantList& arguments, int miLine) {
        QCOMPARE(arguments.size(), 3);
        QCOMPARE(arguments.at(0).toUrl(), debugeeUrl());
        // an MI line is one-based and IDebugSession::currentLine() is zero-based
        QCOMPARE(arguments.at(1).toInt(), miLine - 1);
        VERIFY_VALID_ADDRESS(arguments.at(2).toString());
    };

    QCOMPARE(showStepInSourceSpy.count(), 3);
    verifyArgumentsOfShowStepInSource(showStepInSourceSpy.at(0), breakpointLine);
    RETURN_IF_TEST_FAILED();
    verifyArgumentsOfShowStepInSource(showStepInSourceSpy.at(1), firstStepIntoLine);
    RETURN_IF_TEST_FAILED();
    verifyArgumentsOfShowStepInSource(showStepInSourceSpy.at(2), secondStepIntoLine);
    RETURN_IF_TEST_FAILED();
}

void DebuggerTestBase::testStack()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    const auto* const stackModel = session->frameStackModel();

    addDebugeeBreakpoint(22);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    const auto threadIndex = stackModel->index(0, 0);

    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 1);
    COMPARE_DATA(threadIndex, adjustedStackModelFrameName("#1 at foo"));
    COMPARE_ENTIRE_MAIN_THREAD_STACK_FRAME_COUNT(stackModel, threadIndex, 2);
    COMPARE_DATA(stackModel->index(0, 1, threadIndex), adjustedStackModelFrameName("foo"));
    COMPARE_DATA(stackModel->index(0, 2, threadIndex), debugeeLocationAt(23));
    COMPARE_DATA(stackModel->index(1, 1, threadIndex), "main");
    COMPARE_DATA(stackModel->index(1, 2, threadIndex), debugeeLocationAt(29));

    STEP_OUT_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 1);
    COMPARE_DATA(threadIndex, "#1 at main");
    COMPARE_ENTIRE_MAIN_THREAD_STACK_FRAME_COUNT(stackModel, threadIndex, 1);
    COMPARE_DATA(stackModel->index(0, 1, threadIndex), "main");
    // As the What's This for KDevelop's Step Out action says, the debugger should
    // now display the line after the original call to the stepped-out function: 30.
    if (!isLldb()) {
        // On a given system, all GDB/MI output records consistently report a certain line number as current
        // after the step-out. But for some reason since 2020 this MI line number is 29 (wrong) on the openSUSE
        // Tumbleweed CI servers. The line number remains 30 (correct) on developers' local GNU/Linux systems
        // and on the FreeBSD CI server. Work around the difference by accepting either line number.
        // TODO: why does this difference in line numbers persists for many years?
        //       Is this some openSUSE bug or a CI server configuration issue?
        const auto currentLocation = stackModel->index(0, 2, threadIndex).data().toString();
        if (currentLocation.endsWith(":29")) {
            QCOMPARE(currentLocation, debugeeLocationAt(29));
            QEXPECT_FAIL("", "Wrong line number after stepping out: 29 instead of the expected 30", Continue);
        }
    }
    COMPARE_DATA(stackModel->index(0, 2, threadIndex), debugeeLocationAt(30));

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testStackFetchMore()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeerecursion");
    const auto fileName = findSourceFile("debugeerecursion.cpp");
    constexpr auto recursionDepth = 295;

    auto* const stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    const auto threadIndex = stackModel->index(0, 0);

    QCOMPARE(fetchFramesCallCount(stackModel), 1);
    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 1);
    COMPARE_DATA(threadIndex, adjustedStackModelFrameName("#1 at foo"));
    QCOMPARE(stackModel->rowCount(threadIndex), 21);
    COMPARE_DATA(stackModel->index(0, 1, threadIndex), adjustedStackModelFrameName("foo"));
    COMPARE_DATA(stackModel->index(0, 2, threadIndex), fileName + ":26");
    COMPARE_DATA(stackModel->index(1, 1, threadIndex), adjustedStackModelFrameName("foo"));
    COMPARE_DATA(stackModel->index(1, 2, threadIndex), fileName + ":24");
    COMPARE_DATA(stackModel->index(2, 1, threadIndex), adjustedStackModelFrameName("foo"));
    COMPARE_DATA(stackModel->index(2, 2, threadIndex), fileName + ":24");

    stackModel->fetchMoreFrames();
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    QCOMPARE(fetchFramesCallCount(stackModel), 2);

    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 1);
    QCOMPARE(stackModel->rowCount(threadIndex), 41);

    stackModel->fetchMoreFrames();
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    QCOMPARE(fetchFramesCallCount(stackModel), 3);

    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 1);
    QCOMPARE(stackModel->rowCount(threadIndex), 121);

    stackModel->fetchMoreFrames();
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    QCOMPARE(fetchFramesCallCount(stackModel), 4);

    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 1);
    COMPARE_ENTIRE_MAIN_THREAD_STACK_FRAME_COUNT(stackModel, threadIndex, recursionDepth + 1);
    COMPARE_DATA(stackModel->index(recursionDepth, 1, threadIndex), "main");
    COMPARE_DATA(stackModel->index(recursionDepth, 2, threadIndex), fileName + ":30");

    stackModel->fetchMoreFrames(); // nothing to fetch, we are at the end
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    QCOMPARE(fetchFramesCallCount(stackModel), 4);

    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 1);
    COMPARE_ENTIRE_MAIN_THREAD_STACK_FRAME_COUNT(stackModel, threadIndex, recursionDepth + 1);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testStackSwitchThread()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeethreads");
    const auto fileName = findSourceFile("debugeethreads.cpp");

    auto* const stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 43); // QThread::msleep(600);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    auto threadIndex = stackModel->index(0, 0);
    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 4);
    COMPARE_DATA(threadIndex, "#1 at main");
    COMPARE_ENTIRE_MAIN_THREAD_STACK_FRAME_COUNT(stackModel, threadIndex, 1);
    COMPARE_DATA(stackModel->index(0, 1, threadIndex), "main");
    COMPARE_DATA(stackModel->index(0, 2, threadIndex), fileName + ":44"); // QThread::msleep(600);

    threadIndex = stackModel->index(1, 0);
    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 4);
    QVERIFY(stackModel->data(threadIndex).toString().startsWith("#2 at "));
    QCOMPARE(stackModel->rowCount(threadIndex), 0); // frames are not fetched for non-current threads

    stackModel->setCurrentThread(2);
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);

    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 4);
    QVERIFY(stackModel->data(threadIndex).toString().startsWith("#2 at "));
    QCOMPARE_GE(stackModel->rowCount(threadIndex), 4);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testCoreFile()
{
    QFileInfo f("core");
    f.setCaching(false); // don't cache information
    if (f.exists()) {
        QVERIFY(QFile::remove(f.canonicalFilePath()));
    }

    KProcess debugeeProcess;
    debugeeProcess.setOutputChannelMode(KProcess::MergedChannels);
    debugeeProcess << "bash" << "-c"
                   << "ulimit -c unlimited; " + findExecutable("debuggee_crash").toLocalFile();
    debugeeProcess.start();
    debugeeProcess.waitForFinished();
    qDebug() << "Debuggee output:\n" << debugeeProcess.readAll();

    auto coreFileFound = f.exists();
    if (!coreFileFound) {
        qDebug() << "try to use coredumpctl";
        const auto coredumpctl = QStandardPaths::findExecutable("coredumpctl");
        if (!coredumpctl.isEmpty()) {
            KProcess::execute(coredumpctl, {"-1", "-o", f.absoluteFilePath(), "dump", "debuggee_crash"}, 5000);
            // coredumpctl seems to create an empty file "core" even if no cores can be delivered
            // (like when run inside docker containers as on KDE CI or with kernel.core_pattern=|/dev/null)
            // so also check for size != 0
            coreFileFound = f.exists() && (f.size() > 0);
        }
    }
    if (!coreFileFound) {
        QSKIP("no core dump found, check your system configuration (see /proc/sys/kernel/core_pattern).");
    }

    auto* const session = createTestDebugSession();
    session->examineCoreFile(findExecutable("debuggee_crash"), QUrl::fromLocalFile(f.canonicalFilePath()));

    const auto* const stackModel = session->frameStackModel();

    WAIT_FOR_STATE(session, IDebugSession::StoppedState);

    const auto threadIndex = stackModel->index(0, 0);
    VALIDATE_COLUMN_COUNTS_THREAD_COUNT_AND_STACK_FRAME_NUMBERS(threadIndex, 1);
    COMPARE_DATA(threadIndex, adjustedStackModelFrameName("#1 at foo"));

    session->stopDebugger();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testVariablesLocalsStruct()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;
    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    addDebugeeBreakpoint(39);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    WAIT_FOR_A_WHILE(session, 1000);

    const auto i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);

    auto structIndex = 0;
    for (auto j = 0; j < 3; ++j) {
        if (variableCollection()->index(j, 0, i).data().toString() == QLatin1String("ts")) {
            structIndex = j;
        }
    }

    COMPARE_DATA(variableCollection()->index(structIndex, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(structIndex, 1, i), "{...}");
    const auto ts = variableCollection()->index(structIndex, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "...");

    EXPAND_VARIABLE_COLLECTION(ts);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    COMPARE_DATA(variableCollection()->index(structIndex, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(structIndex, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testVariablesWatches()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    variableCollection()->variableWidgetShown();

    addDebugeeBreakpoint(39);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    variableCollection()->watches()->add("ts");
    WAIT_FOR_A_WHILE(session, 300);

    const auto i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    const auto ts = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "...");

    EXPAND_VARIABLE_COLLECTION(ts);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testVariablesStopDebugger()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    addDebugeeBreakpoint(39);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    session->stopDebugger();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testVariablesStartSecondSession()
{
    auto* session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    addDebugeeBreakpoint(39);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    session = createTestDebugSession();
    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    addDebugeeBreakpoint(39);
    const ActiveStateSessionSpy sessionSpy2(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy2);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

// Bug 301287
void DebuggerTestBase::testVariablesSameWatchInSecondSession()
{
    auto* session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(IVariableController::UpdateWatches);

    addDebugeeBreakpoint(29);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    variableCollection()->watches()->add("argc");
    WAIT_FOR_A_WHILE(session, 300);

    auto watchesIndex = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(watchesIndex), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, watchesIndex), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, watchesIndex), "1");

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);

    // start second debug session (same cfg)
    session = createTestDebugSession();
    session->variableController()->setAutoUpdate(IVariableController::UpdateWatches);

    const ActiveStateSessionSpy sessionSpy2(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy2);

    watchesIndex = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(watchesIndex), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, watchesIndex), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, watchesIndex), "1");

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testVariablesSwitchFrame()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    auto* const stackModel = session->frameStackModel();

    addDebugeeBreakpoint(25);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    auto localsIndex = variableCollection()->index(1, 0);
    COMPARE_DATA(localsIndex, "Locals");

    int jRow;
    if (isLldb()) {
        // only the non-static variable `j` works
        QCOMPARE(variableCollection()->rowCount(localsIndex), 1);
        jRow = 0;
    } else {
        QCOMPARE(variableCollection()->rowCount(localsIndex), 2);
        COMPARE_DATA(variableCollection()->index(0, 0, localsIndex), "i");
        COMPARE_DATA(variableCollection()->index(0, 1, localsIndex), "1");
        jRow = 1;
    }
    COMPARE_DATA(variableCollection()->index(jRow, 0, localsIndex), "j");
    COMPARE_DATA(variableCollection()->index(jRow, 1, localsIndex), "1");

    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 200);

    localsIndex = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(localsIndex), 4);
    COMPARE_DATA(variableCollection()->index(2, 0, localsIndex), "argc");
    COMPARE_DATA(variableCollection()->index(2, 1, localsIndex), "1");
    COMPARE_DATA(variableCollection()->index(3, 0, localsIndex), "argv");

    breakpoints()->removeRow(0);
    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testVariablesQuicklySwitchFrame()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    auto* const stackModel = session->frameStackModel();

    addDebugeeBreakpoint(25);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

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

    const auto localsIndex = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(localsIndex), 4);
    QStringList locals;
    for (auto row = 0; row < variableCollection()->rowCount(localsIndex); ++row) {
        locals.push_back(variableCollection()->index(row, 0, localsIndex).data().toString());
    }
    QVERIFY(locals.contains("argc"));
    QVERIFY(locals.contains("argv"));
    QVERIFY(locals.contains("x"));

    breakpoints()->removeRow(0);
    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testVariablesChanged()
{
    // kdevlldb xfails the test because of a workaround in LLDB::DebugSession::updateAllVariables()
    // that updates all variables manually and never marks variables as changed.

    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    auto* const collection = variableCollection();
    collection->variableWidgetShown();

    auto* const breakpoint = addDebugeeBreakpoint(23);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(currentMiLine(session), 23);

    // Prepare "static int i = 0;"
    // In LLDB, "i" doesn't exist in Locals, so we have to do this via a watch.
    const auto* const iVariable = collection->watches()->add("i");
    QVERIFY(iVariable);
    // Expand the "Auto" collection.
    const auto watchesIndex = collection->index(0, 0);
    COMPARE_DATA(watchesIndex, "Auto");
    collection->expanded(watchesIndex);
    // Expand the Locals collection.
    const auto localsIndex = collection->index(1, 0);
    COMPARE_DATA(localsIndex, "Locals");
    collection->expanded(localsIndex);

    // Wait for the watch to be added and the collections to be expanded.
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    QCOMPARE(collection->rowCount(watchesIndex), 1);

    QCOMPARE(iVariable->value(), "0");
    // Top-level variables are initialized as unchanged.
    QCOMPARE(iVariable->isChanged(), false);

    STEP_OVER_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 24);

    QCOMPARE(iVariable->value(), "1");
    if (isLldb()) {
        QEXPECT_FAIL("", "kdevlldb does not properly support reporting isChanged()", Continue);
    }
    QCOMPARE(iVariable->isChanged(), true);

    STEP_OVER_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    QCOMPARE(iVariable->value(), "1");
    QCOMPARE(iVariable->isChanged(), false);

    // The row of a local variable "j" is different in LLDB (0) vs. GDB (1).
    const auto jLocalsRow = isLldb() ? 0 : 1;
    QCOMPARE_GT(collection->rowCount(localsIndex), jLocalsRow);
    const auto jIndex = collection->index(jLocalsRow, 0, localsIndex);
    COMPARE_DATA(jIndex, "j");
    const auto* const jVariable = qobject_cast<KDevelop::Variable*>(collection->itemForIndex(jIndex));
    QVERIFY(jVariable);

    QCOMPARE(jVariable->value(), "1");
    QCOMPARE(jVariable->isChanged(), false);

    // Return and enter the debugee function foo() a second time, stopping at the first "noop();" line.
    breakpoint->setLine(breakpoint->line() + 1);
    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 24);

    QCOMPARE(jVariable->value(), "2");
    if (isLldb()) {
        QEXPECT_FAIL("", "kdevlldb does not properly support reporting isChanged()", Continue);
    }
    QCOMPARE(jVariable->isChanged(), true);

    STEP_OVER_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    QCOMPARE(jVariable->value(), "2");
    QCOMPARE(jVariable->isChanged(), false);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testSwitchFrameDebuggerConsole()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    auto* const stackModel = session->frameStackModel();

    addDebugeeBreakpoint(25);
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);

    QCOMPARE(stackModel->currentFrame(), 0);
    stackModel->setCurrentFrame(1);
    QCOMPARE(stackModel->currentFrame(), 1);
    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(stackModel->currentFrame(), 1);

    // NOTE: the debugger refuses to print `x` from the frame #1 and replies with an error
    //       because stackModel->currentFrame() does not affect user commands, should it?
    session->addUserCommand("print x");
    WAIT_FOR_A_WHILE(session, 500);
    // currentFrame must not reset to 0; Bug 222882
    QCOMPARE(stackModel->currentFrame(), 1);
}

// Bug 274390
void DebuggerTestBase::testCommandOrderFastStepping()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeeqt");

    breakpoints()->addCodeBreakpoint("main");
    const ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_E(session, cfg);

    for (auto i = 0; i < 20; ++i) {
        session->stepInto();
    }

    WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testSegfaultDebugee()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_crash");
    const auto fileName = findSourceFile("debugeecrash.cpp");

    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 23);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(session->currentLine(), 23);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(currentMiLine(session), 25);

    session->stopDebugger();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testDebugInExternalTerminal_data()
{
    QTest::addColumn<QString>("terminalExecutable");
    QTest::addColumn<QString>("terminalCommand");

    const auto externalTerminalCommands = executePlugin()->defaultExternalTerminalCommands();
    for (const auto& terminalCommand : externalTerminalCommands) {
        // Do the same as ExecutePlugin::terminal(), but without backward
        // compatibility support and with stricter error handling.
        QVERIFY(!terminalCommand.isEmpty());

        KShell::Errors err;
        const auto splitCommand = KShell::splitArgs(terminalCommand, KShell::TildeExpand | KShell::AbortOnMeta, &err);
        QCOMPARE(err, KShell::NoError);
        QVERIFY(!splitCommand.empty());

        const auto terminalExecutable = splitCommand.constFirst();
        QTest::newRow(qPrintable(terminalExecutable)) << terminalExecutable << terminalCommand;
    }
}

void DebuggerTestBase::testDebugInExternalTerminal()
{
    QFETCH(const QString, terminalExecutable);
    QFETCH(const QString, terminalCommand);

    if (QStandardPaths::findExecutable(terminalExecutable).isEmpty()) {
        QSKIP(qPrintable(QLatin1String{"Skipping because `%1` executable is not available"}.arg(terminalExecutable)));
    }

    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    cfg.config().writeEntry(IExecutePlugin::useTerminalEntry, true);
    cfg.config().writeEntry(IExecutePlugin::terminalEntry, terminalCommand);

    const auto* const breakpoint = addDebugeeBreakpoint(29);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(breakpoint->state(), Breakpoint::CleanState);

    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

QString DebuggerTestBase::adjustedStackModelFrameName(QString frameName) const
{
    if (isLldb()) {
        frameName += "()";
    }
    return frameName;
}

#include "moc_debuggertestbase.cpp"
