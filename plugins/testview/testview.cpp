/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "testview.h"
#include "testviewplugin.h"
#include "testviewdebug.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/isession.h>

#include <util/executecompositejob.h>

#include <language/duchain/indexeddeclaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>

#include <KActionCollection>
#include <KLocalizedString>
#include <KJob>
#include <KRecursiveFilterProxyModel>
#include <QLineEdit>
#include <KConfigGroup>

#include <QAction>
#include <QIcon>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>
#include <QVBoxLayout>

using namespace KDevelop;

enum CustomRoles {
    ProjectRole = Qt::UserRole + 1,
    SuiteRole,
    CaseRole
};

//BEGIN TestViewFilterAction

TestViewFilterAction::TestViewFilterAction(  const QString &initialFilter, QObject* parent )
    : QAction( parent )
    , m_intialFilter(initialFilter)
{
    setIcon(QIcon::fromTheme("view-filter"));
    setText(i18n("Filter..."));
    setToolTip(i18n("Insert wildcard patterns to filter the test view"
                    " for matching test suites and cases."));
}

QWidget* TestViewFilterAction::createWidget( QWidget* parent )
{
    QLineEdit* edit = new QLineEdit(parent);
    edit->setPlaceholderText(i18n("Filter..."));
    edit->setClearButtonEnabled(true);
    connect(edit, SIGNAL(textChanged(QString)), this, SIGNAL(filterChanged(QString)));
    if (!m_intialFilter.isEmpty()) {
        edit->setText(m_intialFilter);
    }

    return edit;
}

//END TestViwFilterAction

static const char sessionConfigGroup[] = "TestView";
static const char filterConfigKey[] = "filter";

TestView::TestView(TestViewPlugin* plugin, QWidget* parent)
: QWidget(parent)
, m_plugin(plugin)
, m_tree(new QTreeView(this))
, m_filter(new KRecursiveFilterProxyModel(this))
{
    setWindowIcon(QIcon::fromTheme("preflight-verifier"));
    setWindowTitle(i18n("Unit Tests"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(m_tree);

    m_tree->setSortingEnabled(true);
    m_tree->header()->hide();
    m_tree->setIndentation(10);
    m_tree->setEditTriggers(QTreeView::NoEditTriggers);
    m_tree->setSelectionBehavior(QTreeView::SelectRows);
    m_tree->setSelectionMode(QTreeView::SingleSelection);
    m_tree->setExpandsOnDoubleClick(false);
    m_tree->sortByColumn(0, Qt::AscendingOrder);
    connect(m_tree, SIGNAL(activated(QModelIndex)), SLOT(doubleClicked(QModelIndex)));

    m_model = new QStandardItemModel(this);
    m_filter->setSourceModel(m_model);
    m_tree->setModel(m_filter);

    QAction* showSource = new QAction( QIcon::fromTheme("code-context"), i18n("Show Source"), this );
    connect (showSource, SIGNAL(triggered(bool)), SLOT(showSource()));
    m_contextMenuActions << showSource;

    QAction* runSelected = new QAction( QIcon::fromTheme("system-run"), i18n("Run Selected Tests"), this );
    connect (runSelected, SIGNAL(triggered(bool)), SLOT(runSelectedTests()));
    m_contextMenuActions << runSelected;

    addAction(plugin->actionCollection()->action("run_all_tests"));
    addAction(plugin->actionCollection()->action("stop_running_tests"));

    QString filterText;
    KConfigGroup config(ICore::self()->activeSession()->config(), sessionConfigGroup);
    if (config.hasKey(filterConfigKey))
    {
        filterText = config.readEntry(filterConfigKey, QString());
    }

    TestViewFilterAction* filterAction = new TestViewFilterAction(filterText, this);
    connect(filterAction, SIGNAL(filterChanged(QString)),
            m_filter, SLOT(setFilterFixedString(QString)));
    addAction(filterAction);

    IProjectController* pc = ICore::self()->projectController();
    connect (pc, SIGNAL(projectClosed(KDevelop::IProject*)), SLOT(removeProject(KDevelop::IProject*)));

    ITestController* tc = ICore::self()->testController();
    connect (tc, SIGNAL(testSuiteAdded(KDevelop::ITestSuite*)),
             SLOT(addTestSuite(KDevelop::ITestSuite*)));
    connect (tc, SIGNAL(testSuiteRemoved(KDevelop::ITestSuite*)),
             SLOT(removeTestSuite(KDevelop::ITestSuite*)));
    connect (tc, SIGNAL(testRunFinished(KDevelop::ITestSuite*, KDevelop::TestResult)),
             SLOT(updateTestSuite(KDevelop::ITestSuite*, KDevelop::TestResult)));
    connect (tc, SIGNAL(testRunStarted(KDevelop::ITestSuite*, QStringList)),
             SLOT(notifyTestCaseStarted(KDevelop::ITestSuite*, QStringList)));

    foreach (ITestSuite* suite, tc->testSuites())
    {
        addTestSuite(suite);
    }
}

TestView::~TestView()
{

}

void TestView::updateTestSuite(ITestSuite* suite, const TestResult& result)
{
    QStandardItem* item = itemForSuite(suite);
    if (!item)
    {
        return;
    }

    qCDebug(PLUGIN_TESTVIEW) << "Updating test suite" << suite->name();

    item->setIcon(iconForTestResult(result.suiteResult));

    for (int i = 0; i < item->rowCount(); ++i)
    {
        qCDebug(PLUGIN_TESTVIEW) << "Found a test case" << item->child(i)->text();
        QStandardItem* caseItem = item->child(i);
        if (result.testCaseResults.contains(caseItem->text()))
        {
            TestResult::TestCaseResult caseResult = result.testCaseResults.value(caseItem->text(), TestResult::NotRun);
            caseItem->setIcon(iconForTestResult(caseResult));
        }
    }
}

void TestView::notifyTestCaseStarted(ITestSuite* suite, const QStringList& test_cases)
{
    QStandardItem* item = itemForSuite(suite);
    if (!item)
    {
        return;
    }

    qCDebug(PLUGIN_TESTVIEW) << "Notify a test of the suite " << suite->name() << " has started";

    // Global test suite icon
    item->setIcon(QIcon::fromTheme("process-idle"));

    for (int i = 0; i < item->rowCount(); ++i)
    {
        qCDebug(PLUGIN_TESTVIEW) << "Found a test case" << item->child(i)->text();
        QStandardItem* caseItem = item->child(i);
        if (test_cases.contains(caseItem->text()))
        {
            // Each test case icon
            caseItem->setIcon(QIcon::fromTheme("process-idle"));
        }
    }
}


QIcon TestView::iconForTestResult(TestResult::TestCaseResult result)
{
    switch (result)
    {
        case TestResult::NotRun:
            return QIcon::fromTheme("code-function");

        case TestResult::Skipped:
            return QIcon::fromTheme("task-delegate");

        case TestResult::Passed:
            return QIcon::fromTheme("dialog-ok-apply");

        case TestResult::UnexpectedPass:
            // This is a very rare occurrence, so the icon should stand out
            return QIcon::fromTheme("dialog-warning");

        case TestResult::Failed:
            return QIcon::fromTheme("edit-delete");

        case TestResult::ExpectedFail:
            return QIcon::fromTheme("dialog-ok");

        case TestResult::Error:
            return QIcon::fromTheme("dialog-cancel");

        default:
            return QIcon::fromTheme("");
    }
}

QStandardItem* TestView::itemForSuite(ITestSuite* suite)
{
    foreach (QStandardItem* item, m_model->findItems(suite->name(), Qt::MatchRecursive))
    {
        if (item->parent() && item->parent()->text() == suite->project()->name()
            && !item->parent()->parent())
        {
            return item;
        }
    }
    return 0;
}

QStandardItem* TestView::itemForProject(IProject* project)
{
    foreach (QStandardItem* item, m_model->findItems(project->name()))
    {
        return item;
    }

    return addProject(project);
}


void TestView::runSelectedTests()
{
    QModelIndexList indexes = m_tree->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
    {
        return;
    }

    QList<KJob*> jobs;
    ITestController* tc = ICore::self()->testController();

    /*
     * NOTE: If a test suite or a single test case was selected,
     * the job is launched in Verbose mode with raised output window.
     * If a project is selected, it is launched silently.
     *
     * This is the somewhat-intuitive approach. Maybe a configuration should be offered.
     */

    foreach (const QModelIndex& idx, indexes)
    {
        QModelIndex index = m_filter->mapToSource(idx);
        if (index.parent().isValid() && indexes.contains(index.parent()))
        {
            continue;
        }
        QStandardItem* item = m_model->itemFromIndex(index);
        if (item->parent() == 0)
        {
            // A project was selected
            IProject* project = ICore::self()->projectController()->findProjectByName(item->data(ProjectRole).toString());
            foreach (ITestSuite* suite, tc->testSuitesForProject(project))
            {
                jobs << suite->launchAllCases(ITestSuite::Silent);
            }
        }
        else if (item->parent()->parent() == 0)
        {
            // A suite was selected
            IProject* project = ICore::self()->projectController()->findProjectByName(item->parent()->data(ProjectRole).toString());
            ITestSuite* suite =  tc->findTestSuite(project, item->data(SuiteRole).toString());
            jobs << suite->launchAllCases(ITestSuite::Verbose);
        }
        else
        {
            // This was a single test case
            IProject* project = ICore::self()->projectController()->findProjectByName(item->parent()->parent()->data(ProjectRole).toString());
            ITestSuite* suite =  tc->findTestSuite(project, item->parent()->data(SuiteRole).toString());
            const QString testCase = item->data(CaseRole).toString();
            jobs << suite->launchCase(testCase, ITestSuite::Verbose);
        }
    }

    if (!jobs.isEmpty())
    {
        KDevelop::ExecuteCompositeJob* compositeJob = new KDevelop::ExecuteCompositeJob(this, jobs);
        compositeJob->setObjectName(i18np("Run 1 test", "Run %1 tests", jobs.size()));
        compositeJob->setProperty("test_job", true);
        ICore::self()->runController()->registerJob(compositeJob);
    }
}

void TestView::showSource()
{
    QModelIndexList indexes = m_tree->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
    {
        return;
    }

    IndexedDeclaration declaration;
    ITestController* tc = ICore::self()->testController();

    QModelIndex index = m_filter->mapToSource(indexes.first());
    QStandardItem* item = m_model->itemFromIndex(index);
    if (item->parent() == 0)
    {
        // No sense in finding source code for projects.
        return;
    }
    else if (item->parent()->parent() == 0)
    {
        IProject* project = ICore::self()->projectController()->findProjectByName(item->parent()->data(ProjectRole).toString());
        ITestSuite* suite =  tc->findTestSuite(project, item->data(SuiteRole).toString());
        declaration = suite->declaration();
    }
    else
    {
        IProject* project = ICore::self()->projectController()->findProjectByName(item->parent()->parent()->data(ProjectRole).toString());
        ITestSuite* suite =  tc->findTestSuite(project, item->parent()->data(SuiteRole).toString());
        declaration = suite->caseDeclaration(item->data(CaseRole).toString());
    }

    DUChainReadLocker locker(DUChain::lock());
    Declaration* d = declaration.data();
    if (!d)
    {
        return;
    }

    QUrl url = d->url().toUrl();
    KTextEditor::Cursor cursor = d->rangeInCurrentRevision().start();
    locker.unlock();

    IDocumentController* dc = ICore::self()->documentController();
    qCDebug(PLUGIN_TESTVIEW) << "Activating declaration in" << url;
    dc->openDocument(url, cursor);
}

void TestView::addTestSuite(ITestSuite* suite)
{
    QStandardItem* projectItem = itemForProject(suite->project());
    Q_ASSERT(projectItem);

    QStandardItem* suiteItem = new QStandardItem(QIcon::fromTheme("view-list-tree"), suite->name());

    suiteItem->setData(suite->name(), SuiteRole);
    foreach (QString caseName, suite->cases())
    {
        QStandardItem* caseItem = new QStandardItem(iconForTestResult(TestResult::NotRun), caseName);
        caseItem->setData(caseName, CaseRole);
        suiteItem->appendRow(caseItem);
    }
    projectItem->appendRow(suiteItem);
}

void TestView::removeTestSuite(ITestSuite* suite)
{
    QStandardItem* item = itemForSuite(suite);
    item->parent()->removeRow(item->row());
}

QStandardItem* TestView::addProject(IProject* project)
{
    QStandardItem* projectItem = new QStandardItem(QIcon::fromTheme("project-development"), project->name());
    projectItem->setData(project->name(), ProjectRole);
    m_model->appendRow(projectItem);
    return projectItem;
}

void TestView::removeProject(IProject* project)
{
    QStandardItem* projectItem = itemForProject(project);
    m_model->removeRow(projectItem->row());
}

void TestView::doubleClicked(const QModelIndex& index)
{
    m_tree->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    runSelectedTests();
}

QList< QAction* > TestView::contextMenuActions()
{
    return m_contextMenuActions;
}

