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

namespace GDBDebugger
{

enum DBGStateFlags
{
  s_dbgNotStarted     = 1,
  s_appNotStarted     = 2,
  s_waitForWrite      = 8,
  s_programExited     = 16,
  s_viewBT            = 128,
  s_viewBP            = 256,
  s_attached          = 512,
  s_core              = 1024,
  s_waitTimer         = 2048,
  // Set when 'slotStopDebugger' started executing, to avoid
  // entering that function several times.
  s_shuttingDown      = 4096,
  s_explicitBreakInto = (s_shuttingDown << 1),
  s_dbgBusy           = (s_explicitBreakInto << 1),
  s_appRunning        = (s_dbgBusy << 1),
  s_lastDbgState      = (s_appRunning << 1)

};

}

#endif
