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

#ifndef _GDBBreakpointWidget_H_
#define _GDBBreakpointWidget_H_

#include <qhbox.h>

class QDomElement;
class QToolButton;
class KURL;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{
class Breakpoint;
class BreakpointTableRow;
class GDBTable;

class GDBBreakpointWidget : public QHBox
{
    Q_OBJECT

public:
    GDBBreakpointWidget( QWidget* parent=0, const char* name=0 );
    virtual ~GDBBreakpointWidget();

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
    void slotParseGDBBrkptList(char *str);
    void slotParseGDBBreakpointSet(char *str, int BPKey);

    void slotRefreshBP(const KURL &filename);
    
protected:
    virtual void focusInEvent(QFocusEvent *e);

private slots:
    void slotRemoveBreakpoint();
    void slotRemoveAllBreakpoints();
    void slotEditBreakpoint(const QString &fileName, int lineNum);
    void slotEditBreakpoint();
    void slotAddBreakpoint();
    void slotAddBlankBreakpoint(int idx);
    void slotRowSelected(int row, int col, int button, const QPoint & mousePos);
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
    GDBTable*       m_table;

    QToolButton*    m_add;
    QToolButton*    m_delete;
    QToolButton*    m_edit;
    QToolButton*    m_removeAll;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
