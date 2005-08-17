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
//Added by qt3to4:
#include <Q3CString>

namespace GDBDebugger
{

/***************************************************************************/

Q3CString GDBCommand::idlePrompt_ = Q3CString().sprintf("\nset prompt %c%c\n", BLOCK_START, IDLE);

/***************************************************************************/

GDBCommand::GDBCommand(const Q3CString &setCommand, bool isRunCmd, bool isInfoCmd, char setPrompt)
    : DbgCommand(setCommand, isRunCmd, isInfoCmd, setPrompt)
{
    if (prompt_) {
        cmdBuffer_ = Q3CString().sprintf("set prompt %c%c\n", BLOCK_START, prompt_) +
            command_ +
            idlePrompt_;
    }
}

/***************************************************************************/

GDBCommand::~GDBCommand()
{
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

GDBItemCommand::GDBItemCommand( VarItem *item,
                                const Q3CString &command,
                                bool isRunCmd,
                                char prompt)
    : GDBCommand(command, isRunCmd, true, prompt),
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

GDBPointerCommand::GDBPointerCommand(VarItem *item)
    : GDBItemCommand(item,
                     Q3CString("print *")+Q3CString(item->fullName().latin1()),
                     false,
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

//GDBReferenceCommand::GDBReferenceCommand(VarItem *item) :
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

GDBSetBreakpointCommand::GDBSetBreakpointCommand(const Q3CString &command, int key)
    : GDBCommand(command, false, false, SET_BREAKPT),
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

}
