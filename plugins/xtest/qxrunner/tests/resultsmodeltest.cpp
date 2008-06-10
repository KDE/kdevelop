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
#include "modelcreation.h"
#include <kasserts.h>
#include <resultsmodel.h>
#include <runnermodel.h>
#include <runneritem.h>

#include <QIcon>
#include <QString>
#include <QStringList>
#include <qtest_kde.h>

using QxRunner::ResultsModel;
using QxRunner::RunnerModel;
using QxRunner::RunnerItem;
using ModelCreation::RunnerModelStub;
using QxRunner::ut::ResultsModelTest;

void ResultsModelTest::init()
{
    QStringList headers;
    headers << "col0" << "col1" << "col2";
    model = new ResultsModel(headers);
    runnerModel = new RunnerModelStub();
}

void ResultsModelTest::cleanup()
{
    if (model) delete model;
    if (runnerModel) delete runnerModel;
}

// test command
void ResultsModelTest::default_()
{
    // size check
    KOMPARE(0, model->rowCount());
    KOMPARE(3, model->columnCount());

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

    KOMPARE(2, model->rowCount());
    KOMPARE(QxRunner::RunSuccess, model->result(0));
    KOMPARE(QxRunner::RunFatal, model->result(1));

    checkRow(0);
    checkRow(1);
}

// test command
void ResultsModelTest::mapIndices()
{
    fillRows();
    QModelIndex runIndex0 = runnerModel->index(0, 0);
    QModelIndex resultIndex0 = model->index(0, 0);

    KOMPARE(runIndex0, model->mapToRunnerItemIndex(resultIndex0));
    KOMPARE(resultIndex0, model->mapFromRunnerItemIndex(runIndex0));

    KOMPARE(resultIndex0, model->mapFromRunnerItemIndex(model->mapToRunnerItemIndex(resultIndex0)));
    KOMPARE(runIndex0, model->mapToRunnerItemIndex(model->mapFromRunnerItemIndex(runIndex0)));
}

// test command
void ResultsModelTest::errorHandling()
{
    fillRows();
    QVariant illegal; // default constructed variant denotes trouble
    KOMPARE_MSG(illegal, model->data(model->index(0, 0), Qt::CheckStateRole),
                "Results have no items with checked state");
    KOMPARE(illegal, model->data(model->index(0, 0), Qt::EditRole));
    KOMPARE(illegal, model->data(model->index(0, 1), Qt::DecorationRole));

    KOMPARE(illegal, model->headerData(0, Qt::Vertical, Qt::DisplayRole));
    KOMPARE(illegal, model->headerData(0, Qt::Horizontal, Qt::EditRole));

    KOMPARE(QxRunner::NoResult, model->result(-1));
    KOMPARE(QModelIndex(), model->mapToRunnerItemIndex(QModelIndex()));
    KOMPARE(QModelIndex(), model->mapFromRunnerItemIndex(QModelIndex()));
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
//     QVariant actual = model->data(model->index(row,0), Qt::DecorationRole);
//     QIcon actualIcon = qVariantCast<QIcon>(actual);
//     KOMPARE(icon, actualIcon);
// }

void ResultsModelTest::assertColumnHeader(const QVariant& expected, int index)
{
    KOMPARE_MSG(expected, model->headerData(index, Qt::Horizontal), "Incorrect column header caption");
}

void ResultsModelTest::assertDataAt(const QVariant& expected, int row, int column)
{
    QVariant actual = model->data(model->index(row, column), Qt::DisplayRole);
    KOMPARE_MSG(expected, actual, QString("Expected: ") + QTest::toString(expected));
}

void ResultsModelTest::checkRow(int index)
{
    QString rowStr = QString::number(index);
    assertDataAt(rowStr + '0', index, 0);
    assertDataAt(rowStr + '1', index, 1);
    assertDataAt(rowStr + '2', index, 2);
}

void ResultsModelTest::fillRows()
{
    model->addResult(runnerModel->index(0, 0)); // invoke slot
    model->addResult(runnerModel->index(1, 0)); // invoke slot
}

QTEST_KDEMAIN(ResultsModelTest, NoGUI)
