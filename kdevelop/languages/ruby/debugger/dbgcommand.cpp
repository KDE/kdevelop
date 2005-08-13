/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
	
                          Adapted for ruby debugging
                          --------------------------
    begin                : Mon Nov 1 2004
    copyright            : (C) 2004 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
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
namespace RDBDebugger
{


DbgCommand::DbgCommand(const QCString& command, bool isRunCmd, bool isInfoCmd) :
  command_(command),
  isRunCmd_(isRunCmd),
  isInfoCmd_(isInfoCmd),
  sent_(false),
  waitForReply_(true)
{
  cmdBuffer_ = command_+"\n";
}

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
