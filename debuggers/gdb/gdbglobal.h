/***************************************************************************
                          gdbglobal.h
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

#ifndef _GDBGLOBAL_H_
#define _GDBGLOBAL_H_

#include <QFlags>

namespace GDBDebugger
{

enum DBGStateFlag
{
    s_none              = 0,
    s_dbgNotStarted     = 1 << 0,
    s_appNotStarted     = 1 << 1,
    s_programExited     = 1 << 2,
    s_attached          = 1 << 3,
    s_core              = 1 << 4,
    /// Set when 'slotStopDebugger' started executing, to avoid
    /// entering that function several times.
    s_shuttingDown      = 1 << 6,
    s_dbgBusy           = 1 << 8,
    s_appRunning        = 1 << 9,
    /// Set when we suspect GDB to be in a state where it does not listen for new commands
    /// while the inferior is running
    s_dbgNotListening   = 1 << 10,
    s_interruptSent     = 1 << 11,
    /// Once GDB is completely idle, send an automatic ExecContinue to resume from an interruption
    /// by CmdImmediately commands
    s_automaticContinue = 1 << 12,
};
Q_DECLARE_FLAGS(DBGStateFlags, DBGStateFlag)

enum DataType { typeUnknown, typeValue, typePointer, typeReference,
            typeStruct, typeArray, typeQString, typeWhitespace,
            typeName };

}

Q_DECLARE_OPERATORS_FOR_FLAGS(GDBDebugger::DBGStateFlags)

#endif
