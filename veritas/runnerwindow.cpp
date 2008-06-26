/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*!
 * \file  runnerwindow.cpp
 *
 * \brief Implements class RunnerWindow.
 */

#include "runnerwindow.h"
#include "runnerviewcontroller.h"
#include "resultsviewcontroller.h"
#include "runnermodel.h"
#include "resultsmodel.h"
#include "runnerproxymodel.h"
#include "resultsproxymodel.h"
#include "statuswidget.h"
#include "stoppingdialog.h"
#include "utils.h"
#include "selectionmanager.h"

#include <QMessageBox>
#include <QCloseEvent>

// Helper function needed to expand Q_INIT_RESOURCE outside the namespace.
static void initVeritasResource()
{
    Q_INIT_RESOURCE(qxrunner);
}

namespace Veritas
{

RunnerWindow::RunnerWindow(QWidget* parent, Qt::WFlags flags)
        : QMainWindow(parent, flags)
{
    initVeritasResource();

    m_ui.setupUi(this);

    // Controllers for the tree views, it's best to create them at the
    // very beginning.
    m_runnerViewController = new RunnerViewController(this, runnerView());
    m_resultsViewController = new ResultsViewController(this, resultsView());

    // Adjust GUI.
    m_ui.actionMinimalUpdate->setCheckable(true);
    //resultsView()->header()->setResizeMode(QHeaderView::Stretch);

    // Replace results menu item which serves as a placeholder
    // for the action from the dock widget.
    QAction* actionResults = m_ui.dockResults->toggleViewAction();
    actionResults->setText(m_ui.actionResults->text());

    //m_ui.menuView->insertAction(m_ui.actionResults, actionResults);
    //m_ui.menuView->removeAction(m_ui.actionResults);

    qSwap(m_ui.actionResults, actionResults);
    delete actionResults;

    connect(m_ui.actionResults, SIGNAL(toggled(bool)), SLOT(showResults(bool)));

    // Fine tuning of the focus rect handling because there should
    // always be a focus rect visible in the views.
    connect(runnerView(),  SIGNAL(clicked(const QModelIndex&)),
            SLOT(ensureFocusRect(const QModelIndex&)));
    connect(resultsView(), SIGNAL(clicked(const QModelIndex&)),
            SLOT(ensureFocusRect(const QModelIndex&)));

    // To keep the views synchronized when there are highlighted rows
    // which get clicked again..
    connect(runnerView(),  SIGNAL(pressed(const QModelIndex&)),
            SLOT(scrollToHighlightedRows()));
    connect(resultsView(), SIGNAL(pressed(const QModelIndex&)),
            SLOT(scrollToHighlightedRows()));

    // Set-up statusbar.
    m_statusWidget = new StatusWidget(0);
    statusBar()->addPermanentWidget(m_statusWidget, 1);
    statusWidget()->progressRun->hide();

    // Disable user interaction while there is no data.
    enableItemActions(false);
    enableResultsFilter(false);

    // Initial results.
    displayNumInfos(0);
    displayNumWarnings(0);
    displayNumErrors(0);
    displayNumFatals(0);
    displayNumExceptions(0);

    // Results filter commands.
    connect(m_ui.buttonInfos,    SIGNAL(toggled(bool)),
            SLOT(setResultsFilter()));
    connect(m_ui.buttonWarnings, SIGNAL(toggled(bool)),
            SLOT(setResultsFilter()));
    connect(m_ui.buttonErrors,   SIGNAL(toggled(bool)),
            SLOT(setResultsFilter()));
    connect(m_ui.buttonFatals,   SIGNAL(toggled(bool)),
            SLOT(setResultsFilter()));

    // File commands.
    connect(m_ui.actionExit, SIGNAL(triggered(bool)), SLOT(close()));

    // Item commands.
    m_ui.actionStart->setShortcut(QKeySequence(tr("Ctrl+R")));
    connect(m_ui.actionStart, SIGNAL(triggered(bool)), SLOT(runItems()));

    m_ui.actionStop->setShortcut(QKeySequence(tr("Ctrl+K")));
    connect(m_ui.actionStop, SIGNAL(triggered(bool)), SLOT(stopItems()));

    // View commands
    m_ui.actionSelectAll->setShortcut(QKeySequence(tr("Ctrl+A")));
    connect(m_ui.actionSelectAll, SIGNAL(triggered(bool)),
            runnerController(),
            SLOT(selectAll()));

    m_ui.actionUnselectAll->setShortcut(QKeySequence(tr("Ctrl+U")));
    connect(m_ui.actionUnselectAll, SIGNAL(triggered(bool)),
            runnerController(),
            SLOT(unselectAll()));

    m_ui.actionExpandAll->setShortcut(QKeySequence(tr("Ctrl++")));
    connect(m_ui.actionExpandAll, SIGNAL(triggered(bool)),
            runnerController(),
            SLOT(expandAll()));

    m_ui.actionCollapseAll->setShortcut(QKeySequence(tr("Ctrl+-")));
    connect(m_ui.actionCollapseAll, SIGNAL(triggered(bool)),
            runnerController(),
            SLOT(collapseAll()));

    // Set initially available.
    m_sema.release();

    runnerView()->setMouseTracking(true);
    m_selection = new SelectionManager(runnerView());
}

RunnerWindow::~RunnerWindow()
{
    // Deleting the model is left to the owner of the model instance.
}

void RunnerWindow::show()
{
    setUpdatesEnabled(false);   // Reduce flickering

    QMainWindow::show();

    // Workaround: The results tree view has a maximum height defined in
    // Designer which now is reset. Without the height restriction the
    // dock window would occupy half the main window after construcution
    // instead of sitting "nicely" at the bottom.
    // This is only for the situation when the main window geometry isn't
    // restored from the settings.
    resultsView()->setMaximumSize(QSize(16777215, 16777215));

    setUpdatesEnabled(true);

    // Change the results tree view to manual adjustment after the columns
    // have been stretched over the tree view at initial display.
    // This is only for the situation when the column sizes aren't restored
    // from the settings.
    resultsView()->header()->setResizeMode(QHeaderView::Interactive);

    // Some menus depend on current state.
    adjustMenus();

    QApplication::setActiveWindow(this);
}

void RunnerWindow::setModel(RunnerModel* model)
{
    // It's not expected to set another model at the moment but if done so then
    // at least remove the previous model from the views to prevent the runner
    // from crashing. Deleting a previous model is left to the owner of the model
    // instance. The fact that a previous model could have a running thread is
    // taken into account by simply asking the model to stop it without further
    // verification. A better solution must consider this in a later version.
    RunnerModel* prevModel = runnerModel();

    if (prevModel) {
        // Hope it stops the thread if it is running.
        prevModel->stopItems();

        // As long as the proxy models can't be set from outside they
        // get deleted.
        RunnerProxyModel* m1 = runnerProxyModel();
        ResultsProxyModel* m2 = resultsProxyModel();

        runnerView()->setModel(0);
        resultsView()->setModel(0);
        runnerView()->reset();
        resultsView()->reset();

        delete m1;
        delete m2;

        prevModel->disconnect();
    }

    // Set initial title.
    QStringList tokens = windowTitle().split(" ");
    setWindowTitle(tokens[0].trimmed());

    // No user interaction without a model or a model without columns.
    bool valid = true;

    if (!model) {
        valid = false;
    } else if (model->columnCount() < 1) {
        valid = false;
    }

    if (!valid) {
        enableItemActions(false);
        enableResultsFilter(false);
        return;
    }

    // Set title with model name.
    if (!model->name().trimmed().isEmpty()) {
        setWindowTitle(windowTitle() + " - " + model->name());
    }

    // Hide filter buttons for results that are not expected.
    int results = model->expectedResults();

    bool expected;

/*    expected = results & Veritas::RunInfo;
    m_ui.buttonInfos->setVisible(expected);
    expected = results & Veritas::RunWarning;
    m_ui.buttonWarnings->setVisible(expected);*/
    expected = results & Veritas::RunError;
    m_ui.buttonErrors->setVisible(expected);
    expected = results & Veritas::RunFatal;
    m_ui.buttonFatals->setVisible(expected);

    // Proxy models for the views with the source model as their parent
    // to have the proxies deleted when the model gets deleted.
    RunnerProxyModel* runnerProxyModel = new RunnerProxyModel(model);
    runnerProxyModel->setSourceModel(model);

    runnerView()->setModel(runnerProxyModel);

    // Results model is contained in the runner model.
    ResultsModel* resultsModel = model->resultsModel();
    ResultsProxyModel* resultsProxyModel = new ResultsProxyModel(model);
    resultsProxyModel->setSourceModel(resultsModel);

    resultsView()->setModel(resultsProxyModel);

    // Filter actions enabled, item actions only when there is data.
    enableResultsFilter(true);

    // Get the relevant column count.
    int columnCount = model->columnCount();

    // Set the defaults for enabled and thus visible columns.
    QBitArray enabledRunnerColumns(columnCount);
    QBitArray enabledResultsColumns(columnCount, true);

    for (int i = 0; i < 2; i++) {
        if (i < columnCount) {
            runnerView()->showColumn(i);
            enabledRunnerColumns[i] = true;
        }
    }

    for (int i = 1; i < columnCount; i++) {
        // hide all runner columns
        runnerView()->hideColumn(i);
    }


    for (int i = 0; i < columnCount; i++) {
        resultsView()->showColumn(i);
    }
    enabledResultsColumns[1] = 0;
    resultsView()->hideColumn(1);

    // Set the defaults in the proxy models.
    runnerProxyModel->setEnabledColumns(enabledRunnerColumns);
    resultsProxyModel->setEnabledColumns(enabledResultsColumns);

    // Suppress results data display if view isn't shown.
    showResults(isResultsViewVisible());

    // Very limited user interaction without data.
    if (model->rowCount() < 1) {
        enableItemActions(false);
        m_ui.actionColumns->setEnabled(true);
        m_ui.actionSettings->setEnabled(true);
        return;
    }

    // Item statistics.
    connect(model, SIGNAL(numTotalChanged(int)),
            SLOT(displayNumTotal(int)));
    connect(model, SIGNAL(numSelectedChanged(int)),
            SLOT(displayNumSelected(int)));
    connect(model, SIGNAL(numSuccessChanged(int)),
            SLOT(displayNumSuccess(int)));
    connect(model, SIGNAL(numInfosChanged(int)),
            SLOT(displayNumInfos(int)));
    connect(model, SIGNAL(numWarningsChanged(int)),
            SLOT(displayNumWarnings(int)));
    connect(model, SIGNAL(numErrorsChanged(int)),
            SLOT(displayNumErrors(int)));
    connect(model, SIGNAL(numFatalsChanged(int)),
            SLOT(displayNumFatals(int)));
    connect(model, SIGNAL(numExceptionsChanged(int)),
            SLOT(displayNumExceptions(int)));

    // This will fire the signals connected above.
    model->countItems();

    // Expand the branches, do it in the brackground to reduce flickering.
    runnerView()->setUpdatesEnabled(false);

    runnerController()->expandAll();
    for (int i = 0; i < columnCount; i++) {
        runnerView()->resizeColumnToContents(i);
    }

    runnerView()->setUpdatesEnabled(true);

    // How much data is wanted when running the items.
    connect(m_ui.actionMinimalUpdate, SIGNAL(triggered(bool)),
            model,
            SLOT(setMinimalUpdate(bool)));

    model->setMinimalUpdate(isMinimalUpdate());

    // Get notified of items run.
    connect(model, SIGNAL(numStartedChanged(int)),
            SLOT(displayProgress(int)));
    connect(model, SIGNAL(itemStarted(const QModelIndex&)),
            SLOT(highlightRunningItem(const QModelIndex&)));
    connect(model, SIGNAL(numCompletedChanged(int)),
            SLOT(displayNumCompleted(int)));
    connect(model, SIGNAL(allItemsCompleted()),
            SLOT(displayCompleted()));
    connect(model, SIGNAL(itemCompleted(const QModelIndex&)),
            resultsModel,
            SLOT(addResult(const QModelIndex&)));

    enableItemActions(true);
    m_ui.actionStop->setDisabled(true);

    // Set the filter in the results model.
    setResultsFilter();

    // Start with top row highlighted.
    QModelIndex index = runnerView()->indexBelow(runnerView()->rootIndex());
    runnerView()->setCurrentIndex(index);
}

QTreeView* RunnerWindow::runnerView() const
{
    return m_ui.treeRunner;
}

QTreeView* RunnerWindow::resultsView() const
{
    return m_ui.treeResults;
}

RunnerModel* RunnerWindow::runnerModel() const
{
    return runnerController()->runnerModel();
}

RunnerProxyModel* RunnerWindow::runnerProxyModel() const
{
    return runnerController()->runnerProxyModel();
}

ResultsModel* RunnerWindow::resultsModel() const
{
    // The results model is contained in the runner model
    // (but could also be retrieved from the results view controller).
    return runnerModel()->resultsModel();
}

ResultsProxyModel* RunnerWindow::resultsProxyModel() const
{
    return resultsController()->resultsProxyModel();
}

void RunnerWindow::displayProgress(int numItems) const
{
    // Display only when there are selected items
    if (statusWidget()->progressRun->maximum() > 0) {
        statusWidget()->progressRun->setValue(numItems);
        statusWidget()->progressRun->show();
    }
}

void RunnerWindow::displayCompleted() const
{
    statusWidget()->progressRun->hide();
    enableControlsAfterRunning();
}

bool RunnerWindow::isResultsViewVisible() const
{
    return m_ui.actionResults->isChecked();
}

void RunnerWindow::displayNumTotal(int numItems) const
{
    statusWidget()->labelNumTotal->setText(QString().setNum(numItems));
}

void RunnerWindow::displayNumSelected(int numItems) const
{
    statusWidget()->labelNumSelected->setText(QString().setNum(numItems));

    // During item selection the progress bar shouldn't be visible.
    statusWidget()->progressRun->hide();
    statusWidget()->progressRun->setMaximum(numItems);
}

void RunnerWindow::displayNumCompleted(int numItems) const
{
    statusWidget()->labelNumRun->setText(QString().setNum(numItems));
}

void RunnerWindow::displayNumSuccess(int numItems) const
{
    displayStatusNum(statusWidget()->labelNumSuccess,
                     statusWidget()->labelNumSuccess, numItems);

    // Num success always visible.
    statusWidget()->labelNumSuccess->show();
}

void RunnerWindow::displayNumInfos(int numItems) const
{
    if (numItems != 1) {
        m_ui.buttonInfos->setText(QString().setNum(numItems) +
                                tr(" Infos"));
    } else {
        m_ui.buttonInfos->setText(tr("1 Info"));
    }

    displayStatusNum(statusWidget()->labelNumInfos,
                     statusWidget()->labelNumInfosPic, numItems);
}

void RunnerWindow::displayNumWarnings(int numItems) const
{
    if (numItems != 1) {
        m_ui.buttonWarnings->setText(QString().setNum(numItems) +
                                   tr(" Warnings"));
    } else {
        m_ui.buttonWarnings->setText(tr("1 Warning"));
    }

    displayStatusNum(statusWidget()->labelNumWarnings,
                     statusWidget()->labelNumWarningsPic, numItems);
}

void RunnerWindow::displayNumErrors(int numItems) const
{
    if (numItems != 1) {
        m_ui.buttonErrors->setText(QString().setNum(numItems) +
                                 tr(" Errors"));
    } else {
        m_ui.buttonErrors->setText(tr("1 Error"));
    }

    displayStatusNum(statusWidget()->labelNumErrors,
                     statusWidget()->labelNumErrorsPic, numItems);
}

void RunnerWindow::displayNumFatals(int numItems) const
{
    if (numItems != 1) {
        m_ui.buttonFatals->setText(QString().setNum(numItems) +
                                 tr(" Fatals"));
    } else {
        m_ui.buttonFatals->setText(tr("1 Fatal"));
    }

    displayStatusNum(statusWidget()->labelNumFatals,
                     statusWidget()->labelNumFatalsPic, numItems);
}

void RunnerWindow::displayNumExceptions(int numItems) const
{

    displayStatusNum(statusWidget()->labelNumExceptions,
                     statusWidget()->labelNumExceptionsPic, numItems);
}

void RunnerWindow::highlightRunningItem(const QModelIndex& testItemIndex) const
{
    if (isMinimalUpdate()) {
        return;
    }

    // Determine runner model proxy index and highlight related row.
    QModelIndex index;
    index = Utils::proxyIndexFromModel(runnerProxyModel(), testItemIndex);
    runnerController()->setHighlightedRow(index);
}

void RunnerWindow::setResultsFilter() const
{
    // Remember currently highlighted result.
    QModelIndex resultIndex;
    QModelIndexList indexes;
    indexes = resultsView()->selectionModel()->selectedIndexes();

    if (indexes.count() > 0) {
        resultIndex = Utils::modelIndexFromProxy(resultsProxyModel(),
                      indexes.first());
    }

    // Determine filter settings.
    int filter = 0;

//     if (m_ui.buttonInfos->isEnabled() && m_ui.buttonInfos->isChecked()) {
//         filter = Veritas::RunInfo;
//     }
// 
//     if (m_ui.buttonWarnings->isEnabled() && m_ui.buttonWarnings->isChecked()) {
//         filter = filter | Veritas::RunWarning;
//     }

    if (m_ui.buttonErrors->isEnabled() && m_ui.buttonErrors->isChecked()) {
        filter = filter | Veritas::RunError;
    }

    if (m_ui.buttonFatals->isEnabled() && m_ui.buttonFatals->isChecked()) {
        filter = filter | Veritas::RunFatal;
    }

    // Setting the filter updates the view.
    resultsProxyModel()->setFilter(filter);

    // When no result was highlighted before than try to highlight the
    // one that corresponds to the currently highlighted runner item.
    if (!resultIndex.isValid()) {
        syncResultWithTest(runnerView()->selectionModel()->selection(),
                                 runnerView()->selectionModel()->selection());
        return;
    }

    // At least there should be a current but not necessarily highlighted result
    // in order to see the focus rect when the results view gets the focus.
    ensureCurrentResult();

    // Try to highlight same result as was highlighted before.
    QModelIndex currentIndex;
    currentIndex = Utils::proxyIndexFromModel(resultsProxyModel(), resultIndex);

    if (!currentIndex.isValid()) {
        // Previously highlighted result is filtered out now.
        return;
    }

    // Highlight result without affecting the runner view.
    enableResultSync(false);
    resultsView()->setCurrentIndex(currentIndex);
    enableResultSync(true);

    // Make the row in every tree view visible and expand corresponding parents.
    scrollToHighlightedRows();
}

void RunnerWindow::syncResultWithTest(const QItemSelection& selected,
        const QItemSelection& deselected) const
{
    Q_UNUSED(deselected);
    // Do nothing when there are no results or no runner item is selected.
    if (!resultsView()->indexBelow(resultsView()->rootIndex()).isValid()) {
        return;
    }

    QModelIndexList indexes = selected.indexes();

    if (indexes.count() < 1 ) {
        return;
    }

    // Prevent from circular dependencies.
    enableResultSync(false);

    // Try to highlight the corresponding result.
    resultsView()->clearSelection();

    // Get the results model index that corresponds to the runner item index.
    QModelIndex testItemIndex;
    testItemIndex = Utils::modelIndexFromProxy(runnerProxyModel(), indexes.first());
    QModelIndex resultIndex = resultsModel()->mapFromTestIndex(testItemIndex);

    // At least there should be a current but not necessarily highlighted result
    // in order to see the focus rect when the results view gets the focus.
    ensureCurrentResult();

    // If not found then there is no result for this runner item.
    if (!resultIndex.isValid()) {
        // Enable selection handler again.
        enableResultSync(true);
        return;
    }

    // Prepare row highlighting.
    QModelIndex currentIndex = Utils::proxyIndexFromModel(resultsProxyModel(),
                               resultIndex);

    // When results proxy model index not exists it is filtered out.
    if (!currentIndex.isValid()) {
        // Enable selection handler again.
        enableResultSync(true);
        return;
    }

    // Highlight corresponding result now.
    resultsView()->setCurrentIndex(currentIndex);

    // Make the row in every tree view visible and expand corresponding parents.
    scrollToHighlightedRows();

    // Enable selection handler again.
    enableResultSync(true);
}

void RunnerWindow::syncTestWithResult(const QItemSelection& selected,
        const QItemSelection& deselected) const
{
    Q_UNUSED(deselected);
    // Do nothing when no result is selected.
    QModelIndexList indexes = selected.indexes();

    if (indexes.count() < 1 ) {
        return;
    }

    // Determine the results model index.
    QModelIndex resultIndex;
    resultIndex = Utils::modelIndexFromProxy(resultsProxyModel(), indexes.first());

    // Get the corresponding runner item index contained in the results model.
    QModelIndex testItemIndex;
    testItemIndex = resultsModel()->mapToTestIndex(resultIndex);

    // Prevent from circular dependencies.
    enableTestSync(false);

    // Determine the proxy model index and highlight it.
    QModelIndex currentIndex;
    currentIndex = Utils::proxyIndexFromModel(runnerProxyModel(), testItemIndex);
    runnerView()->setCurrentIndex(currentIndex);

    // Make the row in every tree view visible and expand corresponding parents.
    scrollToHighlightedRows();

    // Enable selection handler again.
    enableTestSync(true);
}

void RunnerWindow::ensureFocusRect(const QModelIndex&  index)
{
    QTreeView* treeView;

    if (runnerView()->hasFocus()) {
        treeView = runnerView();
    } else {
        treeView = resultsView();
    }

    // No relevance when selections not allowed.
    if (treeView->selectionMode() == QAbstractItemView::NoSelection) {
        return;
    }

    // Focus rect is there when column entry not empty.
    QString data = index.data().toString();

    if (!data.trimmed().isEmpty()) {
        return;
    }

    // No relevance when column 0 is empty.
    if (index.column() == 0) {
        return;
    }

    // Tree views are already synchronized.
    enableTestSync(false);
    enableResultSync(false);

    // This ensures that there is a focus rect.
    treeView->clearSelection();

    QItemSelectionModel* selectionModel = treeView->selectionModel();
    selectionModel->setCurrentIndex(index.sibling(index.row(), 0),
                                    QItemSelectionModel::Select |
                                    QItemSelectionModel::Rows);

    // Enable selection handler again.
    enableTestSync(true);
    enableResultSync(true);
}

void RunnerWindow::scrollToHighlightedRows() const
{
    // No relevance when selections not allowed.
    if (runnerView()->selectionMode() == QAbstractItemView::NoSelection ||
            resultsView()->selectionMode() == QAbstractItemView::NoSelection) {
        return;
    }

    // Note: It's important not to use the current index but work with the
    // selection instead due to the fact that these indexes might not be the same.

    QModelIndex index;
    QModelIndexList indexes;

    indexes = runnerView()->selectionModel()->selectedIndexes();

    if (indexes.count() > 0) {
        index = indexes.first();
    }

    if (index.isValid()) {
        runnerView()->scrollTo(index);
    }

    index = QModelIndex();
    indexes = resultsView()->selectionModel()->selectedIndexes();

    if (indexes.count() > 0) {
        index = indexes.first();
    }

    if (index.isValid()) {
        resultsView()->scrollTo(index);
    } else {
        // Try to highlight a result.
        syncResultWithTest(runnerView()->selectionModel()->selection(),
                                 runnerView()->selectionModel()->selection());
    }
}

void RunnerWindow::runItems()
{
    // Do not interfere with stopping the items. Could happen because Qt
    // input processing could be faster than executing event handlers.
    if (!m_sema.tryAcquire()) {
        return;
    }

    disableControlsBeforeRunning();
    runnerModel()->runItems();
    m_sema.release();
}

void RunnerWindow::stopItems()
{
    // Do not interfere with running the items. Could happen because Qt
    // input processing could be faster than executing event handlers.
//     if (!m_sema.tryAcquire()) {
//         return;
//     }

    m_ui.actionStop->setDisabled(true);
    QCoreApplication::processEvents();  // Disable command immediately

    // Stopping is done in a dialog which shows itself only when
    // it takes several attempts to succeed (if ever).
    StoppingDialog dlg(this, runnerModel());

    int r = dlg.exec();

    if (r == QDialog::Accepted) {
        enableControlsAfterRunning();
        m_sema.release();
        return;
    }

    // Give a chance for another stop request.
    m_ui.actionStop->setEnabled(true);
    m_sema.release();
}

void RunnerWindow::showResults(bool show)
{
    if (!runnerModel()) {
        return;
    }

    resultsProxyModel()->setActive(show);

    // An invisible results view means that the dock widget was closed
    // and is shown now. In this case the data is retrieved again.
    bool visible = m_ui.treeResults->isVisible();

    if (show && !visible) {
        // Show data according to current filtering.
        resultsProxyModel()->clear();
        setResultsFilter();

        // Make sure that highlighted row sync works as expected.
        m_ui.dockResults->show();
    }

    if (!show || runnerModel()->isRunning()) {
        return;
    }
    // Let the dock widget get its position and size before
    // synchronizing the highlighted rows display.
    QCoreApplication::processEvents();
    scrollToHighlightedRows();
}

void RunnerWindow::disableControlsBeforeRunning()
{
    enableItemActions(false);
    resultsController()->enableSorting(false);

    m_ui.actionStop->setEnabled(true);
    runnerView()->setCursor(QCursor(Qt::BusyCursor));
    runnerView()->setFocus();
    runnerView()->setSelectionMode(QAbstractItemView::NoSelection);
    resultsView()->setSelectionMode(QAbstractItemView::NoSelection);
    enableTestSync(false);
    enableResultSync(false);

    // Change color for highlighted rows to orange. If a similar color is
    // defined for the background then green is used. Determining a color
    // could be more sophisticated but must suffice for now.
    QPalette palette(runnerView()->palette());

    // Save current highlighting color for restoring it later.
    m_highlightBrush = palette.brush(QPalette::Active, QPalette::Highlight);

    // Create kind of orange ('pure' orange is QColor(255, 165, 0, 255)).
    QBrush orange(QColor(255, 153, 51, 255));

    QBrush newBrush;

    // Look at RGB values of background (base).
    QBrush baseBrush = palette.brush(QPalette::Active, QPalette::Base);

    bool rOk = (baseBrush.color().red() == 255) || (baseBrush.color().red() < 205);
    bool gOk = (baseBrush.color().green() > orange.color().green() + 50) ||
               (baseBrush.color().green() < orange.color().green() - 50);
    bool bOk = (baseBrush.color().blue() > orange.color().blue() + 50) ||
               (baseBrush.color().blue() < orange.color().blue() - 50);

    if (rOk && gOk && bOk) {
        newBrush = orange;
    } else {
        newBrush = QBrush(QColor(Qt::green));
    }

    palette.setBrush(QPalette::Active, QPalette::Highlight, newBrush);
    runnerView()->setPalette(palette);
}

void RunnerWindow::enableControlsAfterRunning() const
{
    // Wait until all items stopped.
    while (runnerModel()->isRunning(100)) {
        // Prevent GUI from freezing.
        QCoreApplication::processEvents();
    }

    // Show results now if minimal update was active.
    if (isMinimalUpdate()) {
        runnerModel()->emitItemResults();
    }

    // Give the GUI a chance to update.
    QCoreApplication::processEvents();

    // Enable user interaction.
    enableItemActions(true);

    m_ui.actionStop->setDisabled(true);
    runnerView()->setCursor(QCursor());
    runnerView()->setFocus();
    runnerView()->setSelectionMode(QAbstractItemView::SingleSelection);
    resultsView()->setSelectionMode(QAbstractItemView::SingleSelection);
    enableTestSync(true);
    enableResultSync(true);
    ensureCurrentResult();

    // Reset color for highlighted rows.
    QPalette palette(runnerView()->palette());
    palette.setBrush(QPalette::Active, QPalette::Highlight, m_highlightBrush);
    runnerView()->setPalette(palette);

    if (!isMinimalUpdate()) {
        return;
    }

    // Scroll to the last processed item when in minimal update mode.
    // Determine the results model index first.
    int row = resultsModel()->rowCount() - 1;

    if (row < 1) {
        return;
    }

    QModelIndex resultIndex = resultsModel()->index(row, 0);
    QModelIndex testItemIndex;
    testItemIndex = resultsModel()->mapToTestIndex(resultIndex);
    QModelIndex currentIndex = Utils::proxyIndexFromModel(runnerProxyModel(),
                               testItemIndex);

    // Suppress synchronization of results view.
    enableTestSync(false);

    // Highlight row of runner item.
    runnerController()->setHighlightedRow(currentIndex);

    // Enable selection handler again.
    enableTestSync(true);
}

void RunnerWindow::enableItemActions(bool enable) const
{
    m_ui.actionStart->setEnabled(enable);
    m_ui.actionStop->setEnabled(enable);
    m_ui.actionSelectAll->setEnabled(enable);
    m_ui.actionUnselectAll->setEnabled(enable);
    m_ui.actionExpandAll->setEnabled(enable);
    m_ui.actionCollapseAll->setEnabled(enable);
    m_ui.actionMinimalUpdate->setEnabled(enable);
    m_ui.actionColumns->setEnabled(enable);
    m_ui.actionSettings->setEnabled(enable);
}

void RunnerWindow::enableResultsFilter(bool enable) const
{
    m_ui.buttonInfos->setEnabled(enable);
    m_ui.buttonWarnings->setEnabled(enable);
    m_ui.buttonErrors->setEnabled(enable);
    m_ui.buttonFatals->setEnabled(enable);
}

void RunnerWindow::enableTestSync(bool enable) const
{
    if (enable) {
        connect(runnerView()->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                SLOT(syncResultWithTest(const QItemSelection&, const QItemSelection&)));
    } else {
        disconnect(runnerView()->selectionModel(),
                   SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                   this,
                   SLOT(syncResultWithTest(const QItemSelection&, const QItemSelection&)));
    }
}

void RunnerWindow::enableResultSync(bool enable) const
{
    if (enable) {
        connect(resultsView()->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                SLOT(syncTestWithResult(const QItemSelection&, const QItemSelection&)));
    } else {
        disconnect(resultsView()->selectionModel(),
                   SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                   this,
                   SLOT(syncTestWithResult(const QItemSelection&, const QItemSelection&)));
    }
}

void RunnerWindow::ensureCurrentResult() const
{
    // Do nothing if there is a current index or no data at all.
    if (resultsView()->currentIndex().isValid()) {
        return;
    }

    // Try to make first visible index the current one.
    QModelIndex currentIndex = resultsView()->indexAt(QPoint(1, 1));

    if (!currentIndex.isValid()) {
        return;
    }

    // Set current index without highlighting.
    resultsView()->selectionModel()->setCurrentIndex(currentIndex,
            QItemSelectionModel::NoUpdate);
}

void RunnerWindow::adjustMenus() const
{
    bool haveChildren = false;

    // Look for an item with children.
    QModelIndex index;
    index = runnerView()->indexBelow(runnerView()->rootIndex());

    while (index.isValid()) {
        if (index.child(0, 0).isValid()) {
            haveChildren = true;
            break;
        }

        index = index.sibling(index.row() + 1, 0);
    }

    /*QList<QAction*> viewActions = m_ui.menuView->actions();
      for (int i = 3; i < 6; i++) {
            viewActions[i]->setVisible(haveChildren);
        }*/
}

void RunnerWindow::displayStatusNum(QLabel* labelForText,
                                    QLabel* labelForPic, int numItems) const
{
    labelForText->setText(": " + QString().setNum(numItems));

    bool visible;

    if (numItems > 0) {
        visible = true;
    } else {
        visible =false;
    }

    labelForText->setVisible(visible);
    labelForPic->setVisible(visible);
}

bool RunnerWindow::isMinimalUpdate() const
{
    return m_ui.actionMinimalUpdate->isChecked();
}

Ui::StatusWidget* RunnerWindow::statusWidget() const
{
    return &(m_statusWidget->ui);
}

RunnerViewController* RunnerWindow::runnerController() const
{
    return m_runnerViewController;
}

ResultsViewController* RunnerWindow::resultsController() const
{
    return m_resultsViewController;
}

void RunnerWindow::closeEvent(QCloseEvent* event)
{
    if (!runnerModel()) {
        return;
    }

    // Stopping is done in a dialog which shows itself only when
    // it takes several attempts to succeed (if ever).
    StoppingDialog dlg(this, runnerModel());

    int r = dlg.exec();

    if (r == QDialog::Accepted) {
        return;
    }

    // Items not stoppable.
    QString msg;
    msg = tr("There are items running which can not be stopped immediately.\n"
             "Should the program exit anyway which could result in inconsistent data?");

    r = QMessageBox::warning(this, tr("Veritas"), msg,
                             tr("&Yes"), tr("&No"), 0, 0, 1);
    if (r) {
        event->ignore();
        return;
    }

    // Exit the hard way.
    QApplication::quit();
    exit(1);
}

} // namespace
