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

#include <runnerviewcontroller.h>
#include <runnerproxymodel.h>
#include <ui_runnerwindow.h>
#include <QMainWindow>

#include "kasserts.h"
#include "runnerviewcontrollertest.h"
#include "modelcreation.h"

using QxRunner::RunnerViewController;
using QxRunner::RunnerProxyModel;
using ModelCreation::createRunnerModelStub;

Ui::RunnerWindow* ui;
QMainWindow* w;

void RunnerViewControllerTest::init()
{
    // TODO to instantiate a view a full blown gui is required
    // fix this later with real UI tests isntead of unit tests.
    view = new QTreeView(0);
    viewController = new RunnerViewController(NULL, view);
    model = createRunnerModelStub();
    proxy = new RunnerProxyModel(model);
    proxy->setSourceModel(model);
    view->setModel(proxy);
}

void RunnerViewControllerTest::cleanup()
{
    if (viewController) delete viewController;
    if (ui) delete ui;
    if(model) delete model;
    if (proxy) delete proxy;
    if (w) delete w;
}

void RunnerViewControllerTest::select()
{
    viewController->unselectAll(); // bypass default
    viewController->selectAll(); // invoke slot
    assertAllItemsChecked(true);
    viewController->unselectAll(); // invoke slot
    assertAllItemsChecked(false);
}

void RunnerViewControllerTest::assertAllItemsChecked(bool checked)
{
    for (int row=0; row<2; row++)
    {
        for (int col=0; col<3; col++)
        {    viewController->unselectAll();
            bool actualCheckState = model->data(model->index(row, col), Qt::CheckStateRole).toBool();
            KVERIFY_MSG(actualCheckState == checked, checked ? "Should be checked" : "Should not be checked");
        }
    }
}


QTEST_MAIN( RunnerViewControllerTest );
