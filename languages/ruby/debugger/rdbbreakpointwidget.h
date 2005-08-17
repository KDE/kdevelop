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

#ifndef _RDBBreakpointWidget_H_
#define _RDBBreakpointWidget_H_

#include <q3hbox.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QFocusEvent>

class QDomElement;
class QToolButton;
class KURL;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace RDBDebugger
{
class Breakpoint;
class BreakpointTableRow;
class RDBTable;

class RDBBreakpointWidget : public Q3HBox
{
    Q_OBJECT

public:
    RDBBreakpointWidget( QWidget* parent=0, const char* name=0 );
    virtual ~RDBBreakpointWidget();

    void reset();

    void savePartialProjectSession(QDomElement* el);
    void restorePartialProjectSession(const QDomElement* el);


public slots:
    // Connected to from the editor widget:
    void slotToggleBreakpoint(const QString &filename, int lineNum);
    void slotToggleBreakpointEnabled(const QString &fileName, int lineNum);

    // Connected to from the variable widget:
    void slotToggleWatchpoint(const QString &varName);

    // Connected to from the dbgcontroller:
    void slotSetPendingBPs();
    void slotUnableToSetBPNow(int BPNo);
    void slotParseRDBBrkptList(char *str);
    void slotParseRDBBreakpointSet(char *str, int BPKey);

    void slotRefreshBP(const KURL &filename);
    
protected:
    enum BW_ITEMS { BW_ITEM_Show, BW_ITEM_Edit, BW_ITEM_Disable, BW_ITEM_Delete };
    virtual void focusInEvent(QFocusEvent *e);

private slots:
    void slotRemoveBreakpoint();
    void slotRemoveAllBreakpoints();
    void slotEditBreakpoint(const QString &fileName, int lineNum);
    void slotEditBreakpoint();
    void slotAddBreakpoint();
    void slotAddBlankBreakpoint(int idx);
    void slotRowDoubleClicked(int row, int col, int button, const QPoint & mousePos);
    void slotContextMenuShow( int row, int col, const QPoint &mousePos );
    void slotContextMenuSelect( int item );
    void slotEditRow(int row, int col, const QPoint & mousePos);
    void slotNewValue(int row, int col);

signals:
    void publishBPState(const Breakpoint& brkpt);
    void refreshBPState(const Breakpoint& brkpt);
    void gotoSourcePosition(const QString &fileName, int lineNum);
    void clearAllBreakpoints();

private:
    BreakpointTableRow* find(Breakpoint *bp);
    BreakpointTableRow* findId(int id);
    BreakpointTableRow* findKey(int BPKey);

    void setActive();
    BreakpointTableRow* addBreakpoint(Breakpoint *bp);
    void removeBreakpoint(BreakpointTableRow* btr);

private:
    RDBTable*       m_table;

    QToolButton*    m_add;
    QToolButton*    m_delete;
    QToolButton*    m_edit;
    QToolButton*    m_removeAll;
    Q3PopupMenu*     m_ctxMenu;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
