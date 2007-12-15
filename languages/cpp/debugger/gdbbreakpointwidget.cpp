/*
 * GDB Debugger Support
 *
 * Copyright 2003 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "gdbbreakpointwidget.h"
#include "debuggertracingdialog.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"
#include "gdbglobal.h"
#include "debuggerplugin.h"
#include "breakpointcontroller.h"

#include "breakpoint.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <khbox.h>

#include <QFileInfo>
#include <QToolButton>
#include <QToolTip>
#include <QMenu>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QContextMenuEvent>

#include <stdlib.h>
#include <ctype.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <idocument.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

using namespace GDBMI;
using namespace GDBDebugger;

GDBBreakpointWidget::GDBBreakpointWidget(CppDebuggerPlugin* plugin, GDBController* controller, QWidget *parent)
    : QTableView(parent)
    , controller_(controller)
{
    setWindowTitle(i18n("Debugger Breakpoints"));
    setWhatsThis(i18n("<b>Breakpoint list</b><p>"
                      "Displays a list of breakpoints with "
                      "their current status. Clicking on a "
                      "breakpoint item allows you to change "
                      "the breakpoint and will take you "
                      "to the source in the editor window."));
    setWindowIcon( KIcon("process-stop") );

    setSelectionMode(QAbstractItemView::SingleSelection);
    setModel(breakpoints());

    //m_table->setColumnWidth( Enable, 20);

    // TODO: delegate with
    //DebuggerTracingDialog* d = new DebuggerTracingDialog(

    m_ctxMenu = new QMenu( this );

    QMenu* newBreakpoint = m_ctxMenu->addMenu( i18nc("New breakpoint", "New") );

    QAction* action = newBreakpoint->addAction( i18nc("Code breakpoint", "Code"), this, SLOT(slotAddBlankBreakpoint()) );
    // Use this action also to provide a local shortcut
    action->setShortcut(Qt::Key_A + Qt::ShiftModifier + Qt::AltModifier);
    addAction(action);

    newBreakpoint->addAction( i18nc("Data breakpoint", "Data write"), this, SLOT(slotAddBlankWatchpoint()) );
    newBreakpoint->addAction( i18nc("Data read breakpoint", "Data read"), this, SLOT(slotAddBlankReadWatchpoint()) );

    m_breakpointShow = m_ctxMenu->addAction( i18n( "Show text" ) );

    m_breakpointEdit = m_ctxMenu->addAction( i18n( "Edit" ) );
    m_breakpointEdit->setShortcut(Qt::Key_Enter);

    m_breakpointDisable = m_ctxMenu->addAction( i18n( "Disable" ) );

    m_breakpointDelete = m_ctxMenu->addAction( KIcon("breakpoint_delete"), i18n( "Delete" ), this, SLOT(slotRemoveBreakpoint()) );
    // Use this action also to provide a local shortcut
    m_breakpointDelete->setShortcut(Qt::Key_Delete);
    addAction(m_breakpointDelete);

    m_ctxMenu->addSeparator();

    m_breakpointDisableAll = m_ctxMenu->addAction( i18n( "Disable all") );
    m_breakpointEnableAll = m_ctxMenu->addAction( i18n( "Enable all") );
    m_breakpointDeleteAll = m_ctxMenu->addAction( i18n( "Delete all"), this, SLOT(slotRemoveAllBreakpoints()));

    connect( m_ctxMenu,     SIGNAL(triggered(QAction*)),
             this,          SLOT(slotContextMenuSelect(QAction*)) );

    connect(controller,
            SIGNAL(watchpointHit(int, const QString&, const QString&)),
            this,
            SLOT(slotWatchpointHit(int, const QString&, const QString&)));

    connect( this, SIGNAL(gotoSourcePosition(const QString&, int)),
             plugin,   SLOT(slotGotoSource(const QString&, int)) );

    connect( controller,   SIGNAL(breakpointHit(int)),
             this,         SLOT(slotBreakpointHit(int)));

    connect(plugin, SIGNAL(reset()), this, SLOT(reset()));

    connect( controller->breakpoints(), SIGNAL(toggledBreakpoint(const QString &, int)),
             this, SLOT(slotToggleBreakpoint(const QString &, int)) );
    connect( controller->breakpoints(), SIGNAL(editedBreakpoint(const QString &, int)),
             this, SLOT(slotEditBreakpoint(const QString &, int)) );
    connect( controller->breakpoints(), SIGNAL(toggledBreakpointEnabled(const QString &, int)),
             this, SLOT(slotToggleBreakpointEnabled(const QString &, int)) );
    connect(plugin, SIGNAL(toggleBreakpoint(const KUrl&, const KTextEditor::Cursor&)), this, SLOT(slotToggleBreakpoint(const KUrl&, const KTextEditor::Cursor&)));

    connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(slotMaybeEditTracing(QModelIndex)));
}

/***************************************************************************/

GDBBreakpointWidget::~GDBBreakpointWidget()
{
}

/***************************************************************************/

void GDBBreakpointWidget::reset()
{
    /*for ( int row = 0; row < m_table->numRows(); row++ )
    {
        BreakpointTableRow* btr = (BreakpointTableRow *) m_table->item(row, Control);
        if (btr)
        {
            btr->reset();
            sendToGdb(*(btr->breakpoint()));
        }
    }*/
}

/***************************************************************************/

void GDBBreakpointWidget::slotBreakpointHit(int id)
{
    Breakpoint* b = breakpoints()->findBreakpointById(id);

    // FIXME: should produce an message, this is most likely
    // an error.
    if (!b)
        return;

    if (b->tracingEnabled())
    {
        controller_->addCommand(
            new CliCommand(DataEvaluateExpression, b->traceRealFormatString(),
                           this,
                           &GDBBreakpointWidget::handleTracingPrintf));

        controller_->addCommand(new
                            GDBCommand(ExecContinue));

    }
    else
    {
        plugin_->demandAttention();
    }
}

void GDBBreakpointWidget::slotWatchpointHit(int id,
                                            const QString& oldValue,
                                            const QString& newValue)
{
   Breakpoint* b = breakpoints()->findBreakpointById(id);

    // FIXME: should produce an message, this is most likely
    // an error.
    if (!b)
        return;

    Watchpoint* w = qobject_cast<Watchpoint*>(b);

    if (!w)
        return;

    KMessageBox::information(
        this,
        i18n("<b>Data write breakpoint</b><br>"
             "Expression: %1<br>"
             "Address: 0x%2<br>"
             "Old value: %3<br>"
             "New value: %4", w->varName(), QString::number(w->address(), 16), oldValue, newValue));
}

/***************************************************************************/

void GDBBreakpointWidget::slotToggleBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = breakpoints()->findBreakpoint(fileName, lineNum);

    if (fpBP)
        breakpoints()->removeBreakpoint(fpBP);
    else
        breakpoints()->addBreakpoint(new FilePosBreakpoint(breakpoints(), fileName, lineNum));
}

/***************************************************************************/

void GDBBreakpointWidget::slotToggleBreakpointEnabled(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *fpBP = breakpoints()->findBreakpoint(fileName, lineNum);

    if (fpBP)
    {
        fpBP->setEnabled(!fpBP->isEnabled());
        fpBP->sendToGdb();
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotToggleWatchpoint(const QString &varName)
{
    Watchpoint *watchpoint = breakpoints()->findWatchpoint(varName);

    if (watchpoint)
        breakpoints()->removeBreakpoint(watchpoint);
    else
        breakpoints()->addBreakpoint(new Watchpoint(breakpoints(), varName, false, true));
}

void GDBBreakpointWidget::handleTracingPrintf(const QStringList& s)
{
    // The first line of output is the command itself, which we don't need.
    for(int i = 1; i < s.size(); ++i)
        emit tracingOutput(s[i].toLocal8Bit());
}

/***************************************************************************/

void GDBBreakpointWidget::slotAddBlankBreakpoint()
{
    editBreakpoint(breakpoints()->addBreakpoint(new FilePosBreakpoint(breakpoints())));
}

/***************************************************************************/

void GDBBreakpointWidget::slotAddBlankWatchpoint()
{
    editBreakpoint(breakpoints()->addBreakpoint(new Watchpoint(breakpoints(), "")));
}

/***************************************************************************/

void GDBBreakpointWidget::slotAddBlankReadWatchpoint()
{
    editBreakpoint(breakpoints()->addBreakpoint(new ReadWatchpoint(breakpoints(), "")));
}

/***************************************************************************/

void GDBBreakpointWidget::editBreakpoint(Breakpoint* bp)
{
    if (bp)
        openPersistentEditor(breakpoints()->indexForBreakpoint(bp, BreakpointController::Location));
}

/***************************************************************************/

void GDBBreakpointWidget::slotRemoveBreakpoint()
{
    if (selectionModel()->selection().indexes().isEmpty())
        return;
    
    QModelIndex selected = selectionModel()->selection().indexes().first();
    Breakpoint* breakpoint = breakpoints()->breakpointForIndex(selected);
    if (breakpoint)
    {
        breakpoints()->removeBreakpoint(breakpoint);
    }
}

/***************************************************************************/

void GDBBreakpointWidget::slotRemoveAllBreakpoints()
{
    breakpoints()->removeAllBreakpoints();
}

/***************************************************************************/

void GDBBreakpointWidget::contextMenuEvent(QContextMenuEvent* event)
{
    Breakpoint *bp = breakpoints()->breakpointForIndex(indexAt(event->pos()));

    if (!bp)
    {
        bp = breakpoints()->breakpointForIndex(currentIndex());
    }

    if (bp)
    {
        m_breakpointShow->setEnabled(bp->hasFileAndLine());

        if (bp->isEnabled( ))
        {
            m_breakpointDisable->setText( i18n("Disable") );
        }
        else
        {
            m_breakpointDisable->setText( i18n("Enable") );
        }
    }
    else
    {
        m_breakpointShow->setEnabled(false);
    }

    m_breakpointDisable->setEnabled(bp);
    m_breakpointDelete->setEnabled(bp);
    m_breakpointEdit->setEnabled(bp);

    bool has_bps = !breakpoints()->breakpoints().isEmpty();
    m_breakpointDisableAll->setEnabled(has_bps);
    m_breakpointEnableAll->setEnabled(has_bps);
    m_breakpointDelete->setEnabled(has_bps);

    m_ctxMenuBreakpoint = bp;
    m_ctxMenu->popup( event->globalPos() );
}

void GDBBreakpointWidget::slotContextMenuSelect( QAction* action )
{
    int                  row, col;
    Breakpoint          *bp = m_ctxMenuBreakpoint;

    if ( action == m_breakpointShow ) {
        if (FilePosBreakpoint* fbp = dynamic_cast<FilePosBreakpoint*>(bp))
            emit gotoSourcePosition(fbp->fileName(), fbp->lineNum()-1);

    } else if ( action == m_breakpointEdit ) {
        col = currentIndex().column();
        if (col == BreakpointController::Location || col ==  BreakpointController::Condition || col == BreakpointController::IgnoreCount)
            openPersistentEditor(model()->index(row, col, QModelIndex()));

    } else if ( action == m_breakpointDisable ) {
        bp->setEnabled( !bp->isEnabled( ) );
        bp->sendToGdb();

    } else if ( action == m_breakpointDisableAll || action == m_breakpointEnableAll ) {
        foreach (Breakpoint* breakpoint, breakpoints()->breakpoints())
        {
            breakpoint->setEnabled(action == m_breakpointEnableAll);
            breakpoint->sendToGdb();
        }
    }
}

void GDBBreakpointWidget::slotEditBreakpoint(const QString &fileName, int lineNum)
{
    FilePosBreakpoint *bp = breakpoints()->findBreakpoint(fileName, lineNum);

    if (bp)
        openPersistentEditor(breakpoints()->indexForBreakpoint(bp, BreakpointController::Location));
}

void GDBBreakpointWidget::slotEditBreakpoint()
{
    QModelIndex index = currentIndex().sibling(currentIndex().row(), BreakpointController::Location);
    openPersistentEditor(index);
}

void GDBBreakpointWidget::slotToggleBreakpoint(const KUrl & url, const KTextEditor::Cursor & cursor)
{
    slotToggleBreakpoint(url.path(), cursor.line() + 1);
}

BreakpointController* GDBBreakpointWidget::breakpoints() const
{
    return controller_->breakpoints();
}

void GDBBreakpointWidget::slotMaybeEditTracing(const QModelIndex & index)
{
    if (!index.isValid() || index.column() != BreakpointController::Tracing)
        return;

    Breakpoint* bp = breakpoints()->breakpointForIndex(index);
    if (!bp)
        return;

    DebuggerTracingDialog* dialog = new DebuggerTracingDialog(bp, this);
    dialog->show();
}

#include "gdbbreakpointwidget.moc"
