/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MIDEBUGJOBS_H
#define MIDEBUGJOBS_H

#include <outputview/outputjob.h>

class IExecutePlugin;
namespace KDevelop
{
class OutputModel;
class ILaunchConfiguration;
}

namespace KDevMI {

class MIDebuggerPlugin;
class MIDebugSession;

class MIDebugJob : public KDevelop::OutputJob
{
    Q_OBJECT
public:
    MIDebugJob(MIDebuggerPlugin* p, KDevelop::ILaunchConfiguration* launchcfg, IExecutePlugin* plugin,
             QObject* parent = nullptr);
    void start() override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void stdoutReceived(const QStringList&);
    void stderrReceived(const QStringList&);
    void done();

private:
    KDevelop::OutputModel* model();

    MIDebugSession* m_session;
    KDevelop::ILaunchConfiguration* m_launchcfg;
    IExecutePlugin* m_execute;
};

class MIExamineCoreJob : public KJob
{
    Q_OBJECT
public:
    explicit MIExamineCoreJob(MIDebuggerPlugin *plugin, QObject *parent = nullptr);

    void start() override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void done();

private:
    MIDebugSession *m_session;
};

class MIAttachProcessJob : public KJob
{
    Q_OBJECT
public:
    MIAttachProcessJob(MIDebuggerPlugin *plugin, int pid, QObject *parent = nullptr);

    void start() override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void done();

private:
    int m_pid;
    MIDebugSession *m_session;
};

} // end of namespace KDevMI

#endif // MIDEBUGJOBS_H
