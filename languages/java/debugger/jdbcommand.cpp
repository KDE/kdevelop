/***************************************************************************
                          jdbcommand.cpp  -  description
                             -------------------
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

#include "jdbcommand.h"
#include "breakpoint.h"
#include "variablewidget.h"


namespace JAVADebugger
{

JDBCommand::JDBCommand(const QCString &setCommand, bool isRunCmd, bool isInfoCmd, char setPrompt)
    : DbgCommand(setCommand, isRunCmd, isInfoCmd, setPrompt)
{
}

/***************************************************************************/

JDBCommand::~JDBCommand()
{
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

JDBItemCommand::JDBItemCommand( VarItem *item,
                                const QCString &command,
                                bool isRunCmd,
                                char prompt)
    : JDBCommand(command, isRunCmd, true, prompt),
      item_(item)
{
}

/***************************************************************************/

JDBItemCommand::~JDBItemCommand()
{
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

JDBPointerCommand::JDBPointerCommand(VarItem *item)
    : JDBItemCommand(item,
                     QCString("print *")+QCString(item->fullName().latin1()),
                     false,
                     DATAREQUEST)
{
}

/***************************************************************************/

JDBPointerCommand::~JDBPointerCommand()
{
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

//JDBReferenceCommand::JDBReferenceCommand(VarItem *item) :
//  JDBItemCommand(item, "print "+item->fullName(), false,
//                                                  DATAREQUEST)
//{
//}
//
///***************************************************************************/
//
//JDBReferenceCommand::~JDBReferenceCommand()
//{
//}
//
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

JDBSetBreakpointCommand::JDBSetBreakpointCommand(const QCString &command, int key)
    : JDBCommand(command, false, false, SET_BREAKPT),
      key_(key)
{
}

/***************************************************************************/

JDBSetBreakpointCommand::~JDBSetBreakpointCommand()
{
}

/***************************************************************************/

}
