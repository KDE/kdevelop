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

#ifndef _RDBCOMMAND_H_
#define _RDBCOMMAND_H_

#include "dbgcommand.h"

namespace RDBDebugger
{

class Breakpoint;
class VarItem;

// sigh - namespace's don't work on some of the older compilers
enum RDBCmd
{
  CONSTANTS     = 'C',
  CVARS         = 'V',    
  IVARS         = 'I',    
  LOCALS        = 'L'
};

#define RUNCMD      (true)
#define NOTRUNCMD   (false)
#define INFOCMD     (true)
#define NOTINFOCMD  (false)

/**
 * @author John Birch
 */

class RDBCommand : public DbgCommand
{
public:
    RDBCommand(const QCString& command, bool isRunCmd=false, bool isInfoCmd=true);
    virtual ~RDBCommand();

private:
    static QCString idlePrompt_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class RDBItemCommand : public RDBCommand
{
public:
    RDBItemCommand(VarItem *item, const QCString &command,
                   bool isRunCmd=false);
    virtual ~RDBItemCommand();

    VarItem *getItem()      { return item_; }

private:
    VarItem *item_;
};


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class RDBSetBreakpointCommand : public RDBCommand
{
public:
    RDBSetBreakpointCommand(const QCString& setCommand, int key);
    virtual ~RDBSetBreakpointCommand();

    int getKey() const        { return key_; }

private:
    int key_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
