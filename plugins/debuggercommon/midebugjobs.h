/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MIDEBUGJOBS_H
#define MIDEBUGJOBS_H

#include <outputview/outputjob.h>

#include <QUrl>

#include <memory>

class IExecutePlugin;
namespace KDevelop
{
class ILaunchConfiguration;
}

namespace KDevMI {

struct InferiorStartupInfo;
class MIDebuggerPlugin;
class MIDebugSession;

template<class JobBase>
class MIDebugJobBase : public JobBase
{
public:
    explicit MIDebugJobBase(MIDebuggerPlugin* plugin, QObject* parent);
    ~MIDebugJobBase() override;

protected:
    /**
     * Finish this job without stopping the debug session.
     *
     * Call this function when the debug session finishes or fails to start.
     */
    void done();

    /**
     * Cease tracking the state of the debug session and stop its debugger.
     *
     * The caller of this function must finish the job by means other than the usual debug session end.
     */
    void stopDebugger();

    bool doKill() override;

    MIDebugSession* m_session;
};

class MIDebugJob : public MIDebugJobBase<KDevelop::OutputJob>
{
    Q_OBJECT
public:
    enum {
        // Add a "random" number to KJob::UserDefinedError and hopefully avoid clashes with OutputJob's error codes.
        InvalidExecutable = UserDefinedError + 231,
        ExecutableIsNotExecutable,
        InvalidArguments,
        InvalidExternalTerminal,
    };

    MIDebugJob(MIDebuggerPlugin* p, KDevelop::ILaunchConfiguration* launchcfg, IExecutePlugin* plugin,
             QObject* parent = nullptr);
    void start() override;

private:
    void initializeStartupInfo(IExecutePlugin* execute, KDevelop::ILaunchConfiguration* launchConfiguration);

    std::unique_ptr<InferiorStartupInfo> m_startupInfo;
};

class MIExamineCoreJob : public MIDebugJobBase<KJob>
{
    Q_OBJECT
public:
    struct CoreInfo
    {
        QUrl executableFile;
        QUrl coreFile;
    };

    explicit MIExamineCoreJob(MIDebuggerPlugin* plugin, CoreInfo coreInfo, QObject* parent = nullptr);

    void start() override;

private:
    CoreInfo m_coreInfo;
};

class MIAttachProcessJob : public MIDebugJobBase<KJob>
{
    Q_OBJECT
public:
    MIAttachProcessJob(MIDebuggerPlugin *plugin, int pid, QObject *parent = nullptr);

    void start() override;

private:
    int m_pid;
};

} // end of namespace KDevMI

#endif // MIDEBUGJOBS_H
