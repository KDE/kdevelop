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

#include <qobject.h>
#include <qstring.h>
#include <qvaluevector.h>

#include "mi/gdbmi.h"
#include <qguardedptr.h>

namespace GDBDebugger
{


class Breakpoint;
class VarItem;
class ValueCallback;

/**
 * @author John Birch
 */

class GDBCommand
{
public:
    GDBCommand(const QString& command);

    template<class Handler>
    GDBCommand(const QString& command,
               Handler* handler_this, 
               void (Handler::* handler_method)(const GDBMI::ResultRecord&),
               bool handlesError = false);

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

    const QValueVector<QString>& allStreamOutput() const;

private:
    QString command_;
    QGuardedPtr<QObject> handler_this;
    typedef void (QObject::* handler_t)(const GDBMI::ResultRecord&);
    handler_t handler_method;
    QValueVector<QString> lines;

protected: // FIXME: should be private, after I kill the first ctor
    // that is obsolete and no longer necessary.
    bool handlesError_;

};

class UserCommand : public GDBCommand
{
public:
    UserCommand(const QString& s);

    bool isUserCommand() const;
};

/** This command is used to change some property of breakpoint.
    It holds a pointer to a Breakpoint object and will substitute
    breakpoint id into the command string.

    So, the command can be issued before the breakpoint id is know. That
    is, it's possible to queue add + modify pair. The add command will
    set breakpoint id and modify command will use it.
*/
class ModifyBreakpointCommand : public GDBCommand
{
public:
    /** The 'comamnd' should include a single format specifier "%1" that
        will be replaced with the id of breakpoint.
    */
    ModifyBreakpointCommand(const QString& command, const Breakpoint* bp);

public: // DbgCommand overrides
    virtual QString cmdToSend();

private:
    const Breakpoint* bp_;    
};

/** This is a class for raw CLI commands. Instead of invoking
    user provided hook with MI response, it invokes the a hook
    with lists of strings.
*/
class CliCommand : public GDBCommand
{
public:
    template<class Handler>
    CliCommand(const QString& command,
               Handler* handler_this, 
               void (Handler::* handler_method)(const QValueVector<QString>&),
               bool handlesError = false);



public: // GDBCommand overrides
    bool invokeHandler(const GDBMI::ResultRecord& r);

private:
		QGuardedPtr<QObject> cli_handler_this;
    typedef void (QObject::* cli_handler_t)(const QValueVector<QString>&);
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
    : GDBCommand(""),
      handler_this(handler_this),
      handler_method(static_cast<handler_method_t>(handler_method))
    {}

    void invokeHandler()
    {
        (handler_this->*handler_method)();
    }

    QString cmdToSend()             
    {
        return "";
    }

private:
		QGuardedPtr<QObject> handler_this;
    handler_method_t handler_method;

};

/* Command for which we don't want any reply.  */
class ResultlessCommand : public QObject, public GDBCommand
{
public:
    ResultlessCommand(const QString& command, bool handlesError = false)
    : GDBCommand(command, this, &ResultlessCommand::handle, handlesError)
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
    : GDBCommand(("-data-evaluate-expression " + expression).ascii(), this,
                 &ExpressionValueCommand::handleResponse),
      handler_this(handler_this),
      handler_method(static_cast<handler_method_t>(handler_method))
    {}

    void handleResponse(const GDBMI::ResultRecord& r)
    {
        (handler_this->*handler_method)(r["value"].literal());
    }

private:
		QGuardedPtr<QObject> handler_this;
    handler_method_t handler_method;
};



template<class Handler>
GDBCommand::GDBCommand(
    const QString& command,
    Handler* handler_this,
    void (Handler::* handler_method)(const GDBMI::ResultRecord&),
    bool handlesError)
: command_(command),
  handler_this(handler_this), 
  handler_method(static_cast<handler_t>(handler_method)),
  handlesError_(handlesError)
{
}

template<class Handler>
CliCommand::CliCommand(
    const QString& command,
    Handler* handler_this,
    void (Handler::* handler_method)(const QValueVector<QString>&),
    bool handlesError)
: GDBCommand(command.latin1()),
  cli_handler_this(handler_this), 
  cli_handler_method(static_cast<cli_handler_t>(handler_method))
{
    handlesError_ = handlesError;
}




}





#endif
