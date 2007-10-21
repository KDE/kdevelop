/* KDevelop Util Library
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "commandexecutor.h"

#include "processlinemaker.h"
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <kprocess.h>

namespace KDevelop
{

class CommandExecutorPrivate
{
public:
    CommandExecutorPrivate( CommandExecutor* cmd )
        : m_exec(cmd)
    {
    }
    CommandExecutor* m_exec;
    KProcess* m_process;
    ProcessLineMaker* m_lineMaker;
    QString m_command;
    QStringList m_args;
    QString m_workDir;
    QMap<QString,QString> m_env;
    void procError( QProcess::ProcessError error )
    {
        Q_UNUSED(error)
        emit m_exec->failed();
    }
    void procFinished( int code, QProcess::ExitStatus status )
    {
        Q_UNUSED(code)
        if( status == QProcess::NormalExit )
            emit m_exec->completed();
        else
            emit m_exec->failed();
    }
};

CommandExecutor::CommandExecutor( const QString& command, QObject* parent )
  : QObject(parent), d(new CommandExecutorPrivate(this))
{
    d->m_process = new KProcess(this);
    d->m_process->setOutputChannelMode( KProcess::SeparateChannels );
    d->m_lineMaker = new ProcessLineMaker( d->m_process );
    d->m_command = command;
    connect( d->m_lineMaker, SIGNAL(receivedStdoutLines( const QStringList& ) ),
             this, SIGNAL( receivedStandardOutput( const QStringList& ) ) );
    connect( d->m_lineMaker, SIGNAL(receivedStderrLines( const QStringList& ) ),
             this, SIGNAL( receivedStandardError( const QStringList& ) ) );
    connect( d->m_process, SIGNAL( error( QProcess::ProcessError ) ),
             this, SLOT( procError( QProcess::ProcessError ) ) );
    connect( d->m_process, SIGNAL( finished( int, QProcess::ExitStatus ) ),
             this, SLOT( procFinished( int, QProcess::ExitStatus ) ) );
}


CommandExecutor::~CommandExecutor()
{
    delete d->m_lineMaker;
    delete d;
}

void CommandExecutor::setEnvironment( const QMap<QString,QString>& env )
{
    d->m_env = env;
}

void CommandExecutor::setArguments( const QStringList& args )
{
    d->m_args = args;
}

void CommandExecutor::setWorkingDirectory( const QString& dir )
{
    d->m_workDir = dir;
}

void CommandExecutor::start()
{
    Q_FOREACH( QString s, d->m_env.keys() )
    {
        d->m_process->setEnv( s, d->m_env[s] );
    }
    d->m_process->setWorkingDirectory( d->m_workDir );
    d->m_process->setProgram( d->m_command, d->m_args );
    d->m_process->start();
}

void CommandExecutor::setCommand( const QString& command )
{
    d->m_command = command;
}

}

#include "commandexecutor.moc"
