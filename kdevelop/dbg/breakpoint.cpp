/***************************************************************************
                          breakpoint.cpp  -  description
                             -------------------                                         
    begin                : Sun Aug 8 1999                                           
    copyright            : (C) 1999 by John Birch
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

#include "breakpoint.h"
#include "breakpointdialog.h"

#include <qfileinfo.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qstring.h>

#include <stdio.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

static int BPKey_ = 0;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
Breakpoint::Breakpoint(bool temporary, bool enabled) :
  QListBoxItem (),
  s_pending_(true),
  s_actionAdd_(false),
  s_actionClear_(false),
  s_actionModify_(false),
  s_actionDie_(false),
  s_dbgProcessing_(false),
  s_enabled_(enabled),
  s_temporary_(temporary),
  s_changedCondition_(false),
  s_changedIgnoreCount_(false),
  s_changedEnable_(false),
  s_hardwareBP_(false),
  dbgId_(-1),
  hits_(0),
  key_(BPKey_++),
  active_(0),
  ignoreCount_(0),
  condition_("")
{
  *display_ = 0;
}

/***************************************************************************/

Breakpoint::~Breakpoint()
{
}

/***************************************************************************/

int Breakpoint::height(const QListBox *lb ) const
{
  return lb->fontMetrics().lineSpacing() + 1 ;
}

/***************************************************************************/

int Breakpoint::width(const QListBox *lb ) const
{
  return lb->fontMetrics().width( text() ) + 6;
}

/***************************************************************************/

void Breakpoint::paint( QPainter *p )
{
  QFontMetrics fm = p->fontMetrics();
  int yPos = fm.ascent() + fm.leading()/2;
  p->drawText( 0, yPos, text() );
}

/***************************************************************************/

const char* Breakpoint::text () const
{
  return display_;
}

/***************************************************************************/

void Breakpoint::configureDisplay()
{
  if (s_temporary_)
    strcat(display_, "\ttemporary");

  if (!s_enabled_)
    strcat(display_, "\tdisabled");

  if (!condition_.isEmpty())
  {
    QString t(QString().sprintf("\tif %s", condition_.data()));
    strcat(display_, t.data());
  }

  if (hits_)
  {
    QString t(QString().sprintf("\thits %d", hits_));
    strcat(display_, t.data());
  }

  if (ignoreCount_)
  {
    QString t(QString().sprintf("\tignore count %d", ignoreCount_));
    strcat(display_, t.data());
  }

  if (s_hardwareBP_)
  {
    QString t(display_);
    sprintf(display_, "hw %s", t.data());
  }

  if (dbgId_>0)
  {
    QString t(display_);
    sprintf(display_, "%d %s", dbgId_, t.data());
  }

  if (s_pending_)
  {
    QString pending("Pending ");
    if (s_actionAdd_)
      pending += "add ";
    if (s_actionClear_)
      pending += "clear ";
    if (s_actionModify_)
      pending += "modify ";

    QString t(display_);
    sprintf(display_, "%s>\t%s", pending.data(), t.data());
  }

  ASSERT(strlen(display_) < 199);   //  eeekkk!
}

/***************************************************************************/

QString Breakpoint::dbgRemoveCommand() const
{
	if (dbgId_>0)
		return QString().sprintf("delete %d", dbgId_);

	return QString();
}

/***************************************************************************/

bool Breakpoint::hasSourcePosition() const
{
  return false;
}

/***************************************************************************/

QString Breakpoint::filename() const
{
  return QString();
}

/***************************************************************************/

int Breakpoint::lineNo() const
{
  return 0;
}

/***************************************************************************/

// called when debugger ends
void Breakpoint::reset()
{
  dbgId_                = -1;
  s_pending_            = true;
  s_actionAdd_          = true;     // waiting for the debugger to start
  s_actionClear_        = false;
  s_changedCondition_   = !condition_.isEmpty();
  s_changedIgnoreCount_ = (ignoreCount_>0);
  s_changedEnable_      = !s_enabled_;
  s_actionModify_       = s_changedCondition_ || s_changedIgnoreCount_ || s_changedEnable_;
  s_dbgProcessing_      = false;
  s_hardwareBP_         = false;
  hits_                 = 0;

  configureDisplay();
}

/***************************************************************************/

void Breakpoint::setActive(int active, int id)
{
  active_           = active;
  dbgId_            = id;

  if (s_pending_ && !(s_actionAdd_ && s_actionModify_))
  {
    s_pending_ = false;
    s_actionModify_ = false;
  }

  s_actionAdd_          = false;
  s_actionClear_        = false;
  s_actionDie_          = false;
  s_dbgProcessing_      = false;

  if (!s_actionModify_)
  {
    s_changedCondition_   = false;
    s_changedIgnoreCount_ = false;
    s_changedEnable_      = false;
  }

  configureDisplay();
}
/***************************************************************************/

bool Breakpoint::modifyDialog()
{
  BPDialog* modifyBPDialog = new BPDialog(this);
  if (modifyBPDialog->exec())
  {
    if (condition_ != modifyBPDialog->getConditional())
    {
      setConditional(modifyBPDialog->getConditional());
      s_changedCondition_ = true;
    }

    if (ignoreCount_ != modifyBPDialog->getIgnoreCount())
    {
      setIgnoreCount(modifyBPDialog->getIgnoreCount());
      s_changedIgnoreCount_ = true;
    }

    if (s_enabled_ != modifyBPDialog->isEnabled())
    {
      setEnabled(modifyBPDialog->isEnabled());
      s_changedEnable_ = true;
    }
  }

  delete modifyBPDialog;
  return (s_changedCondition_ || s_changedIgnoreCount_ || s_changedEnable_);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

FilePosBreakpoint::FilePosBreakpoint(
        const QString& fileName, int lineNo, bool temporary, bool enabled) :
  Breakpoint(temporary, enabled),
  fileName_(fileName),
  lineNo_(lineNo)
{
  configureDisplay();
}

/***************************************************************************/

FilePosBreakpoint::~FilePosBreakpoint()
{
}

/***************************************************************************/

QString FilePosBreakpoint::dbgSetCommand() const
{
  QString cmdStr;
  if (fileName_ == "")
    cmdStr.sprintf("break %d",lineNo_);
  else
  {
    QFileInfo fi(fileName_);
    cmdStr.sprintf("break %s:%d", fi.fileName().data(), lineNo_);
  }

  if (isTemporary())
    cmdStr = "t"+cmdStr;

  return cmdStr;
}

/***************************************************************************/

bool FilePosBreakpoint::match(const Breakpoint* brkpt) const
{
  // simple case
  if (this == brkpt)
    return true;

  // Type case
  const FilePosBreakpoint* check = dynamic_cast<const FilePosBreakpoint*>(brkpt);
  if (!check)
    return false;

  // member case
  return  ( (fileName_ == check->fileName_) &&
            (lineNo_ == check->lineNo_));
}

/***************************************************************************/

void FilePosBreakpoint::configureDisplay()
{
  ::sprintf(display_, "breakpoint at %s:%d", fileName_.data(), lineNo_);
  Breakpoint::configureDisplay();
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

Watchpoint::Watchpoint(
               const QString& varName, bool temporary, bool enabled) :
  Breakpoint(temporary, enabled),
  varName_(varName)
{
  configureDisplay();
}

/***************************************************************************/

Watchpoint::~Watchpoint()
{
}

/***************************************************************************/

QString Watchpoint::dbgSetCommand() const
{
  return QString("watch ")+varName_;
}

/***************************************************************************/

void Watchpoint::configureDisplay()
{
  sprintf(display_, "watchpoint on %s", varName_.data());
  Breakpoint::configureDisplay();
}

/***************************************************************************/

bool Watchpoint::match(const Breakpoint* brkpt) const
{
  // simple case
  if (this == brkpt)
    return true;

  // Type case
  const Watchpoint* check = dynamic_cast<const Watchpoint*>(brkpt);
  if (!check)
    return false;

  // member case
  return (varName_ == check->varName_);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

//ExitBreakpoint::ExitBreakpoint(bool temporary, bool enabled) :
//  Breakpoint(temporary, enabled)
//{
//}
//
///***************************************************************************/
//
//ExitBreakpoint::~ExitBreakpoint()
//{
//}
//
///***************************************************************************/
//
//QString ExitBreakpoint::dbgSetCommand() const
//{
//  return "";
//}
//
///***************************************************************************/
//
//void ExitBreakpoint::configureDisplay()
//{
//  *display_ = 0;
//  Breakpoint::configureDisplay();
//}
//
///***************************************************************************/
//
//bool ExitBreakpoint::match(const Breakpoint* brkpt) const
//{
//  // simple case
//  if (this == brkpt)
//    return true;
//
//  // Type case
//  const ExitBreakpoint* check = dynamic_cast<const ExitBreakpoint*>(brkpt);
//  if (!check)
//    return false;
//
//  // member case
//  return true;
//}
//
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
//
// These are implemented in gdb but can cause a lot of breakpoints
// to be set. This needs more thought before being implemented

//RegExpBreakpoint::RegExpBreakpoint(bool temporary, bool enabled) :
//  Breakpoint(temporary, enabled)
//{
//}
//
///***************************************************************************/
//
//RegExpBreakpoint::~RegExpBreakpoint()
//{
//}
//
///***************************************************************************/
//
//QString RegExpBreakpoint::dbgSetCommand() const
//{
//  return "";
//}
//
///***************************************************************************/
//
////QString RegExpBreakpoint::dbgRemoveCommand() const
////{
////  return "";
////}
//
///***************************************************************************/
//
//void RegExpBreakpoint::configureDisplay()
//{
//  *display_ = 0;
//  Breakpoint::configureDisplay();
//}
//
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

// Most catch options arn't implemented in gdb so ignore this for now.

//CatchBreakpoint::CatchBreakpoint(bool temporary, bool enabled) :
//  Breakpoint(temporary, enabled)
//{
//}
//
///***************************************************************************/
//
//CatchBreakpoint::~CatchBreakpoint()
//{
//}
//
///***************************************************************************/
//
//QString CatchBreakpoint::dbgSetCommand() const
//{
//  return "";
//}
//
///***************************************************************************/
//
////QString CatchBreakpoint::dbgRemoveCommand() const
////{
////  return "";
////}
//
///***************************************************************************/
//
//void CatchBreakpoint::configureDisplay()
//{
//  *display_ = 0;
//  Breakpoint::configureDisplay();
//}
//
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

