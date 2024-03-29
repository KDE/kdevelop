/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVDBG_TESTHELPER_H
#define KDEVDBG_TESTHELPER_H

#include <debugger/interfaces/idebugsession.h>
#include <interfaces/ilaunchconfiguration.h>

#include <KConfigGroup>
#include <KSharedConfig>

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

void testEnvironmentSet(MIDebugSession* session, const QString& profileName,
                        IExecutePlugin* executePlugin);

} // end of namespace KDevMI

#endif // KDEVDBG_TESTHELPER_H
