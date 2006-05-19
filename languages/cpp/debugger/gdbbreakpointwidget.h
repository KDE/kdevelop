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
#include <qpopupmenu.h>
#include <qtable.h>
#include <qguardedptr.h>
#include <qvaluevector.h>

#include "mi/gdbmi.h"
#include "gdbcontroller.h"

class QDomElement;
class QToolButton;
class QLabel;
class KURL;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{
class Breakpoint;
class BreakpointTableRow;
class GDBTable;
class GDBController;

class GDBBreakpointWidget : public QHBox
{
    Q_OBJECT

public:
    GDBBreakpointWidget( GDBController* controller,
                         QWidget* parent=0, const char* name=0 );
    virtual ~GDBBreakpointWidget();

    void reset();

    void savePartialProjectSession(QDomElement* el);
    void restorePartialProjectSession(const QDomElement* el);

    bool hasWatchpointForAddress(unsigned long long address) const;


public slots:
    // Connected to from the editor widget:
    void slotToggleBreakpoint(const QString &filename, int lineNum);
    void slotToggleBreakpointEnabled(const QString &fileName, int lineNum);

    // Connected to from the variable widget:
    void slotToggleWatchpoint(const QString &varName);

    void slotBreakpointSet(Breakpoint*);

    void slotRefreshBP(const KURL &filename);

    void slotBreakpointHit(int id);

    
protected:
    virtual void focusInEvent(QFocusEvent *e);


    friend class BreakpointActionCell; // for access to slotNewValue
private slots:
    void slotRemoveBreakpoint();
    void slotRemoveAllBreakpoints();
    void slotEditBreakpoint(const QString &fileName, int lineNum);
    void slotEditBreakpoint();
    void slotAddBlankBreakpoint(int idx);
    void slotRowDoubleClicked(int row, int col, int button, const QPoint & mousePos);
    void slotContextMenuShow( int row, int col, const QPoint &mousePos );
    void slotContextMenuSelect( int item );
    void slotEditRow(int row, int col, const QPoint & mousePos);
    void slotNewValue(int row, int col);
    void editTracing(QTableItem* item);
    void slotBreakpointModified(Breakpoint*);

    void slotEvent(GDBController::event_t);
    void slotWatchpointHit(int id,
                           const QString& oldValue,
                           const QString& newValue);

signals:
    void publishBPState(const Breakpoint& brkpt);
    void refreshBPState(const Breakpoint& brkpt);
    void gotoSourcePosition(const QString &fileName, int lineNum);
    // Emitted when output from yet another passed tracepoint is available.
    void tracingOutput(const char*);

private:
    BreakpointTableRow* find(Breakpoint *bp);
    BreakpointTableRow* findId(int id);
    BreakpointTableRow* findKey(int BPKey);

    void setActive();
    BreakpointTableRow* addBreakpoint(Breakpoint *bp);
    void removeBreakpoint(BreakpointTableRow* btr);

    void sendToGdb(Breakpoint &);

    void handleBreakpointList(const GDBMI::ResultRecord&);
    void handleTracingPrintf(const QValueVector<QString>& s);

private:
    GDBController*  controller_;

    GDBTable*       m_table;
    QPopupMenu*     m_ctxMenu;
};

class BreakpointTableRow;

/** Custom table cell class for cells that require complex editing.
    When current, the cell shows a "..." on the right. When clicked,
    the 'edit' signal is emitted that can be be used to pop-up
    a dialog box. 

    When editing is done, the receiver of 'edit' should change the
    value in the table, and then call the 'updateValue' method.    
 */
class ComplexEditCell : public QObject, public QTableItem
{
    Q_OBJECT
public:

    ComplexEditCell(QTable* table);

    /** Called by Qt when the current cell should become editable.
        In our case, when the item becomes current. Creates a widget
        that will be shown in the cell and should be able to edit cell
        content. In our case -- text plus "..." button that invokes 
        action dialog.        
    */
    QWidget* createEditor() const;    

    void updateValue();

private slots:

    /** Called when the "..." button is clicked. */
    void slotEdit();

signals:
    void edit(QTableItem*);

private:
    mutable QGuardedPtr<QLabel> label_;
};


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
