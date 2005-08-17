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

#ifndef DBGCOMMAND_H
#define DBGCOMMAND_H

#include <qstring.h>
//Added by qt3to4:
#include <Q3CString>

/**
 * @author John Birch
 */

namespace GDBDebugger
{

class DbgCommand
{
public:
    DbgCommand(const Q3CString& command, bool isRunCmd, bool isInfoCmd, char prompt);
    virtual ~DbgCommand() {};

    virtual Q3CString& cmdToSend()             { sent_ = true;  return cmdBuffer_; }
    virtual int cmdLength()                   { return cmdBuffer_.length(); }

    Q3CString rawDbgCommand() const            { return command_; }
    bool isARunCmd() const                    { return isRunCmd_;}
    bool isAnInfoCmd() const                  { return isInfoCmd_; }
    bool moreToSend() const                   { return !sent_; }
    bool expectReply() const                  { return waitForReply_; }
    bool typeMatch(char cmdType) const        { return (prompt_ == cmdType); }

protected:
    Q3CString cmdBuffer_;
    Q3CString command_;
    bool    isRunCmd_;
    bool    isInfoCmd_;
    bool    sent_;
    bool    waitForReply_;
    char    prompt_;
};

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
