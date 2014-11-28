/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>
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

#include "breakpointwidget.h"

#include <QIcon>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>

#include <KLocalizedString>
#include <KPassivePopup>

#include "breakpointdetails.h"
#include "../breakpoint/breakpoint.h"
#include "../breakpoint/breakpointmodel.h"
#include "util/debug.h"
#include <interfaces/idebugcontroller.h>
#include <util/autoorientedsplitter.h>

#define IF_DEBUG(x)
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <util/placeholderitemproxymodel.h>

using namespace KDevelop;

BreakpointWidget::BreakpointWidget(IDebugController *controller, QWidget *parent)
: QWidget(parent), m_firstShow(true), m_debugController(controller),
  m_breakpointDisableAllAction(0), m_breakpointEnableAllAction(0), m_breakpointRemoveAll(0)
{
    setWindowTitle(i18nc("@title:window", "Debugger Breakpoints"));
    setWhatsThis(i18nc("@info:whatsthis", "Displays a list of breakpoints with "
                                          "their current status. Clicking on a "
                                          "breakpoint item allows you to change "
                                          "the breakpoint and will take you "
                                          "to the source in the editor window."));
    setWindowIcon( QIcon::fromTheme("process-stop") );

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    QSplitter *s = new AutoOrientedSplitter(this);
    layout->addWidget(s);
    m_splitter = s;

    m_breakpointsView = new QTableView(s);
    m_breakpointsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_breakpointsView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_breakpointsView->horizontalHeader()->setHighlightSections(false);
    m_breakpointsView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_details = new BreakpointDetails(s);

    s->setStretchFactor(0, 2);

    m_breakpointsView->verticalHeader()->hide();

    PlaceholderItemProxyModel* proxyModel = new PlaceholderItemProxyModel(this);
    proxyModel->setSourceModel(m_debugController->breakpointModel());
    proxyModel->setColumnHint(Breakpoint::LocationColumn, i18n("New code breakpoint ..."));
    proxyModel->setColumnHint(Breakpoint::ConditionColumn, i18n("Enter condition ..."));
    m_breakpointsView->setModel(proxyModel);
    connect(proxyModel, &PlaceholderItemProxyModel::dataInserted, this, &BreakpointWidget::slotDataInserted);
    m_proxyModel = proxyModel;

    connect(m_breakpointsView, &QTableView::activated, this, &BreakpointWidget::slotOpenFile);
    connect(m_breakpointsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BreakpointWidget::slotUpdateBreakpointDetail);
    connect(m_debugController->breakpointModel(), &BreakpointModel::rowsInserted, this, &BreakpointWidget::slotUpdateBreakpointDetail);
    connect(m_debugController->breakpointModel(), &BreakpointModel::rowsRemoved, this, &BreakpointWidget::slotUpdateBreakpointDetail);
    connect(m_debugController->breakpointModel(), &BreakpointModel::modelReset, this, &BreakpointWidget::slotUpdateBreakpointDetail);
    connect(m_debugController->breakpointModel(), &BreakpointModel::dataChanged, this, &BreakpointWidget::slotUpdateBreakpointDetail);


    connect(m_debugController->breakpointModel(),
            &BreakpointModel::hit,
            this, &BreakpointWidget::breakpointHit);

    connect(m_debugController->breakpointModel(),
            &BreakpointModel::error,
            this, &BreakpointWidget::breakpointError);

    setupPopupMenu();
}

void BreakpointWidget::setupPopupMenu()
{
    m_popup = new QMenu(this);

    QMenu* newBreakpoint = m_popup->addMenu( i18nc("New breakpoint", "&New") );
    newBreakpoint->setIcon(QIcon::fromTheme("list-add"));

    QAction* action = newBreakpoint->addAction(
        i18nc("Code breakpoint", "&Code"),
        this,
        SLOT(slotAddBlankBreakpoint()) );
    // Use this action also to provide a local shortcut
    action->setShortcut(QKeySequence(Qt::Key_B + Qt::CTRL,
                                        Qt::Key_C));
    addAction(action);

    newBreakpoint->addAction(
        i18nc("Data breakpoint", "Data &Write"),
        this, SLOT(slotAddBlankWatchpoint()));
    newBreakpoint->addAction(
        i18nc("Data read breakpoint", "Data &Read"),
        this, SLOT(slotAddBlankReadWatchpoint()));
    newBreakpoint->addAction(
        i18nc("Data access breakpoint", "Data &Access"),
        this, SLOT(slotAddBlankAccessWatchpoint()));

    QAction* breakpointDelete = m_popup->addAction(
        QIcon::fromTheme("edit-delete"),
        i18n( "&Delete" ),
        this,
        SLOT(slotRemoveBreakpoint()));
    breakpointDelete->setShortcut(Qt::Key_Delete);
    breakpointDelete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(breakpointDelete);


    m_popup->addSeparator();
    m_breakpointDisableAllAction = m_popup->addAction(i18n("Disable &All"), this, SLOT(slotDisableAllBreakpoints()));
    m_breakpointEnableAllAction = m_popup->addAction(i18n("&Enable All"), this, SLOT(slotEnableAllBreakpoints()));
    m_breakpointRemoveAll = m_popup->addAction(i18n("&Remove All"), this, SLOT(slotRemoveAllBreakpoints()));

    connect(m_popup,&QMenu::aboutToShow, this, &BreakpointWidget::slotPopupMenuAboutToShow);
}


void BreakpointWidget::contextMenuEvent(QContextMenuEvent* event)
{
    m_popup->popup(event->globalPos());
}

void BreakpointWidget::slotPopupMenuAboutToShow()
{
    if (m_debugController->breakpointModel()->rowCount() < 2) {
        m_breakpointDisableAllAction->setDisabled(true);
        m_breakpointEnableAllAction->setDisabled(true);
        m_breakpointRemoveAll->setDisabled(true);
    } else {
        m_breakpointRemoveAll->setEnabled(true);
        bool allDisabled = true;
        bool allEnabled = true;
        for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
            Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
            if (bp->enabled())
                allDisabled = false;
            else
                allEnabled = false;
        }
        m_breakpointDisableAllAction->setDisabled(allDisabled);
        m_breakpointEnableAllAction->setDisabled(allEnabled);
    }

}

void BreakpointWidget::showEvent(QShowEvent *)
{
    if (m_firstShow) {
        QHeaderView* header = m_breakpointsView->horizontalHeader();

        for (int i = 0; i < m_breakpointsView->model()->columnCount(); ++i) {
            if(i == Breakpoint::LocationColumn){
                continue;
            }
            m_breakpointsView->resizeColumnToContents(i);
        }
        //for some reasons sometimes width can be very small about 200... But it doesn't matter as we use tooltip anyway.
        int width = m_breakpointsView->size().width();

        header->resizeSection(Breakpoint::LocationColumn, width > 400 ? width/2 : header->sectionSize(Breakpoint::LocationColumn)*2 );
        m_firstShow = false;
    }
}

void BreakpointWidget::edit(KDevelop::Breakpoint *n)
{
    QModelIndex index = m_proxyModel->mapFromSource(m_debugController->breakpointModel()->breakpointIndex(n, Breakpoint::LocationColumn));
    m_breakpointsView->setCurrentIndex(index);
    m_breakpointsView->edit(index);
}

void BreakpointWidget::slotDataInserted(int column, const QVariant& value)
{
    Breakpoint* breakpoint = m_debugController->breakpointModel()->addCodeBreakpoint();
    breakpoint->setData(column, value);
}

void BreakpointWidget::slotAddBlankBreakpoint()
{
    edit(m_debugController->breakpointModel()->addCodeBreakpoint());
}

void BreakpointWidget::slotAddBlankWatchpoint()
{
    edit(m_debugController->breakpointModel()->addWatchpoint());
}

void BreakpointWidget::slotAddBlankReadWatchpoint()
{
    edit(m_debugController->breakpointModel()->addReadWatchpoint());
}


void KDevelop::BreakpointWidget::slotAddBlankAccessWatchpoint()
{
    edit(m_debugController->breakpointModel()->addAccessWatchpoint());
}


void BreakpointWidget::slotRemoveBreakpoint()
{
    QItemSelectionModel* sel = m_breakpointsView->selectionModel();
    QModelIndexList selected = sel->selectedIndexes();
    IF_DEBUG( qCDebug(DEBUGGER) << selected; )
    if (!selected.isEmpty()) {
        m_debugController->breakpointModel()->removeRow(selected.first().row());
    }
}

void BreakpointWidget::slotRemoveAllBreakpoints()
{
    m_debugController->breakpointModel()->removeRows(0, m_debugController->breakpointModel()->rowCount());
}


void BreakpointWidget::slotUpdateBreakpointDetail()
{
    QModelIndexList selected = m_breakpointsView->selectionModel()->selectedIndexes();
    IF_DEBUG( qCDebug(DEBUGGER) << selected; )
    if (selected.isEmpty()) {
        m_details->setItem(0);
    } else {
        m_details->setItem(m_debugController->breakpointModel()->breakpoint(selected.first().row()));
    }
}

void BreakpointWidget::breakpointHit(KDevelop::Breakpoint* b)
{
    IF_DEBUG( qCDebug(DEBUGGER) << b; )
    const QModelIndex index = m_proxyModel->mapFromSource(m_debugController->breakpointModel()->breakpointIndex(b, 0));
    m_breakpointsView->selectionModel()->select(
        index,
        QItemSelectionModel::Rows
        | QItemSelectionModel::ClearAndSelect);
}

void BreakpointWidget::breakpointError(KDevelop::Breakpoint* b, const QString& msg, int column)
{
    IF_DEBUG( qCDebug(DEBUGGER) << b << msg << column; )

    // FIXME: we probably should prevent this error notification during
    // initial setting of breakpoint, to avoid a cloud of popups.
    if (!m_breakpointsView->isVisible())
        return;

    const QModelIndex index = m_proxyModel->mapFromSource(m_debugController->breakpointModel()->breakpointIndex(b, column));
    QPoint p = m_breakpointsView->visualRect(index).topLeft();
    p = m_breakpointsView->mapToGlobal(p);

    KPassivePopup *pop = new KPassivePopup(m_breakpointsView);
    pop->setPopupStyle(KPassivePopup::Boxed);
    pop->setAutoDelete(true);
    // FIXME: the icon, too.
    pop->setView("", msg);
    pop->setTimeout(-1);
    pop->show(p);
}

void BreakpointWidget::slotOpenFile(const QModelIndex& breakpointIdx)
{
    if (breakpointIdx.column() != Breakpoint::LocationColumn){
        return;
    }
    Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(breakpointIdx.row());
    if (!bp || bp->line() == -1 || bp->url().isEmpty() ){
        return;
    }

    ICore::self()->documentController()->openDocument(bp->url(), KTextEditor::Cursor(bp->line(), 0), IDocumentController::DoNotFocus);
}

void BreakpointWidget::slotDisableAllBreakpoints()
{
    for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
        Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
        bp->setData(Breakpoint::EnableColumn, Qt::Unchecked);
    }
}

void BreakpointWidget::slotEnableAllBreakpoints()
{
    for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
        Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
        bp->setData(Breakpoint::EnableColumn, Qt::Checked);
    }
}


