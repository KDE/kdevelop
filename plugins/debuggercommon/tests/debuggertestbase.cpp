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
#include <QSignalSpy>
#include <QStringList>
#include <QTest>
#include <QVariant>

#include <algorithm>
#include <array>

using namespace KDevelop;
using namespace KDevMI;
using KDevMI::Testing::breakpointMiLine;
using KDevMI::Testing::currentMiLine;
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

void DebuggerTestBase::testBreakpointsOnNoOpLines()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg;

    const auto* const licenseBreakpoint = addDebugeeBreakpoint(9);
    const auto* const blankLineBreakpoint = addDebugeeBreakpoint(34);
    const auto* const lastLineBreakpoint = addDebugeeBreakpoint(42);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg);
    const auto debuggerMovesBreakpointFromLicenseNotice = currentMiLine(session) == 20;

    if (debuggerMovesBreakpointFromLicenseNotice) {
        // The lines 9-19 consist of no-op code, so GDB versions older than 16
        // move the breakpoint from line 9 to line 20. The contents
        // of the line 20 is "void noop() {}", so GDB stops at it 4 times (4 is the number of calls to noop()).
        for (int noopCall = 0; noopCall < 4; ++noopCall) {
            QCOMPARE(currentMiLine(session), 20);
            session->run();
            WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
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

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg);

    const auto debuggerStopsOnInvalidCondition = isLldb();
    if (debuggerStopsOnInvalidCondition) {
        QCOMPARE(currentMiLine(session), 29);
        session->run();
        WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
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

    qDebug() << "adding breakpoint";
    auto* const breakpoint =
        breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 30); // std::cout << i << std::endl;
    breakpoints()->removeBreakpoint(breakpoint);

    QCOMPARE(session->state(), IDebugSession::ActiveState);
#ifdef Q_OS_FREEBSD
    if (isLldb()) {
        QEXPECT_FAIL("",
                     "LLDB-MI always manages to stop at the breakpoint soon after it is added and before "
                     "it is removed, no matter how long this test function waits before adding the breakpoint",
                     Abort);
    }
#endif
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void DebuggerTestBase::testChangeBreakpointWhileRunning()
{
    auto* const session = createTestDebugSession();
    TestLaunchConfiguration cfg("debuggee_debugeeslow");
    auto* const breakpoint = breakpoints()->addCodeBreakpoint("debugeeslow.cpp:30"); // ++i;

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE_E(session, cfg);
    QCOMPARE(currentMiLine(session), 30);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::ActiveState);

    qDebug() << "Disabling breakpoint";
    breakpoint->setData(Breakpoint::EnableColumn, Qt::Unchecked);
    // to make one loop
    WAIT_FOR_A_WHILE(session, 2000);

    qDebug() << "Waiting for active";
    WAIT_FOR_STATE(session, IDebugSession::ActiveState);

    qDebug() << "Enabling breakpoint";
    breakpoint->setData(Breakpoint::EnableColumn, Qt::Checked);
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

#include "moc_debuggertestbase.cpp"
