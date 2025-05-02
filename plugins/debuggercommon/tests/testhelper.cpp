/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testhelper.h"

#include "debuggers-tests-config.h"
#include "midebugsession.h"

#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <tests/testhelpers.h>
#include <util/environmentprofilelist.h>

#include <QAbstractItemModel>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QModelIndex>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QStringList>
#include <QTest>
#include <QVariant>

#include <algorithm>
#include <array>

using namespace KDevelop;

namespace KDevMI::Testing {

QUrl findExecutable(const QString& name)
{
    QString exeExtension;
#ifdef Q_OS_WIN
    exeExtension = QStringLiteral(".exe");
#endif
    QFileInfo info(QString::fromLocal8Bit(DEBUGGEE_BIN_DIR), name + exeExtension);
    Q_ASSERT_X(info.exists(), "findExecutable", info.filePath().toLocal8Bit());
    Q_ASSERT(info.isExecutable());
    return QUrl::fromLocalFile(info.canonicalFilePath());
}

QString findSourceFile(const QString& name)
{
    return findFile(DEBUGGEE_SRC_DIR, name);
}

QString findFile(const char* dir, const QString& name)
{
    QFileInfo info(QString::fromLocal8Bit(dir), name);
    Q_ASSERT_X(info.exists(), "findFile", info.filePath().toLocal8Bit());

    return info.canonicalFilePath();
}

bool isAttachForbidden(const char *file, int line)
{
    // if on linux, ensure we can actually attach
    QFile canRun(QStringLiteral("/proc/sys/kernel/yama/ptrace_scope"));
    if (canRun.exists()) {
        if (!canRun.open(QIODevice::ReadOnly)) {
            QTest::qFail("Something is wrong: /proc/sys/kernel/yama/ptrace_scope exists but cannot be read", file, line);
            return true;
        }
        if (canRun.read(1).toInt() != 0) {
            QTest::qSkip("ptrace attaching not allowed, skipping test. To enable it, set /proc/sys/kernel/yama/ptrace_scope to 0.", file, line);
            return true;
        }
    }

    return false;
}

QString debugeeFilePath()
{
    static const QString ret = findSourceFile(QStringLiteral("debugee.cpp"));
    return ret;
}
QUrl debugeeUrl()
{
    static const QUrl ret = QUrl::fromLocalFile(debugeeFilePath());
    return ret;
}

BreakpointModel* breakpoints()
{
    return ICore::self()->debugController()->breakpointModel();
}

Breakpoint* addDebugeeBreakpoint(int miLine)
{
    return breakpoints()->addCodeBreakpoint(debugeeUrl(), miLine - 1);
}

int breakpointMiLine(const Breakpoint* breakpoint)
{
    QVERIFY_RETURN(breakpoint, -12345);
    return breakpoint->line() + 1;
}
int currentMiLine(const IDebugSession* session)
{
    QVERIFY_RETURN(session, -12345);
    return session->currentLine() + 1;
}

bool compareData(const QModelIndex& index, const QString& expected, const char *file, int line, bool useRE)
{
    QString s = index.model()->data(index, Qt::DisplayRole).toString();
    bool matched = true;
    if (useRE) {
        QRegularExpression re(expected);
        matched = re.match(s).hasMatch();
    } else {
        matched = s == expected;
    }
    return QTest::qVerify(matched, "Comparison of data", qPrintable(QString("'%0' didn't match expected '%1' in %2:%3")
                                       .arg(s, expected, file).arg(line)),
                          file, line);
}

void validateColumnCountsThreadCountAndStackFrameNumbers(const QModelIndex& threadIndex, int expectedThreadCount)
{
    const auto* const stackModel = threadIndex.model();
    QVERIFY(stackModel);

    QCOMPARE(stackModel->columnCount(), 3);
    QCOMPARE(stackModel->columnCount(threadIndex), 3);

    QCOMPARE(stackModel->rowCount(), expectedThreadCount);

    const auto stackFrameCount = stackModel->rowCount(threadIndex);
    for (int row = 0; row < stackFrameCount; ++row) {
        COMPARE_DATA(stackModel->index(row, 0, threadIndex), QString::number(row));
    }
}

bool waitForAWhile(MIDebugSession *session, int ms, const char *file, int line)
{
    QPointer<MIDebugSession> s(session); //session can get deleted in DebugController
    QTest::qWait(ms);
    if (!s) {
        QTest::qFail("Session ended while waiting", file, line);
        return false;
    }
    return true;
}

bool waitForState(MIDebugSession *session, KDevelop::IDebugSession::DebuggerState state,
                  const char *file, int line, bool waitForIdle)
{
    QPointer<MIDebugSession> s(session); //session can get deleted in DebugController
    QElapsedTimer stopWatch;
    stopWatch.start();

    // legacy behavior for tests that implicitly may require waiting for idle,
    // but which were written before waitForIdle was added
    waitForIdle = waitForIdle || state != MIDebugSession::EndedState;

    while (s && (s->state() != state
                || (waitForIdle && s->debuggerStateIsOn(s_dbgBusy)))) {
        if (stopWatch.elapsed() > 50000) {
            qWarning() << "current state" << s->state() << "waiting for" << state;
            QTest::qFail(qPrintable(QString("Timeout before reaching state %0").arg(state)),
                         file, line);
            return false;
        }
        QTest::qWait(20);
    }

    // NOTE: don't wait anymore after leaving the loop. Waiting reenters event loop and
    // may change session state.

    if (!s && state != MIDebugSession::EndedState) {
        QTest::qFail(qPrintable(QString("Session ended before reaching state %0").arg(state)),
                        file, line);
        return false;
    }

    qDebug() << "Reached state " << state << " in " << file << ':' << line;
    return true;
}

TestWaiter::TestWaiter(MIDebugSession * session_, const char * condition_, const char * file_, int line_)
    : session(session_)
    , condition(condition_)
    , file(file_)
    , line(line_)
{
    stopWatch.start();
}

bool TestWaiter::waitUnless(bool ok)
{
    if (ok) {
        qDebug() << "Condition " << condition << " reached in " << file << ':' << line;
        return false;
    }

    if (stopWatch.elapsed() > 5000) {
        QTest::qFail(qPrintable(QString("Timeout before reaching condition %0").arg(condition)),
            file, line);
        return false;
    }

    QTest::qWait(100);

    if (!session) {
        QTest::qFail(qPrintable(QString("Session ended without reaching condition %0").arg(condition)),
            file, line);
        return false;
    }

    return true;
}

TestLaunchConfiguration::TestLaunchConfiguration(const QUrl& executable, const QUrl& workingDirectory)
{
    qDebug() << "FIND" << executable;
    c = KSharedConfig::openConfig();
    static constexpr const char* groupName = "launch";
    c->deleteGroup(groupName);
    cfg = c->group(groupName);
    cfg.writeEntry(IExecutePlugin::isExecutableEntry, true);
    cfg.writeEntry(IExecutePlugin::executableEntry, executable);
    cfg.writeEntry(IExecutePlugin::workingDirEntry, workingDirectory);
}

void startDebuggingAndWaitForPausedState(MIDebugSession* session, TestLaunchConfiguration* launchConfiguration,
                                         IExecutePlugin* executePlugin)
{
    QCOMPARE_NE(session, nullptr);
    QVERIFY(session->startDebugging(launchConfiguration, executePlugin));
    WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    if (session->currentLine() == -1) {
        qDebug() << "caught a temporary paused state, so wait again for the nontransient pause on start";
        WAIT_FOR_A_WHILE(session, 100);
        WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    }
    QCOMPARE_NE(session->currentLine(), -1);
}

namespace {
class WritableEnvironmentProfileList : public KDevelop::EnvironmentProfileList
{
public:
    explicit WritableEnvironmentProfileList(KConfig* config) : EnvironmentProfileList(config) {}

    using EnvironmentProfileList::variables;
    using EnvironmentProfileList::saveSettings;
    using EnvironmentProfileList::removeProfile;
};
} // end of namespace

void testEnvironmentSet(MIDebugSession* session, const QString& profileName,
                        IExecutePlugin* executePlugin)
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_debugeeechoenv"));

    cfg.config().writeEntry(IExecutePlugin::environmentProfileEntry, profileName);

    WritableEnvironmentProfileList envProfiles(cfg.rootConfig());
    envProfiles.removeProfile(profileName);
    auto& envs = envProfiles.variables(profileName);
    envs[QStringLiteral("VariableA")] = QStringLiteral("-A' \" complex --value");
    envs[QStringLiteral("VariableB")] = QStringLiteral("-B' \" complex --value");
    envProfiles.saveSettings(cfg.rootConfig());

    QSignalSpy outputSpy(session, &MIDebugSession::inferiorStdoutLines);

    QVERIFY(session->startDebugging(&cfg, executePlugin));
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    QVERIFY(outputSpy.count() > 0);

    QStringList outputLines;
    while (outputSpy.count() > 0) {
        const QList<QVariant> arguments = outputSpy.takeFirst();
        for (const auto& item : arguments) {
            outputLines.append(item.toStringList());
        }
    }
    QCOMPARE(outputLines, QStringList() << "-A' \" complex --value"
                                        << "-B' \" complex --value");
}

void testUnsupportedUrlExpressionBreakpoints(MIDebugSession* session, IExecutePlugin* executePlugin,
                                             bool debuggerSupportsNonAsciiExpressions)
{
    QVERIFY(session);
    QVERIFY(executePlugin);

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
    std::transform(expressions.cbegin(), expressions.cend(), bpoints.begin(), [](const char* expression) {
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

    QVERIFY(session->startDebugging(&cfg, executePlugin));
    WAIT_FOR_STATE(session, IDebugSession::EndedState);

    verifyBreakpoints(debuggerSupportsNonAsciiExpressions);
    RETURN_IF_TEST_FAILED();
}

void testBreakpointsOnNoOpLines(MIDebugSession* session, IExecutePlugin* executePlugin)
{
    TestLaunchConfiguration cfg;

    const auto* const licenseBreakpoint = addDebugeeBreakpoint(9);
    const auto* const blankLineBreakpoint = addDebugeeBreakpoint(34);
    const auto* const lastLineBreakpoint = addDebugeeBreakpoint(42);

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, executePlugin);
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
        QCOMPARE(licenseBreakpoint->state(), KDevelop::Breakpoint::CleanState);
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

void testBreakpointErrors(MIDebugSession* session, IExecutePlugin* executePlugin, bool debuggerStopsOnInvalidCondition)
{
    QVERIFY(session);
    QVERIFY(executePlugin);

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

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, executePlugin);

    if (debuggerStopsOnInvalidCondition) {
        QCOMPARE(currentMiLine(session), 29);
        session->run();
        WAIT_FOR_STATE_AND_IDLE(session, IDebugSession::PausedState);
    }

    QCOMPARE(currentMiLine(session), 30);
    session->run();

    WAIT_FOR_STATE(session, IDebugSession::EndedState);
}

void testChangeBreakpointWhileRunning(MIDebugSession* session, IExecutePlugin* executePlugin)
{
    QVERIFY(session);
    QVERIFY(executePlugin);

    TestLaunchConfiguration cfg("debuggee_debugeeslow");
    auto* const breakpoint = breakpoints()->addCodeBreakpoint("debugeeslow.cpp:30"); // ++i;

    START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, &cfg, executePlugin);
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

} // end of namespace KDevMI::Testing
