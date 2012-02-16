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
#include <interfaces/iruncontroller.h>

using namespace KDevelop;

TestView::TestView(TestViewPlugin* plugin, QWidget* parent): QTreeView(parent)
, m_plugin(plugin)
{
    setWindowIcon(KIcon("preflight-verifier"));
    setIndentation(10);
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    buildTestModel();
    
    QAction* action;

    KAction* reloadAction = new KAction( KIcon("view-refresh"), i18n("Reload"), this );
    connect (reloadAction, SIGNAL(triggered(bool)), SLOT(reloadTests()));
    addAction(reloadAction);
    
    KAction* runSelected = new KAction( KIcon("system-run"), i18n("Run selected"), this );
    addAction(runSelected);
    
    action = plugin->actionCollection()->action("run_all_tests");
    addAction(action);
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
        foreach (ITestSuite* suite, tc->testSuitesForProject(project))
        {
            QStandardItem* suiteItem = new QStandardItem(KIcon("preflight-verifier"), suite->name());
            foreach (QString caseName, suite->cases())
            {
                QStandardItem* caseItem = new QStandardItem(caseName);
                suiteItem->appendRow(caseItem);
            }
            projectItem->appendRow(suiteItem);
        }
        m_model->appendRow(projectItem);
    }
}


