/*
 * Common helpers for MI debugger unit tests
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "testhelper.h"

#include "midebugsession.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QtTest/QTest>

namespace KDevMI { namespace LLDB {

QUrl findExecutable(const QString& name)
{
    QFileInfo info(qApp->applicationDirPath()  + "/unittests/debugees/" + name);
    Q_ASSERT(info.exists());
    Q_ASSERT(info.isExecutable());
    return QUrl::fromLocalFile(info.canonicalFilePath());
}

// Try to find file in the same folder as `file`,
// if not found, go down to debugees folder.
QString findSourceFile(const char *file, const QString& name)
{
    QDir baseDir = QFileInfo(file).dir();
    QFileInfo info(baseDir.absoluteFilePath(name));
    if (info.exists()) {
        return info.canonicalFilePath();
    }

    baseDir.cd("debugees");
    info = baseDir.absoluteFilePath(name);
    Q_ASSERT(info.exists());
    return info.canonicalFilePath();
}

bool isAttachForbidden(const char *file, int line)
{
    // if on linux, ensure we can actually attach
    QFile canRun("/proc/sys/kernel/yama/ptrace_scope");
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

bool compareData(QModelIndex index, QString expected, const char *file, int line)
{
    QString s = index.model()->data(index, Qt::DisplayRole).toString();
    if (s != expected) {
        QTest::qFail(qPrintable(QString("'%0' didn't match expected '%1' in %2:%3")
                                .arg(s).arg(expected).arg(file).arg(line)),
                     file, line);
        return false;
    }
    return true;
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
    QTime stopWatch;
    stopWatch.start();

    // legacy behavior for tests that implicitly may require waiting for idle,
    // but which were written before waitForIdle was added
    waitForIdle = waitForIdle || state != MIDebugSession::EndedState;

    while (s && (s->state() != state
                || (waitForIdle && s->debuggerStateIsOn(s_dbgBusy)))) {
        if (stopWatch.elapsed() > 5000) {
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

} // end of namespace LLDB
} // end of namespace KDevMI
