/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MIDEBUGSESSION_H
#define MIDEBUGSESSION_H

#include <debugger/interfaces/idebugsession.h>

#include "dbgglobal.h"
#include "mibreakpointcontroller.h"
#include "mi/mi.h"
#include "mi/micommand.h"

#include <QMap>
#include <QPointer>

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
class MIDebuggerPlugin;
class MIVariable;
class STTY;
class MIDebugSession : public KDevelop::IDebugSession
{
    Q_OBJECT
public:
    explicit MIDebugSession(MIDebuggerPlugin *plugin = nullptr);
    ~MIDebugSession() override;

Q_SIGNALS:
    /**
     * Emits when received standard output lines from inferior
     */
    void inferiorStdoutLines(const QStringList& lines);

    /**
     * Emits when received standard error lines from inferior
     */
    void inferiorStderrLines(const QStringList& lines);

    void inferiorStopped(const MI::AsyncRecord& r);

    void inferiorRunning();

    /**
     * Emits when received standard output from debugger for user commands
     */
    void debuggerUserCommandOutput(const QString &output);

    /**
     * Emits when received standard output from debugger for internal commands
     */
    void debuggerInternalCommandOutput(const QString& output);

    /**
     * Emits when received internal output from debugger
     */
    void debuggerInternalOutput(const QString& output) const;

    /**
     * Emits when received standard output from inferior's tty
     */
    void inferiorTtyStdout(const QByteArray& output);

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
    void killDebuggerNow() override;
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
    virtual std::unique_ptr<MI::MICommand> createCommand(MI::CommandType type, const QString& arguments,
                                                         MI::CommandFlags flags = {}) const;
    virtual std::unique_ptr<MI::MICommand> createUserCommand(const QString& cmd) const;
    /** Adds a command to the end of queue of commands to be executed
        by debugger. The command will be actually sent to debugger only when
        replies from all previous commands are received and full processed.

        The literal command sent to debugger is obtained by calling
        cmd->cmdToSend. The call is made immediately before sending the
        command, so it's possible to use results of prior commands when
        computing the exact command to send.
    */
    void addUserCommand(const QString &cmd);

    void addCommand(std::unique_ptr<MI::MICommand> cmd);

    /** Same as above, but internally constructs MICommand using createCommand() */
    void addCommand(MI::CommandType type, const QString& arguments = QString(),
                    MI::CommandFlags flags = {});

    void addCommand(MI::CommandType type, const QString& arguments,
                    MI::MICommandHandler* handler,
                    MI::CommandFlags flags = {});

    void addCommand(MI::CommandType type, const QString& arguments,
                    const MI::FunctionCommandHandler::Function& callback,
                    MI::CommandFlags flags = {});

    template<class Handler>
    void addCommand(MI::CommandType type, const QString& arguments,
                    Handler* handler_this,
                    void (Handler::* handler_method)(const MI::ResultRecord&),
                    MI::CommandFlags flags = {});

    QMap<QString, MIVariable*> & variableMapping();
    MIVariable* findVariableByVarobjName(const QString &varobjName) const;
    void markAllVariableDead();

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

protected:
    void queueCmd(std::unique_ptr<MI::MICommand> cmd);

    /** Try to execute next command in the queue.  If GDB is not
        busy with previous command, and there's a command in the
        queue, sends it.  */
    void executeCmd();
    void destroyCmds();

    virtual void ensureDebuggerListening();

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
     * Do per launch configuration.
     */
    virtual void configInferior(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec,
                                const QString &executable) = 0;

    /**
     * Start the inferior program (either local or remote).
     */
    virtual bool execInferior(KDevelop::ILaunchConfiguration *cfg, IExecutePlugin *iexec,
                              const QString &executable) = 0;

    /**
     * Further config the debugger and load the core dump
     */
    virtual bool loadCoreFile(KDevelop::ILaunchConfiguration *cfg,
                              const QString &debugee, const QString &corefile) = 0;

    /**
     * Manipulate debugger instance state
     */
    void setDebuggerStateOn(DBGStateFlags stateOn);
    void setDebuggerStateOff(DBGStateFlags stateOff);
    void setDebuggerState(DBGStateFlags newState);

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
    // Pops up a dialog box with some hopefully
    // detailed information about which state debugger
    // is in, which commands were sent and so on.
    void explainDebuggerStatus();

protected:
    KDevelop::ProcessLineMaker *m_procLineMaker;

    std::unique_ptr<MI::CommandQueue> m_commandQueue;

    // Though the misleading class name, this is the session level state.
    // see m_debuggerState for debugger instance state
    DebuggerState m_sessionState = NotStartedState;

    MIDebugger *m_debugger = nullptr;
    DBGStateFlags m_debuggerState;

    bool m_stateReloadInProgress = false;
    bool m_stateReloadNeeded = false;

    std::unique_ptr<STTY> m_tty;

    bool m_hasCrashed = false;
    bool m_sourceInitFile = true;

    // Map from GDB varobj name to MIVariable.
    QMap<QString, MIVariable*> m_allVariables;

    QPointer<MIDebuggerPlugin> m_plugin;

private:
    void addGdbExitCommand();
    void killDebuggerImpl();
};

template<class Handler>
void MIDebugSession::addCommand(MI::CommandType type, const QString& arguments,
                                Handler* handler_this,
                                void (Handler::* handler_method)(const MI::ResultRecord&),
                                MI::CommandFlags flags)
{
    auto cmd = createCommand(type, arguments, flags);
    cmd->setHandler(handler_this, handler_method);
    queueCmd(std::move(cmd));
}

} // end of namespace KDevMI

#endif // MIDEBUGSESSION_H
