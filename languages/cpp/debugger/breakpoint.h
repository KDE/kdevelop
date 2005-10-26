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

#ifndef _BREAKPOINT_H_
#define _BREAKPOINT_H_

#include <klocale.h>

#include <qstring.h>
#include <qstringlist.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

enum BP_TYPES
{
    BP_TYPE_Invalid,
    BP_TYPE_FilePos,
    BP_TYPE_Watchpoint,
    BP_TYPE_ReadWatchpoint,
    BP_TYPE_Address,
    BP_TYPE_Function
};

class Breakpoint
{
public:
    Breakpoint(bool temporary=false, bool enabled=true);
    virtual ~Breakpoint();

    virtual QString dbgSetCommand() const                 = 0;
    virtual QString dbgRemoveCommand() const;
    virtual bool match(const Breakpoint* brkpt) const     = 0;
    virtual void reset();

    void setActive(int active, int id);
    bool isActive(int active) const                 { return (active_ == active) ||
                                                        (s_pending_ && !s_actionClear_); }
    void setEnabled(bool enabled)                   { s_changedEnable_ = (s_enabled_ != enabled);
                                                      s_enabled_ = enabled; }
    bool isEnabled() const                          { return s_enabled_; }
    void setTemporary(bool temporary)               { s_temporary_ = temporary; }
    bool isTemporary() const                        { return s_temporary_; }
    void setHardwareBP(bool hardwareBP)             { s_hardwareBP_ = hardwareBP; }
    bool isHardwareBP() const                       { return s_hardwareBP_; }
    void setIgnoreCount(int ignoreCount)            { s_changedIgnoreCount_ =
                                                          (ignoreCount_ != ignoreCount);
                                                      ignoreCount_ = ignoreCount; }
    int ignoreCount() const                         { return ignoreCount_; }
    void setAddress(const QString &address)         { address_ = address; }
    QString address() const                         { return address_; }
    void setConditional(const QString &condition)   { s_changedCondition_ =                                                          
                                                          (condition_ != condition);
                                                      condition_ = condition; }
    QString conditional() const                     { return condition_; }


    bool changedCondition() const                   { return s_changedCondition_; }
    bool changedIgnoreCount() const                 { return s_changedIgnoreCount_; }
    bool changedEnable() const                      { return s_changedEnable_; }

    void setPending(bool pending)                   { s_pending_ = pending; }
    bool isPending() const                          { return s_pending_; }
    void setActionAdd(bool actionAdd)               { s_actionDie_ = false;
                                                      s_actionAdd_ = actionAdd; }
    bool isActionAdd() const                        { return s_actionAdd_; }
    void setActionClear(bool actionClear)           { s_actionClear_ = actionClear; }
    bool isActionClear() const                      { return s_actionClear_; }
    void setActionModify(bool actionModify)         { s_actionDie_ = false;
                                                      s_actionModify_ = actionModify; }
    bool isActionModify() const                     { return s_actionModify_; }
    void setDbgProcessing(bool dbgProcessing)       { s_dbgProcessing_ = dbgProcessing; }
    bool isDbgProcessing() const                    { return s_dbgProcessing_; }
    void setActionDie()                             { s_actionDie_ = true;
                                                      s_actionClear_ = false; }
    bool isActionDie() const                        { return s_actionDie_; }

    int key() const                                 { return key_; }
    void setDbgId(int dbgId)                        { dbgId_ = dbgId; }
    int  dbgId() const                              { return dbgId_; }
    void setHits(int hits)                          { hits_ = hits; }
    int  hits() const                               { return hits_; }

    virtual QString statusDisplay(int activeFlag) const;
    virtual BP_TYPES type() const                   { return BP_TYPE_Invalid; }
    virtual QString displayType() const             { return i18n( "Invalid" ); }


    bool tracingEnabled() const                     { return s_tracingEnabled_; }
    void setTracingEnabled(bool enable)             { s_changedTracing_ |= 
                                                          (s_tracingEnabled_ != enable);
                                                      s_tracingEnabled_ = enable; }
    bool changedTracing() const                     { return s_changedTracing_; }
    void resetChangedTracing()                      { s_changedTracing_ = false; }

    const QStringList& tracedExpressions() const    { return tracedExpressions_; }
    void setTracedExpressions(const QStringList& l) { s_changedTracing_ |= 
                                                          (tracedExpressions_ != l);
                                                      tracedExpressions_ = l; }

    bool traceFormatStringEnabled() const           { return s_traceFormatStringEnabled_; }
    void setTraceFormatStringEnabled(bool en)       { s_changedTracing_ |=
                                                          (s_traceFormatStringEnabled_ != en);
                                                      s_traceFormatStringEnabled_ = en; }

    const QString& traceFormatString() const        { return traceFormatString_; }
    void setTraceFormatString(const QString& s)     { s_changedTracing_ |=
                                                          (traceFormatString_ != s);
                                                      traceFormatString_ = s; }

    QString traceRealFormatString() const;  

    virtual QString location(bool compact=true) const = 0;
    virtual void setLocation(const QString& )       = 0;
    virtual bool isValid() const                    = 0;

private:
    bool s_pending_             :1;
    bool s_actionAdd_           :1;
    bool s_actionClear_         :1;
    bool s_actionModify_        :1;
    bool s_actionDie_           :1;
    bool s_dbgProcessing_       :1;
    bool s_enabled_             :1;
    bool s_temporary_           :1;
    bool s_changedCondition_    :1;
    bool s_changedIgnoreCount_  :1;
    bool s_changedEnable_       :1;
    bool s_hardwareBP_          :1;     // assigned by gdb
    bool s_changedTracing_      :1;
    bool s_tracingEnabled_      :1;
    bool s_traceFormatStringEnabled_ :1;

    int dbgId_;                         // assigned by gdb
    int hits_;                          // assigned by gdb

    int key_;                           // internal unique key
    int active_;                        // counter incremented on receipt of all BP's

    int ignoreCount_;
    QString address_;
    QString condition_;
    QStringList tracedExpressions_;
    QString traceFormatString_;

//    QString type_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class FilePosBreakpoint : public Breakpoint
{
public:
    FilePosBreakpoint(const QString &fileName, int lineNum,
                      bool temporary=false, bool enabled=true);
    virtual ~FilePosBreakpoint();
    virtual QString dbgSetCommand() const;
    virtual bool match(const Breakpoint *brkpt) const;

    BP_TYPES type () const                      { return BP_TYPE_FilePos; }
    QString displayType() const                 { return i18n( "File:line" ); }
    void setFileName(const QString& fileName)   { fileName_ = fileName; }
    QString fileName() const                    { return fileName_; }
    void setLineNum(int lineNum)                { lineNo_ = lineNum; }
    int lineNum() const                         { return lineNo_; }
    QString location(bool compact=true) const;
    void setLocation(const QString& location);
    bool isValid() const                        { return lineNo_>0 && !fileName_.isEmpty(); }

private:
    QString fileName_;
    int lineNo_;
};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
//class RegExpBreakpoint : public Breakpoint
//{
//public:
//  RegExpBreakpoint(bool temporary=false, bool enabled=true);
//  virtual ~RegExpBreakpoint();
//  virtual QString dbgSetCommand() const;
//};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
//class CatchBreakpoint : public Breakpoint
//{
//public:
//  CatchBreakpoint(bool temporary=false, bool enabled=true);
//  virtual ~CatchBreakpoint();
//  virtual QString dbgSetCommand() const;
//  virtual CatchBreakpoint& operator=(const CatchBreakpoint& rhs);
//};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
//class ExitBreakpoint : public Breakpoint
//{
//public:
//  ExitBreakpoint(bool temporary=false, bool enabled=true);
//  virtual ~ExitBreakpoint();
//  virtual QString dbgSetCommand() const;
//  bool match(const Breakpoint* brkpt) const;
//  virtual void configureDisplay();
//};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class Watchpoint : public Breakpoint
{
public:
    Watchpoint(const QString &varName, bool temporary=false, bool enabled=true);
    virtual ~Watchpoint();
    virtual QString dbgSetCommand() const;
    bool match(const Breakpoint *brkpt) const;

    BP_TYPES type () const                      { return BP_TYPE_Watchpoint; }
    QString displayType() const                 { return i18n("Watchpoint"); }
    void setVarName(const QString& varName)     { varName_ = varName; }
    QString varName() const                     { return varName_; }
    QString location(bool) const                { return varName_; }
    void setLocation(const QString& location)   { varName_ = location; }
    bool isValid() const                        { return !varName_.isEmpty(); }

private:
    QString varName_;
};

class ReadWatchpoint : public Watchpoint
{
public:
    ReadWatchpoint(const QString &varName, bool temporary=false, bool enabled=true);
    virtual QString dbgSetCommand() const;
    bool match(const Breakpoint *brkpt) const;

    BP_TYPES type () const                      { return BP_TYPE_ReadWatchpoint; }
    QString displayType() const                 { return i18n("Read Watchpoint"); }
};


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class AddressBreakpoint : public Breakpoint
{
public:
    AddressBreakpoint(const QString &breakAddress, bool temporary=false, bool enabled=true);
    virtual ~AddressBreakpoint();
    virtual QString dbgSetCommand() const;
    bool match(const Breakpoint *brkpt) const;

    BP_TYPES type () const                              { return BP_TYPE_Address; }
    QString displayType() const                         { return i18n("Address"); }
    void setBreakAddress(const QString& breakAddress)   { m_breakAddress = breakAddress; }
    QString breakAddress() const                        { return m_breakAddress; }
    QString location(bool) const                        { return m_breakAddress; };
    void setLocation(const QString& location)           { m_breakAddress = location; }
    bool isValid() const                                { return !m_breakAddress.isEmpty(); }

private:
    QString m_breakAddress;
};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class FunctionBreakpoint : public Breakpoint
{
public:
    FunctionBreakpoint(const QString &functionName, bool temporary=false, bool enabled=true);
    virtual ~FunctionBreakpoint();
    virtual QString dbgSetCommand() const;
    bool match(const Breakpoint *brkpt) const;

    BP_TYPES type () const                              { return BP_TYPE_Function; }
    QString displayType() const                         { return i18n("Method()"); }
    void setfunctionName(const QString& functionName)   { m_functionName = functionName; }
    QString functionName() const                        { return m_functionName; }
    QString location(bool) const                        { return m_functionName; };
    void setLocation(const QString& location)           { m_functionName = location; }
    bool isValid() const                                { return !m_functionName.isEmpty(); }

private:
    QString m_functionName;
};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
