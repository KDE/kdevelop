/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "testview.h"
#include "testviewplugin.h"
#include <debug.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/isession.h>

#include <util/executecompositejob.h>
#include <util/wildcardhelpers.h>

#include <language/duchain/indexeddeclaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>

#include <KActionCollection>
#include <KJob>
#include <KLocalizedString>

#include <QAction>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QSortFilterProxyModel>

using namespace KDevelop;

enum CustomRoles {
    ProjectRole = Qt::UserRole + 1,
    SuiteRole,
    CaseRole
};

TestView::TestView(TestViewPlugin* plugin, QWidget* parent)
    : QWidget(parent)
    , m_plugin(plugin)
    , m_tree(new QTreeView(this))
    , m_filter(new QSortFilterProxyModel(this))
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("preflight-verifier"), windowIcon()));
    setWindowTitle(i18nc("@title:window", "Unit Tests"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(m_tree);

    m_tree->setSortingEnabled(true);
    m_tree->header()->hide();
    m_tree->setIndentation(10);
    m_tree->setEditTriggers(QTreeView::NoEditTriggers);
    m_tree->setSelectionBehavior(QTreeView::SelectRows);
    m_tree->setSelectionMode(QTreeView::ExtendedSelection);
    m_tree->setExpandsOnDoubleClick(false);
    m_tree->sortByColumn(0, Qt::AscendingOrder);
    connect(m_tree, &QTreeView::doubleClicked, this, &TestView::doubleClicked);

    m_model = new QStandardItemModel(this);
    m_filter->setRecursiveFilteringEnabled(true);
    m_filter->setSourceModel(m_model);
    m_tree->setModel(m_filter);

    auto* showSource = new QAction( QIcon::fromTheme(QStringLiteral("code-context")), i18nc("@action:inmenu", "Show Source"), this );
    connect (showSource, &QAction::triggered, this, &TestView::showSource);
    m_contextMenuActions << showSource;

    addAction(plugin->actionCollection()->action(QStringLiteral("run_all_tests")));
    addAction(plugin->actionCollection()->action(QStringLiteral("stop_running_tests")));

    auto* runSelected = new QAction( QIcon::fromTheme(QStringLiteral("system-run")), i18nc("@action", "Run Selected Tests"), this );
    connect (runSelected, &QAction::triggered, this, &TestView::runSelectedTests);
    addAction(runSelected);

    auto* edit = new QLineEdit(parent);
    edit->setPlaceholderText(i18nc("@info:placeholder", "Filter..."));
    edit->setClearButtonEnabled(true);
    auto* widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(edit);
    connect(edit, &QLineEdit::textChanged, this, &TestView::changeFilter);
    addAction(widgetAction);

    setFocusProxy(edit);

    IProjectController* pc = ICore::self()->projectController();
    connect (pc, &IProjectController::projectClosed, this, &TestView::removeProject);

    ITestController* tc = ICore::self()->testController();
    connect (tc, &ITestController::testSuiteAdded,
             this, &TestView::addTestSuite);
    connect (tc, &ITestController::testSuiteRemoved,
             this, &TestView::removeTestSuite);
    connect (tc, &ITestController::testRunFinished,
             this, &TestView::updateTestSuite);
    connect (tc, &ITestController::testRunStarted,
             this, &TestView::notifyTestCaseStarted);

    const auto suites = tc->testSuites();
    for (ITestSuite* suite : suites) {
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
        const auto resultIt = result.testCaseResults.constFind(caseItem->text());
        if (resultIt != result.testCaseResults.constEnd()) {
            caseItem->setIcon(iconForTestResult(*resultIt));
        }
    }
}

void TestView::changeFilter(const QString &newFilter)
{
    WildcardHelpers::setFilterNonPathWildcard(*m_filter, newFilter);
    if (newFilter.isEmpty()) {
        m_tree->collapseAll();
    } else {
        m_tree->expandAll();
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
    item->setIcon(QIcon::fromTheme(QStringLiteral("process-idle")));

    for (int i = 0; i < item->rowCount(); ++i)
    {
        qCDebug(PLUGIN_TESTVIEW) << "Found a test case" << item->child(i)->text();
        QStandardItem* caseItem = item->child(i);
        if (test_cases.contains(caseItem->text()))
        {
            // Each test case icon
            caseItem->setIcon(QIcon::fromTheme(QStringLiteral("process-idle")));
        }
    }
}


QIcon TestView::iconForTestResult(TestResult::TestCaseResult result)
{
    switch (result)
    {
        case TestResult::NotRun:
            return QIcon::fromTheme(QStringLiteral("code-function"));

        case TestResult::Skipped:
            return QIcon::fromTheme(QStringLiteral("task-delegate"));

        case TestResult::Passed:
            return QIcon::fromTheme(QStringLiteral("dialog-ok-apply"));

        case TestResult::UnexpectedPass:
            // This is a very rare occurrence, so the icon should stand out
            return QIcon::fromTheme(QStringLiteral("dialog-warning"));

        case TestResult::Failed:
            return QIcon::fromTheme(QStringLiteral("edit-delete"));

        case TestResult::ExpectedFail:
            return QIcon::fromTheme(QStringLiteral("dialog-ok"));

        case TestResult::Error:
            return QIcon::fromTheme(QStringLiteral("dialog-cancel"));
    }
    Q_UNREACHABLE();
}

QStandardItem* TestView::itemForSuite(ITestSuite* suite)
{
    const auto items = m_model->findItems(suite->name(), Qt::MatchRecursive);
    auto it = std::find_if(items.begin(), items.end(), [&](QStandardItem* item) {
        return (item->parent() && item->parent()->text() == suite->project()->name()
                && !item->parent()->parent());
    });
    return (it != items.end()) ? *it : nullptr;
}

QStandardItem* TestView::itemForProject(IProject* project)
{
    QList<QStandardItem*> itemsForProject = m_model->findItems(project->name());
    if (!itemsForProject.isEmpty()) {
        return itemsForProject.first();
    }
    return addProject(project);
}


void TestView::runSelectedTests()
{
    QModelIndexList indexes = m_tree->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
    {
        //if there's no selection we'll run all of them (or only the filtered)
        //in case there's a filter.
        const int rc = m_filter->rowCount();
        indexes.reserve(rc);
        for(int i=0; i<rc; ++i) {
            indexes << m_filter->index(i, 0);
        }
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

    for (const QModelIndex& idx : std::as_const(indexes)) {
        QModelIndex index = m_filter->mapToSource(idx);
        if (index.parent().isValid() && indexes.contains(index.parent()))
        {
            continue;
        }
        QStandardItem* item = m_model->itemFromIndex(index);
        if (item->parent() == nullptr)
        {
            // A project was selected
            IProject* project = ICore::self()->projectController()->findProjectByName(item->data(ProjectRole).toString());
            const auto suites = tc->testSuitesForProject(project);
            for (ITestSuite* suite : suites) {
                jobs << suite->launchAllCases(ITestSuite::Silent);
            }
        }
        else if (item->parent()->parent() == nullptr)
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
        auto* compositeJob = new KDevelop::ExecuteCompositeJob(this, jobs);
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
    if (item->parent() == nullptr)
    {
        // No sense in finding source code for projects.
        return;
    }
    else if (item->parent()->parent() == nullptr)
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

    DUChainReadLocker locker;
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

    auto* suiteItem = new QStandardItem(QIcon::fromTheme(QStringLiteral("view-list-tree")), suite->name());

    suiteItem->setData(suite->name(), SuiteRole);
    const auto caseNames = suite->cases();
    for (const QString& caseName : caseNames) {
        auto* caseItem = new QStandardItem(iconForTestResult(TestResult::NotRun), caseName);
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
    auto* projectItem = new QStandardItem(QIcon::fromTheme(QStringLiteral("project-development")), project->name());
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

#include "moc_testview.cpp"
