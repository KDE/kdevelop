/***************************************************************************
                          brkptmanager.h  -  description                              
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

#ifndef __brkptmanager_h__
#define __brkptmanager_h__

#include <qlistbox.h>

class Breakpoint;
class QMouseEvent;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

//TODO derive a GDB specific manager to handle the GDB specific
// data in the parsers!!

class BreakpointManager : public QListBox
{
  Q_OBJECT

public:
  BreakpointManager(QWidget* parent=0, const char* name=0, WFlags f=0);
  virtual ~BreakpointManager();

  void reset();
  void removeAll();
  void refreshBP(const QString& filename);

private:
  void setActive();
  void addBreakpoint(Breakpoint* BP);
  void removeBreakpoint(int index);
  void modifyBreakpoint(int index);

  int findIndex(const Breakpoint* BP) const;
  Breakpoint* findId(int id) const;
  Breakpoint* findKey(int BPKey) const;

  void mouseReleaseEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent *e);
  void breakpointPopup(Breakpoint *BP);

public slots:
  void slotToggleStdBreakpoint(const QString& filename, int lineNo, bool RMBClicked);
  void slotToggleWatchpoint(const QString& varName);

  void slotParseGDBBrkptList(char* str);
  void slotParseGDBBreakpointSet(char* str, int BPKey);

  void slotSetPendingBPs();
  void slotRemoveBreakpoint();
  void slotModifyBreakpoint();
  void slotGotoBreakpointSource();
 	void slotUnableToSetBPNow(int BPNo);

signals:
  void publishBPState(Breakpoint* brkpt);
  void refreshBPState(const Breakpoint* brkpt);
  void gotoSourcePosition(const QString& filename, int lineno);

private:
  int activeFlag_;
};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
