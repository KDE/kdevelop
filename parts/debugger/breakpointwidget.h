/***************************************************************************
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

#ifndef _BREAKPOINTWIDGET_H_
#define _BREAKPOINTWIDGET_H_

#include <klistview.h>

class Breakpoint;
class QMouseEvent;
class BreakpointItem;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

//TODO derive a GDB specific manager to handle the GDB specific
// data in the parsers!!

class BreakpointWidget : public KListView
{
    Q_OBJECT
    
public:
    BreakpointWidget( QWidget* parent=0, const char* name=0 );
    virtual ~BreakpointWidget();
    
    enum Column {
        Status     = 0,
        File       = 1,
        Line       = 2,
        Hits       = 3,
        Condition  = 4
    }; 
    
public slots:
    void reset();
    void refreshBP(const QString &filename);
    
    // Connected to from the editor widget:
    void slotToggleBreakpoint(const QString &filename, int lineNum);
    void slotEditBreakpoint(const QString &fileName, int lineNum);
    void slotToggleBreakpointEnabled(const QString &fileName, int lineNum);

    // Connected to from the dbgcontroller:
    void slotSetPendingBPs();
    void slotUnableToSetBPNow(int BPNo);
    void slotParseGDBBrkptList(char *str);
    void slotParseGDBBreakpointSet(char *str, int BPKey);
  
private slots:
    void slotExecuted(QListViewItem *item);
    void slotContextMenu(QListViewItem *item);

signals:
    void publishBPState( const Breakpoint& );
    void refreshBPState( const Breakpoint& );
    void gotoSourcePosition(const QString &fileName, int lineNum);
    void clearAllBreakpoints();

private:
    friend class BreakpointItem;
    
    BreakpointItem* find(const Breakpoint& BP) const;
    BreakpointItem* findId(int id) const;
    BreakpointItem* findKey(int BPKey) const;

    void setActive();
    void removeAllBreakpoints();

private:
    int activeFlag_;
};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
