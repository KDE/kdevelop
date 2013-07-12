/*
* GDB Debugger Support
*
* Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
* Copyright 2007 Hamish Rodda <rodda@kde.org>
* Copyright 2009 Andreas Pakulat <apaku@gmx.de>
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
#ifndef GDBDEBUGJOB
#define GDBDEBUGJOB

#include <outputview/outputjob.h>

class IExecutePlugin;
namespace KDevelop
{
class OutputModel;
class ILaunchConfiguration;
}

namespace GDBDebugger
{
class CppDebuggerPlugin;
class DebugSession;


class DebugJob : public KDevelop::OutputJob
{
Q_OBJECT
public:
    DebugJob( GDBDebugger::CppDebuggerPlugin* p, KDevelop::ILaunchConfiguration* launchcfg,
              IExecutePlugin* plugin, QObject* parent = 0 );
    virtual void start();
protected:
    virtual bool doKill();
private slots:
    void stdoutReceived(const QStringList&);
    void stderrReceived(const QStringList&);
    void done();
private:
    KDevelop::OutputModel* model();
    DebugSession* m_session;
    KDevelop::ILaunchConfiguration* m_launchcfg;
    IExecutePlugin* m_execute;
};

//this job is just here to be able to kill the debug session
class KillSessionJob : public KJob
{
Q_OBJECT
public:
    KillSessionJob(DebugSession *session, QObject *parent = 0);
    virtual void start();
protected:
    virtual bool doKill();
private:
    DebugSession* m_session;
private slots:
    void sessionFinished();
};

}

#endif
