/***************************************************************************
                          jdbcommand.h  -  description
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

#ifndef _JDBCOMMAND_H_
#define _JDBCOMMAND_H_

#include "dbgcommand.h"

namespace JAVADebugger
{

class Breakpoint;
class VarItem;


// sigh - namespace's don't work on some of the older compilers
enum JDBCmd
{
  BLOCK_START     = '\32',
  SRC_POSITION    = '\32',    // Hmmm, same value may not work for all compilers

  BPLIST          = 'B',
  SET_BREAKPT     = 'b',

  DATAREQUEST     = 'D',
  DISASSEMBLE     = 'd',

  FRAME           = 'F',
  FILE_START      = 'f',

  INITIALISE      = 'I',
  IDLE            = 'i',

  LOCALS          = 'L',
  LIBRARIES       = 'l',

  MEMDUMP         = 'M',

  RUN             = 'R',
  REGISTERS       = 'r',

  PROGRAM_STOP    = 'S',

  STEP            = 's',

  BACKTRACE       = 'T',

  SETWATCH        = 'W',
  UNSETWATCH      = 'w',

  DETACH          = 'z',

  WAIT            = '0'
};

#define RUNCMD      (true)
#define NOTRUNCMD   (false)
#define INFOCMD     (true)
#define NOTINFOCMD  (false)

/**
 * @author John Birch
 */

class JDBCommand : public DbgCommand
{
public:
    JDBCommand(const QCString& command, bool isRunCmd=false, bool isInfoCmd=true,
               char prompt=WAIT);
    virtual ~JDBCommand();

private:
    static QCString idlePrompt_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class JDBItemCommand : public JDBCommand
{
public:
    JDBItemCommand(VarItem *item, const QCString &command,
                   bool isRunCmd=false, char prompt=DATAREQUEST);
    virtual ~JDBItemCommand();

    VarItem *getItem()      { return item_; }

private:
    VarItem *item_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class JDBPointerCommand : public JDBItemCommand
{
public:
  JDBPointerCommand(VarItem *item);
  virtual ~JDBPointerCommand();
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
//class JDBReferenceCommand : public JDBItemCommand
//{
//public:
//  JDBReferenceCommand(VarItem* item);
//	virtual ~JDBReferenceCommand();
//};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class JDBSetBreakpointCommand : public JDBCommand
{
public:
    JDBSetBreakpointCommand(const QCString& setCommand, int key);
    virtual ~JDBSetBreakpointCommand();

    int getKey() const        { return key_; }

private:
    int key_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
