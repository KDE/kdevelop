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

#include <QObject>
#include <QPointer>
#include <QString>
#include <QElapsedTimer>
#include <QUrl>

namespace KDevelop {
class Breakpoint;
}

class IExecutePlugin;
class QModelIndex;
class QSignalSpy;

#define WAIT_FOR_STATE(session, state) \
    do { if (!KDevMI::Testing::waitForState((session), (state), __FILE__, __LINE__)) return; } while (0)

#define WAIT_FOR_STATE_AND_IDLE(session, state) \
    do { if (!KDevMI::Testing::waitForState((session), (state), __FILE__, __LINE__, true)) return; } while (0)

/**
 * Wait for a given debug session to first enter the active state
 * then reach the paused state, and for the debugger to become idle.
 *
 * @warning The calling function must ensure that the paused state is reached, e.g. by setting a breakpoint.
 */
#define WAIT_FOR_PAUSED_STATE(session, sessionSpy)                                                                     \
    do {                                                                                                               \
        if (!KDevMI::Testing::waitForState(session, KDevelop::IDebugSession::PausedState, __FILE__, __LINE__, true,    \
                                           &sessionSpy)) {                                                             \
            return;                                                                                                    \
        }                                                                                                              \
    } while (false)

#define START_DEBUGGING_AND_WAIT_FOR_PAUSED_STATE(session, launchConfiguration, executePlugin, sessionSpy)             \
    do {                                                                                                               \
        QVERIFY(session->startDebugging(&launchConfiguration, executePlugin));                                         \
        WAIT_FOR_PAUSED_STATE(session, sessionSpy);                                                                    \
    } while (false)

#define RUN_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy, runMode)                                                    \
    do {                                                                                                               \
        KDevMI::Testing::resetAndRun(sessionSpy, session, runMode);                                                    \
        RETURN_IF_TEST_FAILED();                                                                                       \
        WAIT_FOR_PAUSED_STATE(session, sessionSpy);                                                                    \
    } while (false)

#define CONTINUE_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy)                                                        \
    RUN_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy, KDevMI::Testing::RunMode::Continue)

#define STEP_INTO_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy)                                                       \
    RUN_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy, KDevMI::Testing::RunMode::StepInto)

#define STEP_OUT_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy)                                                        \
    RUN_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy, KDevMI::Testing::RunMode::StepOut)

#define STEP_OVER_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy)                                                       \
    RUN_AND_WAIT_FOR_PAUSED_STATE(session, sessionSpy, KDevMI::Testing::RunMode::StepOver)

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

namespace KDevMI {

class MIDebugSession;

namespace Testing {

QUrl findExecutable(const QString& name);
QString findSourceFile(const QString& name);
QString findFile(const char* dir, const QString& name);
bool isAttachForbidden(const char* file, int line);

/// @return one-based MI line of a given breakpoint
int breakpointMiLine(const KDevelop::Breakpoint* breakpoint);
/// @return current one-based MI line in a given session
int currentMiLine(const KDevelop::IDebugSession* session);

bool compareData(const QModelIndex& index, const QString& expected, const char* file, int line, bool useRE = false);

/// Verify that a given thread index's frame stack model has 3 columns, an expected number of threads
/// and returns correct stack frame numbers (at column=0) for the thread index (as the parent index).
/// Check success with RETURN_IF_TEST_FAILED().
void validateColumnCountsThreadCountAndStackFrameNumbers(const QModelIndex& threadIndex, int expectedThreadCount);

/**
 * This class tracks and remembers when a debug session enters the active state.
 *
 * Using this class allows to prevent LLDB-MI test flakiness on FreeBSD.
 */
class ActiveStateSessionSpy : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a debug session spy.
     *
     * @param session a non-null debug session to track
     */
    explicit ActiveStateSessionSpy(const KDevelop::IDebugSession* session);
    /**
     * @return whether the tracked debug session has entered the active state since the last call to
     *         reset(), or since the construction of this spy if reset() has never been called on it
     */
    bool hasEnteredActiveState() const;
    /**
     * Forget about all previously entered active states.
     */
    void reset();

private:
    void sessionStateChanged(KDevelop::IDebugSession::DebuggerState state);

    bool m_hasEnteredActiveState = false;
};

enum class RunMode {
    Continue,
    StepInto,
    StepOut,
    StepOver
};

/**
 * Reset a given debug session spy and run (continue) in a specified mode a given debug session.
 *
 * @param session a non-null debug session
 *
 * @pre @p spy.hasEnteredActiveState() returns @c true
 */
void resetAndRun(ActiveStateSessionSpy& spy, KDevelop::IDebugSession* session, RunMode runMode);

/**
 * Wait until a given debug session enters a given state and (optionally) becomes idle.
 *
 * @param sessionSpy if not null, this function also waits until @p sessionSpy->hasEnteredActiveState() returns @c true
 */
bool waitForState(MIDebugSession* session, KDevelop::IDebugSession::DebuggerState state, const char* file, int line,
                  bool waitForIdle = false, const ActiveStateSessionSpy* sessionSpy = nullptr);

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

template<class DebugSession, class BaseFrameStackModel>
class TestFrameStackModel : public BaseFrameStackModel
{
public:
    explicit TestFrameStackModel(DebugSession* session)
        : BaseFrameStackModel(session)
    {
    }

    int fetchFramesCalled = 0;

protected:
    void fetchFrames(int threadNumber, int from, int to) override
    {
        fetchFramesCalled++;
        BaseFrameStackModel::fetchFrames(threadNumber, from, to);
    }
};

/**
 * This class processes and verifies output of @c debugeeslow inferior.
 */
class DebugeeslowOutputProcessor
{
    Q_DISABLE_COPY_MOVE(DebugeeslowOutputProcessor)
public:
    /**
     * Create a processor of @c debugeeslow output.
     *
     * @param outputSpy a spy that listens to the signal MIDebugSession::inferiorStdoutLines()
     *                  of a debug session that runs @c debugeeslow as the inferior
     *
     * The caller must ensure that @p outputSpy is created early enough to not miss any output of @c debugeeslow.
     * @p outputSpy must remain valid throughout the lifetime of this processor object.
     */
    explicit DebugeeslowOutputProcessor(QSignalSpy& outputSpy);

    /**
     * Verify output and clear the processed output spy.
     *
     * Call RETURN_IF_TEST_FAILED() after this function.
     */
    void processOutput();

    /**
     * @return the number of already processed output lines of @c debugeeslow
     */
    int processedLineCount() const;

private:
    QSignalSpy& m_outputSpy;
    int m_processedLineCount = 0;
};

} // namespace Testing

} // end of namespace KDevMI

#endif // KDEVDBG_TESTHELPER_H
