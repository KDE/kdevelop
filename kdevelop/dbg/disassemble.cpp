/***************************************************************************
                          disassemble.cpp  -  description                              
                             -------------------                                         
    begin                : Tues Jan 3 2000                                           
    copyright            : (C) 2000 by John Birch
    email                : jb.nz@writeme.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "disassemble.h"

#include <kapp.h>
#include <keditcl.h>

#include <qdict.h>
#include <qheader.h>

#include <stdlib.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

Disassemble::Disassemble(KApplication* a,QWidget* parent,char* name) :
  KEdit(a, parent, name),
  active_(false),
  lowerBound_(0),
  upperBound_(0)
{
//  setCaption("Disassemble");
  setFont(a->fixedFont);
}

/***************************************************************************/

Disassemble::~Disassemble()
{
}

/***************************************************************************/

bool Disassemble::displayCurrent()
{
  long address = strtol(currentAddress_.data(), 0, 0);
  if (address < lowerBound_ || address > upperBound_)
    return false;

  int line;
  bool found = false;
  for (line=0; line < numLines(); line++)
  {
    if (strncmp(textLine(line), currentAddress_.data(), currentAddress_.length()) == 0)
    {
      found = true;
      // put cursor at start of line and highlight the line
      int col = qstrlen(textLine(line));
      setCursorPosition(line, col);
      setCursorPosition(line, 0, true);
      break;
    }
  }

  return found;
}

/***************************************************************************/

void Disassemble::slotBPState(const Breakpoint* BP)
{
  if (!active_)
    return;
}

/***************************************************************************/

void Disassemble::slotDisassemble(char* buf)
{
  if (!active_)
    return;

  clear();
  insertLine(strchr(buf, '\n')+1);
  removeLine(numLines()-1);
  removeLine(numLines()-1);

  if (numLines())
  {
    lowerBound_ = strtol(textLine(0), 0, 0);
    upperBound_ = strtol(textLine(numLines()-1), 0, 0);
    displayCurrent();
  }
  else
  {
    lowerBound_ = 0;
    upperBound_ = 0;
  }
}

/***************************************************************************/

void Disassemble::slotActivate(bool activate)
{
  if (active_ != activate)
  {
    active_ = activate;;
    if (active_)
      emit disassemble("", "");
  }
}

/***************************************************************************/

void Disassemble::slotShowStepInSource(const QString& filename, int lineno,
                                        const QString& address)
{
  currentAddress_ = address;
  if (!active_)
    return;

  if (!displayCurrent())
    emit disassemble("", "");
}

/***************************************************************************/
