/* KDevelop xUnit plugin
 *
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

#include "runnerwindowtest.h"

#include <qtest_kde.h>
#include <KDebug>
#include <kasserts.h>

#include "modelcreation.h"

#include "../mvc/runnerproxymodel.h"
#include "../mvc/runnerwindow.h"
#include "../mvc/runnermodel.h"
#include "../mvc/resultsproxymodel.h"

using Veritas::RunnerWindow;
using Veritas::Test;
using Veritas::ResultsModel;
using Veritas::ResultsProxyModel;

using Veritas::TestStub;
using Veritas::RunnerModelStub;
using Veritas::createRunnerModelStub;
using Veritas::RunnerWindowTest;

// fixture
void RunnerWindowTest::init()
{
    model = createRunnerModelStub(false);
    model->fill2();
    QStringList resultHeaders;
    resultHeaders << i18n("Test Name") << i18n("Result") << i18n("Message")
                  << i18n("File Name") << i18n("Line Number");
    window = new RunnerWindow(new ResultsModel(resultHeaders));
    window->setModel(model);
    //window->show();
    m_ui = window->ui();
    m_proxy = window->runnerProxyModel();
    m_view = window->runnerView();
    m_resultsProxy = window->resultsProxyModel();
    TestStub::executedItems.clear();
}

// fxiture
void RunnerWindowTest::cleanup()
{
    m_ui->actionExit->trigger();
    if (window) delete window;
}

// helper
void RunnerWindowTest::checkAllItems(checkMemberFun f)
{
    KVERIFY( (this->*f)(model->item1) );
    KVERIFY( (this->*f)(model->child11) );
    KVERIFY( (this->*f)(model->child12) );
    KVERIFY( (this->*f)(model->item2) );
    KVERIFY( (this->*f)(model->child21) );
}

// helper
bool RunnerWindowTest::isSelected(TestStub* item)
{
    return item->isChecked();
}

// helper
void RunnerWindowTest::selectSome()
{
    model->item1->check();
    model->child11->check();
    model->child12->unCheck();
    model->item2->unCheck();
    model->child21->check();
}

// command
void RunnerWindowTest::selectAll()
{
    selectSome();
    model->countItems();
    QTest::qWait(100);
    m_ui->actionSelectAll->trigger();
    QTest::qWait(100);
    checkAllItems(&RunnerWindowTest::isSelected);
}

// helper
bool RunnerWindowTest::isNotSelected(TestStub* item)
{
    return !item->isChecked();
}

// command
void RunnerWindowTest::unselectAll()
{
    selectSome();
    m_ui->actionUnselectAll->trigger();
    QTest::qWait(100);
    checkAllItems(&RunnerWindowTest::isNotSelected);
}

// helper
bool RunnerWindowTest::isExpanded(TestStub* item)
{
    QModelIndex i = m_proxy->mapFromSource(item->index());
    return m_view->isExpanded(i);
}

// helper
void RunnerWindowTest::expandSome()
{
    m_view->expand(m_proxy->index(0,0));
}

// command
void RunnerWindowTest::expandAll()
{
    expandSome();
    m_ui->actionExpandAll->trigger();
    QTest::qWait(100);
    KVERIFY( isExpanded(model->item1) );
    KVERIFY( isExpanded(model->item2) );
}

// helper
bool RunnerWindowTest::isCollapsed(TestStub* item)
{
    QModelIndex i = m_proxy->mapFromSource(item->index());
    return !m_view->isExpanded(i);
}

// command
void RunnerWindowTest::collapseAll()
{
    expandSome();
    m_ui->actionCollapseAll->trigger();
    QTest::qWait(100);
    KVERIFY( isCollapsed(model->item1) );
    KVERIFY( isCollapsed(model->item2) );
}

// command
void RunnerWindowTest::startItems()
{
    runAllTests();

    // check they got indeed executed
    // the rows of items that got executed are stored
    // in the stub
    QCOMPARE(0, TestStub::executedItems.value(0));
    QCOMPARE(1, TestStub::executedItems.value(1));
    QCOMPARE(0, TestStub::executedItems.value(2));

    // validate the test content
    QCOMPARE(Veritas::RunSuccess, model->child11->state());
    QCOMPARE(Veritas::RunSuccess, model->child12->state());
    QCOMPARE(Veritas::RunSuccess, model->child21->state());
}

// command
void RunnerWindowTest::deselectItems()
{
    // select only one of the runner items
    // validate that the other one didn't get executed
    TDD_TODO;
}

// command
void RunnerWindowTest::newModel()
{
    // init() has loaded a model, now replace
    // it with another one, with different items
    RunnerModelStub* model = createRunnerModelStub(false);
    model->fill1();
    window->setModel(model);
    //window->show();

    // it should now contain 2 top level items without
    // children. since thats what model->fill1() does.
    RunnerModel* actual = window->runnerModel();
    KOMPARE(model, actual);
    QModelIndex c1 = actual->index(0,0);
    KVERIFY(c1.isValid());
    KOMPARE("00", actual->data(c1));
    KVERIFY(!c1.child(0,0).isValid());

    QModelIndex c2 = actual->index(1,0);
    KVERIFY(c2.isValid());
    KOMPARE("10", actual->data(c2));
    KVERIFY(!c2.child(0,0).isValid());

    KVERIFY(!actual->index(2,0).isValid());
}

// helper
void RunnerWindowTest::runAllTests()
{
    // invoke the run action
    m_ui->actionStart->trigger();
    //window->show();

    // de-comment the line below to inspect the window manually
    //QTest::qWait(5000);
}

// helper
void RunnerWindowTest::assertResultItemEquals(const QModelIndex& i, const QString& content)
{
    QTreeView* resv = window->resultsView();

    KOMPARE(content, m_resultsProxy->data(i, Qt::DisplayRole));
    KVERIFY(i.isValid());
    KVERIFY(!resv->isFirstColumnSpanned(i.row(),QModelIndex()));
}

// debug helper
void RunnerWindowTest::printModel(const QModelIndex& mi, int lvl)
{
    QModelIndex i = mi;
    char space[512];
    for (int j=0; j<2*lvl; j++) {
        space[j] = '+';
    }
    space[2*lvl] = 0x0;

    while (i.isValid()) {
        kDebug() << space << i.row()
                 << i.model()->data(i, Qt::DisplayRole).toString();
        if (i.child(0,0).isValid()) {
            printModel(i.child(0,0), lvl+1);
        }
        i = i.sibling(i.row()+1, 0);
    }
}

#define PRINT_MODEL(mdl) printModel(mdl->index(0,0), 0)

// command
void RunnerWindowTest::clickRunnerResults()
{
    model->child11->m_state = Veritas::RunError;
    model->child21->m_state = Veritas::RunError;

    runAllTests();

    // fake a click on  the second root test.
    // this is expected to filter all but the result of
    // child21.
    // r0
    //   -child10
    //   -child11 [failed]
    // r1 <- clicked
    //   -child21 [failed]
    QModelIndex i = m_proxy->index(1,0);
    m_view->selectionModel()->select(i, QItemSelectionModel::Select);

    // since the 2nd item in the runnertree was set to fail,
    // and we selected it's parent this should be the only
    // item currently visible in the resultsview.
    QModelIndex result21 = m_resultsProxy->index(0,0);
    KVERIFY_MSG(result21.isValid(), 
        "Was expecting to find something in the resultsview, "
        "however it is empty (filtered).");
    //QTest::qWait(5000);
    KVERIFY_MSG(!m_resultsProxy->index(1,0).isValid(),
        "Resultsview should contain only a single item.");
    KOMPARE("child21", m_resultsProxy->data(result21));

}

// command
void RunnerWindowTest::progressBarMaxedOutAfterRun()
{
    runAllTests();
    QProgressBar* bar = window->ui()->progressRun;
    KOMPARE(bar->maximum(), bar->value());
}

QTEST_KDEMAIN(RunnerWindowTest, GUI)
