/* KDevelop xUnit plugin
 *
 * Copyright 2006 Ernst Huber <qxrunner@systest.ch>
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

#include "resultsmodel.h"
#include "resultsproxymodel.h"
#include "runnermodel.h"
#include "runnerproxymodel.h"
#include "selectionmanager.h"
#include "verbosemanager.h"
#include "verbosetoggle.h"
#include "selectiontoggle.h"
#include "../testexecutor.h"

#include <ktexteditor/cursor.h>
#include "interfaces/icore.h"
#include "interfaces/idocumentcontroller.h"

#include "veritas/utils.h"
#include "mvc/resultswidget.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <KLocale>
#include <KAction>
#include <KActionMenu>
#include <KSelectAction>
#include <KIcon>
#include <KDebug>
#include <KColorScheme>
#include <QBrush>
#include <QColor>
#include <QHeaderView>

using KDevelop::IProject;
Q_DECLARE_METATYPE(KDevelop::IProject*)

static void initVeritasResource()
{
    Q_INIT_RESOURCE(qxrunner);
}

using KDevelop::ICore;
using KDevelop::IDocumentController;

using Veritas::RunnerWindow;
using Veritas::RunnerModel;
using Veritas::RunnerProxyModel;
using Veritas::ResultsModel;
using Veritas::ResultsProxyModel;
using Veritas::VerboseManager;
using Veritas::TestExecutor;

const Ui::RunnerWindow* RunnerWindow::ui() const
{
    return m_ui;
}

RunnerWindow::RunnerWindow(ResultsModel* rmodel, QWidget* parent, Qt::WFlags flags)
        : QWidget(parent, flags), m_executor(0), m_isRunning(false)
{
    initVeritasResource();
    m_ui = new Ui::RunnerWindow;
    m_ui->setupUi(this);
    m_results = new ResultsWidget();
    runnerView()->setRootIsDecorated(false);
    runnerView()->setUniformRowHeights(true);

    connectFocusStuff();
    setGreenBar();
    ui()->progressRun->setTextVisible(false);
    ui()->progressRun->show();
    addProjectMenu();

    // Disable user interaction while there is no data.
    enableItemActions(false);

    initItemStatistics();
    connectActions();
    runnerView()->setMouseTracking(true);

    ResultsProxyModel* rproxy = new ResultsProxyModel(this);
    rproxy->setSourceModel(rmodel);
    int filter = Veritas::RunError | Veritas::RunFatal | Veritas::RunInfo;
    rproxy->setFilter(filter); // also updates the view
    resultsView()->setModel(rproxy);

    m_selection = new SelectionManager(runnerView());
    SelectionToggle* selectionToggle = new SelectionToggle(runnerView()->viewport());
    m_selection->setButton(selectionToggle);
    m_verbose = new VerboseManager(runnerView());
    VerboseToggle* verboseToggle = new VerboseToggle(runnerView()->viewport());
    m_verbose->setButton(verboseToggle);

    QPixmap refresh = KIconLoader::global()->loadIcon("view-refresh", KIconLoader::Small);
    m_ui->actionReload->setIcon(refresh);
    QPixmap run = KIconLoader::global()->loadIcon("arrow-right", KIconLoader::Small);
    m_ui->actionStart->setIcon(run);
    QPixmap stop = KIconLoader::global()->loadIcon("window-close", KIconLoader::Small);
    m_ui->actionStop->setIcon(stop);
    QPixmap select = KIconLoader::global()->loadIcon("list-add", KIconLoader::Small);
    m_ui->actionSelectAll->setIcon(select);
    QPixmap deselect = KIconLoader::global()->loadIcon("list-remove", KIconLoader::Small);
    m_ui->actionUnselectAll->setIcon(deselect);

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

    resultsView()->header()->setResizeMode(QHeaderView::Interactive);

    connect(runnerView(),  SIGNAL(clicked(QModelIndex)),
            SLOT(expandOrCollapse(QModelIndex)));

    const char* whatsthis = "xTest runner. First select a project from the rightmost dropdown box. Next, load the test tree by clicking on the green circular arrow icon. Run your tests with a click on the leftmost green arrow icon.";
    setWhatsThis( i18n(whatsthis) );
    resultsView()->setWhatsThis( i18n(whatsthis) );

    runnerView()->setSelectionMode(QAbstractItemView::SingleSelection);
    runnerView()->setSelectionBehavior(QAbstractItemView::SelectRows);
}

// helper for RunnerWindow(...)
void RunnerWindow::addProjectMenu()
{
    KSelectAction *m = new KSelectAction(i18n("Project"), this);
    m->setToolTip(i18n("Select project"));
    m->setToolBarMode(KSelectAction::MenuMode);
    m->setEditable(true);
    m_ui->runnerToolBar->addSeparator();
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
    m_project2action[proj] = p;
}

void RunnerWindow::rmProjectFromPopup(IProject* proj)
{
    if (m_project2action.contains(proj)) {
        QAction* p = m_project2action[proj];
        m_projectPopup->removeAction(p);
        m_project2action.remove(proj);
    }
}

void RunnerWindow::resetProgressBar() const
{
    ui()->progressRun->setValue(0);
    ui()->progressRun->update();
    if (ui()->progressRun->maximum() == 0) {
        ui()->progressRun->setMaximum(1);
    }
}

// helper for RunnerWindow(...)
void RunnerWindow::initItemStatistics()
{
    displayNumErrors(0);
    displayNumFatals(0);
    displayNumExceptions(0);
}

// helper for RunnerWindow(...)
void RunnerWindow::connectFocusStuff()
{
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
            this, SLOT(selectAll()));
    m_ui->actionUnselectAll->setShortcut(QKeySequence(tr("Ctrl+U")));
    connect(m_ui->actionUnselectAll, SIGNAL(triggered(bool)),
            this, SLOT(unselectAll()));
    m_ui->actionExpandAll->setShortcut(QKeySequence(tr("Ctrl++")));
    connect(m_ui->actionExpandAll, SIGNAL(triggered(bool)),
            runnerView(), SLOT(expandAll()));
    m_ui->actionCollapseAll->setShortcut(QKeySequence(tr("Ctrl+-")));
    connect(m_ui->actionCollapseAll, SIGNAL(triggered(bool)),
            runnerView(), SLOT(collapseAll()));
}

void RunnerWindow::unselectAll()
{
    runnerModel()->uncheckAll();
    resetProgressBar();
    runnerView()->viewport()->update();
}

void RunnerWindow::selectAll()
{
    runnerModel()->checkAll();
    resetProgressBar();
    runnerView()->viewport()->update();
}

RunnerWindow::~RunnerWindow()
{
    // Deleting the model is left to the owner of the model instance.
    if (m_selection) delete m_selection;
    if (m_verbose) delete m_verbose;
    if (m_executor) {
        m_executor->stop();
        delete m_executor;
    }
    if (runnerModel()) delete runnerModel();
    delete m_ui;
}

// helper for setModel(RunnerModel*)
void RunnerWindow::stopPreviousModel()
{
    RunnerModel* prevModel = runnerModel();
    if (prevModel) {
        if (m_executor) m_executor->stop();

        RunnerProxyModel* m1 = runnerProxyModel();
        runnerView()->setModel(0);
        runnerView()->reset();
        delete m1;

        resultsView()->reset();
        resultsModel()->clear();
        prevModel->disconnect();
        delete prevModel;
    }
}

// helper for setModel(RunnerModel*)
void RunnerWindow::initProxyModels(RunnerModel* model)
{
    // Proxy models for the views with the source model as their parent
    // to have the proxies deleted when the model gets deleted.
    RunnerProxyModel* proxy = new RunnerProxyModel(model);
    proxy->setSourceModel(model);
    runnerView()->setModel(proxy);
    m_stopWatch = QTime();
}

// helper for setModel(RunnerModel*)
void RunnerWindow::connectItemStatistics(RunnerModel* model)
{
    // Item statistics.
    connect(model, SIGNAL(numTotalChanged(int)),
            SLOT(displayNumTotal(int)));
    connect(model, SIGNAL(numSelectedChanged(int)),
            SLOT(displayNumSelected(int)));
    connect(model, SIGNAL(numErrorsChanged(int)),
            SLOT(displayNumErrors(int)));
    connect(model, SIGNAL(numFatalsChanged(int)),
            SLOT(displayNumFatals(int)));
    connect(model, SIGNAL(numExceptionsChanged(int)),
            SLOT(displayNumExceptions(int)));

    model->countItems(); // this fires the signals connected above.
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
    connect(model, SIGNAL(itemCompleted(QModelIndex)),
            resultsModel(), SLOT(addResult(QModelIndex)));
}

void RunnerWindow::setModel(RunnerModel* model)
{
    m_verbose->reset();
    m_selection->reset();
    stopPreviousModel();
    if (!model || model->columnCount() < 1) {
        // No user interaction without a model or a model without columns.
        enableItemActions(false);
        return;
    }
    initProxyModels(model);

    // Very limited user interaction without data.
    if (model->rowCount() < 1) {
        enableItemActions(false);
        m_ui->actionColumns->setEnabled(true);
        m_ui->actionSettings->setEnabled(true);
        return;
    }
    connectItemStatistics(model);

    connectProgressIndicators(model);
    enableItemActions(true);
    m_ui->actionStop->setDisabled(true);

    connect(m_selection, SIGNAL(selectionChanged()),
            runnerModel(), SLOT(countItems()));

    // set top row higlighted
    runnerView()->setCurrentIndex(runnerProxyModel()->index(0, 0));
    enableToSource();
    enableTestSync(true);
    m_verbose->makeConnections();
    m_selection->makeConnections();
    runnerView()->resizeColumnToContents(0);
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
    if (!m_isRunning) return;
    ui()->progressRun->setValue(ui()->progressRun->maximum());
    enableControlsAfterRunning();
    displayElapsed();
    m_isRunning = false;
    emit runCompleted();
}

void RunnerWindow::displayNumTotal(int numItems) const
{
    Q_UNUSED(numItems);
}

void RunnerWindow::displayNumSelected(int numItems) const
{
    if (numItems == 0) { numItems++;}
    ui()->progressRun->setMaximum(numItems);
}

void RunnerWindow::displayNumCompleted(int numItems) const
{
    ui()->labelNumRun->setText(QString().setNum(numItems));
    displayElapsed();
}

void RunnerWindow::setGreenBar() const
{
    QProgressBar* bar = ui()->progressRun;
    bar->setStyleSheet(
        QString("QProgressBar {"
            "border: 1px solid grey;"
            "border-radius: 2px;"
            "text-align: center;"
        "}"
        "QProgressBar::chunk {"
            "background-color: #009700;"
            "width: 5px;"
        "}"));
}

void RunnerWindow::setRedBar() const
{
    QProgressBar* bar = ui()->progressRun;
    bar->setStyleSheet(
        QString("QProgressBar {"
            "border: 1px solid grey;"
            "border-radius: 2px;"
            "text-align: center;"
        "}"
        "QProgressBar::chunk {"
            "background-color: #DF1313;"
            "width: 5px;"
        "}"));
}

void RunnerWindow::displayNumErrors(int numItems) const
{
    if (numItems > 0) setRedBar();
}

void RunnerWindow::displayNumFatals(int numItems) const
{
    if (numItems > 0) setRedBar();
}

void RunnerWindow::displayNumExceptions(int numItems) const
{
    if (numItems > 0) setRedBar();
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
        resultsProxyModel()->setTestFilter(t);
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

void RunnerWindow::displayElapsed() const
{
    if (m_stopWatch.isValid()) {
        int mili = m_stopWatch.elapsed();
        QString elapsed = QString("%1.%2").arg(int(mili/1000)).arg(mili%1000);
        ui()->labelElapsed->setText(elapsed);
    } else {
        ui()->labelElapsed->setText("0.000");
    }
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
    if (m_isRunning || !runnerModel()->rootItem()) {
        return;
    }
    m_isRunning = true;

    m_stopWatch.start();
    setGreenBar();
    displayNumCompleted(0);
    ui()->labelElapsed->setText("0.000");

    disableControlsBeforeRunning();
    resultsModel()->clear();
    runnerModel()->clearTree();
    runnerModel()->initCounters();

    if (m_executor) delete m_executor;
    m_executor = new TestExecutor;
    m_executor->setRoot(runnerModel()->rootItem());
    connect(m_executor, SIGNAL(allDone()), SLOT(displayCompleted()));

    m_executor->go();
}

void RunnerWindow::stopItems()
{
    if (!runnerModel() || !m_isRunning) return;
    m_ui->actionStop->setDisabled(true);
    if (m_executor) m_executor->stop();
    displayCompleted();
    enableControlsAfterRunning();
    m_isRunning = false;
}

void RunnerWindow::disableControlsBeforeRunning()
{
    enableItemActions(false);

    m_ui->actionStop->setEnabled(true);
    m_projectPopup->setEnabled(false);
    m_ui->actionReload->setEnabled(false);
    runnerView()->setCursor(QCursor(Qt::BusyCursor));
    runnerView()->setFocus();
    runnerView()->setSelectionMode(QAbstractItemView::NoSelection);
    resultsView()->setSelectionMode(QAbstractItemView::NoSelection);
    enableTestSync(false);
    enableResultSync(false);
}

void RunnerWindow::enableControlsAfterRunning() const
{
    enableItemActions(true);

    m_ui->actionStop->setDisabled(true);
    m_ui->actionReload->setEnabled(true);
    m_projectPopup->setEnabled(true);
    runnerView()->setCursor(QCursor());
    runnerView()->setFocus();
    runnerView()->setSelectionMode(QAbstractItemView::SingleSelection);
    resultsView()->setSelectionMode(QAbstractItemView::SingleSelection);
    enableTestSync(true);
    enableResultSync(true);

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
                SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                SLOT(syncResultWithTest(QItemSelection,QItemSelection)));
    } else {
        disconnect(runnerView()->selectionModel(),
                SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(syncResultWithTest(QItemSelection,QItemSelection)));
    }
}

void RunnerWindow::enableToSource() const
{
    connect(resultsView()->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(jumpToSource(QItemSelection,QItemSelection)));
}

void RunnerWindow::jumpToSource(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);
    QModelIndexList indexes = selected.indexes();

    if (indexes.count() < 1) {
        return; // Do nothing when no result is selected.
    }

    // Determine the results model index.
    QModelIndex resultIndex;
    resultIndex = Utils::modelIndexFromProxy(resultsProxyModel(), indexes.first());
    Test* t = resultsModel()->testFromIndex(resultIndex);
    TestResult* r = t->result();

    KTextEditor::Cursor range(r->line() - 1, 0);
    IDocumentController* dc = ICore::self()->documentController();
    dc->openDocument(KUrl(r->file().pathOrUrl()), range);
}

void RunnerWindow::enableResultSync(bool enable) const
{
    if (enable) {
        connect(resultsView()->selectionModel(),
                SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                SLOT(syncTestWithResult(QItemSelection,QItemSelection)));
    } else {
        disconnect(resultsView()->selectionModel(),
                SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(syncTestWithResult(QItemSelection,QItemSelection)));
    }
}

void RunnerWindow::displayStatusNum(QLabel* labelForText,
                                    QLabel* labelForPic, int numItems) const
{
    labelForText->setText(": " + QString().setNum(numItems));
    bool visible = (numItems > 0);
    labelForText->setVisible(visible);
    labelForPic->setVisible(visible);
}

void RunnerWindow::expandOrCollapse(const QModelIndex& index) const
{
    runnerView()->isExpanded(index) ?
        runnerView()->collapse(index) :
        runnerView()->expand(index);
}

////////////////// GETTERS /////////////////////////////////////////////////////////////

QTreeView* RunnerWindow::runnerView() const
{
    return m_ui->treeRunner;
}

QTreeView* RunnerWindow::resultsView() const
{
    return m_results->tree();
}

QWidget* RunnerWindow::resultsWidget() const
{
    return m_results;
}

RunnerModel* RunnerWindow::runnerModel() const
{
    RunnerProxyModel* proxy = runnerProxyModel();
    return proxy ?
        qobject_cast<RunnerModel*>(proxy->sourceModel()) :
        0;
}

RunnerProxyModel* RunnerWindow::runnerProxyModel() const
{
    return qobject_cast<RunnerProxyModel*>(runnerView()->model());
}

ResultsModel* RunnerWindow::resultsModel() const
{
    ResultsProxyModel* proxy = resultsProxyModel();
    return proxy ?
        qobject_cast<ResultsModel*>(proxy->sourceModel()) :
        0;
}

ResultsProxyModel* RunnerWindow::resultsProxyModel() const
{
    return qobject_cast<ResultsProxyModel*>(resultsView()->model());
}

VerboseManager* RunnerWindow::verboseManager() const
{
    return m_verbose;
}
