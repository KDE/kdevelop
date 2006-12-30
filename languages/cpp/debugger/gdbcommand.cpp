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

#include "gdbcommand.h"
#include "breakpoint.h"
#include "variablewidget.h"

namespace GDBDebugger
{

GDBCommand::GDBCommand(const QString &command)
: command_(command), run(false), handler_this(0)
{
}

QString GDBCommand::cmdToSend()
{
    return initialString() + "\n";
}

QString GDBCommand::initialString() const
{
    return command_;
}

bool GDBCommand::isUserCommand() const
{
    return false;
}

bool
GDBCommand::invokeHandler(const GDBMI::ResultRecord& r)
{
    if (handler_this) {
        (handler_this->*handler_method)(r);
        return true;
    }
    else {
        return false;
    }
}

void GDBCommand::newOutput(const QString& line)
{
    lines.push_back(line);
}

const QValueVector<QString>& GDBCommand::allStreamOutput() const
{
    return lines;
}

bool GDBCommand::handlesError() const
{
    return handlesError_;
}

GDBCommand::~GDBCommand()
{
}

bool GDBCommand::isRun() const
{
    return run;
}

void GDBCommand::setRun(bool run)
{
    this->run = run;
}


UserCommand::UserCommand(const QString& s)
: GDBCommand(s)
{
}

bool UserCommand::isUserCommand() const
{
    return true;
}


ModifyBreakpointCommand::ModifyBreakpointCommand(
    const QString& command, const Breakpoint* bp)
: GDBCommand(command.local8Bit()),
  bp_(bp)
{}

QString
ModifyBreakpointCommand::cmdToSend()
{
    if (bp_->dbgId() > 0)
    {
        QString s(initialString());
        s = s.arg(bp_->dbgId()) + "\n";
        return s.local8Bit();
    }
    else
    {
        // The ID can be -1 either if breakpoint set command
        // failed, or if breakpoint is somehow already deleted.
        // In either case, should not do anything.
        return "";
    }
}


bool CliCommand::invokeHandler(const GDBMI::ResultRecord& r)
{
    // On error, do nothing.
    if (r.reason != "done")
        return true;

    if (cli_handler_this) {
        (cli_handler_this->*cli_handler_method)(allStreamOutput());
        return true;
    }
    else {
        return false;
    }
    
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}
