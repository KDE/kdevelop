/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GDBCOMMAND_H_
#define _GDBCOMMAND_H_

#include <functional>

#include <QString>
#include <QStringList>
#include <QPointer>

#include "mi/gdbmi.h"

namespace GDBDebugger
{


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

//base class for handlers
class GDBCommandHandler
{
public:
    virtual ~GDBCommandHandler() {}
    virtual void handle(const GDBMI::ResultRecord&) = 0;
    virtual bool handlesError() { return false; }

    /**
     * If the handler object should be deleted after the handle() call.
     */
    virtual bool autoDelete() { return true; }
};

class FunctionCommandHandler : public GDBCommandHandler {
public:
    typedef std::function<void (const GDBMI::ResultRecord&)> Function;

    FunctionCommandHandler(const Function& callback, CommandFlags flags = 0);

    virtual void handle(const GDBMI::ResultRecord&) override;
    virtual bool handlesError() override;

private:
    CommandFlags _flags;
    Function _callback;
};

/**
 * @author John Birch
 */

class GDBCommand
{
public:
    GDBCommand(GDBMI::CommandType type, const QString& arguments = QString(), CommandFlags flags = 0);

    template<class Handler>
    GDBCommand(GDBMI::CommandType type, const QString& arguments,
               Handler* handler_this,
               void (Handler::* handler_method)(const GDBMI::ResultRecord&),
               CommandFlags flags = 0);

    GDBCommand(GDBMI::CommandType type, const QString& arguments, GDBCommandHandler* handler,
               CommandFlags flags = 0);

    GDBCommand(
        GDBMI::CommandType type, const QString& arguments,
        const FunctionCommandHandler::Function& callback,
        CommandFlags flags = 0);

    virtual ~GDBCommand();

    GDBMI::CommandType type() const;
    QString gdbCommand() const;

    CommandFlags flags() const {return flags_;}

    /**
     * Returns the MI token with which the command is sent, allowing the parser to match up
     * the result message with the command.
     */
    uint32_t token() const {return token_;}

    /**
     * Set the MI token. This is done by \ref GDBCommandQueue.
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
    void setHandler(GDBCommandHandler* handler);
    
    /* The command that should be sent to gdb.
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
    bool invokeHandler(const GDBMI::ResultRecord& r);

    // Returns 'true' if 'invokeHandler' should be invoked even
    // on MI errors.
    bool handlesError() const;

    // Called by gdbcontroller for each new output string
    // gdb emits for this command. In MI mode, this includes
    // all "stream" messages, but does not include MI responses.
    void newOutput(const QString&);

    const QStringList& allStreamOutput() const;

    QString command() const;

    void setStateReloading(bool f);

    bool stateReloading() const;

private:
    GDBMI::CommandType type_;
    CommandFlags flags_;
    uint32_t token_ = 0;
    QString command_;
    GDBCommandHandler *commandHandler_;
    QStringList lines;
    bool stateReloading_;

private:
    int m_thread;
    int m_frame;
};

class UserCommand : public GDBCommand
{
public:
    UserCommand(GDBMI::CommandType type, const QString& s);

    bool isUserCommand() const;
};

/** This is a class for raw CLI commands. Instead of invoking
    user provided hook with MI response, it invokes the a hook
    with lists of strings.
*/
class CliCommand : public GDBCommand
{
public:
    template<class Handler>
    CliCommand(GDBMI::CommandType type, const QString& command,
               Handler* handler_this,
               void (Handler::* handler_method)(const QStringList&),
               CommandFlags flags = 0);
};

/** Command that does nothing and can be just used to invoke
    a user provided handler when all preceeding commands are
    executed.
*/
class SentinelCommand : public GDBCommand
{
public:
    typedef std::function<void ()> Function;

    template<class Handler>
    SentinelCommand(Handler* handler_this,
                    void (Handler::* handler_method)(),
                    CommandFlags flags = 0)
        : GDBCommand(GDBMI::NonMI, QString(), flags)
    {
        QPointer<Handler> guarded_this(handler_this);
        handler = [guarded_this, handler_method]() {
            if (guarded_this) {
                (guarded_this.data()->*handler_method)();
            }
        };
    }

    SentinelCommand(const Function& handler, CommandFlags flags = 0)
        : GDBCommand(GDBMI::NonMI, QString(), flags)
        , handler(handler)
    {
    }

    using GDBCommand::invokeHandler;
    void invokeHandler()
    {
        handler();
    }

    QString cmdToSend()
    {
        return "";
    }

private:
    Function handler;
};

class ExpressionValueCommand : public QObject, public GDBCommand
{
public:
    typedef void (QObject::*handler_method_t)(const QString&);

    template<class Handler>
    ExpressionValueCommand(
        const QString& expression,
        Handler* handler_this,
        void (Handler::* handler_method)(const QString&))
    : GDBCommand(GDBMI::DataEvaluateExpression, expression, this,
                 &ExpressionValueCommand::handleResponse),
      handler_this(handler_this),
      handler_method(static_cast<handler_method_t>(handler_method))
    {}

    void handleResponse(const GDBMI::ResultRecord& r)
    {
        (handler_this.data()->*handler_method)(r["value"].literal());
    }

private:
    QPointer<QObject> handler_this;
    handler_method_t handler_method;
};


template<class Handler>
GDBCommand::GDBCommand(
    GDBMI::CommandType type,
    const QString& command,
    Handler* handler_this,
    void (Handler::* handler_method)(const GDBMI::ResultRecord&),
    CommandFlags flags)
: type_(type),
  flags_(flags & ~CmdHandlesError),
  command_(command),
  commandHandler_(nullptr),
  stateReloading_(false),
  m_thread(-1),
  m_frame(-1)
{
    QPointer<Handler> guarded_this(handler_this);
    setHandler(new FunctionCommandHandler([guarded_this, handler_method](const GDBMI::ResultRecord& r) {
        if (guarded_this) {
            (guarded_this.data()->*handler_method)(r);
        }
    }, flags));
}

template<class Handler>
CliCommand::CliCommand(
    GDBMI::CommandType type,
    const QString& command,
    Handler* handler_this,
    void (Handler::* handler_method)(const QStringList&),
    CommandFlags flags)
: GDBCommand(type, command)
{
    QPointer<Handler> guarded_this(handler_this);
    setHandler(new FunctionCommandHandler([this, guarded_this, handler_method](const GDBMI::ResultRecord&) {
        if (guarded_this) {
            (guarded_this.data()->*handler_method)(this->allStreamOutput());
        }
    }, flags));
}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(GDBDebugger::CommandFlags)

#endif
