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

#include <resultsproxymodel.h>
#include <qtest_kde.h>
#include <kasserts.h>
#include "resultsproxymodeltest.h"
#include "modelcreation.h"

using QxRunner::ResultsProxyModel;
using QxRunner::ResultsModel;
using ModelCreation::createResultsModelStub;

void ResultsProxyModelTest::init()
{
    source = createResultsModelStub();
    proxy  = new ResultsProxyModel(source);
    proxy->setSourceModel(source);
}

void ResultsProxyModelTest::cleanup()
{
    if (proxy)  delete proxy;
    if (source) delete source;
}

// test command
void ResultsProxyModelTest::default_()
{
    KVERIFY(proxy->isActive());
    KOMPARE(QxRunner::AllResults, proxy->filter());
    // all columns are disabled by default ...
    assertRowFiltered(0);
    assertRowFiltered(1);
}

// test command
void ResultsProxyModelTest::deactivate()
{
    setAllColumnsEnabled();
    proxy->setActive(false);

    assertRowFiltered(0);
    assertRowFiltered(1);
}

// test command
void ResultsProxyModelTest::enableColumns()
{
    setAllColumnsEnabled(); // evrything should get through

    assertRowContains(0, "00", "01", "02"); // row 0
    assertRowContains(1, "10", "11", "12"); // row 1
}

//test command
void ResultsProxyModelTest::disableColumn()
{
    // disable column 1
    QBitArray cols(3);
    cols.setBit(0);
    cols.clearBit(1);
    cols.setBit(2);
    proxy->setEnabledColumns(cols);

    assertRowContains(0, "00", QVariant(), "02"); // row 0
    assertRowContains(1, "10", QVariant(), "12"); // row 1
}

//test command
void ResultsProxyModelTest::filter()
{
    setAllColumnsEnabled();
    proxy->setFilter(QxRunner::RunFatal); // show only these

    // resultsmodel row zero has 'RunSuccess' so should be filtered
    // resulstmodel row one has 'RunFatal' so becomes proxy row zero
    assertRowContains(0, "10", "11", "12");
    assertRowFiltered(1);
}

void ResultsProxyModelTest::assertRowFiltered(int row)
{
    assertRowContains(row, QVariant(), QVariant(), QVariant());
}

void ResultsProxyModelTest::setAllColumnsEnabled()
{
    QBitArray cols(3);
    cols.setBit(0);
    cols.setBit(1);
    cols.setBit(2);
    proxy->setEnabledColumns(cols);
}

void ResultsProxyModelTest::assertDataAt(int row, int column, const QVariant& expected)
{
    QVariant actual = proxy->data(proxy->index(row,column), Qt::DisplayRole);
    KOMPARE_MSG(expected, actual, QString("Expected: ") + QTest::toString(expected));
}

void ResultsProxyModelTest::assertRowContains(int row, const QVariant& col1, const QVariant& col2, const QVariant& col3)
{
    assertDataAt(row,0, col1);
    assertDataAt(row,1, col2);
    assertDataAt(row,2, col3);
}

QTEST_KDEMAIN( ResultsProxyModelTest, NoGUI )
