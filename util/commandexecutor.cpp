/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "commandexecutor.h"

#include "processlinemaker.h"
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <kprocess.h>
#include <kshell.h>

namespace KDevelop
{

class CommandExecutorPrivate
{
public:
    CommandExecutorPrivate( CommandExecutor* cmd )
        : m_exec(cmd), m_useShell(false)
    {
    }
    CommandExecutor* m_exec;
    KProcess* m_process;
    ProcessLineMaker* m_lineMaker;
    QString m_command;
    QStringList m_args;
    QString m_workDir;
    QMap<QString,QString> m_env;
    bool m_useShell;
    void procError( QProcess::ProcessError error )
    {
        Q_UNUSED(error)
        m_lineMaker->flushBuffers();
        emit m_exec->failed( error );
    }
    void procFinished( int code, QProcess::ExitStatus status )
    {
        m_lineMaker->flushBuffers();
        if( status == QProcess::NormalExit )
            emit m_exec->completed(code);
    }
};

CommandExecutor::CommandExecutor( const QString& command, QObject* parent )
  : QObject(parent), d(new CommandExecutorPrivate(this))
{
    d->m_process = new KProcess(this);
    d->m_process->setOutputChannelMode( KProcess::SeparateChannels );
    d->m_lineMaker = new ProcessLineMaker( d->m_process );
    d->m_command = command;
    connect( d->m_lineMaker, &ProcessLineMaker::receivedStdoutLines,
             this, &CommandExecutor::receivedStandardOutput );
    connect( d->m_lineMaker, &ProcessLineMaker::receivedStderrLines,
             this, &CommandExecutor::receivedStandardError );
    connect( d->m_process, static_cast<void(KProcess::*)(QProcess::ProcessError)>(&KProcess::error),
             this, [&] (QProcess::ProcessError error) { d->procError(error); } );
    connect( d->m_process, static_cast<void(KProcess::*)(int,QProcess::ExitStatus)>(&KProcess::finished),
             this, [&] (int code, QProcess::ExitStatus status) { d->procFinished(code, status); } );
}


CommandExecutor::~CommandExecutor()
{
    delete d->m_process;
    delete d->m_lineMaker;
    delete d;
}

void CommandExecutor::setEnvironment( const QMap<QString,QString>& env )
{
    d->m_env = env;
}

void CommandExecutor::setEnvironment( const QStringList& env )
{
    QMap<QString,QString> envmap;
    foreach( const QString& var, env )
    {
        int sep = var.indexOf( '=' );
        envmap.insert( var.left( sep ), var.mid( sep + 1 ) );
    }
    d->m_env = envmap;
}

void CommandExecutor::setArguments( const QStringList& args )
{
    d->m_args = args;
}

void CommandExecutor::setWorkingDirectory( const QString& dir )
{
    d->m_workDir = dir;
}

bool CommandExecutor::useShell() const
{
    return d->m_useShell;
}

void CommandExecutor::setUseShell( bool shell )
{
    d->m_useShell = shell;
}

void CommandExecutor::start()
{
    Q_FOREACH( const QString &s, d->m_env.keys() )
    {
        d->m_process->setEnv( s, d->m_env[s] );
    }
    d->m_process->setWorkingDirectory( d->m_workDir );
    if( !d->m_useShell ) {
        d->m_process->setProgram( d->m_command, d->m_args );
    } else {
        QStringList arguments;
        Q_FOREACH( const QString &a, d->m_args ) arguments << KShell::quoteArg( a );
        d->m_process->setShellCommand( d->m_command + ' ' + arguments.join( " " ) );
    }

    d->m_process->start();
}

void CommandExecutor::setCommand( const QString& command )
{
    d->m_command = command;
}

void CommandExecutor::kill()
{
    d->m_process->kill();
}

QString CommandExecutor::command() const
{
    return d->m_command;
}

QStringList CommandExecutor::arguments() const
{
    return d->m_args;
}

QString CommandExecutor::workingDirectory() const
{
    return d->m_workDir;
}

}

#include "moc_commandexecutor.cpp"
