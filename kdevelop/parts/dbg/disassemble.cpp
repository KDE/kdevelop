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

Disassemble::Disassemble(QWidget* parent,char* name) :
  QObject(parent, name),
//  OutputView(parent, name),
  active_(false),
  lower_(0),
  upper_(0),
  address_(0)
{
//  setCaption("Disassemble");
//  setFont(a->fixedFont);
}

/***************************************************************************/

Disassemble::~Disassemble()
{
}

/***************************************************************************/

bool Disassemble::displayCurrent()
{
  ASSERT(address_ >= lower_ || address_ <= upper_);
#warning FIXME
/*
  int line;
  int addressLength = currentAddress_.length();
  for (line=0; line < numLines(); line++)
  {
    if (strncmp(textLine(line), currentAddress_.data(), addressLength) == 0)
    {
      // put cursor at start of line and highlight the line
      setCursorPosition(line, qstrlen(textLine(line)));
      setCursorPosition(line, 0, true);
      return true;;
    }
  }
*/
  return false;
}

/***************************************************************************/

void Disassemble::slotBPState(const Breakpoint*)
{
  if (!active_)
    return;
}

/***************************************************************************/

void Disassemble::slotDisassemble(char* buf)
{
  if (!active_)
    return;

  //clear();
  // Skip the first line (just header info)
  char* start = strchr(buf, '\n');

  //Make sure there is something there
  if (start)
  {
#warning FIXME
/*
    insertLine(start+1);
    // Skip the last two lines (just trailer info)
    removeLine(numLines()-1);
    removeLine(numLines()-1);

    if (numLines())
    {
      lower_ = strtol(textLine(0), 0, 0);
      upper_ = strtol(textLine(numLines()-1), 0, 0);
      displayCurrent();
    }
    else
    {
      lower_ = 0;
      upper_ = 0;
    }
*/
  }
}

/***************************************************************************/

void Disassemble::slotActivate(bool activate)
{
  if (active_ != activate)
  {
    active_ = activate;;
    if (active_)
      getNextDisplay();
  }
}

/***************************************************************************/

void Disassemble::slotShowStepInSource(const QString&, int,
                                        const QString& currentAddress)
{
  currentAddress_ = currentAddress;
  address_ = strtol(currentAddress_.data(), 0, 0);
  if (!active_)
    return;

  if (address_ < lower_ || address_ > upper_ || !displayCurrent())
    getNextDisplay();
}

/***************************************************************************/

void Disassemble::getNextDisplay()
{
  if (address_)
  {
    ASSERT(currentAddress_);

    // restrict this to a managable size - some functions are _big_
    QString endAddress;
    endAddress.sprintf("0x%x", address_+128);
    emit disassemble(currentAddress_, endAddress);
  }
  else
    emit disassemble("", "");
}

/***************************************************************************/
