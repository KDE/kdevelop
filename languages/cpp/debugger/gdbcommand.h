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

#ifndef _GDBCOMMAND_H_
#define _GDBCOMMAND_H_

#include "dbgcommand.h"

namespace GDBDebugger
{

class Breakpoint;
class VarItem;
class ValueCallback;

// sigh - namespace's don't work on some of the older compilers
enum GDBCmd
{
  BLOCK_START     = '\32',
  SRC_POSITION    = '\32',    // Hmmm, same value may not work for all compilers

  ARGS            = 'A',

  BPLIST          = 'B',
  SET_BREAKPT     = 'b',

  DATAREQUEST     = 'D',
  DISASSEMBLE     = 'd',

  FRAME           = 'F',
  FILE_START      = 'f',

  WHATIS          = 'H',

  INITIALISE      = 'I',
  IDLE            = 'i',

  BACKTRACE       = 'K',

  LOCALS          = 'L',
  LIBRARIES       = 'l',

  MEMDUMP         = 'M',

  WAIT            = '0',

  TRACING_PRINTF  = 'P',

  RUN             = 'R',
  REGISTERS       = 'r',

  PROGRAM_STOP    = 'S',
  SHARED_CONT     = 's',

  INFOTHREAD      = 'T',
  SWITCHTHREAD    = 't',

  USERCMD         = 'U',

  SETVALUE        = 'V',

  SETWATCH        = 'W',
  UNSETWATCH      = 'w',

  DETACH          = 'z'
 
};

#define RUNCMD      (true)
#define NOTRUNCMD   (false)
#define INFOCMD     (true)
#define NOTINFOCMD  (false)

/**
 * @author John Birch
 */

class GDBCommand : public DbgCommand
{
public:
    GDBCommand(const QCString& command, bool isRunCmd=false, bool isInfoCmd=true,
               char prompt=WAIT);
    virtual ~GDBCommand();

private:
    static QCString idlePrompt_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class GDBItemCommand : public GDBCommand
{
public:
    GDBItemCommand(ValueCallback* item, const QCString &command,
                   bool isRunCmd=false, char prompt=DATAREQUEST);
    virtual ~GDBItemCommand();

    ValueCallback* getItem()      { return item_; }

private:
    ValueCallback* item_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
//class GDBReferenceCommand : public GDBItemCommand
//{
//public:
//  GDBReferenceCommand(VarItem* item);
//	virtual ~GDBReferenceCommand();
//};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class GDBSetBreakpointCommand : public GDBCommand
{
public:
    GDBSetBreakpointCommand(const QCString& setCommand, const Breakpoint* bp);
    virtual ~GDBSetBreakpointCommand();

    const Breakpoint* breakpoint() const        { return bp_; }

private:
    const Breakpoint* bp_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
