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
#include "gdbcontroller.h"
#include "gdbcommand.h"

#include <klocale.h>
#include <kdebug.h>

#include <qfileinfo.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qstring.h>

#include <stdio.h>
#include <typeinfo>

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
      s_hardwareBP_(false),
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

void Breakpoint::sendToGdb(GDBController* controller)
{
    // Need to issue 'modifyBreakpoint' when setting breakpoint is done
    controller_ = controller;

    // FIXME: should either make sure this widget is disabled
    // when needed, or implement simular logic.
    if (controller->stateIsOn(s_dbgNotStarted))
    {
        // Can't modify breakpoint now, will try again later.
        setPending(true);
        return;
    }

    setPending(false);

    bool restart = false;
    // FIXME: this will only catch command for which gdb 
    // produces the "^running" marker.
    // FIXME: this probably won't work if there are other
    // run commands in the thread already.
    if (controller->stateIsOn(s_appRunning))
    {
        kdDebug(9012) << "PAUSING APP\n";
        controller->pauseApp();
        restart = true;
    }
    
    if (isActionAdd())
    {
        // This prevents us from sending breakpoint command to
        // gdb for empty breakpoints, when user haven't even
        // typed function name, or address, or variable.
        //
        // Check for isDbgProcessing makes sure we don't issue
        // several -break-insert commands before getting
        // output from the first one.
        if (isValid() && !isDbgProcessing())
        {
            setBreakpoint(controller);
        }
    }
    else
    {
        if (isActionClear())
        {
            clearBreakpoint(controller);
        }
        else
        {
            if (isActionModify())
            {
                modifyBreakpoint(controller); 
            }
        }
    }

    if (restart) {
        kdDebug(9012) << "RESTARING APP\n";
        controller->addCommand(new GDBCommand("-exec-continue"));
    }
}

void Breakpoint::clearBreakpoint(GDBController* /*c*/)
{
    controller()->addCommand(
        new GDBCommand(dbgRemoveCommand(), 
                       this,
                       &Breakpoint::handleDeleted)); 
}

void Breakpoint::applicationExited(GDBController*) 
{
}


void Breakpoint::setBreakpoint(GDBController* controller)
{
    setDbgProcessing(true);

    // Don't use handler mechanism yet, because the reply
    // should contain internal id of breakpoint (not gdb id), so that we
    // can match gdb id with the breakpoint instance we've set.
            
    // Note that at startup we issue several breakpoint commands, so can't
    // just store last breakpoint. Need to stack of last breakpoint commands,
    // but that for later.
    //
    // When this command is finished, slotParseGDBBreakpointSet
    // will be called by the controller.
    controller->addCommand(
        new GDBCommand(dbgSetCommand(),                        
                       this,
                       &Breakpoint::handleSet));
}


void Breakpoint::modifyBreakpoint(GDBController* controller)
{
    controller->
        addCommand(
            new ModifyBreakpointCommand(QString("-break-condition %1 ") +
                                        conditional(), this));
    controller->
        addCommand(
            new ModifyBreakpointCommand(QString("-break-after %1 ") +
                                        QString::number(ignoreCount()), this));

    controller->
        addCommand(
            new ModifyBreakpointCommand(isEnabled() ? 
                                        QString("-break-enable %1") 
                                        : QString("-break-disable %1"), this));
}

void Breakpoint::removedInGdb()
{
    setActionDie();
    emit modified(this);
}


bool Breakpoint::match(const Breakpoint* breakpoint) const
{
    // simple case
    if (this == breakpoint)
        return true;

    // Type case
    if (typeid(*this) != typeid(*breakpoint))
        return false;

    return match_data(breakpoint);
}

/***************************************************************************/

QString Breakpoint::dbgRemoveCommand() const
{
    if (dbgId_>0)
        return QString("-break-delete %1").arg(dbgId_); // gdb command - not translatable

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
    // All breakpoint properties will be automatically sent to
    // gdb when breakpoint is first added, no matter what value
    // this field has.
    s_actionModify_       = false;
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

void Breakpoint::handleSet(const GDBMI::ResultRecord& r)
{
    // Try to find gdb id. It's a bit harder that it should be,
    // because field names differ depending on breakpoint type.

    int id = -1;
    
    if (r.hasField("bkpt"))
        id = r["bkpt"]["number"].literal().toInt();
    else if (r.hasField("wpt"))
        id = r["wpt"]["number"].literal().toInt();
    else if (r.hasField("hw-rwpt"))
        id = r["hw-rwpt"]["number"].literal().toInt();
    // We don't have access watchpoints in UI yet, but
    // for future.
    else if (r.hasField("hw-awpt"))
        id = r["hw-awpt"]["number"].literal().toInt();
    
    if (id == -1)
    {
        // If can't set because file not found yet,
        // will need to try later.
        setPending(true);
    }
    else
    {
        setActive(0 /* unused m_activeFlag */, id);
    }
    
    // Need to do this so that if breakpoint is not set
    // (because the file is not found)
    // we unset isDbgProcessing flag, so that breakpoint can
    // be set on next stop.
    setDbgProcessing(false);

    // Immediately call modifyBreakpoint to set all breakpoint
    // properties, such as condition.
    modifyBreakpoint(controller_);

    emit modified(this);
}

void Breakpoint::handleDeleted(const GDBMI::ResultRecord& /*r*/)
{
    kdDebug(9012) << "inside handleDeleted\n";
    setActionDie();
    if (FilePosBreakpoint* fp = dynamic_cast<FilePosBreakpoint*>(this))
    {
        kdDebug(9012) << "handleDeleted, line is " << fp->lineNum() << "\n";
    }
    emit modified(this);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

FilePosBreakpoint::FilePosBreakpoint()
: subtype_(filepos),
  line_(-1)
{}

FilePosBreakpoint::FilePosBreakpoint(const QString &fileName, int lineNum,
                                     bool temporary, bool enabled)
    : Breakpoint(temporary, enabled)
{
    // Sets 'subtype'
    setLocation(QString("%1:%2").arg(fileName).arg(lineNum));
}

FilePosBreakpoint::~FilePosBreakpoint()
{
}

QString FilePosBreakpoint::dbgSetCommand() const
{
    QString cmdStr;
    cmdStr = QString("-break-insert %1").arg(location_);

    if (isTemporary())
        cmdStr = cmdStr + " -t";

    return cmdStr;
}

bool FilePosBreakpoint::match_data(const Breakpoint *xb) const
{
    const FilePosBreakpoint* b = static_cast<const FilePosBreakpoint*>(xb);

    if (b)
        return location_ == b->location_;
    else
        return false;
}

QString FilePosBreakpoint::displayType() const
{
    return i18n("Code breakpoint", "Code"); 
}

bool FilePosBreakpoint::isValid() const
{
    return !location_.isEmpty();
}

bool FilePosBreakpoint::hasFileAndLine() const
{
    return line_ != -1;
}

QString FilePosBreakpoint::fileName() const
{
    return fileName_;
}

unsigned FilePosBreakpoint::lineNum() const
{
    return line_;
}



QString FilePosBreakpoint::location(bool compact) const
{
    if (subtype_ == filepos && hasFileAndLine() && compact)
    {
        return QFileInfo(fileName_).fileName()+":"+QString::number(line_);
    }
    else
    {
        return location_;
    }
}

/***************************************************************************/

void FilePosBreakpoint::setLocation(const QString& location)
{
    location_ = location;
   
    QRegExp regExp1("(.*):(\\d+)$");
    regExp1.setMinimal(true);
    if ( regExp1.search(location, 0) >= 0 )
    {
        subtype_ = filepos;

        QString t = regExp1.cap(1);
        QString dirPath = QFileInfo(t).dirPath();
        if ( dirPath == "." )
        {
            QString existingDirPath = QFileInfo(fileName_).dirPath();
            if (existingDirPath != ".")
                fileName_ = existingDirPath+"/"+regExp1.cap(1);
            else
                fileName_ = regExp1.cap(1);
        }
        else
            fileName_ = regExp1.cap(1);

        line_ = regExp1.cap(2).toInt();

        location_ = QString("%1:%2").arg(fileName_).arg(regExp1.cap(2));
    }
    else
    {
        // Could be address as well, but it's treated absolutely
        // the same everywhere.
        subtype_ = function;
    }
}

void FilePosBreakpoint::handleSet(const GDBMI::ResultRecord& r)
{
    // Below logic gets filename and line from gdb response, and
    // allows us to show breakpoint marker even for function
    // breakpoints. Unfortunately, 'fullname' field is available only in 
    // post-6.4 versions of gdb and if we try to use 'file', then
    // KDevelop won't be able to find that file to show the marker.
    if (r.hasField("bkpt"))
    {
        const GDBMI::Value& v = r["bkpt"];
        if (v.hasField("fullname") && v.hasField("line"))
        {
            fileName_ = v["fullname"].literal();
            line_ = v["line"].literal().toInt();        
        }
    }

    Breakpoint::handleSet(r);
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

void Watchpoint::setBreakpoint(GDBController* controller)
{    
    if (isEnabled())
    {
        setDbgProcessing(true);

        controller->addCommand(
            new GDBCommand(
                QString("-data-evaluate-expression &%1").arg(varName_),
                this,
                &Watchpoint::handleAddressComputed));
    }
}

void Watchpoint::handleAddressComputed(const GDBMI::ResultRecord& r)
{
    address_ = r["value"].literal().toULongLong(0, 16);
    controller()->addCommand(
        new GDBCommand(
            QString("-break-watch *%1").arg(r["value"].literal()),
            static_cast<Breakpoint*>(this),
            &Watchpoint::handleSet));
}

void Watchpoint::applicationExited(GDBController* c)
{
    if (!c->stateIsOn(s_dbgNotStarted))
    {
        // Note: not using 'clearBreakpoint' as it will delete breakpoint
        // completely.

        controller()->addCommand(
            new GDBCommand(dbgRemoveCommand()));
        setDbgId(-1);
        setEnabled(false);
        setActionAdd(true);
        address_ = static_cast<unsigned long long>(-1);
        emit modified(this);
    }
}

void Watchpoint::removedInGdb()
{
    // Do nothing. Watchpoints must be preserved
    // even if they are gone in gdb.
}

/***************************************************************************/

QString Watchpoint::dbgSetCommand() const
{
    return QString("-break-watch ")+varName_;    // gdb command - not translatable
}

/***************************************************************************/

bool Watchpoint::match_data(const Breakpoint* xb) const
{
    const Watchpoint* b = static_cast<const Watchpoint*>(xb);

    return (varName_ == b->varName_);
}

ReadWatchpoint::ReadWatchpoint(const QString& varName, bool temporary, bool enabled)
    : Watchpoint(varName, temporary, enabled)
{
}

QString ReadWatchpoint::dbgSetCommand() const
{
   return QString("-break-watch -r ")+varName();
}

bool ReadWatchpoint::match_data(const Breakpoint* xb) const
{
    const ReadWatchpoint* b = static_cast<const ReadWatchpoint*>(xb);

    return (varName() == b->varName());
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

#include "breakpoint.moc"
