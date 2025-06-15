/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testhelper.h"

#include "debuggers-tests-config.h"
#include "midebugsession.h"

#include <debugger/breakpoint/breakpoint.h>
#include <execute/iexecuteplugin.h>

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

ActiveStateSessionSpy::ActiveStateSessionSpy(const IDebugSession* session)
{
    QVERIFY(session);
    connect(session, &IDebugSession::stateChanged, this, &ActiveStateSessionSpy::sessionStateChanged);
}

bool ActiveStateSessionSpy::hasEnteredActiveState() const
{
    return m_hasEnteredActiveState;
}

void ActiveStateSessionSpy::reset()
{
    m_hasEnteredActiveState = false;
}

void ActiveStateSessionSpy::sessionStateChanged(IDebugSession::DebuggerState state)
{
    if (state == IDebugSession::ActiveState) {
        m_hasEnteredActiveState = true;
    }
}

void resetAndRun(ActiveStateSessionSpy& spy, IDebugSession* session)
{
    QVERIFY(session);
    QVERIFY(spy.hasEnteredActiveState());
    spy.reset();
    session->run();
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

bool waitForState(MIDebugSession* session, KDevelop::IDebugSession::DebuggerState state, const char* file, int line,
                  bool waitForIdle, const ActiveStateSessionSpy* sessionSpy)
{
    QPointer<MIDebugSession> s(session); //session can get deleted in DebugController
    QElapsedTimer stopWatch;
    stopWatch.start();

    // legacy behavior for tests that implicitly may require waiting for idle,
    // but which were written before waitForIdle was added
    waitForIdle = waitForIdle || state != MIDebugSession::EndedState;

    const auto areWaitConditionsSatisfied = [session, state, sessionSpy, waitForIdle] {
        return session->state() == state && !(waitForIdle && session->debuggerStateIsOn(s_dbgBusy))
            && (!sessionSpy || sessionSpy->hasEnteredActiveState());
    };

    const auto timeout = waitForIdle ? 50'000 : 10'000;

    while (s && !areWaitConditionsSatisfied()) {
        if (stopWatch.elapsed() > timeout) {
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

DebugeeslowOutputProcessor::DebugeeslowOutputProcessor(QSignalSpy& outputSpy)
    : m_outputSpy{outputSpy}
{
    QVERIFY(m_outputSpy.isValid());
}

void DebugeeslowOutputProcessor::processOutput()
{
    for (const auto& parameters : std::as_const(m_outputSpy)) {
        QCOMPARE_EQ(parameters.size(), 1); // MIDebugSession::inferiorStdoutLines() has one parameter
        const auto outputLines = parameters.constFirst().toStringList();
        qDebug() << "Debuggee output:" << outputLines.join('\n');
        QCOMPARE_GT(outputLines.size(), 0); // why emit the signal if there is no output?

        for (const auto& line : outputLines) {
            ++m_processedLineCount;
            // debugeeslow outputs consecutive integers starting from 1
            QCOMPARE(line, QString::number(m_processedLineCount));
        }
    }
    m_outputSpy.clear();
}

int DebugeeslowOutputProcessor::processedLineCount() const
{
    return m_processedLineCount;
}

} // end of namespace KDevMI::Testing

#include "moc_testhelper.cpp"
