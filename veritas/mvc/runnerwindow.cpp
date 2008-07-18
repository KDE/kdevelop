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

#include "veritas/mvc/runnerwindow.h"

#include "ui_runnerwindow.h"

#include "interfaces/iproject.h"

#include "veritas/mvc/resultsmodel.h"
#include "veritas/mvc/resultsproxymodel.h"
#include "veritas/mvc/resultsviewcontroller.h"
#include "veritas/mvc/runnermodel.h"
#include "veritas/mvc/runnerproxymodel.h"
#include "veritas/mvc/runnerviewcontroller.h"
#include "veritas/mvc/selectionmanager.h"
#include "veritas/mvc/stoppingdialog.h"

#include "veritas/utils.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <KLocale>
#include <KAction>
#include <KActionMenu>
#include <KSelectAction>
#include <KIcon>
#include <KDebug>

using KDevelop::IProject;
Q_DECLARE_METATYPE(KDevelop::IProject*)

static void initVeritasResource()
{
    Q_INIT_RESOURCE(qxrunner);
}

using Veritas::RunnerWindow;
using Veritas::RunnerModel;
using Veritas::RunnerProxyModel;
using Veritas::ResultsModel;
using Veritas::ResultsProxyModel;
using Veritas::RunnerViewController;
using Veritas::ResultsViewController;

const Ui::RunnerWindow* RunnerWindow::ui() const
{
    return m_ui;
}

RunnerWindow::RunnerWindow(QWidget* parent, Qt::WFlags flags)
        : QWidget(parent, flags)
{
    initVeritasResource();
    m_ui=new Ui::RunnerWindow;
    m_ui->setupUi(this);
    runnerView()->setRootIsDecorated(false);
    runnerView()->setUniformRowHeights(true);

    // Controllers for the tree views, it's best to create them at the
    // very beginning.
    m_runnerViewController = new RunnerViewController(this, runnerView());
    m_resultsViewController = new ResultsViewController(this, resultsView());

    connectFocusStuff();
    ui()->progressRun->show();

    // Disable user interaction while there is no data.
    enableItemActions(false);

    initItemStatistics();
    connectActions();
    m_sema.release();
    runnerView()->setMouseTracking(true);
    m_selection = new SelectionManager(runnerView());

    QPixmap refresh = KIconLoader::global()->loadIcon("view-refresh", KIconLoader::Small);
    m_ui->actionReload->setIcon(refresh);

    runnerView()->setStyleSheet(
        "QTreeView::branch{"
        "image: none;"
        "border-image: none"
        "}");
    resultsView()->setStyleSheet(
        "QTreeView::branch{"
        "image: none;"
        "border-image: none"
        "}");

    connect(runnerView(),  SIGNAL(clicked(const QModelIndex&)),
            runnerController(), SLOT(expandOrCollapse(const QModelIndex&)));
    connect(resultsView(), SIGNAL(clicked(const QModelIndex&)),
            resultsController(), SLOT(expandOrCollapse(const QModelIndex&)));
    resultsView()->setWordWrap(true);

    addProjectMenu();
}

// helper for RunnerWindow(...)
void RunnerWindow::addProjectMenu()
{
    KSelectAction *m = new KSelectAction(i18n("Project"), this);
    m->setToolTip(i18n("Select project"));
    m->setToolBarMode(KSelectAction::MenuMode);
    m->setEditable(true);
    m_ui->runnerToolBar->addAction(m);
    m_projectPopup = m;
}

void RunnerWindow::addProjectToPopup(IProject* proj)
{
    kDebug() << "Adding project to popup " << proj->name();
    QAction* p = new QAction(proj->name(), this);
    QVariant v;
    v.setValue(proj);
    p->setData(v);
    m_projectPopup->addAction(p);
    m_ui->runnerToolBar->addAction(m_projectPopup);
}

void RunnerWindow::rmProjectFromPopup(IProject* proj)
{
    kDebug() << "Adding project to popup " << proj->name() << " TODO";
}

// helper for RunnerWindow(...)
void RunnerWindow::initItemStatistics()
{
    // Initial results.
    displayNumInfos(0);
    displayNumWarnings(0);
    displayNumErrors(0);
    displayNumFatals(0);
    displayNumExceptions(0);
}

// helper for RunnerWindow(...)
void RunnerWindow::connectFocusStuff()
{
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
}

KSelectAction* RunnerWindow::projectPopup() const
{
    return m_projectPopup;
}

// helper for RunnerWindow(...) ctor
void RunnerWindow::connectActions()
{
    // File commands.
    connect(m_ui->actionExit, SIGNAL(triggered(bool)), SLOT(close()));
    // Item commands.
    m_ui->actionStart->setShortcut(QKeySequence(tr("Ctrl+R")));
    connect(m_ui->actionStart, SIGNAL(triggered(bool)), SLOT(runItems()));
    m_ui->actionStop->setShortcut(QKeySequence(tr("Ctrl+K")));
    connect(m_ui->actionStop, SIGNAL(triggered(bool)), SLOT(stopItems()));
    // View commands
    m_ui->actionSelectAll->setShortcut(QKeySequence(tr("Ctrl+A")));
    connect(m_ui->actionSelectAll, SIGNAL(triggered(bool)),
            runnerController(), SLOT(selectAll()));
    m_ui->actionUnselectAll->setShortcut(QKeySequence(tr("Ctrl+U")));
    connect(m_ui->actionUnselectAll, SIGNAL(triggered(bool)),
            runnerController(), SLOT(unselectAll()));
    m_ui->actionExpandAll->setShortcut(QKeySequence(tr("Ctrl++")));
    connect(m_ui->actionExpandAll, SIGNAL(triggered(bool)),
            runnerController(), SLOT(expandAll()));
    m_ui->actionCollapseAll->setShortcut(QKeySequence(tr("Ctrl+-")));
    connect(m_ui->actionCollapseAll, SIGNAL(triggered(bool)),
            runnerController(), SLOT(collapseAll()));
}

RunnerWindow::~RunnerWindow()
{
    // Deleting the model is left to the owner of the model instance.
}

// helper for setModel(RunnerModel*)
void RunnerWindow::stopPreviousModel()
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
}

// helper for setModel(RunnerModel*)
void RunnerWindow::initProxyModels(RunnerModel* model)
{
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
}

// helper for setModel(RunnerModel*)
void RunnerWindow::initVisibleColumns(RunnerModel* model)
{
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
    runnerProxyModel()->setEnabledColumns(enabledRunnerColumns);
    resultsProxyModel()->setEnabledColumns(enabledResultsColumns);
}

// helper for setModel(RunnerModel*)
void RunnerWindow::connectItemStatistics(RunnerModel* model)
{
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
}

// helper for setModel(RunnerModel*)
void RunnerWindow::connectProgressIndicators(RunnerModel* model)
{
    // Get notified of items run.
    connect(model, SIGNAL(numStartedChanged(int)),
            SLOT(displayProgress(int)));
    connect(model, SIGNAL(numCompletedChanged(int)),
            SLOT(displayNumCompleted(int)));
    connect(model, SIGNAL(allItemsCompleted()),
            SLOT(displayCompleted()));
    connect(model, SIGNAL(itemCompleted(const QModelIndex&)),
            resultsModel(),
            SLOT(addResult(const QModelIndex&)));
}

void RunnerWindow::setModel(RunnerModel* model)
{
    stopPreviousModel();
    if (!model || model->columnCount() < 1) {
        // No user interaction without a model or a model without columns.
        enableItemActions(false);
        return;
    }
    initProxyModels(model);
    initVisibleColumns(model);

    showResults(true);
    // Very limited user interaction without data.
    if (model->rowCount() < 1) {
        enableItemActions(false);
        m_ui->actionColumns->setEnabled(true);
        m_ui->actionSettings->setEnabled(true);
        return;
    }
    connectItemStatistics(model);

    // How much data is wanted when running the items.
    connect(m_ui->actionMinimalUpdate, SIGNAL(triggered(bool)),
            model, SLOT(setMinimalUpdate(bool)));
    model->setMinimalUpdate(isMinimalUpdate());
    connectProgressIndicators(model);
    enableItemActions(true);
    m_ui->actionStop->setDisabled(true);
    setResultsFilter();

    connect(resultsModel(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            resultsController(), SLOT(spanOutputLines(const QModelIndex&, int, int)));
    connect(m_selection, SIGNAL(selectionChanged()),
            runnerModel(), SLOT(countItems()));
    connect(resultsProxyModel(), SIGNAL(modelReset()),
            resultsController(), SLOT(span()));

    // set top row higlighted
    runnerView()->setCurrentIndex(runnerProxyModel()->index(0, 0));
}

QTreeView* RunnerWindow::runnerView() const
{
    return m_ui->treeRunner;
}

QTreeView* RunnerWindow::resultsView() const
{
    return m_ui->treeResults;
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
    if (ui()->progressRun->maximum() > 0) {
        ui()->progressRun->setValue(numItems);
    }
}

void RunnerWindow::displayCompleted() const
{
    ui()->progressRun->setValue(ui()->progressRun->maximum());
    enableControlsAfterRunning();
}

void RunnerWindow::displayNumTotal(int numItems) const
{
    ui()->labelNumTotal->setText(QString().setNum(numItems));
}

void RunnerWindow::displayNumSelected(int numItems) const
{
    ui()->labelNumSelected->setText(QString().setNum(numItems));
    ui()->progressRun->setMaximum(numItems);
}

void RunnerWindow::displayNumCompleted(int numItems) const
{
    ui()->labelNumRun->setText(QString().setNum(numItems));
}

void RunnerWindow::displayNumSuccess(int numItems) const
{
    displayStatusNum(ui()->labelNumSuccess,
                     ui()->labelNumSuccess, numItems);

    // Num success always visible.
    ui()->labelNumSuccess->show();
}

void RunnerWindow::displayNumInfos(int numItems) const
{
    displayStatusNum(ui()->labelNumInfos,
                     ui()->labelNumInfosPic, numItems);
}

void RunnerWindow::displayNumWarnings(int numItems) const
{
    displayStatusNum(ui()->labelNumWarnings,
                     ui()->labelNumWarningsPic, numItems);
}

void RunnerWindow::displayNumErrors(int numItems) const
{
    displayStatusNum(ui()->labelNumErrors,
                     ui()->labelNumErrorsPic, numItems);
}

void RunnerWindow::displayNumFatals(int numItems) const
{
    displayStatusNum(ui()->labelNumFatals,
                     ui()->labelNumFatalsPic, numItems);
}

void RunnerWindow::displayNumExceptions(int numItems) const
{

    displayStatusNum(ui()->labelNumExceptions,
                     ui()->labelNumExceptionsPic, numItems);
}

void RunnerWindow::setResultsFilter() const
{
    QModelIndex current = selectedResultIndex();
    int filter = Veritas::RunError | Veritas::RunFatal;
    resultsProxyModel()->setFilter(filter); // also updates the view
    highlightResultAgain(current);
}

// helper for setResultsFilter()
void RunnerWindow::highlightResultAgain(const QModelIndex& previous) const
{
    // Try to highlight same result as was highlighted before.
    if (!previous.isValid()) {
        return; // When no result was highlighted before, do nothing
    }
    QModelIndex viewIndex;
    viewIndex = Utils::proxyIndexFromModel(resultsProxyModel(), previous);
    if (!viewIndex.isValid()) {
        return; // Previously highlighted result is filtered out now.
    }
    enableResultSync(false);
    resultsView()->setCurrentIndex(viewIndex);
    enableResultSync(true);
}

// helper for setResultsFilter()
QModelIndex RunnerWindow::selectedResultIndex() const
{
    // Remember currently highlighted result.
    QModelIndex resultIndex;
    QModelIndexList indexes;
    indexes = resultsView()->selectionModel()->selectedIndexes();
    if (indexes.count() > 0) {
        resultIndex = resultsProxyModel()->mapToSource(indexes.first());
    }
    return resultIndex;
}

void RunnerWindow::syncResultWithTest(const QItemSelection& selected,
                                      const QItemSelection& deselected) const
{
    Q_UNUSED(deselected);
    QModelIndexList indexes = selected.indexes();
    // Do nothing when there are no results or no runner item is selected.
    if (indexes.count() < 1 || !runnerProxyModel()->index(0, 0).isValid()) {
        return;
    }
    enableResultSync(false); // Prevent circular reaction

    // Get the results model index that corresponds to the runner item index.
    QModelIndex testItemIndex = runnerProxyModel()->mapToSource(indexes.first());
    QModelIndex resultIndex = resultsModel()->mapFromTestIndex(testItemIndex);
    QModelIndex viewIndex = resultsProxyModel()->mapFromSource(resultIndex);

    // At least there should be a current but not necessarily highlighted result
    // in order to see the focus rect when the results view gets the focus.
    ensureCurrentResult();

    QModelIndex filterIndex;
    if (resultIndex.isValid() && viewIndex.isValid()) {
        resultsView()->clearSelection();
        resultsView()->setCurrentIndex(viewIndex);
        scrollToHighlightedRows();
        filterIndex = testItemIndex.parent();
    } else if (!resultIndex.isValid()) {
        filterIndex = testItemIndex;
    }

    if (filterIndex.isValid()) {
        Test* t = static_cast<Test*>(filterIndex.internalPointer());
        resultsProxyModel()->setTestFilter(t->leafs());
    }

    if (!resultIndex.isValid()) {
        kDebug() << "Failed to find result item for runner stuff";
    } else if (!viewIndex.isValid()) {
        kDebug() << "Looks like result is being filtered";
    }
    enableResultSync(true);
}

void RunnerWindow::syncTestWithResult(const QItemSelection& selected,
                                      const QItemSelection& deselected) const
{
    Q_UNUSED(deselected);
    QModelIndexList indexes = selected.indexes();

    if (indexes.count() < 1) {
        return; // Do nothing when no result is selected.
    }

    // Determine the results model index.
    QModelIndex resultIndex;
    resultIndex = Utils::modelIndexFromProxy(resultsProxyModel(), indexes.first());

    if (resultIndex.parent().isValid()) {
        resultIndex = resultIndex.parent();
    }

    // Get the corresponding runner item index contained in the results model.
    QModelIndex testItemIndex;
    testItemIndex = resultsModel()->mapToTestIndex(resultIndex);

    enableTestSync(false); // prevent circular dependencies.

    // Determine the proxy model index and highlight it.
    QModelIndex currentIndex;
    currentIndex = Utils::proxyIndexFromModel(runnerProxyModel(), testItemIndex);
    runnerView()->setCurrentIndex(currentIndex);

    scrollToHighlightedRows(); // Make the row in every tree view visible 
                               // and expand corresponding parents.
    enableTestSync(true);      // Enable selection handler again.
}

void RunnerWindow::ensureFocusRect(const QModelIndex&  index)
{
    QTreeView* treeView;
    if (runnerView()->hasFocus()) {
        treeView = runnerView();
    } else {
        treeView = resultsView();
    }
    if (treeView->selectionMode() == QAbstractItemView::NoSelection) {
        return;     // No relevance when selections not allowed.
    }

    // Focus rect is there when column entry not empty.
    QString data = index.data().toString();
    if (!data.trimmed().isEmpty()) {
        return;
    }
    if (index.column() == 0) {
        return; // No relevance when column 0 is empty.
    }

    // Tree views are already synchronized.
    enableTestSync(false);
    enableResultSync(false);

    treeView->clearSelection(); // This ensures that there is a focus rect.

    QItemSelectionModel* selectionModel = treeView->selectionModel();
    selectionModel->setCurrentIndex(index.sibling(index.row(), 0),
                                    QItemSelectionModel::Select |
                                    QItemSelectionModel::Rows);

    // Enable selection handlers again.
    enableTestSync(true);
    enableResultSync(true);
}

void RunnerWindow::scrollToHighlightedRows() const
{

    if (runnerView()->selectionMode() == QAbstractItemView::NoSelection ||
            resultsView()->selectionMode() == QAbstractItemView::NoSelection) {
        return; // No relevance when selections not allowed.
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
    m_ui->actionStop->setDisabled(true);
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
    m_ui->actionStop->setEnabled(true);
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
    bool visible = m_ui->treeResults->isVisible();

    if (show && !visible) {
        // Show data according to current filtering.
        resultsProxyModel()->clear();
        setResultsFilter();
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

    m_ui->actionStop->setEnabled(true);
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

    // Give the GUI a chance to update.
    QCoreApplication::processEvents();

    // Enable user interaction.
    enableItemActions(true);

    m_ui->actionStop->setDisabled(true);
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


    enableTestSync(false);     // Suppress synchronization of results view.
    runnerController()->setHighlightedRow(currentIndex); 
    enableTestSync(true);      // Enable selection handler again.
}

void RunnerWindow::enableItemActions(bool enable) const
{
    m_ui->actionStart->setEnabled(enable);
    m_ui->actionStop->setEnabled(enable);
    m_ui->actionSelectAll->setEnabled(enable);
    m_ui->actionUnselectAll->setEnabled(enable);
    m_ui->actionExpandAll->setEnabled(enable);
    m_ui->actionCollapseAll->setEnabled(enable);
    m_ui->actionMinimalUpdate->setEnabled(enable);
    m_ui->actionColumns->setEnabled(enable);
    m_ui->actionSettings->setEnabled(enable);
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

void RunnerWindow::displayStatusNum(QLabel* labelForText,
                                    QLabel* labelForPic, int numItems) const
{
    labelForText->setText(": " + QString().setNum(numItems));
    bool visible;
    if (numItems > 0) {
        visible = true;
    } else {
        visible = false;
    }
    labelForText->setVisible(visible);
    labelForPic->setVisible(visible);
}

bool RunnerWindow::isMinimalUpdate() const
{
    return m_ui->actionMinimalUpdate->isChecked();
}

RunnerViewController* RunnerWindow::runnerController() const
{
    return m_runnerViewController;
}

ResultsViewController* RunnerWindow::resultsController() const
{
    return m_resultsViewController;
}
