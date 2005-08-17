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

#include "rdbcommand.h"
#include "breakpoint.h"
#include "variablewidget.h"
//Added by qt3to4:
#include <Q3CString>

namespace RDBDebugger
{


RDBCommand::RDBCommand(const Q3CString &setCommand, bool isRunCmd, bool isInfoCmd)
    : DbgCommand(setCommand, isRunCmd, isInfoCmd)
{
//    if (prompt_) {
//        cmdBuffer_ = QCString().sprintf("set prompt %c%c\n", BLOCK_START, prompt_) +
//            command_ +
//            idlePrompt_;
//    }
}

/***************************************************************************/

RDBCommand::~RDBCommand()
{
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

RDBItemCommand::RDBItemCommand( VarItem *item,
                                const Q3CString &command,
                                bool isRunCmd)
    : RDBCommand(command, isRunCmd, true),
      item_(item)
{
}

/***************************************************************************/

RDBItemCommand::~RDBItemCommand()
{
}


RDBSetBreakpointCommand::RDBSetBreakpointCommand(const Q3CString &command, int key)
    : RDBCommand(command, false, false),
      key_(key)
{
}

/***************************************************************************/

RDBSetBreakpointCommand::~RDBSetBreakpointCommand()
{
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}
