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

#include <QObject>
#include <QString>
#include <QStringList>
#include <QPointer>

#include "mi/gdbmi.h"

namespace GDBDebugger
{


class VarItem;
class ValueCallback;

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

/**
 * @author John Birch
 */

class GDBCommand
{
public:
    GDBCommand(GDBMI::CommandType type, const QString& arguments = QString());
    GDBCommand(GDBMI::CommandType type, int index);

    template<class Handler>
    GDBCommand(GDBMI::CommandType type, const QString& arguments,
               Handler* handler_this,
               void (Handler::* handler_method)(const GDBMI::ResultRecord&),
               bool handlesError = false);

    template<class Handler>
    GDBCommand(GDBMI::CommandType type, int index,
               Handler* handler_this,
               void (Handler::* handler_method)(const GDBMI::ResultRecord&),
               bool handlesError = false);

    GDBCommand(GDBMI::CommandType type, const QString& arguments, GDBCommandHandler* handler);

    GDBMI::CommandType type() const;
    QString gdbCommand() const;

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
     */
    template<class Handler>
    void setHandler(Handler* handler_this, void (Handler::* handler_method)(const GDBMI::ResultRecord&), bool handlesError = false);
    
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
    virtual bool invokeHandler(const GDBMI::ResultRecord& r);

    // Returns 'true' if 'invokeHandler' should be invoked even
    // on MI errors.
    bool handlesError() const;

    virtual ~GDBCommand();

    // Called by gdbcontroller for each new output string
    // gdb emits for this command. In MI mode, this includes
    // all "stream" messages, but does not include MI responses.
    void newOutput(const QString&);

    const QStringList& allStreamOutput() const;

    // True if this command run then target for
    // unspecified period of time -- that is either 'run' or
    // 'continue'.
    bool isRun() const;

    void setRun(bool run);

    QString command() const;

    void setStateReloading(bool f);

    bool stateReloading() const;

private:
    GDBMI::CommandType type_;
    QString command_;
    QPointer<QObject> handler_this;
    typedef void (QObject::* handler_t)(const GDBMI::ResultRecord&);
    handler_t handler_method;
    GDBCommandHandler *commandHandler_;
    QStringList lines;
    bool run;
    bool stateReloading_;

protected: // FIXME: should be private, after I kill the first ctor
    // that is obsolete and no longer necessary.
    bool handlesError_;

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
               bool handlesError = false);



public: // GDBCommand overrides
    bool invokeHandler(const GDBMI::ResultRecord& r);

private:
    QPointer<QObject> cli_handler_this;
    typedef void (QObject::* cli_handler_t)(const QStringList&);
    cli_handler_t cli_handler_method;
};

/** Command that does nothing and can be just used to invoke
    a user provided handler when all preceeding commands are
    executed.
*/
class SentinelCommand : public GDBCommand
{
public:
    typedef void (QObject::*handler_method_t)();

    template<class Handler>
    SentinelCommand(Handler* handler_this,
                    void (Handler::* handler_method)())
    : GDBCommand(GDBMI::NonMI, ""),
      handler_this(handler_this),
      handler_method(static_cast<handler_method_t>(handler_method))
    {}

    using GDBCommand::invokeHandler;
    void invokeHandler()
    {
        (handler_this.data()->*handler_method)();
    }

    QString cmdToSend()
    {
        return "";
    }

private:
    QPointer<QObject> handler_this;
    handler_method_t handler_method;

};

/* Command for which we don't want any reply.  */
class ResultlessCommand : public QObject, public GDBCommand
{
public:
    ResultlessCommand(GDBMI::CommandType type, const QString& command, bool handlesError = false)
    : GDBCommand(type, command, this, &ResultlessCommand::handle, handlesError)
    {}

private:
    void handle(const GDBMI::ResultRecord&)
    {}
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
    bool handlesError)
: type_(type),
  command_(command),
  handler_this(handler_this),
  handler_method(static_cast<handler_t>(handler_method)),
  commandHandler_(0),
  run(false),
  stateReloading_(false),
  handlesError_(handlesError),
  m_thread(-1),
  m_frame(-1)
{
}

template<class Handler>
GDBCommand::GDBCommand(
    GDBMI::CommandType type,
    int index,
    Handler* handler_this,
    void (Handler::* handler_method)(const GDBMI::ResultRecord&),
    bool handlesError)
: type_(type),
  command_(QString::number(index)),
  handler_this(handler_this),
  handler_method(static_cast<handler_t>(handler_method)),
  commandHandler_(0),
  run(false),
  stateReloading_(false),
  handlesError_(handlesError),
  m_thread(-1),
  m_frame(-1)
{
}

template<class Handler>
CliCommand::CliCommand(
    GDBMI::CommandType type,
    const QString& command,
    Handler* handler_this,
    void (Handler::* handler_method)(const QStringList&),
    bool handlesError)
: GDBCommand(type, command),
  cli_handler_this(handler_this),
  cli_handler_method(static_cast<cli_handler_t>(handler_method))
{
    handlesError_ = handlesError;
}

template<class Handler>
void GDBCommand::setHandler(Handler* handler_this, void (Handler::* handler_method)(const GDBMI::ResultRecord&), bool handlesError)
{
    GDBCommand::handler_this = handler_this;
    GDBCommand::handler_method = static_cast<handler_t>(handler_method);
    handlesError_ = handlesError;
}

}





#endif
