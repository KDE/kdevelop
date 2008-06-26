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
#include <qtest_kde.h>
#include <kasserts.h>

using Veritas::RunnerWindow;
using Veritas::Test;
using Veritas::ut::TestStub;
using Veritas::ut::RunnerModelStub;
using Veritas::ut::createRunnerModelStub;
using Veritas::it::RunnerWindowTest;

void RunnerWindowTest::initTestCase()
{
    model = createRunnerModelStub();
    window = new RunnerWindow();
    window->setModel(model);
    window->show();
    status = window->statusWidget();
}

void RunnerWindowTest::cleanupTestCase()
{
    window->ui().actionExit->trigger();
    if (window) delete window;
    if (model) delete model;
}

void RunnerWindowTest::startItems()
{
    TestStub::executedItems.clear();

    // invoke the run action
    window->ui().actionStart->trigger();

    // wait for all items to be executed
    if (!QTest::kWaitForSignal(window->runnerModel(), SIGNAL(allItemsCompleted()), 2000))
        QFAIL("Timeout while waiting for runner items to complete execution");

    // check they got indeed executed
    QCOMPARE(0, TestStub::executedItems.value(0));
    QCOMPARE(1, TestStub::executedItems.value(1));

    // validate the test content
    assertTestEquals(0, "00", "0_1", "0_2", Veritas::RunSuccess);
    assertTestEquals(1, "10", "1_1", "1_2", Veritas::RunSuccess);

    // validate the status widget
    KOMPARE(QString("2"), status->labelNumTotal->text());
    KOMPARE(QString("2"), status->labelNumSelected->text());
    KOMPARE(QString("2"), status->labelNumRun->text());

    // TODO append ': ' to the labelNumXText instead
    KOMPARE(QString(": 2"), status->labelNumSuccess->text());
    KOMPARE(QString(": 0"), status->labelNumExceptions->text());
    KOMPARE(QString(": 0"), status->labelNumFatals->text());
    KOMPARE(QString(": 0"), status->labelNumErrors->text());
    KOMPARE(QString(": 0"), status->labelNumWarnings->text());
}

void RunnerWindowTest::deselectItems()
{
    // select only one of the runner items
    // validate that the other one didn't get executed
    KTODO;
}

void RunnerWindowTest::assertTestEquals(int itemNr, QVariant col0, QVariant col1, QVariant col2, int result)
{
    Test* item = model->fetchItem(model->index(itemNr, 0));
    KOMPARE(col0, item->data(0));
    KOMPARE(col1, item->data(1));
    KOMPARE(col2, item->data(2));
    KOMPARE(result, item->state());
}

QTEST_KDEMAIN(RunnerWindowTest, GUI)
