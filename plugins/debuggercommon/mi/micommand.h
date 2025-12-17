/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _MICOMMAND_H_
#define _MICOMMAND_H_

#include "mi/mi.h"

#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <QPointer>
#include <QString>
#include <QStringList>

#include <functional>
#include <memory>

namespace KDevMI {

class MIDebugSession;

namespace MI {

class VarItem;
class ValueCallback;

enum CommandFlag {
    /// The command handler also wishes to receive an error responses, overriding the default error handler
    CmdHandlesError = 1 << 0,

    /// The command is expected to cause the inferior to run. Controllers that display the
    /// program's state should refrain from sending commands while a command with this flag
    /// is currently pending; however, note that a command with this flag needn't be guaranteed
    /// to lead to a running state.
    CmdMaybeStartsRunning = 1 << 1,

    /// The command is a temporary-run type command, meaning that it typically causes the program
    /// to run, but only for a short time before it stops again (e.g. Step and StepInto-type
    /// commands). When the program is running due to this type of command, a CmdImmediately
    /// command will wait before forcing an interrupt of the debugger, and the program is _not_
    /// automatically restarted if an interrupt was forced.
    ///
    /// TODO: this special handling has not actually been implemented yet
    CmdTemporaryRun = 1 << 2,

    /// This command should be executed immediately, even if the program is currently running
    /// (e.g. breakpoint setting and modification); however, if the program is interrupted,
    /// it should be resumed after this command has run.
    CmdImmediately = 1 << 3,

    /// This is a command that should interrupt a running program, without resuming.
    CmdInterrupt = 1 << 4,
};
Q_DECLARE_FLAGS(CommandFlags, CommandFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(CommandFlags)

//base class for handlers
class MICommandHandler
{
public:
    virtual ~MICommandHandler() {}
    virtual void handle(const ResultRecord&) = 0;
    virtual bool handlesError() { return false; }
};

class FunctionCommandHandler : public MICommandHandler {
public:
    using Function = std::function<void (const ResultRecord&)>;

    explicit FunctionCommandHandler(const Function& callback, CommandFlags flags = {});

    void handle(const ResultRecord&) override;
    bool handlesError() override;

private:
    CommandFlags _flags;
    Function _callback;
};

/**
 * @author John Birch
 */

class MICommand
{
protected:
    explicit MICommand(CommandType type, const QString& arguments = QString(), CommandFlags flags = {});
    friend class KDevMI::MIDebugSession;

public:

    virtual ~MICommand();

    CommandType type() const;
    /**
     * @return whether this command should be executed in the context of a specific thread and frame
     */
    [[nodiscard]] bool needsContext() const;

    virtual QString miCommand() const;

    CommandFlags flags() const {return flags_;}

    /**
     * Returns the MI token with which the command is sent, allowing the parser to match up
     * the result message with the command.
     */
    uint32_t token() const {return token_;}

    /**
     * Set the MI token. This is done by \ref MICommandQueue.
     */
    void setToken(uint32_t token) {token_ = token;}

    /**
     * Returns the thread that needs to be currently selected when this command is executed,
     * or -1 if there is no requirement.
     */
    int thread() const;

    /**
     * Set the thread required to be currently selected when the command is executed.
     */
    void setThread(int thread);

    /**
     * Returns the frame that needs to be currently selected when this command is executed,
     * or -1 if there is no requirement.
     */
    int frame() const;

    /**
     * Set the frame required to be currently selected when the command is executed.
     */
    void setFrame(int frame);

    /**
     * Sets the handler for results.
     *
     * The command object assumes ownership of @p handler.
     */
    void setHandler(MICommandHandler* handler);
    void setHandler(const FunctionCommandHandler::Function &callback);

    template<class Handler>
    using ResultRecordMethod = void (Handler::*)(const ResultRecord&);

    template<class Handler>
    void setHandler(Handler* handler_this, ResultRecordMethod<Handler> handler_method);

    /* The command that should be sent to debugger.
       This method is virtual so the command can compute this
       dynamically, possibly using results of the previous
       commands.
       If the empty string is returned, nothing is sent. */
    virtual QString cmdToSend();

    /* Returns the initial string that was specified in
       ctor invocation. The actual command will be
       determined by cmdToSend above and the return
       value of this method is only used in various
       diagnostic messages emitted before actually
       sending the command. */
    QString initialString() const;

    /* Returns true if this is command entered by the user
       and so should be always shown in the gdb output window. */
    virtual bool isUserCommand() const;

    // If there's a handler for this command, invokes it and returns true.
    // Otherwise, returns false.
    bool invokeHandler(const ResultRecord& r);

    // Returns 'true' if 'invokeHandler' should be invoked even
    // on MI errors.
    bool handlesError() const;

    // Called by debuggercontroller for each new output string
    // debugger emits for this command. In MI mode, this includes
    // all "stream" messages, but does not include MI responses.
    void newOutput(const QString&);

    /**
     * This function should be called when new log stream output arrives while this command is being executed.
     *
     * @param receivedLine the received log stream message
     */
    void newLogStreamOutput(const QString& receivedLine);

    /**
     * Call this function to store the log stream output along with the console
     * stream output so that allStreamOutput() returns both (interspersed).
     */
    void storeLogStreamOutput();

    /**
     * @return all stored stream output lines (only console stream output is stored by default)
     * @sa storeLogStreamOutput()
     */
    const QStringList& allStreamOutput() const;

    QString command() const;

    void setStateReloading(bool f);

    bool stateReloading() const;

    /// Called when the command has been enqueued in the debug session
    /// and the command is wait for being submitted to GDB.
    void markAsEnqueued();

    /// Called when the command has been submitted to GDB and the command
    /// waits for completion by GDB.
    void markAsSubmitted();

    /// Called when the command has been completed and the response has arrived.
    void markAsCompleted();

    /// returns the amount of time (in ms) passed between submission and completion.
    qint64 gdbProcessingTime() const;

    /// returns the amount of time (in ms) passed between enqueuing and submission.
    qint64 queueTime() const;

    /// returns the amount of time (in ms) passed between enqueuing and completion.
    qint64 totalProcessingTime() const;

protected:
    CommandType type_;
    CommandFlags flags_;
    uint32_t token_ = 0;
    QString command_;
    std::unique_ptr<MICommandHandler> m_commandHandler;
    QStringList lines;
    bool m_storeLogStreamOutput = false;
    bool stateReloading_;

    int m_thread;
    int m_frame;
    // remember the timestamps (in ms since start of the epoch) when this command
    // - was added to the command queue (enqueued)
    // - was submitted to GDB
    // - was completed; response from GDB arrived
    qint64 m_enqueueTimestamp;
    qint64 m_submitTimestamp;
    qint64 m_completeTimestamp;
};

class UserCommand : public MICommand
{
public:
    UserCommand(CommandType type, const QString& s);

    bool isUserCommand() const override;
};

/** This is a class for raw CLI commands. Instead of invoking
    user provided hook with MI response, it invokes the a hook
    with lists of strings.
*/
class CliCommand : public MICommand
{
public:
    template<class Handler>
    CliCommand(CommandType type, const QString& command,
               Handler* handler_this,
               void (Handler::* handler_method)(const QStringList&),
               CommandFlags flags = {});
};

/** Command that does nothing and can be just used to invoke
    a user provided handler when all preceding commands are
    executed.
*/
class SentinelCommand : public MICommand
{
public:
    using Function = std::function<void ()>;

    template<class Handler>
    SentinelCommand(Handler* handler_this,
                    void (Handler::* handler_method)(),
                    CommandFlags flags = {})
        : MICommand(NonMI, QString(), flags)
    {
        QPointer<Handler> guarded_this(handler_this);
        handler = [guarded_this, handler_method]() {
            if (guarded_this) {
                (guarded_this.data()->*handler_method)();
            }
        };
    }

    explicit SentinelCommand(const Function& handler, CommandFlags flags = {})
        : MICommand(NonMI, QString(), flags)
        , handler(handler)
    {
    }

    using MICommand::invokeHandler;
    void invokeHandler()
    {
        handler();
    }

    QString cmdToSend() override
    {
        return QString();
    }

private:
    Function handler;
};

template<class Handler>
void MICommand::setHandler(Handler* handler_this, ResultRecordMethod<Handler> handler_method)
{
    QPointer<Handler> guarded_this(handler_this);
    setHandler(new FunctionCommandHandler([guarded_this, handler_method](const ResultRecord& r) {
        if (guarded_this) {
            (guarded_this.data()->*handler_method)(r);
        }
    }, flags()));
}

template<class Handler>
CliCommand::CliCommand(
    CommandType type,
    const QString& command,
    Handler* handler_this,
    void (Handler::* handler_method)(const QStringList&),
    CommandFlags flags)
: MICommand(type, command)
{
    QPointer<Handler> guarded_this(handler_this);
    setHandler(new FunctionCommandHandler([this, guarded_this, handler_method](const ResultRecord&) {
        if (guarded_this) {
            (guarded_this.data()->*handler_method)(this->allStreamOutput());
        }
    }, flags));
}

} // end of namespace MI
} // end of namespace KDevMI

#endif
