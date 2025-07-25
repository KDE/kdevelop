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
#include <debugger/interfaces/ivariablecontroller.h>
#include <debugger/variable/variablecollection.h>
#include <execute/iexecuteplugin.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>
#include <util/environmentprofilelist.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDebug>
#include <QMetaObject>
#include <QSignalBlocker>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QStringList>
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

void DebuggerTestBase::testEnvironmentSet()
{
    auto* const session = createTestDebugSession();
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

    QSignalSpy outputSpy(session, &MIDebugSession::inferiorStdoutLines);

    START_DEBUGGING_E(session, cfg);
    WAIT_FOR_STATE(session, IDebugSession::EndedState);

    QVERIFY(outputSpy.count() > 0);

    QStringList outputLines;
    while (outputSpy.count() > 0) {
        const QList<QVariant> arguments = outputSpy.takeFirst();
        for (const auto& item : arguments) {
            outputLines.append(item.toStringList());
        }
    }
    QCOMPARE(outputLines, QStringList() << variableA << variableB);
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
    WAIT_FOR_A_WHILE(session, 300);
    breakpoint->setData(Breakpoint::EnableColumn, Qt::Unchecked);

    WAIT_FOR_A_WHILE(session, 300);
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
        QVERIFY(outputSpy.wait(5'000));
        QCOMPARE_GT(outputSpy.size(), 0);
    }

    // If the debuggee has already produced all 3 lines of output,
    // the paused state waited for below will never be reached!
    QCOMPARE_EQ(outputProcessor.processedLineCount(), 2);

    qDebug() << "Waiting for active";
    WAIT_FOR_STATE(session, IDebugSession::ActiveState);

    qDebug() << "Enabling breakpoint";
    breakpoint->setData(Breakpoint::EnableColumn, Qt::Checked);
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);

    outputProcessor.processOutput();
    RETURN_IF_TEST_FAILED();
    QCOMPARE_EQ(outputProcessor.processedLineCount(), 3);
}

void DebuggerTestBase::testBreakpointInSharedLibrary()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("multifile/debuggee_debugeemultifile");
    breakpoints()->addCodeBreakpoint("multifile_main.cpp:14"); // return 0;
    breakpoints()->addCodeBreakpoint("multifile_shared.cpp:11"); // return n * n;

    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(session->currentUrl().fileName(), "multifile_shared.cpp");
    QCOMPARE(currentMiLine(session), 11);

    CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);
    QCOMPARE(session->currentUrl().fileName(), "multifile_main.cpp");
    QCOMPARE(currentMiLine(session), 14);

    session->run();
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

void DebuggerTestBase::testDebugInExternalTerminal_data()
{
    QTest::addColumn<QString>("terminalExecutable");

    for (const auto* const terminalExecutable : {"konsole", "xterm", "xfce4-terminal", "gnome-terminal"}) {
        QTest::newRow(terminalExecutable) << terminalExecutable;
    }
}

void DebuggerTestBase::testDebugInExternalTerminal()
{
    QFETCH(const QString, terminalExecutable);

    if (QStandardPaths::findExecutable(terminalExecutable).isEmpty()) {
        QSKIP(qPrintable(QLatin1String{"Skipping because `%1` executable is not available"}.arg(terminalExecutable)));
    }

    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    cfg.config().writeEntry(IExecutePlugin::useTerminalEntry, true);
    cfg.config().writeEntry(IExecutePlugin::terminalEntry, terminalExecutable);

    const auto* const breakpoint = addDebugeeBreakpoint(29);
    ActiveStateSessionSpy sessionSpy(session);
    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg, sessionSpy);
    QCOMPARE(breakpoint->state(), Breakpoint::CleanState);

    STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

#include "moc_debuggertestbase.cpp"
