/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "breakpointwidget.h"

#include <QIcon>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>

#include <KLocalizedString>

#include "breakpointdetails.h"
#include "../breakpoint/breakpoint.h"
#include "../breakpoint/breakpointmodel.h"
#include <debug.h>
#include <interfaces/idebugcontroller.h>
#include <util/autoorientedsplitter.h>

#define IF_DEBUG(x)
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <util/placeholderitemproxymodel.h>

using namespace KDevelop;

class KDevelop::BreakpointWidgetPrivate
{
public:
    explicit BreakpointWidgetPrivate(IDebugController *controller)
        : debugController(controller)
    {
    }

    QTreeView* breakpointsView = nullptr;
    BreakpointDetails* details = nullptr;
    QMenu* popup = nullptr;
    bool firstShow = true;
    IDebugController* debugController;
    QAction* breakpointDisableAllAction = nullptr;
    QAction* breakpointEnableAllAction = nullptr;
    QAction* breakpointRemoveAll = nullptr;
    QAbstractProxyModel* proxyModel = nullptr;
};

BreakpointWidget::BreakpointWidget(IDebugController *controller, QWidget *parent)
    : AutoOrientedSplitter(parent)
    , d_ptr(new BreakpointWidgetPrivate(controller))
{
    Q_D(BreakpointWidget);

    setWindowTitle(i18nc("@title:window", "Debugger Breakpoints"));
    setWhatsThis(i18nc("@info:whatsthis", "Displays a list of breakpoints with "
                                          "their current status. Clicking on a "
                                          "breakpoint item allows you to change "
                                          "the breakpoint and will take you "
                                          "to the source in the editor window."));
    setWindowIcon( QIcon::fromTheme( QStringLiteral( "media-playback-pause"), windowIcon() ) );

    d->breakpointsView = new QTreeView(this);
    d->breakpointsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->breakpointsView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->breakpointsView->setRootIsDecorated(false);
    // PlaceholderItemProxyModel supports only flat models. Expanding the placeholder item
    // triggers an assertion failure in PlaceholderItemProxyModel::rowCount(). Forbid expanding
    // items to prevent it. None of the items has children, so expanding is useless anyway.
    d->breakpointsView->setItemsExpandable(false);

    // Enable uniform row height tree view optimizations. The checkbox in the EnableColumn makes each
    // non-placeholder row height noticeably greater than the placeholder row height. But the several-pixel
    // vertical space waste for the single placeholder row is well justified by less CPU time spent on calculating
    // row heights. The documentation for the QTreeView::uniformRowHeights property even spells out the
    // implementation details: "The height is obtained from the first item in the view. It is updated when the data
    // changes on that item." These documentation guarantees are perfect for breakpointsView: if there is only a
    // single placeholder item in the tree, its row height is used as the row height; when a non-placeholder item
    // is added, the data changes and the greater non-placeholder first row height becomes used as the row height.
    d->breakpointsView->setUniformRowHeights(true);

    auto detailsContainer = new QGroupBox(i18n("Breakpoint Details"), this);
    auto detailsLayout = new QVBoxLayout(detailsContainer);
    d->details = new BreakpointDetails(detailsContainer);
    detailsLayout->addWidget(d->details);

    setStretchFactor(0, 2);

    auto* proxyModel = new PlaceholderItemProxyModel(this);
    proxyModel->setSourceModel(d->debugController->breakpointModel());
    proxyModel->setColumnHint(Breakpoint::LocationColumn, i18n("New code breakpoint ..."));
    proxyModel->setColumnHint(Breakpoint::ConditionColumn, i18n("Enter condition ..."));
    d->breakpointsView->setModel(proxyModel);
    connect(proxyModel, &PlaceholderItemProxyModel::dataInserted, this, &BreakpointWidget::slotDataInserted);
    d->proxyModel = proxyModel;

    connect(d->breakpointsView, &QTreeView::activated, this, &BreakpointWidget::slotOpenFile);
    connect(d->breakpointsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BreakpointWidget::slotUpdateBreakpointDetail);
    connect(d->debugController->breakpointModel(), &BreakpointModel::rowsInserted, this, &BreakpointWidget::slotUpdateBreakpointDetail);
    connect(d->debugController->breakpointModel(), &BreakpointModel::rowsRemoved, this, &BreakpointWidget::slotUpdateBreakpointDetail);
    connect(d->debugController->breakpointModel(), &BreakpointModel::modelReset, this, &BreakpointWidget::slotUpdateBreakpointDetail);
    connect(d->debugController->breakpointModel(), &BreakpointModel::dataChanged, this, &BreakpointWidget::slotUpdateBreakpointDetail);


    connect(d->debugController->breakpointModel(),
            &BreakpointModel::hit,
            this, &BreakpointWidget::breakpointHit);

    setupPopupMenu();
}

BreakpointWidget::~BreakpointWidget() = default;

void BreakpointWidget::setupPopupMenu()
{
    Q_D(BreakpointWidget);

    d->popup = new QMenu(this);

    QMenu* newBreakpoint = d->popup->addMenu(i18nc("New breakpoint", "&New"));
    newBreakpoint->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));

    QAction* action = newBreakpoint->addAction(
        i18nc("Code breakpoint", "&Code"),
        this,
        SLOT(slotAddBlankBreakpoint()) );
    // Use this action also to provide a local shortcut
    action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B, QKeyCombination{Qt::Key_C}));
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

    QAction* breakpointDelete = d->popup->addAction(
        QIcon::fromTheme(QStringLiteral("edit-delete")),
        i18n( "&Delete" ),
        this,
        SLOT(slotRemoveBreakpoint()));
    breakpointDelete->setShortcut(Qt::Key_Delete);
    breakpointDelete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(breakpointDelete);


    d->popup->addSeparator();
    d->breakpointDisableAllAction = d->popup->addAction(i18n("Disable &All"), this, SLOT(slotDisableAllBreakpoints()));
    d->breakpointEnableAllAction = d->popup->addAction(i18n("&Enable All"), this, SLOT(slotEnableAllBreakpoints()));
    d->breakpointRemoveAll = d->popup->addAction(i18n("&Remove All"), this, SLOT(slotRemoveAllBreakpoints()));

    connect(d->popup, &QMenu::aboutToShow, this, &BreakpointWidget::slotPopupMenuAboutToShow);
}


void BreakpointWidget::contextMenuEvent(QContextMenuEvent* event)
{
    Q_D(BreakpointWidget);

    d->popup->popup(event->globalPos());
}

void BreakpointWidget::slotPopupMenuAboutToShow()
{
    Q_D(BreakpointWidget);

    if (d->debugController->breakpointModel()->rowCount() < 1) {
        d->breakpointDisableAllAction->setDisabled(true);
        d->breakpointEnableAllAction->setDisabled(true);
        d->breakpointRemoveAll->setDisabled(true);
    } else {
        d->breakpointRemoveAll->setEnabled(true);
        bool allDisabled = true;
        bool allEnabled = true;
        for (int i = 0; i < d->debugController->breakpointModel()->rowCount(); ++i) {
            Breakpoint* bp = d->debugController->breakpointModel()->breakpoint(i);
            if (bp->enabled())
                allDisabled = false;
            else
                allEnabled = false;
        }
        d->breakpointDisableAllAction->setDisabled(allDisabled);
        d->breakpointEnableAllAction->setDisabled(allEnabled);
    }

}

void BreakpointWidget::showEvent(QShowEvent *)
{
    Q_D(BreakpointWidget);

    if (d->firstShow && d->debugController->breakpointModel()->rowCount() > 0) {
        for (int i = 0; i < d->breakpointsView->model()->columnCount(); ++i) {
            if(i == Breakpoint::LocationColumn){
                continue;
            }
            d->breakpointsView->resizeColumnToContents(i);
        }
        //for some reasons sometimes width can be very small about 200... But it doesn't matter as we use tooltip anyway.
        int width = d->breakpointsView->size().width();

        QHeaderView* header = d->breakpointsView->header();
        header->resizeSection(Breakpoint::LocationColumn, width > 400 ? width/2 : header->sectionSize(Breakpoint::LocationColumn)*2 );
        d->firstShow = false;
    }
}

void BreakpointWidget::edit(KDevelop::Breakpoint *n)
{
    Q_D(BreakpointWidget);

    QModelIndex index = d->proxyModel->mapFromSource(d->debugController->breakpointModel()->breakpointIndex(n, Breakpoint::LocationColumn));
    d->breakpointsView->setCurrentIndex(index);
    d->breakpointsView->edit(index);
}

void BreakpointWidget::slotDataInserted(int column, const QVariant& value)
{
    Q_D(BreakpointWidget);

    Breakpoint* breakpoint = d->debugController->breakpointModel()->addCodeBreakpoint();
    breakpoint->setData(column, value);
}

void BreakpointWidget::slotAddBlankBreakpoint()
{
    Q_D(BreakpointWidget);

    edit(d->debugController->breakpointModel()->addCodeBreakpoint());
}

void BreakpointWidget::slotAddBlankWatchpoint()
{
    Q_D(BreakpointWidget);

    edit(d->debugController->breakpointModel()->addWatchpoint());
}

void BreakpointWidget::slotAddBlankReadWatchpoint()
{
    Q_D(BreakpointWidget);

    edit(d->debugController->breakpointModel()->addReadWatchpoint());
}


void KDevelop::BreakpointWidget::slotAddBlankAccessWatchpoint()
{
    Q_D(BreakpointWidget);

    edit(d->debugController->breakpointModel()->addAccessWatchpoint());
}


void BreakpointWidget::slotRemoveBreakpoint()
{
    Q_D(BreakpointWidget);

    QItemSelectionModel* sel = d->breakpointsView->selectionModel();
    QModelIndexList selected = sel->selectedIndexes();
    IF_DEBUG( qCDebug(DEBUGGER) << selected; )
    if (!selected.isEmpty()) {
        d->debugController->breakpointModel()->removeRow(selected.first().row());
    }
}

void BreakpointWidget::slotRemoveAllBreakpoints()
{
    Q_D(BreakpointWidget);

    d->debugController->breakpointModel()->removeRows(0, d->debugController->breakpointModel()->rowCount());
}


void BreakpointWidget::slotUpdateBreakpointDetail()
{
    Q_D(BreakpointWidget);

    showEvent(nullptr);
    QModelIndexList selected = d->breakpointsView->selectionModel()->selectedIndexes();
    IF_DEBUG( qCDebug(DEBUGGER) << selected; )
    if (selected.isEmpty()) {
        d->details->setItem(nullptr);
    } else {
        d->details->setItem(d->debugController->breakpointModel()->breakpoint(selected.first().row()));
    }
}

void BreakpointWidget::breakpointHit(int row)
{
    Q_D(BreakpointWidget);

    const QModelIndex index = d->proxyModel->mapFromSource(d->debugController->breakpointModel()->index(row, 0));
    d->breakpointsView->selectionModel()->select(
        index,
        QItemSelectionModel::Rows
        | QItemSelectionModel::ClearAndSelect);
}

void BreakpointWidget::slotOpenFile(const QModelIndex& breakpointIdx)
{
    Q_D(BreakpointWidget);

    if (breakpointIdx.column() != Breakpoint::LocationColumn){
        return;
    }
    Breakpoint* bp = d->debugController->breakpointModel()->breakpoint(breakpointIdx.row());
    if (!bp || bp->line() == -1 || bp->url().isEmpty() ){
        return;
    }

    ICore::self()->documentController()->openDocument(bp->url(), KTextEditor::Cursor(bp->line(), 0), IDocumentController::DoNotFocus);
}

void BreakpointWidget::slotDisableAllBreakpoints()
{
    Q_D(BreakpointWidget);

    for (int i = 0; i < d->debugController->breakpointModel()->rowCount(); ++i) {
        Breakpoint* bp = d->debugController->breakpointModel()->breakpoint(i);
        bp->setData(Breakpoint::EnableColumn, Qt::Unchecked);
    }
}

void BreakpointWidget::slotEnableAllBreakpoints()
{
    Q_D(BreakpointWidget);

    for (int i = 0; i < d->debugController->breakpointModel()->rowCount(); ++i) {
        Breakpoint* bp = d->debugController->breakpointModel()->breakpoint(i);
        bp->setData(Breakpoint::EnableColumn, Qt::Checked);
    }
}

#include "moc_breakpointwidget.cpp"
