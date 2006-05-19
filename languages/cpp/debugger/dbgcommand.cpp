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

#include "dbgcommand.h"

#include <qstring.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
namespace GDBDebugger
{


DbgCommand::DbgCommand(const QString& command, bool isInfoCmd, char prompt) :
  command_(command),
  isInfoCmd_(isInfoCmd),
  waitForReply_(prompt != 0),
  prompt_(prompt)
{
  cmdBuffer_ = command_+"\n";
}

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
