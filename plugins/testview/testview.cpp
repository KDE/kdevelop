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

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>

#include <util/executecompositejob.h>

#include <language/duchain/indexeddeclaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>

#include <KIcon>
#include <KActionCollection>
#include <KAction>
#include <KLocalizedString>
#include <KJob>
#include <KDebug>
#include <KFilterProxySearchLine>
#include <krecursivefilterproxymodel.h>

#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

using namespace KDevelop;

enum CustomRoles {
    ProjectRole = Qt::UserRole + 1,
    SuiteRole,
    CaseRole
};

TestView::TestView(TestViewPlugin* plugin, QWidget* parent)
: QWidget(parent)
, m_plugin(plugin)
, m_model(new QStandardItemModel(this))
, m_tree(new QTreeView(this))
, m_filter(new KRecursiveFilterProxyModel(this))
{
    setWindowIcon(KIcon("preflight-verifier"));

    KFilterProxySearchLine* filterLine = new KFilterProxySearchLine(this);
    filterLine->setProxy(m_filter);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(filterLine);
    layout->addWidget(m_tree);

    m_tree->header()->hide();
    m_tree->setIndentation(10);
    m_tree->setEditTriggers(QTreeView::NoEditTriggers);
    m_tree->setSelectionBehavior(QTreeView::SelectRows);
    m_tree->setSelectionMode(QTreeView::SingleSelection);
    m_tree->setExpandsOnDoubleClick(false);
    connect(m_tree, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(doubleClicked(QModelIndex)));

    m_model = new QStandardItemModel(this);
    m_filter->setSourceModel(m_model);
    m_tree->setModel(m_filter);

    QAction* action;

    KAction* reloadAction = new KAction( KIcon("view-refresh"), i18n("Reload"), this );
    connect (reloadAction, SIGNAL(triggered(bool)), SLOT(reloadTests()));
    addAction(reloadAction);

    KAction* showSource = new KAction( KIcon("code-context"), i18n("Show source"), this );
    connect (showSource, SIGNAL(triggered(bool)), SLOT(showSource()));
    addAction(showSource);

    KAction* runSelected = new KAction( KIcon("system-run"), i18n("Run selected tests"), this );
    connect (runSelected, SIGNAL(triggered(bool)), SLOT(runSelectedTests()));
    addAction(runSelected);

    action = plugin->actionCollection()->action("run_all_tests");
    addAction(action);

    IProjectController* pc = ICore::self()->projectController();
    connect (pc, SIGNAL(projectClosed(KDevelop::IProject*)), SLOT(removeProject(KDevelop::IProject*)));

    ITestController* tc = ICore::self()->testController();
    connect (tc, SIGNAL(testSuiteAdded(KDevelop::ITestSuite*)),
             SLOT(addTestSuite(KDevelop::ITestSuite*)));
    connect (tc, SIGNAL(testSuiteRemoved(KDevelop::ITestSuite*)),
             SLOT(removeTestSuite(KDevelop::ITestSuite*)));
    connect (tc, SIGNAL(testRunFinished(KDevelop::ITestSuite*)),
             SLOT(updateTestSuite(KDevelop::ITestSuite*)));

    foreach (IProject* project, pc->projects())
    {
        addProject(project);
    }

    foreach (ITestSuite* suite, tc->testSuites())
    {
        addTestSuite(suite);
    }
}

TestView::~TestView()
{

}

void TestView::reloadTests()
{
    ITestController* tc = ICore::self()->testController();
    KJob* reloadJob = tc->reloadTestSuites();
    if (reloadJob)
    {
        ICore::self()->runController()->registerJob(reloadJob);
    }
}

void TestView::updateTestSuite(ITestSuite* suite)
{
    QStandardItem* item = itemForSuite(suite);
    if (!item)
    {
        return;
    }

    kDebug() << "Updating test suite" << suite->name();
    TestResult result = suite->result();
    bool failed = false;
    bool passed = false;
    for (int i = 0; i < item->rowCount(); ++i)
    {
        kDebug() << "Found a test case" << item->child(i)->text();
        QStandardItem* caseItem = item->child(i);
        if (result.testCaseResults.contains(caseItem->text()))
        {
            TestResult::TestCaseResult caseResult = result.testCaseResults.value(caseItem->text(), TestResult::NotRun);
            failed |= (caseResult == TestResult::Failed);
            passed |= (caseResult == TestResult::Passed);
            caseItem->setIcon(iconForTestResult(caseResult));
        }
    }
    item->setIcon( failed ? KIcon("dialog-error") : KIcon("dialog-ok-apply") );
}

KIcon TestView::iconForTestResult(TestResult::TestCaseResult result)
{
    kDebug() << result;
    switch (result)
    {
        case TestResult::NotRun:
            return KIcon();

        case TestResult::Skipped:
            return KIcon();

        case TestResult::Passed:
            return KIcon("dialog-ok");

        case TestResult::Failed:
            return KIcon("edit-delete");

        default:
            return KIcon();
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
                jobs << suite->launchAllCases();
            }
        }
        else if (item->parent()->parent() == 0)
        {
            // A suite was selected
            IProject* project = ICore::self()->projectController()->findProjectByName(item->parent()->data(ProjectRole).toString());
            ITestSuite* suite =  tc->findTestSuite(project, item->data(SuiteRole).toString());
            jobs << suite->launchAllCases();
        }
        else
        {
            // This was a single test case
            IProject* project = ICore::self()->projectController()->findProjectByName(item->parent()->parent()->data(ProjectRole).toString());
            ITestSuite* suite =  tc->findTestSuite(project, item->parent()->data(SuiteRole).toString());
            const QString testCase = item->data(CaseRole).toString();
            jobs << suite->launchCase(testCase);
        }
    }

    if (!jobs.isEmpty())
    {
        KDevelop::ExecuteCompositeJob* compositeJob = new KDevelop::ExecuteCompositeJob(this, jobs);
        compositeJob->setObjectName(i18np("Run 1 test", "Run %1 tests", jobs.size()));
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

    KUrl url = d->url().toUrl();
    KTextEditor::Cursor cursor = d->rangeInCurrentRevision().textRange().start();
    locker.unlock();

    IDocumentController* dc = ICore::self()->documentController();
    kDebug() << "Activating declaration in" << url;
    dc->openDocument(url, cursor);
}

void TestView::addTestSuite(ITestSuite* suite)
{
    QStandardItem* projectItem = itemForProject(suite->project());
    Q_ASSERT(projectItem);

    QStandardItem* suiteItem = new QStandardItem(KIcon("view-list-tree"), suite->name());

    suiteItem->setData(suite->name(), SuiteRole);
    foreach (QString caseName, suite->cases())
    {
        QStandardItem* caseItem = new QStandardItem(caseName);
        caseItem->setData(caseName, CaseRole);
        suiteItem->appendRow(caseItem);
    }
    projectItem->appendRow(suiteItem);
}

void TestView::removeTestSuite(ITestSuite* suite)
{
    delete itemForSuite(suite);
}

QStandardItem* TestView::addProject(IProject* project)
{
    QStandardItem* projectItem = new QStandardItem(KIcon("project-development"), project->name());
    projectItem->setData(project->name(), ProjectRole);
    m_model->appendRow(projectItem);
    return projectItem;
}

void TestView::removeProject(IProject* project)
{
    delete itemForProject(project);
}

void TestView::doubleClicked(const QModelIndex& index)
{
    m_tree->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    runSelectedTests();
}
