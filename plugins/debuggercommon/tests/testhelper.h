/*
 * Helpers for MI debugger unit tests
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

#ifndef KDEVDBG_TESTHELPER_H
#define KDEVDBG_TESTHELPER_H

#include <debugger/interfaces/idebugsession.h>
#include <interfaces/ilaunchconfiguration.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDebug>
#include <QPointer>
#include <QString>
#include <QElapsedTimer>
#include <QUrl>

class IExecutePlugin;
class QModelIndex;

#define WAIT_FOR_STATE(session, state) \
    do { if (!KDevMI::waitForState((session), (state), __FILE__, __LINE__)) return; } while (0)

#define WAIT_FOR_STATE_AND_IDLE(session, state) \
    do { if (!KDevMI::waitForState((session), (state), __FILE__, __LINE__, true)) return; } while (0)

#define WAIT_FOR(session, condition) \
    do { \
        KDevMI::TestWaiter w((session), #condition, __FILE__, __LINE__); \
        while (w.waitUnless((condition))) /* nothing */ ; \
    } while(0)

#define COMPARE_DATA(index, expected) \
    do { if (!KDevMI::compareData((index), (expected), __FILE__, __LINE__)) return; } while (0)

#define SKIP_IF_ATTACH_FORBIDDEN() \
    do { \
        if (KDevMI::isAttachForbidden(__FILE__, __LINE__)) \
            return; \
    } while(0)

namespace KDevMI {

class MIDebugSession;

QUrl findExecutable(const QString& name);
QString findSourceFile(const QString& name);
QString findFile(const char* dir, const QString& name);
bool isAttachForbidden(const char* file, int line);

bool compareData(const QModelIndex& index, const QString& expected, const char* file, int line, bool useRE = false);

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
    explicit TestLaunchConfiguration(const QString& executable)
        : TestLaunchConfiguration(findExecutable(executable), QUrl{})
    {}

    explicit TestLaunchConfiguration(
            const QUrl& executable = findExecutable(QStringLiteral("debuggee_debugee")),
            const QUrl& workingDirectory = QUrl{})
    {
        qDebug() << "FIND" << executable;
        c = KSharedConfig::openConfig();
        c->deleteGroup("launch");
        cfg = c->group("launch");
        cfg.writeEntry("isExecutable", true);
        cfg.writeEntry("Executable", executable);
        cfg.writeEntry("Working Directory", workingDirectory);
    }
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

void testEnvironmentSet(MIDebugSession* session, const QString& profileName,
                        IExecutePlugin* executePlugin);

} // end of namespace KDevMI

#endif // KDEVDBG_TESTHELPER_H
