/***************************************************************************
                          breakpoint.h  -  description
                             -------------------
    begin                : Tue Aug 10 1999
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

#ifndef _BREAKPOINT_H_
#define _BREAKPOINT_H_

#include <qstring.h>
#include <qlistbox.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace JAVADebugger
{

class Breakpoint : public QListBoxItem
{
public:
    Breakpoint(bool temporary=false, bool enabled=true);
    virtual ~Breakpoint();

    virtual QString dbgSetCommand() const                 = 0;
    virtual QString dbgRemoveCommand() const;
    virtual bool match(const Breakpoint* brkpt) const     = 0;
    virtual QString text () const;
    virtual void configureDisplay();
    virtual bool modifyDialog();
    virtual void reset();

    int height(const QListBox *lb) const;
    int width(const QListBox *lb) const;
    void paint(QPainter *p);

    void setActive(int active, int id);
    bool isActive(int active) const  {  return (  (active_ == active) ||
                                                  (s_pending_ && !s_actionClear_) ); }

    void setEnabled(bool enabled)                 { s_changedEnable_ = (s_enabled_ != enabled);             s_enabled_ = enabled; }
    bool isEnabled() const                        { return s_enabled_; }
    void setTemporary(bool temporary)             { s_temporary_ = temporary; }
    bool isTemporary() const                      { return s_temporary_; }
    void setHardwareBP(bool hardwareBP)           { s_hardwareBP_ = hardwareBP; }
    bool isHardwareBP() const                     { return s_hardwareBP_; }
    void setIgnoreCount(int ignoreCount)          { s_changedIgnoreCount_ = (ignoreCount_ != ignoreCount);  ignoreCount_ = ignoreCount; }
    int ignoreCount() const                       { return ignoreCount_; }
    void setAddress(const QString &address)       { address_ = address; }
    QString address() const                       { return address_; }
    void setConditional(const QString &condition) { s_changedCondition_ = (condition_ != condition);        condition_ = condition; }
    QString conditional() const                   { return condition_; }

    bool changedCondition()                       { return s_changedCondition_; }
    bool changedIgnoreCount()                     { return s_changedIgnoreCount_; }
    bool changedEnable()                          { return s_changedEnable_; }

    void setPending(bool pending)                 { s_pending_ = pending; }
    bool isPending() const                        { return s_pending_; }
    void setActionAdd(bool actionAdd)             { s_actionAdd_ = actionAdd; }
    bool isActionAdd() const                      { return s_actionAdd_; }
    void setActionClear(bool actionClear)         { s_actionClear_ = actionClear; }
    bool isActionClear() const                    { return s_actionClear_; }
    void setActionModify(bool actionModify)       { s_actionModify_ = actionModify; }
    bool isActionModify() const                   { return s_actionModify_; }
    void setDbgProcessing(bool dbgProcessing)     { s_dbgProcessing_ = dbgProcessing; }
    bool isDbgProcessing() const                  { return s_dbgProcessing_; }
    void setActionDie()                           { s_actionDie_ = true; s_actionClear_ = false; }
    bool isActionDie() const                      { return s_actionDie_; }

    int key() const                               { return key_; }
    void setDbgId(int dbgId)                      { dbgId_ = dbgId; }
    int  dbgId() const                            { return dbgId_; }
    void setHits(int hits)                        { hits_ = hits; }
    int  hits() const                             { return hits_; }

    virtual bool hasSourcePosition() const;
    virtual QString fileName() const;
    virtual int lineNum() const;

protected:
    // Ugggghh - this needs to be removed - it's
    // for the listbox display which seems to get confused
    // if you just change the strings position
    // (eg QString.data()). It works like this but...
    QString display_;

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
    bool s_hardwareBP_          :1;     // assigned by jdb

    int dbgId_;                         // assigned by jdb
    int hits_;                          // assigned by jdb

    int key_;                           // internal unique key
    int active_;                        // counter incremented on receipt of all BP's

    int ignoreCount_;
    QString address_;
    QString condition_;
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
    virtual void configureDisplay();

    QString fileName() const          { return fileName_; }
    int lineNum() const               { return lineNo_; }
    bool hasSourcePosition() const    { return true; }

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
//  virtual void configureDisplay();
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
//  virtual void configureDisplay();
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
    virtual void configureDisplay();

private:
    QString varName_;
};

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
