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

#include "breakpointwidget.h"
#include "breakpoint.h"

#include <qdict.h>
#include <qheader.h>
#include <qfileinfo.h>

#include <kpopupmenu.h>

#include <stdlib.h>
#include <ctype.h>
#include <klocale.h>
#include <qcursor.h>

#include "kdevdebugger.h"

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class BreakpointItem : public QListViewItem
{
public:
    BreakpointItem( BreakpointWidget* parent, const FilePosBreakpoint& BP )
        : QListViewItem( parent ),
          m_breakpoint( BP )
    {
      setRenameEnabled( BreakpointWidget::Condition, true );
      m_breakpoint.setActionAdd( true );
      m_breakpoint.setPending( true );
      emit listView()->publishBPState( m_breakpoint );
    }
    
    BreakpointWidget* listView() { return (BreakpointWidget*)QListViewItem::listView(); }
    
    QString text( int column ) const
    {
        switch( (BreakpointWidget::Column)column ) {
        case BreakpointWidget::Status:
            return m_breakpoint.statusString();
            break;
        case BreakpointWidget::File:
            return QFileInfo( m_breakpoint.fileName() ).fileName();
            break;
        case BreakpointWidget::Line:
            return QString::number( m_breakpoint.lineNum() );
            break;
        case BreakpointWidget::Hits:
            return QString::number( m_breakpoint.hits() );
            break;
        case BreakpointWidget::Condition:
            return m_breakpoint.conditional();
            break;
        }
        return QString::null;
    }
    
    const QPixmap* pixmap( int column ) const
    {
        if( column != BreakpointWidget::Status )
            return 0L;
        if( !m_breakpoint.isEnabled() )
            return KDevDebugger::disabledBreakpointPixmap();
//        if( m_breakpoint.hits() > 0 )
//            return KDevDebugger::reachedBreakpointPixmap();
        if( !m_breakpoint.isPending() )
            return KDevDebugger::activeBreakpointPixmap();
        return KDevDebugger::inactiveBreakpointPixmap();
    }
    
    void reset()
    {
        m_breakpoint.reset();
        listView()->repaintItem( this );
    }
    
    void remove()
    {
        // Pending but the debugger hasn't started processing this BP so
        // we can just remove it.
        if( m_breakpoint.isPending() && !m_breakpoint.isDbgProcessing() ) {
            m_breakpoint.setActionDie();
            emit listView()->publishBPState( m_breakpoint );
            delete this;
        } else {
            m_breakpoint.setPending( true );
            m_breakpoint.setActionClear( true );
            listView()->repaintItem( this );
            emit listView()->publishBPState( m_breakpoint );
        }
    }
    
    void modify()
    {
        if( !m_breakpoint.modifyDialog() )
          return;
          
        m_breakpoint.setPending( true );
        m_breakpoint.setActionModify( true );
        
        listView()->repaintItem( this );
        emit listView()->publishBPState( m_breakpoint );
    }
    
    void toggleEnabled()
    {
        m_breakpoint.setEnabled( !m_breakpoint.isEnabled() );
        m_breakpoint.setPending( true );
        m_breakpoint.setActionModify( true );
        
        listView()->repaintItem( this );
        emit listView()->publishBPState( m_breakpoint );
    }
    
    void update( int active, int id, int hits, int ignore, const QString& condition )
    {
        m_breakpoint.setActive(active, id);
        m_breakpoint.setHits(hits);
        m_breakpoint.setIgnoreCount(ignore);
        m_breakpoint.setConditional(condition);
        listView()->repaintItem( this );
        emit listView()->publishBPState( m_breakpoint );
    }
    
    void update( int active, int id, bool hardware )
    {
        m_breakpoint.setActive(active, id);
        m_breakpoint.setHardwareBP(hardware);
        listView()->repaintItem( this );
        emit listView()->publishBPState( m_breakpoint );
    }
    
    void publishIfPending()
    {
        if( m_breakpoint.isPending() && !m_breakpoint.isDbgProcessing() )
            emit listView()->publishBPState( m_breakpoint );
    }
    
    void setConditional( const QString& text )
    {
        m_breakpoint.setConditional( text );
        listView()->repaintItem( this );
        emit listView()->publishBPState( m_breakpoint );
    }
    
    const Breakpoint& breakpoint() { return m_breakpoint; }
    
private:
    FilePosBreakpoint m_breakpoint;
};

BreakpointWidget::BreakpointWidget(QWidget *parent, const char *name)
    :  KListView(parent, name),
       activeFlag_(0)
{
    addColumn( "Status" );
    addColumn( "File" );
    addColumn( "Line" );
    addColumn( "Hits" );
    addColumn( "Condition" );
    setColumnAlignment( Line, AlignRight );
    setColumnAlignment( Hits, AlignRight );
    setAllColumnsShowFocus( true );
    connect( this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
             SLOT(slotContextMenu(QListViewItem*)) );
    connect( this, SIGNAL(executed(QListViewItem*)),
             SLOT(slotExecuted(QListViewItem*)) );
    connect( this, SIGNAL(itemRenamed(QListViewItem*, int, const QString&)),
             SLOT(slotItemRenamed(QListViewItem*, int, const QString&)) );
}

/***************************************************************************/

BreakpointWidget::~BreakpointWidget()
{
}

/***************************************************************************/

const QPtrList<Breakpoint> BreakpointWidget::breakpoints()
{
    QPtrList<Breakpoint> bps;
    for( QListViewItemIterator it( this ); it.current(); ++it ) {
        bps.append( &((BreakpointItem*)it.current())->breakpoint() );
    }
    return bps;
}

/***************************************************************************/

void BreakpointWidget::reset()
{
    for( QListViewItemIterator it( this ); it.current(); ++it )
        ((BreakpointItem*)it.current())->reset();
}

/***************************************************************************/

// Essentially tells the editor (display window) what lines contain a
// breakpoint. Used when a file is loaded
void BreakpointWidget::refreshBP(const QString &filename)
{
    for( QListViewItemIterator it( this ); it.current(); ++it ) {
        BreakpointItem* item = (BreakpointItem*)it.current();
        const Breakpoint& BP = item->breakpoint();
        if (BP.hasSourcePosition() && (BP.fileName() == filename))
            emit refreshBPState(BP);
    }
}

/***************************************************************************/

BreakpointItem* BreakpointWidget::find( const Breakpoint& breakpoint ) const
{
    BreakpointWidget* that = const_cast<BreakpointWidget*>(this);
    for( QListViewItemIterator it( that ); it.current(); ++it ) {
        BreakpointItem* item = (BreakpointItem*)it.current();
        const Breakpoint& BP = item->breakpoint();
        if (breakpoint.match(&BP))
            return item;
    }
    
    return 0L;
}

/***************************************************************************/

// The Id is supplied by the debugger
BreakpointItem* BreakpointWidget::findId(int dbgId) const
{
    BreakpointWidget* that = const_cast<BreakpointWidget*>(this);
    for( QListViewItemIterator it( that ); it.current(); ++it ) {
        BreakpointItem* item = (BreakpointItem*)it.current();
        const Breakpoint& BP = item->breakpoint();
        if (BP.dbgId() == dbgId)
            return item;
    }
    
    return 0L;
}

/***************************************************************************/

// The key is a unique number supplied by us
BreakpointItem* BreakpointWidget::findKey(int BPKey) const
{
    BreakpointWidget* that = const_cast<BreakpointWidget*>(this);
    for( QListViewItemIterator it( that ); it.current(); ++it ) {
        BreakpointItem* item = (BreakpointItem*)it.current();
        const Breakpoint& BP = item->breakpoint();
        if (BP.key() == BPKey)
            return item;
    }
    
    return 0L;
}

/***************************************************************************/

void BreakpointWidget::removeAllBreakpoints()
{
    for( QListViewItemIterator it( this ); it.current(); ++it ) {
        BreakpointItem* item = (BreakpointItem*)it.current();
        item->remove();
    }
    
    if( childCount() > 0 )
        emit clearAllBreakpoints();
}

/***************************************************************************/

void BreakpointWidget::slotExecuted(QListViewItem *item)
{
    if( !item )
        return;
    
    const Breakpoint& BP = ((BreakpointItem*)item)->breakpoint();
    if (BP.hasSourcePosition())
        emit gotoSourcePosition(BP.fileName(), BP.lineNum()-1);
}

/***************************************************************************/

void BreakpointWidget::slotContextMenu(QListViewItem *item)
{
    KPopupMenu popup(i18n("Breakpoints"), this);
    if( item ) {
      BreakpointItem* bpItem = (BreakpointItem*)item;
      const Breakpoint& BP = bpItem->breakpoint();
      int idRemoveBP        = popup.insertItem( i18n("Remove Breakpoint") );
      int idEditBP          = popup.insertItem( i18n("Edit Breakpoint") );
      int idToggleBPEnabled = popup.insertItem( BP.isEnabled()?
                                                i18n("Disable Breakpoint") :
                                                i18n("Enable Breakpoint") );
      int idGotoSource      = popup.insertItem( i18n("Display Source Code") );
      popup.setItemEnabled(idGotoSource, BP.hasSourcePosition());
      popup.insertSeparator();
      int idClearAll        = popup.insertItem( i18n("Clear All Breakpoints") );
      int res = popup.exec(QCursor::pos());
      if (res == idRemoveBP)
          bpItem->remove();
      else if (res == idEditBP)
          bpItem->modify();
      else if (res == idToggleBPEnabled)
          bpItem->toggleEnabled();
      else if (res == idGotoSource && BP.hasSourcePosition())
          emit gotoSourcePosition(BP.fileName(), BP.lineNum()-1);
      else if (res == idClearAll)
          removeAllBreakpoints();
   } else {
      int idClearAll        = popup.insertItem( i18n("Clear All Breakpoints") );
      int res = popup.exec(QCursor::pos());
      if (res == idClearAll)
          removeAllBreakpoints();
   }
}

/***************************************************************************/
    
void BreakpointWidget::slotItemRenamed(QListViewItem *item, int col, const QString& text)
{
    if( col != Condition )
        return;
    BreakpointItem* bpItem = (BreakpointItem*)item;
    bpItem->setConditional( text );
}

/***************************************************************************/

void BreakpointWidget::slotToggleBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint BP( fileName, lineNum+1 );
    BreakpointItem* item = find( BP );
    if( item ) {
        item->remove();
    } else {
        new BreakpointItem( this, BP );
    }
}

/***************************************************************************/

void BreakpointWidget::slotEditBreakpoint(const QString &fileName, int lineNum)
{
    BreakpointItem* item = find( FilePosBreakpoint( fileName, lineNum+1 ) );
    if( item )
        item->modify();
}

/***************************************************************************/

void BreakpointWidget::slotToggleBreakpointEnabled(const QString &fileName, int lineNum)
{
    BreakpointItem* item = find( FilePosBreakpoint( fileName, lineNum+1 ) );
    if( item )
        item->toggleEnabled();
}

/***************************************************************************/

// The debugger allows us to set pending breakpoints => do it
void BreakpointWidget::slotSetPendingBPs()
{
    for( QListViewItemIterator it( this ); it.current(); ++it ) {
        BreakpointItem* item = (BreakpointItem*)it.current();
        item->publishIfPending();
    }
}

/***************************************************************************/

// The debugger is having trouble with this BP - probably because a library
// was unloaded and invalidated a BP that was previously set in the library
// code. Reset the BP so that we can try again later.
void BreakpointWidget::slotUnableToSetBPNow(int BPid)
{
    if (BPid == -1)
        reset();
    else if (BreakpointItem *item = findId(BPid))
        item->reset();
}

/***************************************************************************/

void BreakpointWidget::slotParseGDBBrkptList(char *str)
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
    activeFlag_++;
    
    // skip the first line which is the header
    while (str && (str = strchr(str, '\n'))) {
        str++;
        int id = atoi(str);
        if (id) {
            // Inner loop handles lines of extra data for this breakpoint
            // eg
            //  3   breakpoint     keep y   0x0804a847 in main at main.cpp:23
            //         breakpoint already hit 1 time"
            int hits = 0;
            int ignore = 0;
            QString condition;
            while (str && (str = strchr(str, '\n'))) {
                str++;
                
                // The char after a newline is a digit hence it's
                // a new breakpt. Breakout to deal with this breakpoint.
                if (isdigit(*str)) {
                    str--;
                    break;
                }
                
                // We're only interested in these fields here.
                if (strncmp(str, "\tbreakpoint already hit ", 24) == 0)
                    hits = atoi(str+24);
                
                if (strncmp(str, "\tignore next ", 13) == 0)
                    ignore = atoi(str+13);
                
                if (strncmp(str, "\tstop only if ", 14) == 0) {
                    char* EOL = strchr(str, '\n');
                    if (EOL)
                        condition = QCString(str+14, EOL-(str+13));
                }
            }
            
            if (BreakpointItem *item = findId(id)) {
                item->update( activeFlag_, id, hits, ignore, condition );
            }
        }
    }
    
    // Remove any inactive breakpoints.
    for( QListViewItemIterator it( this ); it.current(); ++it ) {
        BreakpointItem* item = (BreakpointItem*)it.current();
        if( item->breakpoint().isActive( activeFlag_ ) )
            continue;
        item->remove();
    }
}

/***************************************************************************/

void BreakpointWidget::slotParseGDBBreakpointSet(char *str, int BPKey)
{
    char *startNo=0;
    bool hardware = false;
    BreakpointItem* item = findKey(BPKey);
    if (!item)
        return;   // Why ?? Possibly internal dbgController BPs that shouldn't get here!
    
//    item->setDbgProcessing(false);
    
    if ((strncmp(str, "Breakpoint ", 11) == 0))
        startNo = str+11;
    else {
        if ((strncmp(str, "Hardware watchpoint ", 20) == 0)) {
            hardware = true;
            startNo = str+20;
        } else if ((strncmp(str, "Watchpoint ", 11) == 0))
            startNo = str+11;
    }
    
    if (startNo) {
        int id = atoi(startNo);
        if (id) {
            item->update( activeFlag_, id, hardware );
        }
    }
}

/***************************************************************************/
#include "breakpointwidget.moc"
