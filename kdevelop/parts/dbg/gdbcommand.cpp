/***************************************************************************
                          gdbcommand.cpp  -  description                              
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

#include "gdbcommand.h"
#include "breakpoint.h"
#include "vartree.h"

/***************************************************************************/

QString  GDBCommand::idlePrompt_ = QString().sprintf("\nset prompt %c%c\n", BLOCK_START, IDLE);

/***************************************************************************/

GDBCommand::GDBCommand(const QString& setCommand, bool isRunCmd, bool isInfoCmd, char setPrompt) :
  DbgCommand(setCommand, isRunCmd, isInfoCmd, setPrompt)
{
}

/***************************************************************************/

GDBCommand::~GDBCommand()
{
}

/***************************************************************************/

QString GDBCommand::cmdToSend()
{
  sent_ = true;

  if (prompt_)
  {
    QString startPrompt = QString().sprintf("set prompt %c%c\n", BLOCK_START, prompt_);
    return startPrompt + command_ + idlePrompt_;
  }

  return command_+"\n";
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

GDBItemCommand::GDBItemCommand( VarItem* item,
                                const QString& command,
                                bool isRunCmd,
                                char prompt) :
  GDBCommand(command, isRunCmd, true, prompt),
  item_(item)
{
}

/***************************************************************************/

GDBItemCommand::~GDBItemCommand()
{
}
  
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

GDBPointerCommand::GDBPointerCommand(VarItem* item) :
  GDBItemCommand(item, "print *"+item->fullName(), false,
                                                  DATAREQUEST)
{
}

/***************************************************************************/

GDBPointerCommand::~GDBPointerCommand()
{
}
  
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

//GDBReferenceCommand::GDBReferenceCommand(VarItem* item) :
//  GDBItemCommand(item, "print "+item->fullName(), false,
//                                                  DATAREQUEST)
//{
//}
//
///***************************************************************************/
//
//GDBReferenceCommand::~GDBReferenceCommand()
//{
//}
//  
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

GDBSetBreakpointCommand::GDBSetBreakpointCommand(const QString& setCommand, int key) :
  GDBCommand(setCommand, false, false, SET_BREAKPT),
  key_(key)
{
}

/***************************************************************************/

GDBSetBreakpointCommand::~GDBSetBreakpointCommand()
{
}
  
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
