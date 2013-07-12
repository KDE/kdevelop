/*  This file is part of KDevelop
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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

#ifndef KDEVPLATFORM_COMMANDEXECUTOR_H
#define KDEVPLATFORM_COMMANDEXECUTOR_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include "utilexport.h"

namespace KDevelop
{

/**
 * Simplifying the exeuction of a Command through (QK)Process.
 *
 * This class emits only very basic signals when the process writes something
 * to stdout or stderr and for signaling completed and failed status of running 
 * the process. This means that a process that is executed without a crash or so
 * is considered to be completed, even if it indicates an error during execution
 * using a non-zero return value. This needs to be handled by the user of the class
 * using the argument in the completed signal
 *
 * If you need more fine-grained control use (QK)Process directly and also
 * check whether you can use \ref KDevelop::ProcessLineMaker to use properly
 * terminated lines of output.
 *
 * Also this class provides only asynchronous operation, it doesn't allow to
 * wait for the program to finish.
 *
 * @author Andreas Pakulat <apaku@gmx.de>
 * TODO: Should this be a KJob??
 */
class KDEVPLATFORMUTIL_EXPORT CommandExecutor : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a command using the given executable, arguments and environment
     *
     * The process is not started immediately, instead start() has to be called.
     */
    explicit CommandExecutor( const QString& command, QObject* parent = 0 );
    ~CommandExecutor();

    /**
     * set additional arguments to be used when executing the command
     */
    void setArguments( const QStringList& args );
    /**
     * set additional environment variables to be used when executing the command
     */
    void setEnvironment( const QMap<QString,QString>& env );

    /**
     * set additional environment variables to be used when executing the command
     */
    void setEnvironment( const QStringList& env );

    /**
     * Sets the working directory of the command
     */
    void setWorkingDirectory( const QString& dir );

    /**
     * start the command, after this has been called signals may be emitted
     */
    void start();
    
    /**
     * kill the process, failed() will likely be emitted
     */
    void kill();

    /**
     * set the Command that should be started, now a commandexecutor can be reused
     */
    void setCommand( const QString& command );
    
    /**
     * whether the commands are executed from a shell
     */
    bool useShell() const;

    /**
     * if @p shell is true, the command is executed from a shell
     */
    void setUseShell( bool shell );
    
    /**
     * @returns the arguments
     */
    QStringList arguments() const;
    
    /**
     * @returns the command
     */
    QString command() const;
    
    /**
     * @returns the working directory
     */
    QString workingDirectory() const;
    
Q_SIGNALS:
    void receivedStandardError( const QStringList& );
    void receivedStandardOutput( const QStringList& );
    /**
     * Emitted when there was a severe problem executing the process, for example it
     * could not be started or crashed during execution.
     */
    void failed( QProcess::ProcessError );
    /**
     * Emitted when the process was successfully started and finished without crashing
     * The @p code parameter indicates the return value from executing the process
     */
    void completed(int code);
private:
    Q_PRIVATE_SLOT( d, void procError( QProcess::ProcessError ) )
    Q_PRIVATE_SLOT( d, void procFinished( int, QProcess::ExitStatus ) )
    class CommandExecutorPrivate* const d;
    friend class CommandExecutorPrivate;
};

}

#endif
