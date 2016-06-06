/*
 * Common code for debugger support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef MIDEBUGSESSION_H
#define MIDEBUGSESSION_H

#include <debugger/interfaces/idebugsession.h>

#include "dbgglobal.h"
#include "mibreakpointcontroller.h"
#include "mi/mi.h"
#include "mi/micommand.h"

#include <memory>

class IExecutePlugin;
namespace KDevelop {
class ILaunchConfiguration;
class ProcessLineMaker;
}

namespace KDevMI {

namespace MI {
class CommandQueue;
}

class MIDebugger;
class STTY;
class MIDebugSession : public KDevelop::IDebugSession
{
    Q_OBJECT
public:
    MIDebugSession();
    ~MIDebugSession() override;

Q_SIGNALS:
    /**
     * Emits when received standard output lines from inferior
     */
    void inferiorStdoutLines(const QStringList &lines);

    /**
     * Emits when received standard error lines from inferior
     */
    void inferiorStderrLines(const QStringList &lines);

    void inferiorStopped(const MI::AsyncRecord &r);

    void inferiorRunning();

    /**
     * Emits when received standard output from debugger for user commands
     */
    void debuggerUserCommandOutput(const QString &output);

    /**
     * Emits when received standard output from debugger for internal commands
     */
    void debuggerInternalCommandOutput(const QString &output);

    /**
     * Emits when received standard output from inferior's tty
     */
    void inferiorTtyStdout(const QByteArray &output);

    /**
     * Emits when received standard output from inferior's tty
     */
    void inferiorTtyStderr(const QByteArray &output);

    /**
     * Emits when the debugger instance state changes
     */
    void debuggerStateChanged(DBGStateFlags oldState, DBGStateFlags newState);

    /**
     * Emits when there's message needed to be show to user.
     */
    void showMessage(const QString& message, int timeout);

    /**
     * Emits when the debugger console view need to be raised.
     */
    void raiseDebuggerConsoleViews();

    /**
     * Emits when need to reset
     */
    void reset();

public:
    bool debuggerStateIsOn(DBGStateFlags state) const;
    DBGStateFlags debuggerState() const;

    bool hasCrashed() const;

// BEGIN IDebugSession overrides
public:
    DebuggerState state() const override;
    bool restartAvaliable() const override;

    MIBreakpointController * breakpointController() const override = 0;

public Q_SLOTS:
    void restartDebugger() override;
    void stopDebugger() override;
    void interruptDebugger() override;
    void run() override;
    void runToCursor() override;
    void jumpToCursor() override;
    void stepOver() override;
    void stepIntoInstruction() override;
    void stepInto() override;
    void stepOverInstruction() override;
    void stepOut() override;
// END IDebugSession overrides

public Q_SLOTS:
    /**
     * Run currently executing program to the given \a url and \a line.
     */
    void runUntil(const QUrl& url, int line);

    /**
     * Run currently executing program to the given \a address
     */
    void runUntil(const QString& address);

    /**
     * Move the execution point of the currently executing program to the given \a url and \a line.
     */
    void jumpTo(const QUrl& url, int line);

    /**
     * Move the execution point of the currently executing program to the given \a address.
     *Note: It can be really very dangerous, so use jumpTo instead.
     */
    void jumpToMemoryAddress(const QString& address);

    /**
     * Start the debugger, and execute the inferior program specified by \a cfg.
     */
    bool startDebugging(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec);

    /**
     * Start the debugger, and examine the core file given by \a coreFile.
     */
    bool examineCoreFile(const QUrl &debugee, const QUrl &coreFile);

    /**
     * Start the debugger, and attach to a currently running process with the given \a pid.
     */
    bool attachToProcess(int pid);

public:
    virtual MI::MICommand *createCommand(MI::CommandType type, const QString& arguments,
                                         MI::CommandFlags flags = 0) const;
    /** Adds a command to the end of queue of commands to be executed
        by gdb. The command will be actually sent to gdb only when
        replies from all previous commands are received and full processed.

        The literal command sent to gdb is obtained by calling
        cmd->cmdToSend. The call is made immediately before sending the
        command, so it's possible to use results of prior commands when
        computing the exact command to send.
    */
    void addUserCommand(const QString &cmd);

    void addCommand(MI::MICommand* cmd);

    /** Same as above, but internally constructs MICommand using createCommand() */
    void addCommand(MI::CommandType type, const QString& arguments = QString(),
                    MI::CommandFlags flags = 0);

    void addCommand(MI::CommandType type, const QString& arguments,
                    MI::MICommandHandler* handler,
                    MI::CommandFlags flags = 0);

    void addCommand(MI::CommandType type, const QString& arguments,
                    const MI::FunctionCommandHandler::Function& callback,
                    MI::CommandFlags flags = 0);

    template<class Handler>
    void addCommand(MI::CommandType type, const QString& arguments,
                    Handler* handler_this,
                    void (Handler::* handler_method)(const MI::ResultRecord&),
                    MI::CommandFlags flags = 0);

protected Q_SLOTS:
    virtual void slotDebuggerReady();
    virtual void slotDebuggerExited(bool abnormal, const QString &msg);
    virtual void slotInferiorStopped(const MI::AsyncRecord &r);
    /**
     * Triggered every time program begins/continues it's execution.
     */
    virtual void slotInferiorRunning();

    /**
     * Handle MI async notifications.
     */
    virtual void processNotification(const MI::AsyncRecord &n);

    /** Default handler for errors.
        Tries to guess is the error message is telling that target is
        gone, if so, informs the user.
        Otherwise, shows a dialog box and reloads view state.  */
    virtual void defaultErrorHandler(const MI::ResultRecord &result);

    /**
     * Update session state when debugger state changes, and show messages
     */
    virtual void handleDebuggerStateChange(DBGStateFlags oldState, DBGStateFlags newState);

    void handleNoInferior(const QString &msg);
    void handleInferiorFinished(const QString &msg);

protected:
    void queueCmd(MI::MICommand *cmd);

    /** Try to execute next command in the queue.  If GDB is not
        busy with previous command, and there's a command in the
        queue, sends it.  */
    void executeCmd();
    void ensureDebuggerListening();
    void destroyCmds();

    /**
     * Start the debugger instance
     */
    bool startDebugger(KDevelop::ILaunchConfiguration *cfg);

    /**
     * MIDebugSession takes the ownership of the created instance.
     */
    virtual MIDebugger *createDebugger() const = 0;

    /**
     * Initialize debugger and set default configurations.
     */
    virtual void initializeDebugger() = 0;

    /**
     * Further config the debugger and start the inferior program (either local or remote).
     */
    virtual bool execInferior(KDevelop::ILaunchConfiguration *cfg, const QString &executable) = 0;

    /**
     * Manipulate debugger instance state
     */
    void setDebuggerStateOn(DBGStateFlags stateOn);
    void setDebuggerStateOff(DBGStateFlags stateOff);
    void setDebuggerState(DBGStateFlags newState);

    void debuggerStateChange(DBGStateFlags oldState, DBGStateFlags newState);

    /**
     * Manipulate the session state
     */
    void setSessionState(DebuggerState state);

    void raiseEvent(event_t e) override;

    /** Called when there are no pending commands and 'm_stateReloadNeeded'
        is true. Also can be used to immediately reload program state.
        Issues commands to completely reload all program state shown
        to the user.
    */
    void reloadProgramState();

    void programNoApp(const QString &msg);
    void programFinished(const QString &msg);

    // FIXME: Whether let the debugger source init files when starting,
    // only used in unit test currently, potentially could be made a user
    // configurable option
    void setSourceInitFile(bool enable);

private Q_SLOTS:
    void handleTargetAttach(const MI::ResultRecord& r);
    void handleCoreFile(const MI::ResultRecord& r);
    // Pops up a dialog box with some hopefully
    // detailed information about which state debugger
    // is in, which commands were sent and so on.
    void explainDebuggerStatus();

protected:
    KDevelop::ProcessLineMaker *m_procLineMaker;

    std::unique_ptr<MI::CommandQueue> m_commandQueue;

    // Though the misleading class name, this is the session level state.
    // see m_debuggerState for debugger instance state
    DebuggerState m_sessionState;

    MIDebugger *m_debugger;
    DBGStateFlags m_debuggerState;

    bool m_stateReloadInProgress;
    bool m_stateReloadNeeded;

    std::unique_ptr<STTY> m_tty;

    bool m_hasCrashed;
    bool m_sourceInitFile;
};

template<class Handler>
void MIDebugSession::addCommand(MI::CommandType type, const QString& arguments,
                                Handler* handler_this,
                                void (Handler::* handler_method)(const MI::ResultRecord&),
                                MI::CommandFlags flags)
{
    auto cmd = createCommand(type, arguments, flags);
    cmd->setHandler(handler_this, handler_method);
    queueCmd(cmd);
}

} // end of namespace KDevMI

#endif // MIDEBUGSESSION_H
