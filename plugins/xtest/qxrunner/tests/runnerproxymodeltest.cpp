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

#include <runnerproxymodel.h>

#include "kasserts.h"
#include "runnerproxymodeltest.h"
#include "modelcreation.h"

using QxRunner::RunnerProxyModel;
using ModelCreation::createRunnerModelStub;

void RunnerProxyModelTest::init()
{
    source = createRunnerModelStub();
    proxy  = new RunnerProxyModel(source);
    proxy->setSourceModel(source);
}

void RunnerProxyModelTest::cleanup()
{
    if (proxy)  delete proxy;
    if (source) delete source;
}

// test command
void RunnerProxyModelTest::default_()
{
    KVERIFY(proxy->isActive());
    // all columns are disabled by default ...
    assertRowFiltered(0);
    assertRowFiltered(1);
}

// test command
// void RunnerProxyModelTest::deactivate()
// {
//     setAllColumnsEnabled();
//     proxy->setActive(false);
// 
//     assertRowFiltered(0);
//     assertRowFiltered(1);
// }

// test command
void RunnerProxyModelTest::enableColumns()
{
    setAllColumnsEnabled(); // evrything should get through

    // row 0
    assertRowContains(0, "00", "01", "02");
    // row 1
    assertRowContains(1, "10", "11", "12");
}

//test command
void RunnerProxyModelTest::disableColumn()
{
    // disable column 1
    QBitArray cols(3);
    cols.setBit(0);
    cols.clearBit(1);
    cols.setBit(2);
    proxy->setEnabledColumns(cols);

    // row 0
    assertRowContains(0, "00", QVariant(), "02");
    // row 1
    assertRowContains(1, "10", QVariant(), "12");
}

void RunnerProxyModelTest::errorHandling()
{
    KOMPARE(QVariant(), proxy->data(QModelIndex(), Qt::DisplayRole));
}

void RunnerProxyModelTest::assertRowFiltered(int row)
{
    assertRowContains(row, QVariant(), QVariant(), QVariant());
}

void RunnerProxyModelTest::setAllColumnsEnabled()
{
    QBitArray cols(3);
    cols.setBit(0);
    cols.setBit(1);
    cols.setBit(2);
    proxy->setEnabledColumns(cols);
}

void RunnerProxyModelTest::assertDataAt(int row, int column, const QVariant& expected)
{
    QVariant actual = proxy->data(proxy->index(row,column), Qt::DisplayRole);
    KOMPARE_MSG(expected, actual, QString("\nExpected: ") + QTest::toString(expected) +\
                                  QString("\nActual:   ") + QTest::toString(actual));
}

void RunnerProxyModelTest::assertRowContains(int row, const QVariant& col1, const QVariant& col2, const QVariant& col3)
{
    assertDataAt(row,0, col1);
    assertDataAt(row,1, col2);
    assertDataAt(row,2, col3);
}

QTEST_MAIN( RunnerProxyModelTest );
