/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_COMMANDEXECUTOR_H
#define KDEVPLATFORM_COMMANDEXECUTOR_H

#include <QObject>
#include <QProcess>
#include "utilexport.h"

namespace KDevelop {
class CommandExecutorPrivate;

/**
 * Simplifying the execution of a Command through (QK)Process.
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
    explicit CommandExecutor(const QString& command, QObject* parent = nullptr);
    ~CommandExecutor() override;

    /**
     * set additional arguments to be used when executing the command
     */
    void setArguments(const QStringList& args);
    /**
     * set additional environment variables to be used when executing the command
     */
    void setEnvironment(const QMap<QString, QString>& env);

    /**
     * set additional environment variables to be used when executing the command
     */
    void setEnvironment(const QStringList& env);

    /**
     * Sets the working directory of the command
     */
    void setWorkingDirectory(const QString& dir);

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
    void setCommand(const QString& command);

    /**
     * whether the commands are executed from a shell
     */
    bool useShell() const;

    /**
     * if @p shell is true, the command is executed from a shell
     */
    void setUseShell(bool shell);

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
    void receivedStandardError(const QStringList&);
    void receivedStandardOutput(const QStringList&);
    /**
     * Emitted when there was a severe problem executing the process, for example it
     * could not be started or crashed during execution.
     */
    void failed(QProcess::ProcessError);
    /**
     * Emitted when the process was successfully started and finished without crashing
     * The @p code parameter indicates the return value from executing the process
     */
    void completed(int code);

private:
    const QScopedPointer<class CommandExecutorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(CommandExecutor)
    friend class CommandExecutorPrivate;
};

}

#endif
