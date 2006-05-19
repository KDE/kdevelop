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

/**
 * @author John Birch
 */

namespace GDBDebugger
{

class DbgCommand
{
public:
    DbgCommand(const QString& command, bool isInfoCmd, char prompt);
    virtual ~DbgCommand() {};

    virtual QString cmdToSend()             { return cmdBuffer_; }

    QString rawDbgCommand() const            { return command_; }
    bool isAnInfoCmd() const                  { return isInfoCmd_; }
    bool expectReply() const                  { return waitForReply_; }
    bool typeMatch(char cmdType) const        { return (prompt_ == cmdType); }

protected:
    QString cmdBuffer_;
    QString command_;
    bool    isInfoCmd_;
    bool    waitForReply_;
    char    prompt_;
};

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
