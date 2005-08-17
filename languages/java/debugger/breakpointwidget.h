/***************************************************************************
                          brkptmanager.h  -  description
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

#ifndef _BREAKPOINTWIDGET_H_
#define _BREAKPOINTWIDGET_H_

#include <klistbox.h>
//Added by qt3to4:
#include <QMouseEvent>

namespace JAVADebugger
{


class Breakpoint;
class QMouseEvent;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/// @todo derive a JDB specific manager to handle the JDB specific
/// data in the parsers!!

class BreakpointWidget : public KListBox
{
    Q_OBJECT

public:
    BreakpointWidget( QWidget* parent=0, const char* name=0 );
    virtual ~BreakpointWidget();

    void reset();
    void refreshBP(const QString &filename);

public slots:
    // Connected to from the editor widget:
    void slotToggleBreakpoint(const QString &filename, int lineNum);
    void slotEditBreakpoint(const QString &fileName, int lineNum);
    void slotToggleBreakpointEnabled(const QString &fileName, int lineNum);

    // Conncected to from the variable widget:
    void slotToggleWatchpoint(const QString &varName);

    // Connected to from the dbgcontroller:
    void slotSetPendingBPs();
    void slotUnableToSetBPNow(int BPNo);
    void slotParseJDBBrkptList(char *str);
    void slotParseJDBBreakpointSet(char *str, int BPKey);

private slots:
    void slotExecuted(Q3ListBoxItem *item);
    void slotContextMenu(Q3ListBoxItem *item);

signals:
    void publishBPState(Breakpoint *brkpt);
    void refreshBPState(Breakpoint *brkpt);
    void gotoSourcePosition(const QString &fileName, int lineNum);
    void clearAllBreakpoints();

private:
    int findIndex(const Breakpoint *BP) const;
    Breakpoint *findId(int id) const;
    Breakpoint *findKey(int BPKey) const;

    void setActive();
    void addBreakpoint(Breakpoint *BP);
    void removeBreakpoint(Breakpoint *BP);
    void modifyBreakpoint(Breakpoint *BP);
    void toggleBPEnabled(Breakpoint *BP);
    void removeAllBreakpoints();

private:
  int activeFlag_;
};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
}

#endif
