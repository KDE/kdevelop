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

/***************************************************************************/

QCString GDBCommand::idlePrompt_ = QCString().sprintf("\nset prompt %c%c\n", BLOCK_START, IDLE);

/***************************************************************************/

GDBCommand::GDBCommand(const QCString &setCommand, bool isRunCmd, bool isInfoCmd, char setPrompt)
    : DbgCommand(setCommand, isRunCmd, isInfoCmd, setPrompt)
{
    if (prompt_) {
        cmdBuffer_ = QCString().sprintf("set prompt %c%c\n", BLOCK_START, prompt_) +
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
                                const QCString &command,
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

GDBSetBreakpointCommand::GDBSetBreakpointCommand(const QCString &command, int key)
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
