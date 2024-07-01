/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _DBGGLOBAL_H_
#define _DBGGLOBAL_H_

#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <qobjectdefs.h>

#include <QFlags>

namespace KDevMI
{
Q_NAMESPACE

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
    /// Set when the debugger failed to start
    s_dbgFailedStart    = 1 << 13,
};
Q_ENUM_NS(DBGStateFlag)
Q_DECLARE_FLAGS(DBGStateFlags, DBGStateFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(DBGStateFlags)

enum DataType { typeUnknown, typeValue, typePointer, typeReference,
            typeStruct, typeArray, typeQString, typeWhitespace,
            typeName };

// FIXME: find a more appropriate place for these strings. Possibly a place specific to debugger backend
namespace Config {
static const char StartWithEntry[] = "Start With";
// FIXME: break on start isn't exposed in the UI for GDB
static const char BreakOnStartEntry[] = "Break on Start";
}

namespace GDB { namespace Config {
static const char GdbPathEntry[] = "GDB Path";
static const char DebuggerShellEntry[] = "Debugger Shell";
static const char RemoteGdbConfigEntry[] = "Remote GDB Config Script";
static const char RemoteGdbShellEntry[] = "Remote GDB Shell Script";
static const char RemoteGdbRunEntry[] = "Remote GDB Run Script";
static const char StaticMembersEntry[] = "Display Static Members";
static const char DemangleNamesEntry[] = "Display Demangle Names";
static const char AllowForcedBPEntry[] = "Allow Forced Breakpoint Set";
}
}

namespace LLDB { namespace Config {
static const char LldbExecutableEntry[] = "LLDB Executable";
static const char LldbArgumentsEntry[] = "LLDB Arguments";
static const char LldbEnvironmentEntry[] = "LLDB Environment";
static const char LldbInheritSystemEnvEntry[] = "LLDB Inherit System Env";
static const char LldbConfigScriptEntry[] = "LLDB Config Script";
static const char LldbRemoteDebuggingEntry[] = "LLDB Remote Debugging";
static const char LldbRemoteServerEntry[] = "LLDB Remote Server";
static const char LldbRemotePathEntry[] = "LLDB Remote Path";
}
}

} // end of namespace KDevMI

#endif // _DBGGLOBAL_H_
