/***************************************************************************
                          brkptmanager.cpp  -  description
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

#include "breakpointwidget.h"
#include "breakpoint.h"

#include <qdict.h>
#include <qheader.h>

#include <kpopupmenu.h>

#include <stdlib.h>
#include <ctype.h>
#include <klocale.h>
#include <qcursor.h>

namespace JAVADebugger
{

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

BreakpointWidget::BreakpointWidget(QWidget *parent, const char *name)
    :  KListBox(parent, name),
       activeFlag_(0)
{
    connect( this, SIGNAL(rightButtonPressed(QListBoxItem*, const QPoint&)),
             SLOT(slotContextMenu(QListBoxItem*)) );
    connect ( this, SIGNAL(executed(QListBoxItem*)),
              SLOT(slotExecuted(QListBoxItem*)) );
}

/***************************************************************************/

BreakpointWidget::~BreakpointWidget()
{
}

/***************************************************************************/

void BreakpointWidget::reset()
{
    for (int index=0; index<(int)count(); index++)
        ((Breakpoint*)item(index))->reset();
    repaint();
}

/***************************************************************************/

// Essentially tells the editor (display window) what lines contain a
// breakpoint. Used when a file is loaded
void BreakpointWidget::refreshBP(const QString &filename)
{
    for (int index=0; index<(int)count(); index++) {
        Breakpoint *BP = (Breakpoint*)item(index);
        if (BP->hasSourcePosition() && (BP->fileName() == filename))
            emit refreshBPState(BP);
    }
}

/***************************************************************************/

int BreakpointWidget::findIndex(const Breakpoint *breakpoint) const
{
    // NOTE:- The match doesn't have to be equal. Each type of BP
    // must decide on the match criteria.
    Q_ASSERT (breakpoint);

    for (int index=0; index<(int)count(); index++) {
        Breakpoint *BP = (Breakpoint*)(item(index));
        if (breakpoint->match(BP))
            return index;
    }

    return -1;
}

/***************************************************************************/

// The Id is supplied by the debugger
Breakpoint *BreakpointWidget::findId(int dbgId) const
{
    for (int index=0; index<(int)count(); index++) {
        Breakpoint *BP = (Breakpoint*)item(index);
        if (BP->dbgId() == dbgId)
            return BP;
    }

    return 0;
}

/***************************************************************************/

// The key is a unique number supplied by us
Breakpoint *BreakpointWidget::findKey(int BPKey) const
{
    for (int index=0; index<(int)count(); index++) {
        Breakpoint *BP = (Breakpoint*)item(index);
        if (BP->key() == BPKey)
            return BP;
    }

    return 0;
}


/***************************************************************************/

void BreakpointWidget::addBreakpoint(Breakpoint *BP)
{
    insertItem(BP);
    BP->setActionAdd(true);
    BP->setPending(true);
    emit publishBPState(BP);

    BP->configureDisplay();
    repaint();
}

/***************************************************************************/

void BreakpointWidget::removeBreakpoint(Breakpoint *BP)
{
    // Pending but the debugger hasn't started processing this BP so
    // we can just remove it.
    if (BP->isPending() && !BP->isDbgProcessing()) {
        BP->setActionDie();
        emit publishBPState(BP);
        removeItem(findIndex(BP));
    } else {
        BP->setPending(true);
        BP->setActionClear(true);
        emit publishBPState(BP);

        BP->configureDisplay();
    }

    repaint();
}

/***************************************************************************/

void BreakpointWidget::modifyBreakpoint(Breakpoint *BP)
{
    if (BP->modifyDialog()) {
        BP->setPending(true);
        BP->setActionModify(true);
        emit publishBPState(BP);

        BP->configureDisplay();
        repaint();
    }
}

/***************************************************************************/

void BreakpointWidget::toggleBPEnabled(Breakpoint *BP)
{
    BP->setEnabled(!BP->isEnabled());
    BP->setPending(true);
    BP->setActionModify(true);
    emit publishBPState(BP);

    BP->configureDisplay();
    repaint();
}

/***************************************************************************/

void BreakpointWidget::removeAllBreakpoints()
{
    for (int index=count()-1; index>=0; index--) {
        Breakpoint *BP = (Breakpoint*)item(index);
        if (BP->isPending() && !BP->isDbgProcessing())
            removeBreakpoint(BP);
    }

    if (count())
        emit clearAllBreakpoints();
}

/***************************************************************************/

void BreakpointWidget::slotExecuted(QListBoxItem *item)
{
    if (item) {
        setCurrentItem(item);
        Breakpoint *BP = (Breakpoint*)item;
        if (BP->hasSourcePosition())
            emit gotoSourcePosition(BP->fileName(), BP->lineNum()-1);
    }
}

/***************************************************************************/

void BreakpointWidget::slotContextMenu(QListBoxItem *item)
{
    if (!item)
        return;

    Breakpoint *BP = (Breakpoint*)item;
    KPopupMenu popup(this);
    popup.insertTitle(i18n("Breakpoints"));
    int idRemoveBP        = popup.insertItem( i18n("Remove Breakpoint") );
    int idEditBP          = popup.insertItem( i18n("Edit Breakpoint") );
    int idToggleBPEnabled = popup.insertItem( BP->isEnabled()?
                                              i18n("Disable Breakpoint") :
                                              i18n("Enable Breakpoint") );
    int idGotoSource      = popup.insertItem( i18n("Display Source Code") );
    popup.setItemEnabled(idGotoSource, BP->hasSourcePosition());
    popup.insertSeparator();
    int idClearAll        = popup.insertItem( i18n("Clear All Breakpoints") );

    int res = popup.exec(QCursor::pos());

    if (res == idRemoveBP)
        removeBreakpoint(BP);
    else if (res == idEditBP)
        modifyBreakpoint(BP);
    else if (res == idToggleBPEnabled)
        toggleBPEnabled(BP);
    else if (res == idGotoSource && BP->hasSourcePosition())
        emit gotoSourcePosition(BP->fileName(), BP->lineNum()-1);
    else if (res == idClearAll)
        removeAllBreakpoints();
}

/***************************************************************************/

void BreakpointWidget::slotToggleBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    int found = findIndex(fpBP);
    if (found >= 0) {
        delete fpBP;
        removeBreakpoint((Breakpoint*)item(found));
    } else
        addBreakpoint(fpBP);
}

/***************************************************************************/

void BreakpointWidget::slotEditBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    int found = findIndex(fpBP);
    delete fpBP;
    if (found >= 0)
        modifyBreakpoint((Breakpoint*)item(found));
}

/***************************************************************************/

void BreakpointWidget::slotToggleBreakpointEnabled(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = new FilePosBreakpoint(fileName, lineNum+1);

    int found = findIndex(fpBP);
    delete fpBP;
    if (found >= 0) {
        setCurrentItem(found);
        toggleBPEnabled((Breakpoint*)item(found));
    }
}

/***************************************************************************/

void BreakpointWidget::slotToggleWatchpoint(const QString &varName)
{
    Watchpoint *watchpoint = new Watchpoint(varName, false, true);
    int found = findIndex(watchpoint);
    if (found >= 0) {
        removeBreakpoint((Breakpoint*)item(found));
        delete watchpoint;
    } else
        addBreakpoint(watchpoint);
}

/***************************************************************************/

// The debugger allows us to set pending breakpoints => do it
void BreakpointWidget::slotSetPendingBPs()
{
    for (int index=0; index<(int)count(); index++) {
        Breakpoint *BP = (Breakpoint*)(item(index));
        if (BP->isPending() && !BP->isDbgProcessing())
            emit publishBPState(BP);
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
    else if (Breakpoint *BP = findId(BPid))
        BP->reset();

    repaint();
}

/***************************************************************************/

void BreakpointWidget::slotParseJDBBrkptList(char *str)
{
    // An example of a JDB breakpoint table
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

            if (Breakpoint *BP = findId(id)) {
                BP->setActive(activeFlag_, id);
                BP->setHits(hits);
                BP->setIgnoreCount(ignore);
                BP->setConditional(condition);
                emit publishBPState(BP);

                BP->configureDisplay();
            }
        }
    }

    // Remove any inactive breakpoints.
    for (int index=count()-1; index>=0; index--) {
        Breakpoint* BP = (Breakpoint*)(item(index));
        if (!BP->isActive(activeFlag_)) {
            BP->setActionDie();
            emit publishBPState(BP);

            removeItem(index);
        }
    }

    //  setAutoUpdate(true);
    repaint();
}

/***************************************************************************/

void BreakpointWidget::slotParseJDBBreakpointSet(char *str, int BPKey)
{
    char *startNo=0;
    bool hardware = false;
    Breakpoint *BP = findKey(BPKey);
    if (!BP)
        return;   // Why ?? Possibly internal dbgController BPs that shouldn't get here!

    BP->setDbgProcessing(false);

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
            BP->setActive(activeFlag_, id);
            BP->setHardwareBP(hardware);
            emit publishBPState(BP);

            BP->configureDisplay();
            repaint();
        }
    }
}

/***************************************************************************/

}

#include "breakpointwidget.moc"
