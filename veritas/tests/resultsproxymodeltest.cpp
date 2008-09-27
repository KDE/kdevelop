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

#include "resultsproxymodeltest.h"

#include <qtest_kde.h>

#include "../internal/resultsproxymodel.h"

#include "kasserts.h"
#include "modelcreation.h"

using Veritas::ResultsProxyModel;
using Veritas::ResultsModel;
using Veritas::createResultsModelStub;
using Veritas::ResultsProxyModelTest;

void ResultsProxyModelTest::init()
{
    QPair<ResultsModel*,RunnerModelStub*> models = createResultsModelStub();
    source = models.first;
    runnerModel = models.second;
    proxy  = new ResultsProxyModel(source);
    proxy->setSourceModel(source);
}

void ResultsProxyModelTest::cleanup()
{
    if (proxy)  delete proxy;
    if (source) delete source;
//    delete runnerModel->root;
    delete runnerModel;
}

//test command
void ResultsProxyModelTest::filter()
{
//    setAllColumnsEnabled();
    proxy->setFilter(Veritas::RunFatal); // show only these

    // resultsmodel row zero has 'RunSuccess' so should be filtered
    // resulstmodel row one has 'RunFatal' so becomes proxy row zero
    assertRowContains(0, "10", "11", "12");
    assertRowFiltered(1);
}

// checker
void ResultsProxyModelTest::assertRowFiltered(int row)
{
    assertRowContains(row, QVariant(), QVariant(), QVariant());
}

// checker
void ResultsProxyModelTest::assertDataAt(int row, int column, const QVariant& expected)
{
    QVariant actual = proxy->data(proxy->index(row, column), Qt::DisplayRole);
    QString failMsg =
        QString("Wrong result data at row %1, column %2").arg(row).arg(column);
    KOMPARE_MSG(expected, actual, failMsg);
}

// checker
void ResultsProxyModelTest::assertRowContains(int row, const QVariant& col1, const QVariant& col2, const QVariant& col3)
{
    assertDataAt(row, 0, col1);
    assertDataAt(row, 1, col2);
    assertDataAt(row, 2, col3);
}

QTEST_KDEMAIN(ResultsProxyModelTest, GUI)
