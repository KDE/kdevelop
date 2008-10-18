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

#include "resultsmodeltest.h"

#include <QIcon>
#include <QString>
#include <QStringList>
#include <qtest_kde.h>
#include <KDebug>

#include "../test.h"
#include "../internal/resultsmodel.h"
#include "../internal/runnermodel.h"

#include "kasserts.h"
#include "modelcreation.h"

using Veritas::ResultsModel;
using Veritas::RunnerModel;
using Veritas::RunnerModelStub;
using Veritas::ResultsModelTest;
using Veritas::Test;
using Veritas::TestResult;
using Veritas::TestState;

void ResultsModelTest::init()
{
    QStringList headers;
    headers << "col0" << "col1" << "col2";
    m_model = new ResultsModel(headers);
    m_runnerModel = new RunnerModelStub();
}

void ResultsModelTest::cleanup()
{
    if (m_model) delete m_model;
    if (m_runnerModel) delete m_runnerModel;
}

namespace
{
ResultsModel* createResultsModel()
{
    ResultsModel* rmodel = new ResultsModel(QStringList() << "test" << "msg" << "file" << "line");
    return rmodel;
}

TestResult* createTestResult(Test* owner, TestState state, const KUrl& location, int line, const QString& message)
{
    TestResult* result = new TestResult;
    result->setFile(location);
    result->setLine(line);
    result->setMessage(message);
    result->setState(state);
    if (owner) owner->setResult(result);
    return result;
}

TestResult* createTestResult(TestState state, const QString& testName, const KUrl& location, int line, const QString& message)
{
    Test* t = new Test(testName);
    return createTestResult(t, state, location, line, message);
}

TestResult* createTestResult(TestResult* parent, TestState state, const KUrl& location, int line, const QString& message)
{
    TestResult* result = new TestResult;
    result->setFile(location);
    result->setLine(line);
    result->setMessage(message);
    result->setState(state);
    parent->appendChild(result);
    return result;
}

}

// command
void ResultsModelTest::testFromIndex()
{
    TestResult* foo = createTestResult(Veritas::RunError, "FooTest", KUrl("foo/bar.cpp"), 10, "failure message");
    TestResult* bar = createTestResult(Veritas::RunException, "BarTest", KUrl("bar.cpp"), 16, "descriptive message");

    m_model->addResult(foo);
    m_model->addResult(bar);

    QModelIndex fooIndex = m_model->index(0,0);
    QModelIndex barIndex = m_model->index(1,0);

    Test* fooTest = m_model->testFromIndex(fooIndex);
    Test* barTest = m_model->testFromIndex(barIndex);

    KVERIFY(fooTest);
    KOMPARE(foo->owner(), fooTest);
    KVERIFY(barTest);
    KOMPARE(bar->owner(), barTest);

    KOMPARE(0, m_model->testFromIndex(QModelIndex()));
}

// command
void ResultsModelTest::constructEmpty()
{
    // An untouched model should contain zero items

    assertNrofItemsEquals(0, m_model);
    KOMPARE(3, m_model->columnCount());
    assertColumnHeader("col0", 0);
    assertColumnHeader("col1", 1);
    assertColumnHeader("col2", 2);
}

// command
void ResultsModelTest::addSingleResult()
{
    // A single result in the model. Verify that the model does
    // indeed now contain a single item equal to the one that was inserted

    ResultsModel* rmodel = createResultsModel();
    TestResult* result = createTestResult(Veritas::RunError, "FooTest", KUrl("foo/bar.cpp"), 10, "failure message");

    rmodel->addResult(result);
    assertNrofItemsEquals(1, rmodel);
    assertRowDataEquals(rmodel, 0, result);
}

void ResultsModelTest::assertRowDataEquals(ResultsModel* model, int rowNumber, TestResult* expected)
{
    KOMPARE_MSG(expected->state(), model->result(rowNumber), expected->owner()->name());
    KVERIFY(expected->owner());
    KOMPARE(expected->owner()->name(), model->index(rowNumber,0).data(Qt::DisplayRole).toString());
    KOMPARE(expected->file().pathOrUrl(), model->index(rowNumber,2).data(Qt::DisplayRole).toString());
    KOMPARE(QString::number(expected->line()), model->index(rowNumber,3).data(Qt::DisplayRole).toString());
    KOMPARE(expected->message(), model->index(rowNumber,1).data(Qt::DisplayRole).toString());
}

// command
void ResultsModelTest::addMultipleResults()
{
    // Multiple testresults inserted into the model
    // The model should now contain exactly this number of items
    // with the expected content in the order of insertion.

    ResultsModel* rmodel = createResultsModel();
    TestResult* foo = createTestResult(Veritas::RunError, "FooTest", KUrl("foo/bar.cpp"), 10, "failure message");
    TestResult* bar = createTestResult(Veritas::RunException, "BarTest", KUrl("bar.cpp"), 16, "descriptive message");

    rmodel->addResult(foo);
    rmodel->addResult(bar);

    assertNrofItemsEquals(2, rmodel);
    assertRowDataEquals(rmodel, 0, foo);
    assertRowDataEquals(rmodel, 1, bar);
}

void ResultsModelTest::addSubResult()
{
    // Results can have subresults. Add some of those to
    // this model and check that evrything was inserted as expected

    ResultsModel* rmodel = createResultsModel();
    TestResult* root = createTestResult(Veritas::RunError, "FooTest", KUrl("baz.cpp"), 13, "");
    TestResult* foo = createTestResult(root, Veritas::RunError, KUrl("foo/bar.cpp"), 10, "failure message");
    TestResult* bar = createTestResult(root, Veritas::RunException, KUrl("bar.cpp"), 16, "descriptive message");

    rmodel->addResult(root);

    assertNrofItemsEquals(2, rmodel);
    assertRowDataEquals(rmodel, 0, foo);
    assertRowDataEquals(rmodel, 1, bar);
}

void ResultsModelTest::fetchDataFromEmpty()
{
    // Get data from an empty model

    ResultsModel* rmodel = createResultsModel();
    assertNrofItemsEquals(0, rmodel);
    KOMPARE(-1, rmodel->result(0));
    KOMPARE(-1, rmodel->result(1));
    KOMPARE(-1, rmodel->result(-1));

    KOMPARE(QModelIndex(), rmodel->index(0,0));
    KOMPARE(QVariant(), rmodel->index(0,0).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(0,1).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(0,2).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(0,3).data(Qt::DisplayRole));

    KOMPARE(QModelIndex(), rmodel->index(1,0));
    KOMPARE(QVariant(), rmodel->index(1,0).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(1,1).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(1,2).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(1,3).data(Qt::DisplayRole));
}

void ResultsModelTest::fetchDataIllegalRow()
{
    // Fetch data from the model from rows/indexes that
    // are not contained within the model boundaries (number of rows)

    ResultsModel* rmodel = createResultsModel();
    TestResult* foo = createTestResult(Veritas::RunError, "FooTest", KUrl("foo/bar.cpp"), 10, "failure message");
    TestResult* bar = createTestResult(Veritas::RunException, "BarTest", KUrl("bar.cpp"), 16, "descriptive message");

    rmodel->addResult(foo);
    rmodel->addResult(bar);

    KOMPARE(-1, rmodel->result(2));
    KOMPARE(-1, rmodel->result(-1));

    KOMPARE(QModelIndex(), rmodel->index(3,0));
    KOMPARE(QVariant(), rmodel->index(3,0).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(3,1).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(3,2).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(3,3).data(Qt::DisplayRole));

    KOMPARE(QModelIndex(), rmodel->index(4,0));
    KOMPARE(QVariant(), rmodel->index(4,0).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(4,1).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(4,2).data(Qt::DisplayRole));
    KOMPARE(QVariant(), rmodel->index(4,3).data(Qt::DisplayRole));
}

void ResultsModelTest::fetchDataNoOwner()
{
    // TestResults normally have an owner Test initialized.
    // this command fetches data for results in the model
    // without an owner, ie owner() returns 0

    ResultsModel* rmodel = createResultsModel();
    Test* t = 0;
    TestResult* result = createTestResult(t, Veritas::RunError, KUrl("foo/bar.cpp"), 10, "failure message");

    rmodel->addResult(result);
    assertNrofItemsEquals(1, rmodel);

    QTest::ignoreMessage(QtWarningMsg, "Owner test not set for result. ");
    KOMPARE(QVariant(), rmodel->index(0,0).data(Qt::DisplayRole));
}

// command
void ResultsModelTest::clear()
{
    // First add some testresults to a model
    // next clear this model. It should now contain zero items
    // Finally re-add some items and verify that the correct items
    // were inserted

    ResultsModel* rmodel = createResultsModel();
    TestResult* foo = createTestResult(Veritas::RunError, "FooTest", KUrl("foo/bar.cpp"), 10, "failure message");
    TestResult* bar = createTestResult(Veritas::RunException, "BarTest", KUrl("bar.cpp"), 16, "descriptive message");

    rmodel->addResult(foo);
    rmodel->addResult(bar);
    assertNrofItemsEquals(2, rmodel);

    rmodel->clear();
    assertNrofItemsEquals(0, rmodel);

    rmodel->addResult(foo);
    rmodel->addResult(bar);
    assertNrofItemsEquals(2, rmodel);
    assertRowDataEquals(rmodel, 0, foo);
    assertRowDataEquals(rmodel, 1, bar);
}

// custom assertions
void ResultsModelTest::assertNrofItemsEquals(int expected, ResultsModel* model)
{
    KOMPARE(expected, model->rowCount());
    if (expected == 0) QVERIFY(!model->hasChildren());
    else QVERIFY(model->hasChildren());
    for (int i=0; i<expected; i++) {
        QVERIFY(model->index(i,0).isValid());
        QVERIFY(model->index(i,1).isValid());
        QVERIFY(model->index(i,2).isValid());
        QVERIFY(model->index(i,3).isValid());
        QVERIFY(!model->hasChildren(model->index(i,0)));
    }
    QVERIFY(!model->index(expected,0).isValid());
}

//command
void ResultsModelTest::addNullResult()
{
    // call addResult with a NULL result on empty model.
    // fill the model a bit, now call addResult with NULL again
    // the model is expected to stay unaffected by these NULL calls.

    ResultsModel* rmodel = createResultsModel();
    rmodel->addResult(0);
    assertNrofItemsEquals(0, rmodel);

    TestResult* foo = createTestResult(Veritas::RunError, "FooTest", KUrl("foo/bar.cpp"), 10, "failure message");
    TestResult* bar = createTestResult(Veritas::RunException, "BarTest", KUrl("bar.cpp"), 16, "descriptive message");

    rmodel->addResult(foo);
    rmodel->addResult(bar);
    assertNrofItemsEquals(2, rmodel);

    rmodel->addResult(0);
    assertNrofItemsEquals(2, rmodel);
    rmodel->addResult(0);
    assertNrofItemsEquals(2, rmodel);
}

// helper
void ResultsModelTest::assertColumnHeader(const QVariant& expected, int index)
{
   KOMPARE_MSG(expected, m_model->headerData(index, Qt::Horizontal), "Incorrect column header caption");
}

QTEST_KDEMAIN(ResultsModelTest, GUI)
