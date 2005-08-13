/***************************************************************************
    begin                : Tue May 13 2003
    copyright            : (C) 2003 by John Birch
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

#ifndef _BREAKPOINT_H_
#define _BREAKPOINT_H_

#include <klocale.h>

#include <qstring.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace RDBDebugger
{

enum BP_TYPES
{
    BP_TYPE_Invalid,
    BP_TYPE_FilePos,
    BP_TYPE_Watchpoint,
    BP_TYPE_Catchpoint,
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

    virtual QString statusDisplay(int activeFlag) const;
    virtual BP_TYPES type() const                   { return BP_TYPE_Invalid; }
    virtual QString displayType() const             { return i18n( "Invalid" ); }

    virtual QString location(bool compact=true)     = 0;
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
    bool s_changedEnable_       :1;
    
	int dbgId_;                         // assigned by gdb

    int key_;                           // internal unique key
    int active_;                        // counter incremented on receipt of all BP's

    int ignoreCount_;
    QString condition_;
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
    QString location(bool compact=true);
    void setLocation(const QString& location);
    bool isValid() const                        { return lineNo_>0 && !fileName_.isEmpty(); }

private:
    QString fileName_;
    int lineNo_;
};

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
    QString location(bool)                      { return varName_; }
    void setLocation(const QString& location)   { varName_ = location; }
    bool isValid() const                        { return !varName_.isEmpty(); }

private:
    QString varName_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class Catchpoint : public Breakpoint
{
public:
    Catchpoint(const QString &varName, bool temporary=false, bool enabled=true);
    virtual ~Catchpoint();
    virtual QString dbgSetCommand() const;
    bool match(const Breakpoint *brkpt) const;

    BP_TYPES type () const                      { return BP_TYPE_Catchpoint; }
    QString displayType() const                 { return i18n("Catchpoint"); }
    void setVarName(const QString& varName)     { varName_ = varName; }
    QString varName() const                     { return varName_; }
    QString location(bool)                      { return varName_; }
    void setLocation(const QString& location)   { varName_ = location; }
    bool isValid() const                        { return !varName_.isEmpty(); }

private:
    QString varName_;
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
    QString location(bool)                              { return m_functionName; };
    void setLocation(const QString& location)           { m_functionName = location; }
    bool isValid() const                                { return !m_functionName.isEmpty(); }

private:
    QString m_functionName;
};

}

#endif
