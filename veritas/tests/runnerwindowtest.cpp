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
#include "modelcreation.h"

#include <runnerwindow.h>
#include <runnermodel.h>
#include <runnerproxymodel.h>
#include <qtest_kde.h>
#include <kasserts.h>

using Veritas::RunnerWindow;
using Veritas::Test;
using Veritas::ut::TestStub;
using Veritas::ut::RunnerModelStub;
using Veritas::ut::createRunnerModelStub;
using Veritas::it::RunnerWindowTest;

// fixture
void RunnerWindowTest::init()
{
    model = createRunnerModelStub(false);
    model->fill2();
    window = new RunnerWindow();
    window->setModel(model);
    window->show();
    status = window->statusWidget();
    m_proxy = window->runnerProxyModel();
    m_view = window->runnerView();
}

// fxiture
void RunnerWindowTest::cleanup()
{
    window->ui().actionExit->trigger();
    if (window) delete window;
    if (model) delete model;
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
    return item->selected();
}

// helper
void RunnerWindowTest::selectSome()
{
    model->item1->setSelected(true);
    model->child12->setSelected(false);
    model->item2->setSelected(false);
    model->child21->setSelected(true);
}

// command
void RunnerWindowTest::selectAll()
{
    selectSome();
    window->ui().actionSelectAll->trigger();
    QTest::qWait(100);
    checkAllItems(&RunnerWindowTest::isSelected);
}

// helper
bool RunnerWindowTest::isNotSelected(TestStub* item)
{
    return !item->selected();
}

// command
void RunnerWindowTest::unselectAll()
{
    selectSome();
    window->ui().actionUnselectAll->trigger();
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
    window->ui().actionExpandAll->trigger();
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
    window->ui().actionCollapseAll->trigger();
    QTest::qWait(100);
    KVERIFY( isCollapsed(model->item1) );
    KVERIFY( isCollapsed(model->item2) );
}

// command
void RunnerWindowTest::startItems()
{
    TestStub::executedItems.clear();

    // invoke the run action
    window->ui().actionStart->trigger();

    // wait for all items to be executed
    if (!QTest::kWaitForSignal(window->runnerModel(), SIGNAL(allItemsCompleted()), 2000))
        QFAIL("Timeout while waiting for runner items to complete execution");

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

    // validate the status widget
    KOMPARE(QString("3"), status->labelNumTotal->text());
    KOMPARE(QString("3"), status->labelNumSelected->text());
    KOMPARE(QString("3"), status->labelNumRun->text());

    // TODO append ': ' to the labelNumXText instead
    KOMPARE(QString(": 3"), status->labelNumSuccess->text());
    KOMPARE(QString(": 0"), status->labelNumExceptions->text());
    KOMPARE(QString(": 0"), status->labelNumFatals->text());
    KOMPARE(QString(": 0"), status->labelNumErrors->text());
    KOMPARE(QString(": 0"), status->labelNumWarnings->text());
}

// command
void RunnerWindowTest::deselectItems()
{
    // select only one of the runner items
    // validate that the other one didn't get executed
    KTODO;
}

// command
void RunnerWindowTest::newModel()
{
    // init() has loaded a model, now replace
    // it with another one, with different items
    RunnerModelStub* model = createRunnerModelStub(false);
    model->fill1();
    window->setModel(model);
    window->show();

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


QTEST_KDEMAIN(RunnerWindowTest, GUI)
