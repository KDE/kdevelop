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
#include "../internal/resultsmodel.h"
#include "../testresult.h"
#include "../test.h"
#include "kasserts.h"

using Veritas::ResultsProxyModel;
using Veritas::ResultsModel;
using Veritas::ResultsProxyModelTest;
using Veritas::TestResult;
using Veritas::Test;


namespace
{

ResultsModel* createResultsModel()
{
    QStringList header;
    header << "col0" << "col1" << "col2";

    ResultsModel* model = new ResultsModel(header);

    TestResult* fooResult = new TestResult;
    Test* fooOwner = new Test("Foo");
    fooResult->setOwner(fooOwner);
    fooResult->setFile(KUrl("foo.cpp"));
    fooResult->setState(Veritas::RunError);

    TestResult* barResult = new TestResult;
    Test* barOwner = new Test("Bar");
    barResult->setOwner(barOwner);
    barResult->setFile(KUrl("bar.cpp"));
    barResult->setState(Veritas::RunFatal);

    model->addResult(fooResult); // invoke slot
    model->addResult(barResult); // invoke slot

    return model;
}

}

void ResultsProxyModelTest::init()
{
    source = createResultsModel();
    proxy = new ResultsProxyModel(source);
    proxy->setSourceModel(source);
}

void ResultsProxyModelTest::cleanup()
{
    if (proxy)  delete proxy;
    if (source) delete source;
}

//test command
void ResultsProxyModelTest::filter()
{
//    setAllColumnsEnabled();
    proxy->setFilter(Veritas::RunFatal); // show only these

    // resultsmodel row zero has 'RunError' so should be filtered
    // resulstmodel row one has 'RunFatal' so becomes proxy row zero
    assertRowContains(0, QString("Bar"), QVariant(""), QString("bar.cpp"), QVariant());
    assertRowFiltered(1);
}

// checker
void ResultsProxyModelTest::assertRowFiltered(int row)
{
    assertRowContains(row, QVariant(), QVariant(), QVariant(), QVariant());
}

// checker
void ResultsProxyModelTest::assertDataAt(int row, int column, const QVariant& expected)
{
    QVariant actual = proxy->index(row, column).data(Qt::DisplayRole);
    QString failMsg =
        QString("Wrong result data at row %1, column %2.").arg(row).arg(column);
    KOMPARE_MSG(expected, actual, failMsg);
}

// checker
void ResultsProxyModelTest::assertRowContains(int row, const QVariant& col1, const QVariant& col2, const QVariant& col3, const QVariant& col4)
{
    assertDataAt(row, 0, col1);
    assertDataAt(row, 1, col2);
    assertDataAt(row, 2, col3);
    assertDataAt(row, 3, col4);
}

QTEST_KDEMAIN(ResultsProxyModelTest, GUI)
