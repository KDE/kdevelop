/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
    copyright            : (C) 2016 by Aetf
    email                : aetf@unlimitedcodeworks.xyz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MICOMMAND_H_
#define _MICOMMAND_H_

#include "mi/mi.h"

#include <QString>
#include <QStringList>
#include <QPointer>

#include <functional>

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

//base class for handlers
class MICommandHandler
{
public:
    virtual ~MICommandHandler() {}
    virtual void handle(const ResultRecord&) = 0;
    virtual bool handlesError() { return false; }

    /**
     * If the handler object should be deleted after the handle() call.
     */
    virtual bool autoDelete() { return true; }
};

class FunctionCommandHandler : public MICommandHandler {
public:
    typedef std::function<void (const ResultRecord&)> Function;

    FunctionCommandHandler(const Function& callback, CommandFlags flags = 0);

    virtual void handle(const ResultRecord&) override;
    virtual bool handlesError() override;

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
    MICommand(CommandType type, const QString& arguments = QString(), CommandFlags flags = 0);
    friend class KDevMI::MIDebugSession;
public:

    virtual ~MICommand();

    CommandType type() const;
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
    void setHandler(Handler* handler_this, void (Handler::* handler_method)(const ResultRecord&));

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

    const QStringList& allStreamOutput() const;

    QString command() const;

    void setStateReloading(bool f);

    bool stateReloading() const;

private:
    CommandType type_;
    CommandFlags flags_;
    uint32_t token_ = 0;
    QString command_;
    MICommandHandler *commandHandler_;
    QStringList lines;
    bool stateReloading_;

private:
    int m_thread;
    int m_frame;
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
               CommandFlags flags = 0);
};

/** Command that does nothing and can be just used to invoke
    a user provided handler when all preceeding commands are
    executed.
*/
class SentinelCommand : public MICommand
{
public:
    typedef std::function<void ()> Function;

    template<class Handler>
    SentinelCommand(Handler* handler_this,
                    void (Handler::* handler_method)(),
                    CommandFlags flags = 0)
        : MICommand(NonMI, QString(), flags)
    {
        QPointer<Handler> guarded_this(handler_this);
        handler = [guarded_this, handler_method]() {
            if (guarded_this) {
                (guarded_this.data()->*handler_method)();
            }
        };
    }

    SentinelCommand(const Function& handler, CommandFlags flags = 0)
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
        return "";
    }

private:
    Function handler;
};

class ExpressionValueCommand : public QObject, public MICommand
{
public:
    typedef void (QObject::*handler_method_t)(const QString&);

    template<class Handler>
    ExpressionValueCommand(
        const QString& expression,
        Handler* handler_this,
        void (Handler::* handler_method)(const QString&))
    : MICommand(DataEvaluateExpression, expression),
      handler_this(handler_this),
      handler_method(static_cast<handler_method_t>(handler_method))
    {
        setHandler(this, &ExpressionValueCommand::handleResponse);
    }

    void handleResponse(const ResultRecord& r)
    {
        (handler_this.data()->*handler_method)(r["value"].literal());
    }

private:
    QPointer<QObject> handler_this;
    handler_method_t handler_method;
};

template<class Handler>
FunctionCommandHandler::Function guarded_callback(Handler *handler_this,
                                                 void (Handler::* handler_method)(const ResultRecord&))
{
    QPointer<Handler> guarded_this(handler_this);
    return [guarded_this, handler_method](const ResultRecord& r) {
        if (guarded_this) {
            (guarded_this.data()->*handler_method)(r);
        }
    };
}

template<class Handler>
void MICommand::setHandler(Handler* handler_this,
                           void (Handler::* handler_method)(const ResultRecord&))
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

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevMI::MI::CommandFlags)

#endif
