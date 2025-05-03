/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVDBG_TESTHELPER_H
#define KDEVDBG_TESTHELPER_H

#include <debugger/interfaces/idebugsession.h>
#include <interfaces/ilaunchconfiguration.h>
#include <tests/testhelpermacros.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QPointer>
#include <QString>
#include <QElapsedTimer>
#include <QUrl>

namespace KDevelop {
class Breakpoint;
class BreakpointModel;
}

class IExecutePlugin;
class QModelIndex;

#define WAIT_FOR_STATE(session, state) \
    do { if (!KDevMI::Testing::waitForState((session), (state), __FILE__, __LINE__)) return; } while (0)

#define WAIT_FOR_STATE_AND_IDLE(session, state) \
    do { if (!KDevMI::Testing::waitForState((session), (state), __FILE__, __LINE__, true)) return; } while (0)

#define WAIT_FOR_A_WHILE(session, ms)                                                                                  \
    do {                                                                                                               \
        if (!KDevMI::Testing::waitForAWhile((session), (ms), __FILE__, __LINE__))                                      \
            return;                                                                                                    \
    } while (false)

#define WAIT_FOR(session, condition) \
    do { \
        KDevMI::Testing::TestWaiter w((session), #condition, __FILE__, __LINE__); \
        while (w.waitUnless((condition))) /* nothing */ ; \
    } while(0)

#define COMPARE_DATA(index, expected) \
    do { if (!KDevMI::Testing::compareData((index), (expected), __FILE__, __LINE__)) return; } while (0)

#define SKIP_IF_ATTACH_FORBIDDEN() \
    do { \
        if (KDevMI::Testing::isAttachForbidden(__FILE__, __LINE__)) \
            return; \
    } while(0)

#define START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, launchConfiguration, executePlugin)                         \
    do {                                                                                                               \
        KDevMI::Testing::startDebuggingAndWaitForPausedState((session), (launchConfiguration), (executePlugin));       \
        RETURN_IF_TEST_FAILED();                                                                                       \
    } while (false)

namespace KDevMI {

class MIDebugSession;

namespace Testing {

QUrl findExecutable(const QString& name);
QString findSourceFile(const QString& name);
QString findFile(const char* dir, const QString& name);
bool isAttachForbidden(const char* file, int line);

/// @return the path to the test file debugee.cpp
QString debugeeFilePath();
/// @return the URL of the test file debugee.cpp
QUrl debugeeUrl();

KDevelop::BreakpointModel* breakpoints();

/// Add a code breakpoint to debugee.cpp at a given one-based MI line.
KDevelop::Breakpoint* addDebugeeBreakpoint(int miLine);

/// @return one-based MI line of a given breakpoint
int breakpointMiLine(const KDevelop::Breakpoint* breakpoint);
/// @return current one-based MI line in a given session
int currentMiLine(const KDevelop::IDebugSession* session);

bool compareData(const QModelIndex& index, const QString& expected, const char* file, int line, bool useRE = false);

/// Verify that a given thread index's frame stack model has 3 columns, an expected number of threads
/// and returns correct stack frame numbers (at column=0) for the thread index (as the parent index).
/// Check success with RETURN_IF_TEST_FAILED().
void validateColumnCountsThreadCountAndStackFrameNumbers(const QModelIndex& threadIndex, int expectedThreadCount);

bool waitForState(MIDebugSession* session, KDevelop::IDebugSession::DebuggerState state, const char* file, int line,
                  bool waitForIdle = false);

bool waitForAWhile(MIDebugSession* session, int ms, const char* file, int line);

class TestWaiter
{
public:
    TestWaiter(MIDebugSession* session_, const char* condition_, const char* file_, int line_);

    bool waitUnless(bool ok);

private:
    QElapsedTimer stopWatch;
    QPointer<MIDebugSession> session;
    const char* condition;
    const char* file;
    int line;
};

class TestLaunchConfiguration : public KDevelop::ILaunchConfiguration
{
public:
    explicit TestLaunchConfiguration(const QString& executable = QStringLiteral("debuggee_debugee"),
                                     const QUrl& workingDirectory = QUrl{})
        : TestLaunchConfiguration(findExecutable(executable), workingDirectory)
    {}

    explicit TestLaunchConfiguration(const QUrl& executable, const QUrl& workingDirectory);
    const KConfigGroup config() const override { return cfg; }
    KConfigGroup config() override { return cfg; };
    QString name() const override { return QStringLiteral("Test-Launch"); }
    KDevelop::IProject* project() const override { return nullptr; }
    KDevelop::LaunchConfigurationType* type() const override { return nullptr; }

    KConfig* rootConfig() { return c.data(); }
private:
    KConfigGroup cfg;
    KSharedConfigPtr c;
};

/**
 * Start debugging of a given session with given launch configuration and execute plugin,
 * then wait for the session to reach the paused state and for the debugger to become idle.
 *
 * @param session a non-null debug session
 *
 * The calling function must ensure that the paused state is reached, e.g. by setting a breakpoint.
 */
void startDebuggingAndWaitForPausedState(KDevMI::MIDebugSession* session, TestLaunchConfiguration* launchConfiguration,
                                         IExecutePlugin* executePlugin);

void testEnvironmentSet(MIDebugSession* session, const QString& profileName,
                        IExecutePlugin* executePlugin);

void testUnsupportedUrlExpressionBreakpoints(MIDebugSession* session, IExecutePlugin* executePlugin,
                                             bool debuggerSupportsNonAsciiExpressions);

void testBreakpointsOnNoOpLines(MIDebugSession* session, IExecutePlugin* executePlugin,
                                bool debuggerMovesBreakpointFromLicenseNotice);

void testBreakpointErrors(MIDebugSession* session, IExecutePlugin* executePlugin, bool debuggerStopsOnInvalidCondition);

} // namespace Testing

} // end of namespace KDevMI

#endif // KDEVDBG_TESTHELPER_H
