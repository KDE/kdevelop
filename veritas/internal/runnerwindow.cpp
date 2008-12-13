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

#include "runnerwindow.h"

#include "ui_runnerwindow.h"

#include "interfaces/iproject.h"

#include "resultsmodel.h"
#include "resultsproxymodel.h"
#include "runnermodel.h"
#include "runnerproxymodel.h"
#include "selectionmanager.h"
#include "verbosetoggle.h"
#include "selectiontoggle.h"
#include "tosourcetoggle.h"
#include "testexecutor.h"

#include <ktexteditor/cursor.h>
#include "interfaces/icore.h"
#include "interfaces/idocumentcontroller.h"
#include "interfaces/iruncontroller.h"

#include "utils.h"
#include "test_p.h"
#include "resultswidget.h"

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
#include <interfaces/iprojectcontroller.h>

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
using Veritas::TestExecutor;
using Veritas::Test;
using Veritas::TestBar;

const Ui::RunnerWindow* RunnerWindow::ui() const
{
    return m_ui;
}

TestBar::TestBar(QWidget* parent) : QProgressBar(parent)
{
    turnGreen();
}

TestBar::~TestBar()
{
}

void TestBar::turnGreen()
{
    QPalette pal = palette();
    QBrush brush(QColor("green"));
    pal.setBrush( QPalette::Highlight, brush );
    setPalette(pal);
}

void TestBar::turnRed()
{
    QPalette pal = palette();
    QBrush brush(QColor("red"));
    pal.setBrush( QPalette::Highlight, brush );
    setPalette(pal);
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
    progressBar()->setTextVisible(false);
    progressBar()->show();
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
    m_results->setResizeMode();

    m_selection = new SelectionManager(runnerView());
    SelectionToggle* selectionToggle = new SelectionToggle(runnerView()->viewport());
    m_selection->setButton(selectionToggle);

    m_verbose = new OverlayManager(runnerView());
    m_verboseToggle = new VerboseToggle(runnerView()->viewport());
    connect(m_verboseToggle, SIGNAL(clicked(bool)),SLOT(showVerboseTestOutput()));
    m_verbose->setButton(m_verboseToggle);

    m_toSource = new OverlayManager(runnerView());
    m_toSourceToggle = new ToSourceToggle(runnerView()->viewport());
    connect(m_toSourceToggle, SIGNAL(clicked(bool)), SLOT(openTestSource()));
    m_toSource->setButton(m_toSourceToggle);

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

    connect(runnerView(),  SIGNAL(clicked(QModelIndex)),
            SLOT(expandOrCollapse(QModelIndex)));

    const char* whatsthis = "xTest runner. First select a project from the rightmost dropdown box. Next, load the test tree by clicking on the green circular arrow icon. Run your tests with a click on the leftmost green arrow icon.";
    setWhatsThis( i18n(whatsthis) );
    resultsView()->setWhatsThis( i18n(whatsthis) );

    runnerView()->setSelectionMode(QAbstractItemView::SingleSelection);
    runnerView()->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void RunnerWindow::showVerboseTestOutput()
{
    const QModelIndex index = m_verboseToggle->index();
    if (index.isValid()) {
        Test* t = m_verbose->index2Test(index);
        if (t) {
            KJob* j = t->createVerboseOutputJob();
            if (j) ICore::self()->runController()->registerJob(j);
        }
    }
}

void RunnerWindow::openTestSource()
{
    const QModelIndex index = m_toSourceToggle->index();
    if (index.isValid()) {
        m_toSource->index2Test(index)->toSource();
    }
}

// helper for RunnerWindow(...)
void RunnerWindow::addProjectMenu()
{
    KSelectAction *m = new KSelectAction(i18n("Project"), this);
    m->setToolTip(i18n("Select project"));
    m->setToolBarMode(KSelectAction::MenuMode);
    m_ui->runnerToolBar->addSeparator();
    m_ui->runnerToolBar->addAction(m);
    m_projectPopup = m;
    connect(m_projectPopup, SIGNAL(triggered(QAction*)),
            SLOT(setSelectedProject(QAction*)));
}

void RunnerWindow::setSelectedProject(QAction* action)
{
    if (!action) return;
    KUrl projectRoot = action->data().value<KUrl>();
    Q_ASSERT(m_project2action.contains(projectRoot));
    m_currentProject = projectRoot;
    emit requestReload();
}

void RunnerWindow::addProjectToPopup(IProject* proj)
{
    QAction* p = new QAction(proj->name(), this);
    QVariant v;
    v.setValue(proj->folder());
    p->setData(v);
    m_projectPopup->addAction(p);
    m_project2action[proj->folder()] = p;
}

void RunnerWindow::rmProjectFromPopup(IProject* proj)
{
    KUrl projectRoot = proj->folder();
    if (m_project2action.contains(projectRoot)) {
        QAction* p = m_project2action[projectRoot];
        m_projectPopup->removeAction(p);
        m_project2action.remove(projectRoot);
    }
}

IProject* RunnerWindow::selectedProject() const
{
    if (!m_currentProject.isValid()) return 0;
    IProject* selected = 0;
    foreach(IProject* proj, ICore::self()->projectController()->projects()) {
        if (m_currentProject == proj->folder()) {
            selected = proj;
            break;
        }
    }
    return selected;
}

namespace
{
/*! functor that counts the selected leaf tests */
class SelectedLeafCount
{
public:
    SelectedLeafCount() : result(0) {}
    void operator()(Test* t) {
        if ((t->childCount() == 0) && t->internal()->isChecked()) {
            result++;
        }
    }
    int result;
};

}

void RunnerWindow::resetProgressBar() const
{
    progressBar()->setValue(0);
    progressBar()->update();
    if (runnerModel()) {
        SelectedLeafCount slf;
        traverseTree(runnerModel()->rootItem(), slf);
        if (slf.result == 0) slf.result++; // 0 results in an indeterminate progressbar, not good
        progressBar()->setMaximum(slf.result);
    } else {
        progressBar()->setMaximum(1);
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

        resultsModel()->clear();
        m_results->setResizeMode();
        prevModel->disconnect();
        delete prevModel;
    }
    m_isRunning = false;
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
    if (!model) {
        // No user interaction without a model or an empty one
        enableItemActions(false);
        return;
    }
    initProxyModels(model);
    if (model->rowCount() == 0) {
        enableItemActions(false);
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
    m_toSource->makeConnections();
    runnerView()->resizeColumnToContents(0);
}


void RunnerWindow::displayProgress(int numItems) const
{
    // Display only when there are selected items
    if (progressBar()->maximum() > 0) {
        progressBar()->setValue(numItems);
    }
}

void RunnerWindow::displayCompleted() const
{
    if (!m_isRunning) return;
    progressBar()->setValue(progressBar()->maximum());
    enableControlsAfterRunning();
    updateRunText();
    m_isRunning = false;
    emit runCompleted();
}

void RunnerWindow::displayNumTotal(int numItems) const
{
    Q_UNUSED(numItems);
}

void RunnerWindow::displayNumSelected(int numItems) const
{
    if (numItems == 0) numItems++;
    resetProgressBar();
}

void RunnerWindow::displayNumCompleted(int numItems)
{
    m_numItemsCompleted = numItems;
    updateRunText();
}

void RunnerWindow::displayNumErrors(int numItems) const
{
    if (numItems > 0) progressBar()->turnRed();
}

TestBar* RunnerWindow::progressBar() const
{
    return static_cast<TestBar*>(ui()->progressRun);
}

void RunnerWindow::displayNumFatals(int numItems) const
{
    if (numItems > 0) progressBar()->turnRed();
}

void RunnerWindow::displayNumExceptions(int numItems) const
{
    if (numItems > 0) progressBar()->turnRed();
}

namespace
{
inline Test* testFromIndex(const QModelIndex& index)
{
    return static_cast<Test*>(index.internalPointer());
}
}

void RunnerWindow::syncResultWithTest(const QItemSelection& selected,
                                      const QItemSelection& deselected) const
{
    Q_UNUSED(deselected);
    QModelIndexList indexes = selected.indexes();
    if (indexes.count() < 1 || !runnerProxyModel()->index(0, 0).isValid()) {
        return; // Do nothing when there are no results or no runner item is selected.
    }

    enableResultSync(false); // Prevent circular reaction
    QModelIndex testIndex = runnerProxyModel()->mapToSource(indexes.first());
    if (testIndex.isValid()) {
        Test* t = testFromIndex(testIndex);
        resultsProxyModel()->setTestFilter(t);
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

void RunnerWindow::updateRunText() const
{
    QString elapsed;
    if (m_stopWatch.isValid()) {
        int mili = m_stopWatch.elapsed();
        QString elapsed = QString("%1.%2").arg(int(mili/1000)).arg(mili%1000);
    } else {
        elapsed = "0.000";
    }
    ui()->labelRunText->setText( i18ncp("%2 is a real number like 1.355", "Ran 1 test in %2 seconds", "Ran %1 tests in %2 seconds", m_numItemsCompleted, elapsed) );
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

    m_stopWatch = QTime();
    progressBar()->turnGreen();
    displayNumCompleted(0);
    m_stopWatch.start();

    disableControlsBeforeRunning();
    resultsModel()->clear();
    runnerModel()->clearTree();
    runnerView()->viewport()->update(); // the icons have changed
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
    TestResult* r = resultsModel()->testResult(resultIndex);
    if (!r) return;

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

