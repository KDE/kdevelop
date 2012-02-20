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

#include "interfaces/icore.h"
#include "interfaces/iproject.h"
#include "interfaces/iplugincontroller.h"
#include "interfaces/iprojectcontroller.h"
#include <interfaces/itestcontroller.h>
#include <interfaces/itestsuite.h>

#include <KIcon>
#include <KActionCollection>
#include <KAction>

#include <QtGui/QStandardItemModel>
#include <QtGui/QStandardItem>
#include <KJob>
#include <KDebug>
#include <interfaces/iruncontroller.h>
#include <util/executecompositejob.h>

using namespace KDevelop;

const int ProjectRole = Qt::UserRole + 1;
const int SuiteRole = Qt::UserRole + 2;
const int CaseRole = Qt::UserRole + 3;

TestView::TestView(TestViewPlugin* plugin, QWidget* parent): QTreeView(parent)
, m_plugin(plugin)
{
    setWindowIcon(KIcon("preflight-verifier"));
    setIndentation(10);
    setEditTriggers(NoEditTriggers);
    setSelectionBehavior(SelectRows);
    setSelectionMode(MultiSelection);
    
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    buildTestModel();
    
    QAction* action;

    KAction* reloadAction = new KAction( KIcon("view-refresh"), i18n("Reload"), this );
    connect (reloadAction, SIGNAL(triggered(bool)), SLOT(reloadTests()));
    addAction(reloadAction);
    
    KAction* runSelected = new KAction( KIcon("system-run"), i18n("Run selected tests"), this );
    connect (runSelected, SIGNAL(triggered(bool)), SLOT(runSelectedTests()));
    addAction(runSelected);
    
    action = plugin->actionCollection()->action("run_all_tests");
    addAction(action);
    
    connect (ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController"), SIGNAL(testRunFinished(KDevelop::ITestSuite*)), SLOT(updateTestSuite(KDevelop::ITestSuite*)));
}

TestView::~TestView()
{

}

void TestView::reloadTests()
{
    ITestController* tc = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<ITestController>();
    KJob* reloadJob = tc->reloadTestSuites();
    if (reloadJob)
    {
        connect (reloadJob, SIGNAL(finished(KJob*)), SLOT(buildTestModel()));
        ICore::self()->runController()->registerJob(reloadJob);
    }
}


void TestView::buildTestModel()
{
    m_model->clear();
    ITestController* tc = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<ITestController>();
    foreach (IProject* project, ICore::self()->projectController()->projects())
    {
        QStandardItem* projectItem = new QStandardItem(KIcon("project-development"), project->name());
        projectItem->setData(project->name(), ProjectRole);
        foreach (ITestSuite* suite, tc->testSuitesForProject(project))
        {
            QStandardItem* suiteItem = new QStandardItem(KIcon("preflight-verifier"), suite->name());
            suiteItem->setData(suite->url(), SuiteRole);
            foreach (QString caseName, suite->cases())
            {
                QStandardItem* caseItem = new QStandardItem(caseName);
                caseItem->setData(caseName, CaseRole);
                suiteItem->appendRow(caseItem);
            }
            projectItem->appendRow(suiteItem);
        }
        m_model->appendRow(projectItem);
    }
}

void TestView::updateTestSuite(ITestSuite* suite)
{
    kDebug() << "Updating test suite" << suite->name();
    foreach (QStandardItem* item, m_model->findItems(suite->name(), Qt::MatchRecursive))
    {
        kDebug() << "Found matching item" << item->text();
        if (item->parent() && item->parent()->text() == suite->project()->name() && !item->parent()->parent())
        {
            kDebug() << "Item is really a suite";
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
    }
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

void TestView::runSelectedTests()
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty())
    {
        return;
    }
        
    QList<KJob*> jobs;
    ITestController* tc = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ITestController")->extension<ITestController>();
    
    foreach (const QModelIndex& index, indexes)
    {
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
            ITestSuite* suite =  tc->testSuiteForUrl(item->data(SuiteRole).value<KUrl>());
            jobs << suite->launchAllCases();
        }
        else
        {
            // This was a single test case
            ITestSuite* suite = tc->testSuiteForUrl(item->parent()->data(SuiteRole).value<KUrl>());
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


