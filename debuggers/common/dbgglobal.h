/***************************************************************************
                          dbgglobal.h
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

#ifndef _DBGGLOBAL_H_
#define _DBGGLOBAL_H_

#include <QFlags>

namespace KDevDebugger
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

// FIXME: find a more appropriate place for these strings. Possibly a place specific to debugger backend
static const char gdbPathEntry[] = "GDB Path";
static const char debuggerShellEntry[] = "Debugger Shell";
static const char remoteGdbConfigEntry[] = "Remote GDB Config Script";
static const char remoteGdbShellEntry[] = "Remote GDB Shell Script";
static const char remoteGdbRunEntry[] = "Remote GDB Run Script";
static const char staticMembersEntry[] = "Display Static Members";
static const char demangleNamesEntry[] = "Display Demangle Names";
static const char allowForcedBPEntry[] = "Allow Forced Breakpoint Set";
static const char startWithEntry[] = "Start With";
static const char breakOnStartEntry[] = "Break on Start";

} // end of namespace KDevDebugger

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevDebugger::DBGStateFlags)

#endif // _DBGGLOBAL_H_
