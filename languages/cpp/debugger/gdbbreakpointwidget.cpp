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

#include "breakpoint.h"
#include "domutil.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include <qvbuttongroup.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qtable.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qvbox.h>
#include <qlayout.h>

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
    Hits        = 7
};


#define numCols 8

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

    QCheckTableItem* cti = new QCheckTableItem( table(), "");
    table()->setItem(row, Enable, cti);
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

GDBBreakpointWidget::GDBBreakpointWidget(QWidget *parent, const char *name) :
    QHBox(parent, name)
{
    QFrame* toolbar = new QFrame( this );
    QVBoxLayout *l = new QVBoxLayout(toolbar, 0, 0);

    toolbar->setFrameStyle( QFrame::ToolBarPanel | QFrame::Plain );
    toolbar->setLineWidth( 0 );

    m_add       = new QToolButton( toolbar, "add breakpoint" );
    m_add->setPixmap ( SmallIcon ( "breakpoint_add" ) );
    QToolTip::add ( m_add, i18n ( "Add empty breakpoint" ) + I18N_NOOP(" <Alt+A>"));
    QWhatsThis::add( m_add, i18n("<b>Add empty breakpoint</b><p>Shows a popup menu that allows you to choose "
        "the type of breakpoint, then adds a breakpoint of the selected type to the breakpoints list."));

    m_delete    = new QToolButton( toolbar, "delete breakpoint" );
    m_delete->setPixmap ( SmallIcon ( "breakpoint_delete" ) );
    QToolTip::add ( m_delete, i18n ( "Delete selected breakpoint" ) + I18N_NOOP(" <Delete>") );
    QWhatsThis::add( m_delete, i18n("<b>Delete selected breakpoint</b><p>Deletes the selected breakpoint in the breakpoints list."));

    m_edit      = new QToolButton( toolbar, "edit breakpoint" );
    m_edit->setPixmap ( SmallIcon ( "breakpoint_edit" ) );
    QToolTip::add ( m_edit, i18n ( "Edit selected breakpoint" ) + I18N_NOOP(" <Return>")  );
    QWhatsThis::add( m_edit, i18n("<b>Edit selected breakpoint</b><p>Allows to edit location, condition and ignore count properties of the selected breakpoint in the breakpoints list."));

    m_removeAll      = new QToolButton( toolbar, "Delete all breakppoints" );
    m_removeAll->setPixmap ( SmallIcon ( "breakpoint_delete_all" ) );
    QToolTip::add ( m_removeAll, i18n ( "Remove all breakpoints" ) );
    QWhatsThis::add( m_removeAll, i18n("<b>Remove all breakpoints</b><p>Removes all breakpoints in the project."));

    l->addWidget(m_add);
    l->addWidget(m_edit);
    l->addWidget(m_delete);
    l->addWidget(m_removeAll);
    QSpacerItem* spacer = new QSpacerItem( 5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding );
    l->addItem(spacer);

    QPopupMenu *addMenu = new QPopupMenu( this );
    addMenu->insertItem( i18n( "File:line" ),   BP_TYPE_FilePos );
    addMenu->insertItem( i18n( "Watchpoint" ),  BP_TYPE_Watchpoint );
    addMenu->insertItem( i18n( "Address" ),     BP_TYPE_Address );
    addMenu->insertItem( i18n( "Method()" ),    BP_TYPE_Function );
    m_add->setPopup( addMenu );
    m_add->setPopupDelay(1);

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

    m_table->show();

    connect( addMenu,     SIGNAL(activated(int)),
             this,          SLOT(slotAddBlankBreakpoint(int)) );
    connect( m_delete,      SIGNAL(clicked()),
             this,          SLOT(slotRemoveBreakpoint()) );
    connect( m_edit,        SIGNAL(clicked()),
             this,          SLOT(slotEditBreakpoint()) );
    connect( m_removeAll,   SIGNAL(clicked()),
             this,          SLOT(slotRemoveAllBreakpoints()) );

//    connect( m_table,       SIGNAL(contextMenuRequested(int, int, const QPoint &)),
//             this,          SLOT(slotEditRow(int, int, const QPoint &)));
    connect( m_table,       SIGNAL(clicked(int, int, int, const QPoint &)),
             this,          SLOT(slotRowSelected(int, int, int, const QPoint &)));
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
            emit publishBPState(*(btr->breakpoint()));
        }
    }
}

/***************************************************************************/

// When a file is loaded then we need to tell the editor (display window)
// which lines contain a breakpoint.
void GDBBreakpointWidget::slotRefreshBP(const QString &filename)
{
    for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            FilePosBreakpoint* bp = dynamic_cast<FilePosBreakpoint*>(btr->breakpoint());
            if (bp && (bp->fileName() == filename))
                emit refreshBPState(*bp);
        }
    }
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

/***************************************************************************/

BreakpointTableRow* GDBBreakpointWidget::addBreakpoint(Breakpoint *bp)
{
    BreakpointTableRow* btr =
        new BreakpointTableRow( m_table, QTableItem::WhenCurrent, bp );
    emit publishBPState(*bp);
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

void GDBBreakpointWidget::slotToggleBreakpoint(const QString &fileName, int lineNum)
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

void GDBBreakpointWidget::slotToggleBreakpointEnabled(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    BreakpointTableRow* btr = find(fpBP);
    delete fpBP;
    if (btr)
    {
        Breakpoint* bp=btr->breakpoint();
        bp->setEnabled(!isEnabled());
        emit publishBPState(*bp);
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

/***************************************************************************/

// The debugger allows us to set pending breakpoints => do it
void GDBBreakpointWidget::slotSetPendingBPs()
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
void GDBBreakpointWidget::slotUnableToSetBPNow(int BPid)
{
    if (BPid == -1)
        reset();
    else
        if (BreakpointTableRow *btr = findId(BPid))
            btr->reset();
}

/***************************************************************************/

void GDBBreakpointWidget::slotParseGDBBrkptList(char *str)
{
    // An example of a GDB breakpoint table
    // Num Type           Disp Enb Address    What
    // 1   breakpoint     del  y   0x0804a7fb in main at main.cpp:22
    // 2   hw watchpoint  keep y   thisIsAGlobal_int
    // 3   breakpoint     keep y   0x0804a847 in main at main.cpp:23
    //        stop only if thisIsAGlobal_int == 1
    //        breakpoint already hit 1 time
    // 4   breakpoint     keep y   0x0804a930 in main at main.cpp:28
    //        ignore next 6 hits

    // Another example of a not too uncommon occurance
    // No breakpoints or watchpoints.

    // Set the new active flag so that after we have read the
    // breakpoint list we can trim the breakpoints that have been
    // removed (temporary breakpoints do this)
    m_activeFlag++;

    // skip the first line which is the header
    while (str && (str = strchr(str, '\n')))
    {
        str++;
        int id = atoi(str);
        if (id)
        {
            // Inner loop handles lines of extra data for this breakpoint
            // eg
            //  3   breakpoint     keep y   0x0804a847 in main at main.cpp:23
            //         breakpoint already hit 1 time"
            int hits = 0;
            int ignore = 0;
            QString condition;
            while (str && (str = strchr(str, '\n')))
            {
                str++;

                // The char after a newline is a digit hence it's
                // a new breakpt. Breakout to deal with this breakpoint.
                if (isdigit(*str))
                {
                    str--;
                    break;
                }

                // We're only interested in these fields here.
                if (strncmp(str, "\tbreakpoint already hit ", 24) == 0)
                    hits = atoi(str+24);

                if (strncmp(str, "\tignore next ", 13) == 0)
                    ignore = atoi(str+13);

                if (strncmp(str, "\tstop only if ", 14) == 0)
                {
                    char* EOL = strchr(str, '\n');
                    if (EOL)
                        condition = QCString(str+14, EOL-(str+13));
                }
            }

            BreakpointTableRow* btr = findId(id);
            if (btr)
            {
                Breakpoint *bp = btr->breakpoint();
                bp->setActive(m_activeFlag, id);
                bp->setHits(hits);
                bp->setIgnoreCount(ignore);
                bp->setConditional(condition);
                btr->setRow();
                emit publishBPState(*bp);
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
                removeBreakpoint(btr);
        }
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotParseGDBBreakpointSet(char *str, int BPKey)
{
    char *startNo=0;
    bool hardware = false;
    BreakpointTableRow* btr = findKey(BPKey);
    if (!btr)
        return;

    Breakpoint *bp = btr->breakpoint();
    bp->setDbgProcessing(false);

    if ((strncmp(str, "Breakpoint ", 11) == 0))
        startNo = str+11;
    else
    {
        if ((strncmp(str, "Hardware watchpoint ", 20) == 0))
        {
            hardware = true;
            startNo = str+20;
        }
        else if ((strncmp(str, "Watchpoint ", 11) == 0))
            startNo = str+11;
    }

    if (startNo)
    {
        int id = atoi(startNo);
        if (id)
        {
            bp->setActive(m_activeFlag, id);
            bp->setHardwareBP(hardware);
            emit publishBPState(*bp);
            btr->setRow();
        }
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotAddBlankBreakpoint(int idx)
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

      case BP_TYPE_Address:
          btr = addBreakpoint(new AddressBreakpoint(""));
          break;

      case BP_TYPE_Function:
          btr = addBreakpoint(new FunctionBreakpoint(""));
          break;

      default:
          break;
    }

    if (btr)
    {
        QTableSelection ts;
        ts.init(btr->row(), 0);
        ts.expandTo(btr->row(), numCols );
        m_table->addSelection(ts);
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

void GDBBreakpointWidget::slotRowSelected(int row, int col, int btn, const QPoint &)
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
            if (col == Location || col ==  Condition || col == IgnoreCount)
                m_table->editCell(row, col, false);
        }
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
            QCheckTableItem *item = (QCheckTableItem*)m_table->item ( row, Enable );
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

        case Condition:
        {
            if (bp->conditional() != m_table->text(btr->row(), Condition))
            {
//                kdDebug(9012) << "Old condition [" << bp->conditional() << "]" << endl;
//                kdDebug(9012) << "New condition [" << m_table->text(btr->row(), Condition) << "]" << endl;
                bp->setConditional(m_table->text(btr->row(), Condition));
                bp->setPending(true);
                bp->setActionModify(true);
                changed = true;
            }
            break;
        }

        case IgnoreCount:
        {
            if (bp->ignoreCount() != m_table->text(btr->row(), IgnoreCount).toInt())
            {
//                kdDebug(9012) << "Old ignoreCount [" << bp->ignoreCount() << "]" << endl;
//                kdDebug(9012) << "New ignoreCount [" << m_table->text(btr->row(), IgnoreCount) << "]" << endl;
                bp->setIgnoreCount(m_table->text(btr->row(), IgnoreCount).toInt());
                bp->setPending(true);
                bp->setActionModify(true);
                changed = true;
            }
            break;
        }

        case Type:
        case Status:
        case Hits:
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

/***************************************************************************/

void GDBBreakpointWidget::slotEditBreakpoint()
{
    m_table->editCell(m_table->currentRow(), Location, false);
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

        breakpointListEl.appendChild(breakpointEl);
    }

    if (!breakpointListEl.isNull())
        el->appendChild(breakpointListEl);
}

/***************************************************************************/

void GDBBreakpointWidget::restorePartialProjectSession(const QDomElement* el)
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
            case BP_TYPE_Address:
            {
                bp = new Watchpoint("");
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
                bp->setConditional(breakpointEl.attribute( "condition", ""));

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

void GDBBreakpointWidget::slotAddBreakpoint( )
{
    if (m_add->popup())
    {
        m_add->popup()->popup(mapToGlobal(this->geometry().topLeft()));
    }
}

/***************************************************************************/

void GDBBreakpointWidget::focusInEvent( QFocusEvent */* e*/ )
{
    m_table->setFocus();
}

}


#include "gdbbreakpointwidget.moc"
