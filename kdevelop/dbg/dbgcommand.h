/***************************************************************************
                          dbgcommand.h  -  description                              
                             -------------------                                         
    begin                : Sun Aug 8 1999                                           
    copyright            : (C) 1999 by John Birch
    email                : jb.nz@writeme.com
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
  *@author John Birch
  */


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class DbgCommand
{
public: 
  DbgCommand(const QString& command, bool isRunCmd, bool isInfoCmd, char prompt);
  virtual ~DbgCommand() {};

  virtual QString cmdToSend();

  QString rawDbgCommand() const             { return command_; }
  bool isARunCmd() const                    { return isRunCmd_;}
  bool isAnInfoCmd() const                  { return isInfoCmd_; }
  bool moreToSend() const                   { return !sent_; }
  bool expectReply() const                  { return waitForReply_; }
  bool typeMatch(const char cmdType) const  { return (prompt_ == cmdType); }

  virtual bool isA ( const char * className ) const
                  { return strncmp(className, "DbgCommand", 10) == 0; }

protected:
  QString command_;
  bool    isRunCmd_;
  bool    isInfoCmd_;
  bool    sent_;
  bool    waitForReply_;
  char    prompt_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
