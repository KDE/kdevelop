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

#include "rdbbreakpointwidget.h"
#include "rdbtable.h"

#include "breakpoint.h"
#include "domutil.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kurl.h>

#include <qvbuttongroup.h>
#include <qfileinfo.h>
#include <q3header.h>
#include <q3table.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <q3vbox.h>
#include <qlayout.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QFocusEvent>
#include <Q3Frame>
#include <QVBoxLayout>
#include <Q3PopupMenu>

#include <stdlib.h>
#include <ctype.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace RDBDebugger
{

enum Column {
    Control     = 0,
    Enable      = 1,
    Type        = 2,
    Status      = 3,
    Location    = 4
};


#define numCols 5

static int m_activeFlag = 0;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class BreakpointTableRow : public Q3TableItem
{
public:

    BreakpointTableRow(Q3Table* table, EditType editType, Breakpoint* bp);
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

BreakpointTableRow::BreakpointTableRow(Q3Table* parent, EditType editType,
                                       Breakpoint* bp) :
        Q3TableItem(parent, editType, ""),
        m_breakpoint(bp)
{
    appendEmptyRow();
    setRow();
}

/***************************************************************************/

BreakpointTableRow::~BreakpointTableRow()
{
    delete m_breakpoint;
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

    Q3CheckTableItem* cti = new Q3CheckTableItem( table(), "");
    table()->setItem(row, Enable, cti);
}

/***************************************************************************/

void BreakpointTableRow::setRow()
{
    if ( m_breakpoint )
    {
        Q3TableItem *item =  table()->item ( row(), Enable );
        Q_ASSERT(item->rtti() == 2);
        ((Q3CheckTableItem*)item)->setChecked(m_breakpoint->isEnabled());

        QString status=m_breakpoint->statusDisplay(m_activeFlag);

        table()->setText(row(), Status, status);

        QString displayType = m_breakpoint->displayType();
        table()->setText(row(), Location, m_breakpoint->location());

        if (m_breakpoint->isTemporary())
            displayType = i18n(" temporary");

        table()->setText(row(), Type, displayType);
        table()->adjustColumn(Type);
        table()->adjustColumn(Status);
        table()->adjustColumn(Location);
    }
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

RDBBreakpointWidget::RDBBreakpointWidget(QWidget *parent, const char *name) :
    Q3HBox(parent, name)
{
    Q3Frame* toolbar = new Q3Frame( this );
    QVBoxLayout *l = new QVBoxLayout(toolbar, 0, 0);

    toolbar->setFrameStyle( QFrame::StyledPanel | Q3Frame::Plain );
    toolbar->setLineWidth( 0 );

    m_add       = new QToolButton( toolbar, "add breakpoint" );
    m_add->setPixmap ( SmallIcon ( "breakpoint_add" ) );
    QToolTip::add ( m_add, i18n ( "Add empty breakpoint" ) + I18N_NOOP(" <Alt+A>"));
    Q3WhatsThis::add( m_add, i18n("<b>Add empty breakpoint</b><p>Shows a popup menu that allows you to choose "
        "the type of breakpoint, then adds a breakpoint of the selected type to the breakpoints list."));

    m_delete    = new QToolButton( toolbar, "delete breakpoint" );
    m_delete->setPixmap ( SmallIcon ( "breakpoint_delete" ) );
    QToolTip::add ( m_delete, i18n ( "Delete selected breakpoint" ) + I18N_NOOP(" <Delete>") );
    Q3WhatsThis::add( m_delete, i18n("<b>Delete selected breakpoint</b><p>Deletes the selected breakpoint in the breakpoints list."));

    m_edit      = new QToolButton( toolbar, "edit breakpoint" );
    m_edit->setPixmap ( SmallIcon ( "breakpoint_edit" ) );
    QToolTip::add ( m_edit, i18n ( "Edit selected breakpoint" ) + I18N_NOOP(" <Return>")  );
    Q3WhatsThis::add( m_edit, i18n("<b>Edit selected breakpoint</b><p>Allows to edit location, condition and ignore count properties of the selected breakpoint in the breakpoints list."));

    m_removeAll      = new QToolButton( toolbar, "Delete all breakppoints" );
    m_removeAll->setPixmap ( SmallIcon ( "breakpoint_delete_all" ) );
    QToolTip::add ( m_removeAll, i18n ( "Remove all breakpoints" ) );
    Q3WhatsThis::add( m_removeAll, i18n("<b>Remove all breakpoints</b><p>Removes all breakpoints in the project."));

    l->addWidget(m_add);
    l->addWidget(m_edit);
    l->addWidget(m_delete);
    l->addWidget(m_removeAll);
    QSpacerItem* spacer = new QSpacerItem( 5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    l->addItem(spacer);

    Q3PopupMenu *addMenu = new Q3PopupMenu( this );
    addMenu->insertItem( i18n( "File:line" ),   BP_TYPE_FilePos );
    addMenu->insertItem( i18n( "Watchpoint" ),  BP_TYPE_Watchpoint );
    addMenu->insertItem( i18n( "Catchpoint" ),  BP_TYPE_Catchpoint );
    addMenu->insertItem( i18n( "Method()" ),    BP_TYPE_Function );
    m_add->setPopup( addMenu );
    m_add->setPopupDelay(1);

    m_table = new RDBTable(0, numCols, this, name);
    m_table->setSelectionMode(Q3Table::SingleRow);
    m_table->setShowGrid (false);
    m_table->setLeftMargin(0);
    m_table->setFocusStyle(Q3Table::FollowStyle);

    m_table->hideColumn(Control);
    m_table->setColumnReadOnly(Type, true);
    m_table->setColumnReadOnly(Status, true);
    m_table->setColumnWidth( Enable, 20);

    Q3Header *header = m_table->horizontalHeader();

    header->setLabel( Enable,       "" );
    header->setLabel( Type,         i18n("Type") );
    header->setLabel( Status,       i18n("Status") );
    header->setLabel( Location,     i18n("Location") );

    m_table->show();
	
    m_ctxMenu = new Q3PopupMenu( this );
    m_ctxMenu->insertItem( i18n( "Show" ),    BW_ITEM_Show );
    m_ctxMenu->insertItem( i18n( "Edit" ),    BW_ITEM_Edit );
    m_ctxMenu->insertItem( i18n( "Disable" ), BW_ITEM_Disable );
    m_ctxMenu->insertItem( i18n( "Delete" ),  BW_ITEM_Delete );

    connect( addMenu,     SIGNAL(activated(int)),
             this,          SLOT(slotAddBlankBreakpoint(int)) );
    connect( m_delete,      SIGNAL(clicked()),
             this,          SLOT(slotRemoveBreakpoint()) );
    connect( m_edit,        SIGNAL(clicked()),
             this,          SLOT(slotEditBreakpoint()) );
    connect( m_removeAll,   SIGNAL(clicked()),
             this,          SLOT(slotRemoveAllBreakpoints()) );

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
             this,          SLOT(slotAddBreakpoint()));
}

/***************************************************************************/

RDBBreakpointWidget::~RDBBreakpointWidget()
{
    delete m_table;
}

/***************************************************************************/

void RDBBreakpointWidget::reset()
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            btr->reset();
            emit publishBPState(*(btr->breakpoint()));
        }
    }
}

/***************************************************************************/

// When a file is loaded then we need to tell the editor (display window)
// which lines contain a breakpoint.
void RDBBreakpointWidget::slotRefreshBP(const KURL &filename)
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            FilePosBreakpoint* bp = dynamic_cast<FilePosBreakpoint*>(btr->breakpoint());
            if (bp && (bp->fileName() == filename.path()))
                emit refreshBPState(*bp);
        }
    }
}

/***************************************************************************/

BreakpointTableRow* RDBBreakpointWidget::find(Breakpoint *breakpoint)
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
BreakpointTableRow* RDBBreakpointWidget::findId(int dbgId)
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
BreakpointTableRow* RDBBreakpointWidget::findKey(int BPKey)
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr && btr->breakpoint()->key() == BPKey)
            return btr;
    }

    return 0;
}

/***************************************************************************/

BreakpointTableRow* RDBBreakpointWidget::addBreakpoint(Breakpoint *bp)
{
    BreakpointTableRow* btr =
        new BreakpointTableRow( m_table, Q3TableItem::WhenCurrent, bp );
    emit publishBPState(*bp);
    return btr;
}

/***************************************************************************/

void RDBBreakpointWidget::removeBreakpoint(BreakpointTableRow* btr)
{
    if (!btr)
        return;

    // Pending but the debugger hasn't started processing this bp so
    // we can just remove it.
    Breakpoint* bp = btr->breakpoint();
    if (bp->isPending() && !bp->isDbgProcessing())
    {
        bp->setActionDie();
        emit publishBPState(*bp);
        m_table->removeRow(btr->row());
    }
    else
    {
        bp->setPending(true);
        bp->setActionClear(true);
        emit publishBPState(*bp);
        btr->setRow();
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotToggleBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    BreakpointTableRow* btr = find(fpBP);
    if (btr)
    {
        delete fpBP;
        removeBreakpoint(btr);
    }
    else
        addBreakpoint(fpBP);
}

/***************************************************************************/

void RDBBreakpointWidget::slotToggleBreakpointEnabled(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    BreakpointTableRow* btr = find(fpBP);
    delete fpBP;
    if (btr)
    {
        Breakpoint* bp=btr->breakpoint();
        bp->setEnabled(!bp->isEnabled());
        emit publishBPState(*bp);
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotToggleWatchpoint(const QString &varName)
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

/***************************************************************************/

// The debugger allows us to set pending breakpoints => do it
void RDBBreakpointWidget::slotSetPendingBPs()
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);

        if (btr)
        {
            Breakpoint* bp = btr->breakpoint();
            if (bp->isPending() && !bp->isDbgProcessing() && bp->isValid())
                emit publishBPState(*bp);
        }
    }
}

/***************************************************************************/

// The debugger is having trouble with this bp - probably because a library
// was unloaded and invalidated a bp that was previously set in the library
// code. Reset the bp so that we can try again later.
void RDBBreakpointWidget::slotUnableToSetBPNow(int BPid)
{
    if (BPid == -1)
        reset();
    else
        if (BreakpointTableRow *btr = findId(BPid))
            btr->reset();
}

/***************************************************************************/

void RDBBreakpointWidget::slotParseRDBBrkptList(char *str)
{
    // Another example of a not too uncommon occurance
    // No breakpoints.

    // Set the new active flag so that after we have read the
    // breakpoint list we can trim the breakpoints that have been
    // removed (temporary breakpoints do this)
    m_activeFlag++;
	QRegExp breakpoint_re("(\\d+) [^:]+:\\d+");
	int pos = 0;

	pos = breakpoint_re.search(str, pos);
	while (pos >= 0) {
        int id = breakpoint_re.cap(1).toInt();
		
        BreakpointTableRow* btr = findId(id);
        if (btr)
        {
            Breakpoint *bp = btr->breakpoint();
            bp->setActive(m_activeFlag, id);
            btr->setRow();
            emit publishBPState(*bp);
        }
	
		pos += breakpoint_re.matchedLength();
		pos = breakpoint_re.search(str, pos);
	}
	
	str = strstr(str, "Watchpoints:");
	if (str != 0) {
		QRegExp watchpoint_re("(\\d+) [^\n]+\n");
		int pos = 0;
	
		pos = watchpoint_re.search(str, pos);
		while (pos >= 0) {
			int id = watchpoint_re.cap(1).toInt();
			
			BreakpointTableRow* btr = findId(id);
			if (btr)
			{
				Breakpoint *bp = btr->breakpoint();
				bp->setActive(m_activeFlag, id);
				btr->setRow();
				emit publishBPState(*bp);
			}
		
			pos += watchpoint_re.matchedLength();
			pos = watchpoint_re.search(str, pos);
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
                removeBreakpoint(btr);
        }
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotParseRDBBreakpointSet(char *str, int BPKey)
{
    BreakpointTableRow* btr = findKey(BPKey);
    if (!btr)
        return;

    Breakpoint *bp = btr->breakpoint();
    bp->setDbgProcessing(false);

	QRegExp breakpoint_re("Set breakpoint (\\d+) at [^:]+:\\d+");
	QRegExp watchpoint_re("Set watchpoint (\\d+)");
	
	int id = 0;
	if (breakpoint_re.search(str, 0) != -1) {
		id = breakpoint_re.cap(1).toInt();
    } else if (watchpoint_re.search(str, 0) != -1) {
		id = watchpoint_re.cap(1).toInt();
    }

    if (id > 0)
    {        
		bp->setActive(m_activeFlag, id);
        emit publishBPState(*bp);
        btr->setRow();
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotAddBlankBreakpoint(int idx)
{
    BreakpointTableRow* btr = 0;
    switch (idx)
    {
      case BP_TYPE_FilePos:
          btr = addBreakpoint(new FilePosBreakpoint("", 0));
          break;

      case BP_TYPE_Watchpoint:
          btr = addBreakpoint(new Watchpoint(""));
          break;

      case BP_TYPE_Catchpoint:
          btr = addBreakpoint(new Catchpoint(""));
          break;
      
	  case BP_TYPE_Function:
          btr = addBreakpoint(new FunctionBreakpoint(""));
          break;

      default:
          break;
    }

    if (btr)
    {
        Q3TableSelection ts;
        ts.init(btr->row(), 0);
        ts.expandTo(btr->row(), numCols );
        m_table->addSelection(ts);
        m_table->editCell(btr->row(), Location, false);
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotRemoveBreakpoint()
{
    int row = m_table->currentRow();
    if ( row != -1)
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        removeBreakpoint(btr);
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotRemoveAllBreakpoints()
{
  while (m_table->numRows() > 0)
  {
    for ( int row = m_table->numRows()-1; row>=0; row-- )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        removeBreakpoint(btr);
    }
  }
}

/***************************************************************************/

void RDBBreakpointWidget::slotRowDoubleClicked(int row, int col, int btn, const QPoint &)
{
    if ( btn == Qt::LeftButton )
	{
//    kdDebug(9012) << "in slotRowSelected row=" << row << endl;
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            FilePosBreakpoint* bp = dynamic_cast<FilePosBreakpoint*>(btr->breakpoint());
            if (bp)
                emit gotoSourcePosition(bp->fileName(), bp->lineNum()-1);

            // put the focus back on the clicked item if appropriate
            if (col == Location)
                m_table->editCell(row, col, false);
        }
    }
}

void RDBBreakpointWidget::slotContextMenuShow( int row, int /*col*/, const QPoint &mousePos )
{
    BreakpointTableRow *btr = (BreakpointTableRow *)m_table->item( row, Control );
    
    if (btr != NULL)
    {
        m_ctxMenu->setItemEnabled( BW_ITEM_Show, (btr->breakpoint( )->type( ) == BP_TYPE_FilePos) );
        if (btr->breakpoint( )->isEnabled( ))
        {
            m_ctxMenu->changeItem( BW_ITEM_Disable, i18n("Disable") );
        }
        else
        {
            m_ctxMenu->changeItem( BW_ITEM_Disable, i18n("Enable") );
        }

        //m_ctxMenu->popup( mapToGlobal( mousePos ) );
        m_ctxMenu->popup( mousePos );
    }
}

void RDBBreakpointWidget::slotContextMenuSelect( int item )
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
            if (col == Location)
                m_table->editCell(row, col, false);
            break;
        case BW_ITEM_Disable:
            bp->setEnabled( !bp->isEnabled( ) );
            btr->setRow( );
            emit publishBPState( *bp );
            break;
        case BW_ITEM_Delete:
            slotRemoveBreakpoint( );
            break;
        default:
            // oops, check it out! this case is not in sync with the
            // m_ctxMenu.  Check the enum in the header file.
            return;
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotEditRow(int row, int col, const QPoint &)
{
//    kdDebug(9012) << "in slotEditRow row=" << row << endl;
    BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
    if (btr)
    {
        if (col == Location)
            m_table->editCell(row, col, false);
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotNewValue(int row, int col)
{
//    kdDebug(9012) << "in slotNewValue row=" << row << endl;
    BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);

    if (btr)
    {
        bool changed=false;
        Breakpoint* bp = btr->breakpoint();
        switch (col)
        {

        case Enable:
        {
            Q3CheckTableItem *item = (Q3CheckTableItem*)m_table->item ( row, Enable );
            if ( item->isChecked() != bp->isEnabled() )
            {
                bp->setEnabled(item->isChecked());
                bp->setPending(true);
                bp->setActionModify(true);
                changed = true;
            }
            break;
        }

        case Location:
        {
            if (bp->location() != m_table->text(btr->row(), Location))
            {
//                kdDebug(9012) << "Old location [" << bp->location() << "]" << endl;
//                kdDebug(9012) << "New location [" << m_table->text(btr->row(), Location) << "]" << endl;
                bp->setActionDie();
                emit publishBPState(*bp);
                bp->setPending(true);
                bp->setActionAdd(true);
                bp->setLocation(m_table->text(btr->row(), Location));
                changed = true;
            }
            break;
        }

        case Type:
        case Status:
        default:
            break;
        }

        if (changed)
        {
            btr->setRow();
            emit publishBPState(*bp);
        }
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotEditBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    BreakpointTableRow* btr = find(fpBP);
    delete fpBP;

    if (btr)
    {
        Q3TableSelection ts;
        ts.init(btr->row(), 0);
        ts.expandTo(btr->row(), numCols);
        m_table->addSelection(ts);
        m_table->editCell(btr->row(), Location, false);
    }

}

/***************************************************************************/

void RDBBreakpointWidget::slotEditBreakpoint()
{
    m_table->editCell(m_table->currentRow(), Location, false);
}

/***************************************************************************/

void RDBBreakpointWidget::savePartialProjectSession(QDomElement* el)
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

        breakpointListEl.appendChild(breakpointEl);
    }

    if (!breakpointListEl.isNull())
        el->appendChild(breakpointListEl);
}

/***************************************************************************/

void RDBBreakpointWidget::restorePartialProjectSession(const QDomElement* el)
{
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
                bp = new FilePosBreakpoint("", 0);
                break;
            }
            case BP_TYPE_Watchpoint:
            {
                bp = new Watchpoint("");
                break;
            }
            case BP_TYPE_Catchpoint:
            {
                bp = new Catchpoint("");
                break;
            }
            case BP_TYPE_Function:
            {
                bp = new FunctionBreakpoint("");
                break;
            }
            default:
                break;
            }

            // Common settings for any type of breakpoint
            if (bp)
            {
                bp->setLocation(breakpointEl.attribute( "location", ""));
                bp->setEnabled(breakpointEl.attribute( "enabled", "1").toInt());

                // Add the bp if we don't already have it.
                if (!find(bp))
                    addBreakpoint(bp);
                else
                    delete bp;
            }
        }
    }
}

/***************************************************************************/

void RDBBreakpointWidget::slotAddBreakpoint( )
{
    if (m_add->popup())
    {
        m_add->popup()->popup(mapToGlobal(this->geometry().topLeft()));
    }
}

/***************************************************************************/

void RDBBreakpointWidget::focusInEvent( QFocusEvent */* e*/ )
{
    m_table->setFocus();
}

}


#include "rdbbreakpointwidget.moc"
