/*
 * Common helpers for MI debugger unit tests
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
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

#include "debuggers-tests-config.h"
#include "midebugsession.h"

#include <execute/iexecuteplugin.h>
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

namespace KDevMI {

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
        if (stopWatch.elapsed() > 10000) {
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

} // end of namespace KDevMI
