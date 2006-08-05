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

#include "gdbbreakpointwidget.h"
#include "gdbtable.h"
#include "debuggertracingdialog.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"

#include "breakpoint.h"
#include "domutil.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kurl.h>
#include <kmessagebox.h>

#include <qvbuttongroup.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qtable.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <stdlib.h>
#include <ctype.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

enum Column {
    Control     = 0,
    Enable      = 1,
    Type        = 2,
    Status      = 3,
    Location    = 4,
    Condition   = 5,
    IgnoreCount = 6,
    Hits        = 7,
    Tracing     = 8
};


#define numCols 9

enum BW_ITEMS { BW_ITEM_Show, BW_ITEM_Edit, BW_ITEM_Disable, BW_ITEM_Delete,
                BW_ITEM_DisableAll, BW_ITEM_EnableAll, BW_ITEM_DeleteAll};

static int m_activeFlag = 0;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class BreakpointTableRow : public QTableItem
{
public:

    BreakpointTableRow(QTable* table, EditType editType, Breakpoint* bp);
    ~BreakpointTableRow();

    bool match (Breakpoint* bp) const;
    void reset ();
    void setRow();

    Breakpoint* breakpoint()        { return m_breakpoint; }

private:
    void appendEmptyRow();

private:
    Breakpoint* m_breakpoint;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

BreakpointTableRow::BreakpointTableRow(QTable* parent, EditType editType,
                                       Breakpoint* bp) :
        QTableItem(parent, editType, ""),
        m_breakpoint(bp)
{
    appendEmptyRow();
    setRow();
}

/***************************************************************************/

BreakpointTableRow::~BreakpointTableRow()
{
    m_breakpoint->deleteLater();
}

/***************************************************************************/

bool BreakpointTableRow::match(Breakpoint* breakpoint) const
{
    return m_breakpoint->match(breakpoint);
}

/***************************************************************************/

void BreakpointTableRow::reset()
{
    m_breakpoint->reset();
    setRow();
}

/***************************************************************************/

void BreakpointTableRow::appendEmptyRow()
{
    int row = table()->numRows();
    table()->setNumRows(row+1);

    table()->setItem(row, Control, this);

    QCheckTableItem* cti = new QCheckTableItem( table(), "");
    table()->setItem(row, Enable, cti);

    ComplexEditCell* act = new ComplexEditCell(table());
    table()->setItem(row, Tracing, act);
    QObject::connect(act, SIGNAL(edit(QTableItem*)), 
                     table()->parent(), SLOT(editTracing(QTableItem*)));
}

/***************************************************************************/

void BreakpointTableRow::setRow()
{
    if ( m_breakpoint )
    {
        QTableItem *item =  table()->item ( row(), Enable );
        Q_ASSERT(item->rtti() == 2);
        ((QCheckTableItem*)item)->setChecked(m_breakpoint->isEnabled());

        QString status=m_breakpoint->statusDisplay(m_activeFlag);

        table()->setText(row(), Status, status);
        table()->setText(row(), Condition, m_breakpoint->conditional());
        table()->setText(row(), IgnoreCount, QString::number(m_breakpoint->ignoreCount() ));
        table()->setText(row(), Hits, QString::number(m_breakpoint->hits() ));

        QString displayType = m_breakpoint->displayType();
        table()->setText(row(), Location, m_breakpoint->location());

               
        QTableItem* ce = table()->item( row(), Tracing );
        ce->setText(breakpoint()->tracingEnabled() ? "Enabled" : "Disabled");
        // In case there's editor open in this cell, update it too.
        static_cast<ComplexEditCell*>(ce)->updateValue();


        if (m_breakpoint->isTemporary())
            displayType = i18n(" temporary");
        if (m_breakpoint->isHardwareBP())
            displayType += i18n(" hw");

        table()->setText(row(), Type, displayType);
        table()->adjustColumn(Type);
        table()->adjustColumn(Status);
        table()->adjustColumn(Location);
        table()->adjustColumn(Hits);
        table()->adjustColumn(IgnoreCount);
        table()->adjustColumn(Condition);
    }
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

GDBBreakpointWidget::GDBBreakpointWidget(GDBController* controller,
                                         QWidget *parent, const char *name) :
QHBox(parent, name),
controller_(controller)
{
    m_table = new GDBTable(0, numCols, this, name);
    m_table->setSelectionMode(QTable::SingleRow);
    m_table->setShowGrid (false);
    m_table->setLeftMargin(0);
    m_table->setFocusStyle(QTable::FollowStyle);

    m_table->hideColumn(Control);
    m_table->setColumnReadOnly(Type, true);
    m_table->setColumnReadOnly(Status, true);
    m_table->setColumnReadOnly(Hits, true);
    m_table->setColumnWidth( Enable, 20);

    QHeader *header = m_table->horizontalHeader();

    header->setLabel( Enable,       "" );
    header->setLabel( Type,         i18n("Type") );
    header->setLabel( Status,       i18n("Status") );
    header->setLabel( Location,     i18n("Location") );
    header->setLabel( Condition,    i18n("Condition") );
    header->setLabel( IgnoreCount,  i18n("Ignore Count") );
    header->setLabel( Hits,         i18n("Hits") );
    header->setLabel( Tracing,      i18n("Tracing") );

    QPopupMenu* newBreakpoint = new QPopupMenu(this);
    newBreakpoint->insertItem(i18n("Code breakpoint", "Code"), 
                              BP_TYPE_FilePos);
    newBreakpoint->insertItem(i18n("Data breakpoint", "Data write"), 
                              BP_TYPE_Watchpoint);
    newBreakpoint->insertItem(i18n("Data read breakpoint", "Data read"),
                              BP_TYPE_ReadWatchpoint);


    m_ctxMenu = new QPopupMenu( this );
    m_ctxMenu->insertItem( i18n("New breakpoint", "New"),
                                newBreakpoint);
    m_ctxMenu->insertItem( i18n( "Show text" ),    BW_ITEM_Show );
    int edit_id = 
        m_ctxMenu->insertItem( i18n( "Edit" ),    BW_ITEM_Edit );
    m_ctxMenu->setAccel(Qt::Key_Enter, edit_id);
    m_ctxMenu->insertItem( i18n( "Disable" ), BW_ITEM_Disable );
    int del_id = 
        m_ctxMenu->insertItem( SmallIcon("breakpoint_delete"),
                               i18n( "Delete" ),  BW_ITEM_Delete );
    m_ctxMenu->setAccel(Qt::Key_Delete, del_id);
    m_ctxMenu->insertSeparator();
    m_ctxMenu->insertItem( i18n( "Disable all"), BW_ITEM_DisableAll );
    m_ctxMenu->insertItem( i18n( "Enable all"), BW_ITEM_EnableAll );
    m_ctxMenu->insertItem( i18n( "Delete all"), BW_ITEM_DeleteAll );

    m_table->show();

    connect( newBreakpoint,       SIGNAL(activated(int)),
             this,          SLOT(slotAddBlankBreakpoint(int)) );

    connect( m_table,       SIGNAL(contextMenuRequested(int, int, const QPoint &)),
             this,          SLOT(slotContextMenuShow(int, int, const QPoint & )) );
    connect( m_ctxMenu,     SIGNAL(activated(int)),
             this,          SLOT(slotContextMenuSelect(int)) );

    connect( m_table,       SIGNAL(doubleClicked(int, int, int, const QPoint &)),
             this,          SLOT(slotRowDoubleClicked(int, int, int, const QPoint &)));

    connect( m_table,       SIGNAL(valueChanged(int, int)),
             this,          SLOT(slotNewValue(int, int)));

    connect( m_table,       SIGNAL(returnPressed()),
             this,          SLOT(slotEditBreakpoint()));
//    connect( m_table,       SIGNAL(f2Pressed()),
//             this,          SLOT(slotEditBreakpoint()));
    connect( m_table,       SIGNAL(deletePressed()),
             this,          SLOT(slotRemoveBreakpoint()));
    connect( m_table,       SIGNAL(insertPressed()),
             this,          SLOT(slotAddBlankBreakpoint()));

    // FIXME: maybe, all debugger components should derive from
    // a base class that does this connect.
    connect(controller, SIGNAL(event(GDBController::event_t)),
            this,       SLOT(slotEvent(GDBController::event_t)));

    connect(controller, 
            SIGNAL(watchpointHit(int, const QString&, const QString&)),
            this,
            SLOT(slotWatchpointHit(int, const QString&, const QString&)));
}

/***************************************************************************/

GDBBreakpointWidget::~GDBBreakpointWidget()
{
    delete m_table;
}

/***************************************************************************/

void GDBBreakpointWidget::reset()
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            btr->reset();
            sendToGdb(*(btr->breakpoint()));
        }
    }
}

/***************************************************************************/

// When a file is loaded then we need to tell the editor (display window)
// which lines contain a breakpoint.
void GDBBreakpointWidget::slotRefreshBP(const KURL &filename)
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            FilePosBreakpoint* bp = dynamic_cast<FilePosBreakpoint*>(btr->breakpoint());
            if (bp && bp->hasFileAndLine() 
                && (bp->fileName() == filename.path()))
                emit refreshBPState(*bp);
        }
    }
}

void GDBBreakpointWidget::slotBreakpointHit(int id)
{
    BreakpointTableRow* br = findId(id);

    // FIXME: should produce an message, this is most likely 
    // an error.
    if (!br)
        return;

    Breakpoint* b = br->breakpoint();

    if (b->tracingEnabled())
    {
        controller_->addCommand(
            new CliCommand(("printf " 
                            + b->traceRealFormatString()).latin1(),
                           this,
                           &GDBBreakpointWidget::handleTracingPrintf));
                           
        controller_->addCommand(new 
                            GDBCommand("-exec-continue", NOTINFOCMD, 
                                       0));
        
    }
}

void GDBBreakpointWidget::slotWatchpointHit(int id,
                                            const QString& oldValue,
                                            const QString& newValue)
{
    BreakpointTableRow* br = findId(id);

    // FIXME: should produce an message, this is most likely 
    // an error.
    if (!br)
        return;

    Watchpoint* b = dynamic_cast<Watchpoint*>(br->breakpoint());


    KMessageBox::information(
        0, 
        i18n("<b>Data write breakpoint</b><br>"
             "Expression: %1<br>"
             "Address: 0x%2<br>"
             "Old value: %3<br>"
             "New value: %4")
        .arg(b->varName())
        .arg(b->address(), 0, 16)
        .arg(oldValue)
        .arg(newValue));    
}

/***************************************************************************/

BreakpointTableRow* GDBBreakpointWidget::find(Breakpoint *breakpoint)
{
    // NOTE:- The match doesn't have to be equal. Each type of bp
    // must decide on the match criteria.
    Q_ASSERT (breakpoint);

    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr && btr->match(breakpoint))
            return btr;
    }

    return 0;
}

/***************************************************************************/

// The Id is supplied by the debugger
BreakpointTableRow* GDBBreakpointWidget::findId(int dbgId)
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr && btr->breakpoint()->dbgId() == dbgId)
            return btr;
    }

    return 0;
}

/***************************************************************************/

// The key is a unique number supplied by us
BreakpointTableRow* GDBBreakpointWidget::findKey(int BPKey)
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr && btr->breakpoint()->key() == BPKey)
            return btr;
    }

    return 0;
}

bool GDBBreakpointWidget::hasWatchpointForAddress(
    unsigned long long address) const
{
    for(int i = 0; i < m_table->numRows(); ++i)
    {
        BreakpointTableRow* br = (BreakpointTableRow*)
            m_table->item(i, Control);

        Watchpoint* w = dynamic_cast<Watchpoint*>(br->breakpoint());
        if (w && w->address() == address)
            return true;
    }
    return false;
}

/***************************************************************************/

BreakpointTableRow* GDBBreakpointWidget::addBreakpoint(Breakpoint *bp)
{
    BreakpointTableRow* btr =
        new BreakpointTableRow( m_table, QTableItem::WhenCurrent, bp );

    connect(bp, SIGNAL(modified(Breakpoint*)),
            this, SLOT(slotBreakpointModified(Breakpoint*)));
    
    sendToGdb(*bp);
                                              
    return btr;
}

/***************************************************************************/

void GDBBreakpointWidget::removeBreakpoint(BreakpointTableRow* btr)
{
    if (!btr)
        return;

    // Pending but the debugger hasn't started processing this bp so
    // we can just remove it.   
    Breakpoint* bp = btr->breakpoint();
    // No gdb breakpoint, and no breakpoint addition command in the
    // queue. Just remove.
    if (bp->dbgId() == -1 && !bp->isDbgProcessing())
    {
        bp->setActionDie();
        sendToGdb(*bp);
        m_table->removeRow(btr->row());
    }
    else
    {
        bp->setActionClear(true);
        sendToGdb(*bp);
        btr->setRow();
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotToggleBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    BreakpointTableRow* btr = find(fpBP);
    if (btr)
    {
        removeBreakpoint(btr);
    }
    else
        addBreakpoint(fpBP);
}

/***************************************************************************/

void GDBBreakpointWidget::slotToggleBreakpointEnabled(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    BreakpointTableRow* btr = find(fpBP);
    delete fpBP;
    if (btr)
    {
        Breakpoint* bp=btr->breakpoint();
        bp->setEnabled(!bp->isEnabled());
        sendToGdb(*bp);
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotToggleWatchpoint(const QString &varName)
{
    Watchpoint *watchpoint = new Watchpoint(varName, false, true);
    BreakpointTableRow* btr = find(watchpoint);
    if (btr)
    {
        removeBreakpoint(btr);
        delete watchpoint;
    }
    else
        addBreakpoint(watchpoint);
}

void GDBBreakpointWidget::handleBreakpointList(const GDBMI::ResultRecord& r)
{
    m_activeFlag++;

    const GDBMI::Value& blist = r["BreakpointTable"]["body"];

    for(unsigned i = 0, e = blist.size(); i != e; ++i)
    {
        const GDBMI::Value& b = blist[i];

        int id = b["number"].literal().toInt();
        BreakpointTableRow* btr = findId(id);
        if (btr)
        {
            Breakpoint *bp = btr->breakpoint();
            bp->setActive(m_activeFlag, id);
            bp->setHits(b["times"].toInt());
            if (b.hasField("ignore"))                
                bp->setIgnoreCount(b["ignore"].toInt());
            else
                bp->setIgnoreCount(0);
            if (b.hasField("cond"))
                bp->setConditional(b["cond"].literal());
            else
                bp->setConditional(QString::null);
            btr->setRow();
            emit publishBPState(*bp);
        }
        else
        {
            // It's a breakpoint added outside, most probably
            // via gdb console. Add it now.
            QString type = b["type"].literal();

            if (type == "breakpoint" || type == "hw breakpoint")
            {
                if (b.hasField("fullname") && b.hasField("line"))
                {
                    Breakpoint* bp = new FilePosBreakpoint(
                        b["fullname"].literal(), 
                        b["line"].literal().toInt());

                    bp->setActive(m_activeFlag, id);
                    bp->setActionAdd(false);
                    bp->setPending(false);

                    new BreakpointTableRow(m_table, 
                                           QTableItem::WhenCurrent, 
                                           bp);

                    emit publishBPState(*bp);
                }
            }
            
        }
    }

    // Remove any inactive breakpoints.
    for ( int row = m_table->numRows()-1; row >= 0 ; row-- )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            Breakpoint* bp = btr->breakpoint();
            if (!(bp->isActive(m_activeFlag)))
            {
                // FIXME: need to review is this happens for 
                // as-yet unset breakpoint.
                bp->removedInGdb();
            }
        }
    }
}

void GDBBreakpointWidget::handleTracingPrintf(const QValueVector<QString>& s)
{
    // The first line of output is the command itself, which we don't need.
    for(unsigned i = 1; i < s.size(); ++i)
        emit tracingOutput(s[i].local8Bit());
}

/***************************************************************************/

void GDBBreakpointWidget::slotBreakpointSet(Breakpoint* bp)
{
    // FIXME: why 'key' is used here?
    BreakpointTableRow* btr = findKey(bp->key());
    if (!btr)
    {
        kdDebug(9012) << "Early return\n";
        return;
    }

    btr->setRow();
}

/***************************************************************************/

void GDBBreakpointWidget::slotAddBlankBreakpoint(int idx)
{
    BreakpointTableRow* btr = 0;
    switch (idx)
    {
      case BP_TYPE_FilePos:
          btr = addBreakpoint(new FilePosBreakpoint());
          break;

      case BP_TYPE_Watchpoint:
          btr = addBreakpoint(new Watchpoint(""));
          break;

      case BP_TYPE_ReadWatchpoint:
          btr = addBreakpoint(new ReadWatchpoint(""));
          break;

      default:
          break;
    }

    if (btr)
    {
        m_table->selectRow(btr->row());
        m_table->editCell(btr->row(), Location, false);
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotRemoveBreakpoint()
{
    int row = m_table->currentRow();
    if ( row != -1)
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        removeBreakpoint(btr);
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotRemoveAllBreakpoints()
{
    for ( int row = m_table->numRows()-1; row>=0; row-- )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        removeBreakpoint(btr);
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotRowDoubleClicked(int row, int col, int btn, const QPoint &)
{
    if ( btn == Qt::LeftButton )
    {
//    kdDebug(9012) << "in slotRowSelected row=" << row << endl;
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            FilePosBreakpoint* bp = dynamic_cast<FilePosBreakpoint*>(btr->breakpoint());
            if (bp && bp->hasFileAndLine())
                emit gotoSourcePosition(bp->fileName(), bp->lineNum()-1);

            // put the focus back on the clicked item if appropriate
            if (col == Location || col ==  Condition || col == IgnoreCount)
                m_table->editCell(row, col, false);
        }
    }
}

void GDBBreakpointWidget::slotContextMenuShow( int row, int /*col*/, const QPoint &mousePos )
{
    BreakpointTableRow *btr = (BreakpointTableRow *)m_table->item(row, Control );
    
    if (btr == NULL)
    {
        btr = (BreakpointTableRow *)m_table->item(m_table->currentRow(), 
                                                  Control );
    }

    if (btr != NULL)
    {
        m_ctxMenu->setItemEnabled( 
            BW_ITEM_Show, 
            btr->breakpoint()->hasFileAndLine());

        if (btr->breakpoint( )->isEnabled( ))
        {
            m_ctxMenu->changeItem( BW_ITEM_Disable, i18n("Disable") );
        }
        else
        {
            m_ctxMenu->changeItem( BW_ITEM_Disable, i18n("Enable") );
        }

        m_ctxMenu->setItemEnabled(BW_ITEM_Disable, true);
        m_ctxMenu->setItemEnabled(BW_ITEM_Delete, true);
        m_ctxMenu->setItemEnabled(BW_ITEM_Edit, true);
    }
    else
    {
        m_ctxMenu->setItemEnabled(BW_ITEM_Show, false);
        m_ctxMenu->setItemEnabled(BW_ITEM_Disable, false);
        m_ctxMenu->setItemEnabled(BW_ITEM_Delete, false);
        m_ctxMenu->setItemEnabled(BW_ITEM_Edit, false);
    }

    bool has_bps = (m_table->numRows() != 0);
    m_ctxMenu->setItemEnabled(BW_ITEM_DisableAll, has_bps);
    m_ctxMenu->setItemEnabled(BW_ITEM_EnableAll, has_bps);
    m_ctxMenu->setItemEnabled(BW_ITEM_Delete, has_bps);

    m_ctxMenu->popup( mousePos );
}

void GDBBreakpointWidget::slotContextMenuSelect( int item )
{
    int                  row, col;
    BreakpointTableRow  *btr;
    Breakpoint          *bp;
    FilePosBreakpoint   *fbp;
    
    row= m_table->currentRow( );
    if (row == -1)
        return;
    btr = (BreakpointTableRow *)m_table->item( row, Control );
    if (btr == NULL)
        return;
    bp = btr->breakpoint( );
    if (bp == NULL)
        return;
    fbp = dynamic_cast<FilePosBreakpoint*>(bp);

    switch( item )
    {
        case BW_ITEM_Show:
            if (fbp)
                emit gotoSourcePosition(fbp->fileName(), fbp->lineNum()-1);
            break;
        case BW_ITEM_Edit:
            col = m_table->currentColumn( );
            if (col == Location || col ==  Condition || col == IgnoreCount)
                m_table->editCell(row, col, false);
            break;
        case BW_ITEM_Disable:
            
            bp->setEnabled( !bp->isEnabled( ) );
            btr->setRow( );
            sendToGdb( *bp );
            break;
        case BW_ITEM_Delete:
            slotRemoveBreakpoint( );
            break;
        case BW_ITEM_DeleteAll:
            slotRemoveAllBreakpoints();
            break;
        case BW_ITEM_DisableAll:
        case BW_ITEM_EnableAll:
            for ( int row = 0; row < m_table->numRows(); row++ )
            {
                BreakpointTableRow* btr = (BreakpointTableRow *) 
                    m_table->item(row, Control);

                if (btr)
                {
                    btr->breakpoint()->setEnabled(item == BW_ITEM_EnableAll);
                    btr->setRow();
                    sendToGdb(*btr->breakpoint());
                }
            }
            break;
        default:
            // oops, check it out! this case is not in sync with the
            // m_ctxMenu.  Check the enum in the header file.
            return;
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotEditRow(int row, int col, const QPoint &)
{
//    kdDebug(9012) << "in slotEditRow row=" << row << endl;
    BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
    if (btr)
    {
        if (col == Location || col ==  Condition || col == IgnoreCount)
            m_table->editCell(row, col, false);
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotNewValue(int row, int col)
{
    BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);

    QString new_value = m_table->text(row, col);

    if (btr)
    {
        Breakpoint* bp = btr->breakpoint();
        switch (col)
        {
        case Enable:
        {
            QCheckTableItem *item = 
                (QCheckTableItem*)m_table->item ( row, Enable );
            bp->setEnabled(item->isChecked());
        }
        break;

        case Location:
        {
            if (bp->location() != new_value)
            {
                // GDB does not allow to change location of
                // an existing breakpoint. So, need to remove old
                // breakpoint and add another.
                
                // Announce to editor that breakpoit at its
                // current location is dying.
                bp->setActionDie();
                emit publishBPState(*bp);

                // However, we don't want the line in breakpoint
                // widget to disappear and appear again.
                    
                // Emit delete command. This won't resync breakpoint
                // table (unlike clearBreakpoint), so we won't have
                // nasty effect where line in the table first disappears
                // and then appears again, and won't have internal issues
                // as well.
                if (!controller_->stateIsOn(s_dbgNotStarted))
                    controller_->addCommand(
                        new GDBCommand(bp->dbgRemoveCommand().latin1(), 
                                       NOTRUNCMD, NOTINFOCMD));

                // Now add new breakpoint in gdb. It will correspond to
                // the same 'Breakpoint' and 'BreakpointRow' objects in
                // KDevelop is the previous, deleted, breakpoint.
                
                // Note: clears 'actionDie' implicitly.
                bp->setActionAdd(true);
                bp->setLocation(new_value);
            }
            break;
        }

        case Condition:
        {
            bp->setConditional(new_value);
            break;
        }

        case IgnoreCount:
        {
            bp->setIgnoreCount(new_value.toInt());
            break;
        }
        default:
            break;
        }

        bp->setActionModify(true);


        // This is not needed for most changes, since we've
        // just read a value from table cell to breakpoint, and 
        // setRow will write back the same value to the cell.
        // It's only really needed for tracing column changes,
        // where tracing config dialog directly changes breakpoint,
        // so we need to send those changes to the table.
        btr->setRow();


        sendToGdb(*bp);
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotEditBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    BreakpointTableRow* btr = find(fpBP);
    delete fpBP;

    if (btr)
    {
        QTableSelection ts;
        ts.init(btr->row(), 0);
        ts.expandTo(btr->row(), numCols);
        m_table->addSelection(ts);
        m_table->editCell(btr->row(), Location, false);
    }

}

void GDBBreakpointWidget::sendToGdb(Breakpoint& BP)
{
    // Announce the change in state. We need to do this before
    // everything. For example, if debugger is not yet running, we'll
    // immediate exit after setting pending flag, but we still want changes
    // in "enabled" flag to be shown on the left border of the editor.
    emit publishBPState(BP);

    BP.sendToGdb(controller_);
}

void GDBBreakpointWidget::slotBreakpointModified(Breakpoint* b)
{
    emit publishBPState(*b);

    if (BreakpointTableRow* btr = find(b))
    {
        if (b->isActionDie())
        {
            // Breakpoint was deleted, kill the table row.
            m_table->removeRow(btr->row());
        }
        else
        {
            btr->setRow();
        }        
    }
}

void GDBBreakpointWidget::slotEvent(GDBController::event_t e)
{
    switch(e)
    {
    case GDBController::program_state_changed:
        {
            controller_->addCommand(
                new GDBCommand("-break-list",
                               this,
                               &GDBBreakpointWidget::handleBreakpointList));
            break;
        }

    case GDBController::shared_library_loaded:
    case GDBController::connected_to_program:
        {
            for ( int row = 0; row < m_table->numRows(); row++ )
            {
                BreakpointTableRow* btr = (BreakpointTableRow *) 
                    m_table->item(row, Control);
                
                if (btr)
                {
                    Breakpoint* bp = btr->breakpoint();
                    if ( (bp->dbgId() == -1 ||  bp->isPending())
                         && !bp->isDbgProcessing() 
                         && bp->isValid())
                    {
                        sendToGdb(*bp);
                    }
                }
            }
            break;
        }
    case GDBController::program_exited:
        {
            for(int row = 0; row < m_table->numRows(); ++row)
            {
                Breakpoint* b = static_cast<BreakpointTableRow*>(
                    m_table->item(row, Control))->breakpoint();

                b->applicationExited(controller_);
            }
        }

    default:        
        ;
    }
}


/***************************************************************************/

void GDBBreakpointWidget::slotEditBreakpoint()
{
    m_table->editCell(m_table->currentRow(), Location, false);
}


void GDBBreakpointWidget::editTracing(QTableItem* item)
{
    BreakpointTableRow* btr = (BreakpointTableRow *)
        m_table->item(item->row(), Control);

    DebuggerTracingDialog* d = new DebuggerTracingDialog(
        btr->breakpoint(), m_table, "");

    int r = d->exec();

    // Note: change cell text here and explicitly call slotNewValue here.
    // We want this signal to be emitted when we close the tracing dialog
    // and not when we select some other cell, as happens in Qt by default.
    if (r == QDialog::Accepted)
    {
        // The dialog has modified "btr->breakpoint()" already.
        // Calling 'slotNewValue' will flush the changes back
        // to the table.
        slotNewValue(item->row(), item->col());
    }

    delete d;
}


/***************************************************************************/

void GDBBreakpointWidget::savePartialProjectSession(QDomElement* el)
{
    QDomDocument domDoc = el->ownerDocument();
    if (domDoc.isNull())
        return;

    QDomElement breakpointListEl = domDoc.createElement("breakpointList");
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr =
            (BreakpointTableRow *) m_table->item(row, Control);
        Breakpoint* bp = btr->breakpoint();

        QDomElement breakpointEl =
            domDoc.createElement("breakpoint"+QString::number(row));

        breakpointEl.setAttribute("type", bp->type());
        breakpointEl.setAttribute("location", bp->location(false));
        breakpointEl.setAttribute("enabled", bp->isEnabled());
        breakpointEl.setAttribute("condition", bp->conditional());
        breakpointEl.setAttribute("tracingEnabled", 
                                  QString::number(bp->tracingEnabled()));
        breakpointEl.setAttribute("traceFormatStringEnabled", 
                                  QString::number(bp->traceFormatStringEnabled()));        
        breakpointEl.setAttribute("tracingFormatString",
                                  bp->traceFormatString());

        QDomElement tracedExpressions = 
            domDoc.createElement("tracedExpressions");

        QStringList::const_iterator i, e;
        for(i = bp->tracedExpressions().begin(),
                e = bp->tracedExpressions().end();
            i != e; ++i)
        {
            QDomElement expr = domDoc.createElement("expression");
            expr.setAttribute("value", *i);
            tracedExpressions.appendChild(expr);
        }
        
        breakpointEl.appendChild(tracedExpressions);

        breakpointListEl.appendChild(breakpointEl);
    }

    if (!breakpointListEl.isNull())
        el->appendChild(breakpointListEl);
}

/***************************************************************************/

void GDBBreakpointWidget::restorePartialProjectSession(const QDomElement* el)
{
    /** Eventually, would be best to make each breakpoint type handle loading/
        saving it's data. The only problem is that on load, we need to allocate
        with new different objects, depending on type, and that requires some 
        kind of global registry. Gotta find out if a solution for that exists in 
        KDE (Boost.Serialization is too much dependency, and rolling my own is
        boring).
    */
    QDomElement breakpointListEl = el->namedItem("breakpointList").toElement();
    if (!breakpointListEl.isNull())
    {
        QDomElement breakpointEl;
        for (breakpointEl = breakpointListEl.firstChild().toElement();
                !breakpointEl.isNull();
                breakpointEl = breakpointEl.nextSibling().toElement())
        {
            Breakpoint* bp=0;
            BP_TYPES type = (BP_TYPES) breakpointEl.attribute( "type", "0").toInt();
            switch (type)
            {
            case BP_TYPE_FilePos:
            {
                bp = new FilePosBreakpoint();
                break;
            }
            case BP_TYPE_Watchpoint:
            {
                bp = new Watchpoint("");
                break;
            }
            default:
                break;
            }

            // Common settings for any type of breakpoint
            if (bp)
            {
                bp->setLocation(breakpointEl.attribute( "location", ""));
                if (type == BP_TYPE_Watchpoint)
                {
                    bp->setEnabled(false);
                }
                else
                {
                    bp->setEnabled(
                        breakpointEl.attribute( "enabled", "1").toInt());
                }
                bp->setConditional(breakpointEl.attribute( "condition", ""));

                bp->setTracingEnabled(
                    breakpointEl.attribute("tracingEnabled", "0").toInt());
                bp->setTraceFormatString(
                    breakpointEl.attribute("tracingFormatString", ""));
                bp->setTraceFormatStringEnabled(
                    breakpointEl.attribute("traceFormatStringEnabled", "0")
                    .toInt());

                QDomNode tracedExpr = 
                    breakpointEl.namedItem("tracedExpressions");

                if (!tracedExpr.isNull())
                {
                    QStringList l;

                    for(QDomNode c = tracedExpr.firstChild(); !c.isNull(); 
                        c = c.nextSibling())
                    {
                        QDomElement el = c.toElement();
                        l.push_back(el.attribute("value", ""));
                    }
                    bp->setTracedExpressions(l);
                }

                // Now add the breakpoint. Don't try to check if 
                // breakpoint already exists.
                // It's easy to check that breakpoint on the same
                // line already exists, but it might have different condition,
                // and checking conditions for equality is too complex thing.
                // And anyway, it's will be suprising of realoading a project
                // changes the set of breakpoints.
                addBreakpoint(bp);
            }
        }
    }
}

/***************************************************************************/

void GDBBreakpointWidget::focusInEvent( QFocusEvent */* e*/ )
{
    // Without the following 'if', when we first open the breakpoints
    // widget, the background is all black. This happens only with
    //    m_table->setFocusStyle(QTable::FollowStyle);
    // in constructor, so I suspect Qt bug. But anyway, without
    // current cell keyboard actions like Enter for edit won't work,
    // so keyboard focus does not makes much sense.
    if (m_table->currentRow() == -1 ||
        m_table->currentColumn() == -1)
    {
        m_table->setCurrentCell(0, 0);
    }
    m_table->setFocus();
}

ComplexEditCell::
ComplexEditCell(QTable* table)
: QTableItem(table, QTableItem::WhenCurrent)
{
}


QWidget* ComplexEditCell::createEditor() const
{
    QHBox* box = new QHBox( table()->viewport() );
    box->setPaletteBackgroundColor(
               table()->palette().active().highlight());

    label_ = new QLabel(text(), box, "label");
    label_->setBackgroundMode(Qt::PaletteHighlight);
    // Sorry for hardcode, but '2' is already hardcoded in
    // Qt source, in QTableItem::paint. Since I don't want the 
    // text to jump 2 pixels to the right when editor is activated, 
    // need to set the same indent for label.
    label_->setIndent(2);
    QPalette p = label_->palette();

    p.setColor(QPalette::Active, QColorGroup::Foreground, 
               table()->palette().active().highlightedText());
    p.setColor(QPalette::Inactive, QColorGroup::Foreground, 
               table()->palette().active().highlightedText());

    label_->setPalette(p);
    
    QPushButton* b = new QPushButton("...", box);
    // This is exactly what is done in QDesigner source in the
    // similar context. Haven't had any success making the good look
    // with layout, I suppose that sizeHint for button is always larger
    // than 20.
    b->setFixedWidth( 20 );

    connect(b, SIGNAL(clicked()), this, SLOT(slotEdit()));

    return box;
}

void ComplexEditCell::updateValue()
{
    if (!label_.isNull())
    {
        label_->setText(table()->text(row(), col()));
    }
}

void ComplexEditCell::slotEdit()
{
    emit edit(this);
}

}


#include "gdbbreakpointwidget.moc"
