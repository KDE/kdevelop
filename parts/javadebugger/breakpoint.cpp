/***************************************************************************
                          breakpoint.cpp  -  description
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

#include "breakpoint.h"
#include "breakpointdlg.h"

#include <klocale.h>

#include <qfileinfo.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qstring.h>

#include <stdio.h>

namespace JAVADebugger
{

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

static int BPKey_ = 0;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

Breakpoint::Breakpoint(bool temporary, bool enabled)
    : QListBoxItem (),
      display_(QString::null),
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
      condition_(QString::null)
{
}

/***************************************************************************/

Breakpoint::~Breakpoint()
{
}

/***************************************************************************/

int Breakpoint::height(const QListBox *lb) const
{
    return lb->fontMetrics().lineSpacing() + 1 ;
}

/***************************************************************************/

int Breakpoint::width(const QListBox *lb) const
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

QString Breakpoint::text () const
{
    return display_;
}

/***************************************************************************/

void Breakpoint::configureDisplay()
{
    if (s_temporary_)
        display_ += i18n("\ttemporary");

    if (!s_enabled_)
        display_ += i18n("\tdisabled");

    if (!condition_.isEmpty())
        display_ += i18n("\tif %1").arg(condition_);

    if (hits_)
        display_ += i18n("\thits %1").arg(hits_);

    if (ignoreCount_)
        display_ += i18n("\tignore count %1").arg(ignoreCount_);

    if (s_hardwareBP_)
        display_ = i18n("hw %1").arg(display_);

    if (dbgId_>0) {
        QString t(display_);
        display_ = QString("%1 %2").arg(dbgId_).arg(display_);
    }

    if (s_pending_) {
        QString pending(i18n("Breakpoint state. The 'Pending ' state is the first state displayed",
                             "Pending "));
        if (s_actionAdd_)
            pending += i18n("Breakpoint state. The 'add ' state is appended to the other states",
                            "add ");
        if (s_actionClear_)
            pending += i18n("Breakpoint state. The 'clear ' state is appended to the other states",
                            "clear ");
        if (s_actionModify_)
            pending += i18n("Breakpoint state. The 'modify ' state is appended to the other states",
                            "modify ");

        display_ = i18n("%1>\t%2").arg(pending).arg(display_);
    }
}

/***************************************************************************/

QString Breakpoint::dbgRemoveCommand() const
{
    if (dbgId_>0)
        return QString("delete %1").arg(dbgId_); // jdb command - not translatable

    return QString();
}

/***************************************************************************/

bool Breakpoint::hasSourcePosition() const
{
    return false;
}

/***************************************************************************/

QString Breakpoint::fileName() const
{
    return QString();
}

/***************************************************************************/

int Breakpoint::lineNum() const
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

    if (s_pending_ && !(s_actionAdd_ && s_actionModify_)) {
        s_pending_ = false;
        s_actionModify_ = false;
    }

    s_actionAdd_          = false;
    s_actionClear_        = false;
    s_actionDie_          = false;
    s_dbgProcessing_      = false;

    if (!s_actionModify_) {
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
    if (modifyBPDialog->exec()) {
        setConditional(modifyBPDialog->getConditional());
        setIgnoreCount(modifyBPDialog->getIgnoreCount());
        setEnabled(modifyBPDialog->isEnabled());
    }

    delete modifyBPDialog;
    return (s_changedCondition_ || s_changedIgnoreCount_ || s_changedEnable_);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

FilePosBreakpoint::FilePosBreakpoint(const QString &fileName, int lineNum,
                                     bool temporary, bool enabled)
    : Breakpoint(temporary, enabled),
      fileName_(fileName),
      lineNo_(lineNum)
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
    if (fileName_.isEmpty())
        cmdStr = QString("stop at %1").arg(lineNo_);  // jdb command - not translatable
    else {
        QFileInfo fi(fileName_);
        cmdStr = QString("stop at %1:%2").arg(fi.baseName()).arg(lineNo_); // jdb command
    }

    if (isTemporary())
        cmdStr = "t"+cmdStr;  // jdb command

    return cmdStr;
}

/***************************************************************************/

bool FilePosBreakpoint::match(const Breakpoint *brkpt) const
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
    QFileInfo fi(fileName_);
    display_ = i18n("breakpoint at %1:%2").arg(fi.baseName()).arg(lineNo_);
    Breakpoint::configureDisplay();
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

Watchpoint::Watchpoint(const QString& varName, bool temporary, bool enabled)
    : Breakpoint(temporary, enabled),
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
    return QString("watch ")+varName_;    // jdb command - not translatable
}

/***************************************************************************/

void Watchpoint::configureDisplay()
{
    display_ = i18n("watchpoint on %1").arg(varName_);
    Breakpoint::configureDisplay();
}

/***************************************************************************/

bool Watchpoint::match(const Breakpoint* brkpt) const
{
    // simple case
    if (this == brkpt)
        return true;

    // Type case
    const Watchpoint *check = dynamic_cast<const Watchpoint*>(brkpt);
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
// These are implemented in jdb but can cause a lot of breakpoints
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

// Most catch options arn't implemented in jdb so ignore this for now.

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
}
