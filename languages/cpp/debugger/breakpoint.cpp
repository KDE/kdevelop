/***************************************************************************
    begin                : Tue May 13 2003
    copyright            : (C) 2003 by John Birch
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

#include <klocale.h>

#include <qfileinfo.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qstring.h>

#include <stdio.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

static int BPKey_ = 0;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

Breakpoint::Breakpoint(bool temporary, bool enabled)
    : s_pending_(true),
      s_actionAdd_(true),
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
      s_changedTracing_(false),
      s_tracingEnabled_(false),
      s_traceFormatStringEnabled_(false),

      dbgId_(-1),
      hits_(0),
      key_(BPKey_++),
      active_(-1),
      ignoreCount_(0),
      condition_("")
{
}

/***************************************************************************/

Breakpoint::~Breakpoint()
{
}

/***************************************************************************/

QString Breakpoint::dbgRemoveCommand() const
{
    if (dbgId_>0)
        return QString("delete %1").arg(dbgId_); // gdb command - not translatable

    return QString();
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
    active_               = -1;
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
}

/***************************************************************************/

QString Breakpoint::statusDisplay(int activeFlag) const
{
    QString status="";
    if (!s_enabled_)
        status = i18n("Disabled");
    else
    if (s_pending_)
    {
        if (s_actionAdd_)
            status = i18n("Pending (add)");
        if (s_actionClear_)
            status = i18n("Pending (clear)");
        if (s_actionModify_)
            status = i18n("Pending (modify)");
    }
    else
    if (isActive(activeFlag))
        status = i18n("Active");

    return status;
}

QString Breakpoint::traceRealFormatString() const
{
    QString result;

    if (traceFormatStringEnabled())
    {
        result = traceFormatString();
    }
    else
    {
        result = "Tracepoint";
        if (const FilePosBreakpoint* fb 
            = dynamic_cast<const FilePosBreakpoint*>(this))
        {
            result += " at " + fb->location() + ": ";
        }
        else if (const FunctionBreakpoint* fb
                 = dynamic_cast<const FunctionBreakpoint*>(this))
        {
            result += " at " + location() + ": ";
        }
        else
        {
            result += " " + QString::number(key()) + ": ";
        }
        for(QStringList::const_iterator i = tracedExpressions_.begin(),
                e = tracedExpressions_.end(); i != e; ++i)
        {
            result += " " + *i + " = %d";
        }
    }

    // Quote the thing
    result = "\"" + result + "\\n\"";
    
    for(QStringList::const_iterator i = tracedExpressions_.begin(),
            e = tracedExpressions_.end(); i != e; ++i)
    {
        result += ", " + *i;
    }    

    return result;
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
        cmdStr = QString("break %1").arg(lineNo_);  // gdb command - not translatable
    else {
//        QFileInfo fi(fileName_);
        cmdStr = QString("break %1:%2").arg(fileName_).arg(lineNo_); // gdb command
    }

    if (isTemporary())
        cmdStr = "t"+cmdStr;  // gdb command

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

QString FilePosBreakpoint::location(bool compact) const
{
    if (compact)
        return QFileInfo(fileName_).fileName()+":"+QString::number(lineNo_);

    return fileName_+":"+QString::number(lineNo_);
}

/***************************************************************************/

void FilePosBreakpoint::setLocation(const QString& location)
{
    QRegExp regExp1("(.*):(\\d+)$");
    regExp1.setMinimal(true);
    if ( regExp1.search(location, 0) >= 0 )
    {
        QString t = regExp1.cap(1);
        QString dirPath = QFileInfo(t).dirPath();
        if ( dirPath == "." )
            fileName_ = QFileInfo(fileName_).dirPath()+"/"+regExp1.cap(1);
        else
            fileName_ = regExp1.cap(1);

        lineNo_ = regExp1.cap(2).toInt();
    }
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

Watchpoint::Watchpoint(const QString& varName, bool temporary, bool enabled)
    : Breakpoint(temporary, enabled),
      varName_(varName)
{
}

/***************************************************************************/

Watchpoint::~Watchpoint()
{
}

/***************************************************************************/

QString Watchpoint::dbgSetCommand() const
{
    return QString("watch ")+varName_;    // gdb command - not translatable
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

ReadWatchpoint::ReadWatchpoint(const QString& varName, bool temporary, bool enabled)
    : Watchpoint(varName, temporary, enabled)
{
}

QString ReadWatchpoint::dbgSetCommand() const
{
   return QString("rwatch ")+varName();
}

bool ReadWatchpoint::match(const Breakpoint* brkpt) const
{
    // simple case
    if (this == brkpt)
        return true;

    // Type case
    const ReadWatchpoint *check = dynamic_cast<const ReadWatchpoint*>(brkpt);
    if (!check)
        return false;

    // member case
    return (varName() == check->varName());
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

AddressBreakpoint::AddressBreakpoint(const QString& breakAddress, bool temporary, bool enabled)
    : Breakpoint(temporary, enabled),
      m_breakAddress(breakAddress)
{
}

/***************************************************************************/

AddressBreakpoint::~AddressBreakpoint()
{
}

/***************************************************************************/

QString AddressBreakpoint::dbgSetCommand() const
{
    return QString("break ")+m_breakAddress;    // gdb command - not translatable
}

/***************************************************************************/

bool AddressBreakpoint::match(const Breakpoint* brkpt) const
{
    // simple case
    if (this == brkpt)
        return true;

    // Type case
    const AddressBreakpoint *check = dynamic_cast<const AddressBreakpoint*>(brkpt);
    if (!check)
        return false;

    // member case
    return (m_breakAddress == check->m_breakAddress);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

FunctionBreakpoint::FunctionBreakpoint(const QString& functionName, bool temporary, bool enabled)
    : Breakpoint(temporary, enabled),
      m_functionName(functionName)
{
}

/***************************************************************************/

FunctionBreakpoint::~FunctionBreakpoint()
{
}

/***************************************************************************/

QString FunctionBreakpoint::dbgSetCommand() const
{
    return QString("break ")+m_functionName;    // gdb command - not translatable
}

/***************************************************************************/

bool FunctionBreakpoint::match(const Breakpoint* brkpt) const
{
    // simple case
    if (this == brkpt)
        return true;

    // Type case
    const FunctionBreakpoint *check = dynamic_cast<const FunctionBreakpoint*>(brkpt);
    if (!check)
        return false;

    // member case
    return (m_functionName == check->m_functionName);
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
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}
