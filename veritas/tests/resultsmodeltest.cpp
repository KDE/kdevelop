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
#include "../mvc/resultsmodel.h"
#include "../mvc/runnermodel.h"

#include "kasserts.h"
#include "modelcreation.h"

using Veritas::ResultsModel;
using Veritas::RunnerModel;
using Veritas::Test;
using Veritas::RunnerModelStub;
using Veritas::ResultsModelTest;

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

// test command
void ResultsModelTest::default_()
{
    // size check
    KOMPARE(0, m_model->rowCount());
    KOMPARE(3, m_model->columnCount());

    // verify column headers
    assertColumnHeader("col0", 0);
    assertColumnHeader("col1", 1);
    assertColumnHeader("col2", 2);

    // should still be empty
    assertDataAt(QVariant(), 0, 0);
    assertDataAt(QVariant(), 0, 1);
    assertDataAt(QVariant(), 1, 1);
}

// test command
void ResultsModelTest::appendResults()
{
    fillRows();

    KOMPARE(2, m_model->rowCount());
    KOMPARE(Veritas::RunSuccess, m_model->result(0));
    KOMPARE(Veritas::RunFatal, m_model->result(1));

    QModelIndex topIndex = m_model->index(0,0);
    KVERIFY(topIndex.isValid());
    KVERIFY_MSG(!topIndex.child(0,0).isValid(), "Not expecting children");
    KVERIFY_MSG(!m_model->index(0,0, topIndex).isValid(), "Not expecting children");

    checkRow(0);
    checkRow(1);
}

// test command
void ResultsModelTest::mapIndices()
{
    fillRows();
    QModelIndex runIndex0 = m_runnerModel->index(0, 0);
    KVERIFY(runIndex0.isValid());
    QModelIndex resultIndex0 = m_model->index(0, 0);
    KVERIFY(resultIndex0.isValid());

    KOMPARE(runIndex0, m_model->mapToTestIndex(resultIndex0));
    KOMPARE(resultIndex0, m_model->mapFromTestIndex(runIndex0));

    KOMPARE(resultIndex0, m_model->mapFromTestIndex(m_model->mapToTestIndex(resultIndex0)));
    KOMPARE(runIndex0, m_model->mapToTestIndex(m_model->mapFromTestIndex(runIndex0)));
}

// test command
void ResultsModelTest::errorHandling()
{
    fillRows();
    QVariant illegal; // default constructed variant denotes trouble
    KOMPARE_MSG(illegal, m_model->data(m_model->index(0, 0), Qt::CheckStateRole),
                "Results have no items with checked state");
    KOMPARE(illegal, m_model->data(m_model->index(0, 0), Qt::EditRole));
    KOMPARE(illegal, m_model->data(m_model->index(0, 1), Qt::DecorationRole));

    KOMPARE(illegal, m_model->headerData(0, Qt::Vertical, Qt::DisplayRole));
    KOMPARE(illegal, m_model->headerData(0, Qt::Horizontal, Qt::EditRole));

    //KOMPARE(Veritas::NoResult, m_model->result(-1));
    KOMPARE(QModelIndex(), m_model->mapToTestIndex(QModelIndex()));
    KOMPARE(QModelIndex(), m_model->mapFromTestIndex(QModelIndex()));
}

// command
void ResultsModelTest::testFromIndex()
{
    fillRows();
    QModelIndex testIndex = m_runnerModel->index(0,0);
    Test* actual = m_model->testFromIndex(testIndex);
    Test* expected = static_cast<Test*>(testIndex.internalPointer());

    KVERIFY(expected != NULL); // sanity check
    KVERIFY_MSG(actual != NULL,
        "Null pointer. Failed to map to test through resultsmodel.");
    KOMPARE(actual->name(), expected->name());
}

// test command
// void ResultsModelTest::fetchIcon()
// {
//     fillRows();
//     assertIconAtRow(0, QIcon(":/icons/success.png"));
//     assertIconAtRow(1, QIcon(":/icons/fatal.png"));
// }

// void ResultsModelTest::assertIconAtRow(int row, const QIcon& icon)
// {
//     QVariant actual = m_model->data(model->index(row,0), Qt::DecorationRole);
//     QIcon actualIcon = qVariantCast<QIcon>(actual);
//     KOMPARE(icon, actualIcon);
// }

// helper
void ResultsModelTest::assertColumnHeader(const QVariant& expected, int index)
{
    KOMPARE_MSG(expected, m_model->headerData(index, Qt::Horizontal), "Incorrect column header caption");
}

// helper
void ResultsModelTest::assertDataAt(const QVariant& expected, int row, int column)
{
    QVariant actual = m_model->data(m_model->index(row, column), Qt::DisplayRole);
    KOMPARE_MSG(expected, actual, QString("Expected: ") + QTest::toString(expected) +
                                  " actual " + QTest::toString(actual));
}

// helper
void ResultsModelTest::checkRow(int index)
{
    QString rowStr = QString::number(index);
    assertDataAt(rowStr + '0', index, 0);
    assertDataAt(rowStr + '1', index, 1);
    assertDataAt(rowStr + '2', index, 2);
}

// helper
void ResultsModelTest::fillRows()
{
    m_model->addResult(m_runnerModel->index(0, 0)); // invoke slot
    m_model->addResult(m_runnerModel->index(1, 0)); // invoke slot
}

QTEST_KDEMAIN(ResultsModelTest, GUI)
