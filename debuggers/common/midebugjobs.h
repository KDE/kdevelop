/*
* Common Code for Debugger Support
*
* Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
* Copyright 2007 Hamish Rodda <rodda@kde.org>
* Copyright 2009 Andreas Pakulat <apaku@gmx.de>
* Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
             QObject* parent = 0);
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
    MIExamineCoreJob(MIDebuggerPlugin *plugin, QObject *parent = nullptr);

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
