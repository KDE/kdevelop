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

#include "runnermodeltest.h"
#include "modelcreation.h"

#include <runnermodel.h>
#include <runneritem.h>

#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QMap>
#include <qtest_kde.h>
#include <kasserts.h>

using QxRunner::RunnerModel;
using QxRunner::RunnerItem;
using QxRunner::ut::createRunnerModelStub;
using QxRunner::ut::RunnerModelStub;
using QxRunner::ut::RunnerModelTest;

void RunnerModelTest::init()
{
    model = createRunnerModelStub(false);
}

void RunnerModelTest::cleanup()
{
    if (model) delete model;
}

// test command
void RunnerModelTest::default_()
{
    KOMPARE(0, model->rowCount());
    KOMPARE(0, model->columnCount());
    KVERIFY(!model->isRunning());
    KOMPARE(QxRunner::AllResults, model->expectedResults());
}

// test command
void RunnerModelTest::appendResults()
{
    model->fill();

    // size check
    KOMPARE(2, model->rowCount());
    KOMPARE(3, model->columnCount());

    // verify column headers
    assertColumnHeader(model->col0Caption, 0);
    assertColumnHeader(model->col1Caption, 1);
    assertColumnHeader(model->col2Caption, 2);

    // should contain the right stuff
    verifyRowContent(0);
    verifyRowContent(1);
}

// test command
void RunnerModelTest::changeItems()
{
    model->fill();
    KVERIFY_MSG(!model->setData(model->index(0, 1), "new_content"),
                "Only allowed to select/deselect, not change arbitrary data");
    KVERIFY_MSG(!model->setData(model->index(0, 0), true, Qt::EditRole),
                "Only allowed to select/deselect, not change arbitrary data");
}

// test command
void RunnerModelTest::flags()
{
    model->fill();
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable), model->flags(model->index(0, 0)));
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable), model->flags(model->index(0, 1)));
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable), model->flags(model->index(0, 2)));
}

// test command
void RunnerModelTest::runItems()
{
    QMap<QString, QSignalSpy*> spies;
    spies["startedC"] = new QSignalSpy(model, SIGNAL(numStartedChanged(int)));
    setUpResultSpies(spies);

    model->runItems();

    foreach(QSignalSpy* spy, spies) {
        KOMPARE(1, spy->size());
        QCOMPARE(QVariant(0), spy->takeFirst().at(0));
        // using KOMPARE here makes qlist assert
        // QFATAL : RunnerModelTest::runItems() ASSERT: "!isEmpty()" in file /usr/include/QtCore/qlist.h, line 252
    }

    foreach(QSignalSpy* spy, spies) delete spy;
}

//test command
void RunnerModelTest::errorHandling()
{
    QVariant illegal;
    KOMPARE(illegal, model->data(QModelIndex()));
    KOMPARE(illegal, model->data(model->index(0, 1), Qt::CheckStateRole));
    KOMPARE(illegal, model->data(model->index(0, 0), Qt::CheckStateRole));

    KOMPARE(false, model->setData(QModelIndex(), QVariant(), Qt::DisplayRole));
    KOMPARE(Qt::ItemIsEnabled, model->flags(QModelIndex()));

    KOMPARE(QModelIndex(), model->parent(QModelIndex()));

    KOMPARE(illegal, model->headerData(0, Qt::Vertical, Qt::DisplayRole));
    KOMPARE(illegal, model->headerData(0, Qt::Horizontal, Qt::CheckStateRole));
    model->decapitate();
    KOMPARE(illegal, model->headerData(0, Qt::Horizontal, Qt::DisplayRole));
}

//test command
void RunnerModelTest::countItems()
{
    QMap<QString, QSignalSpy*> spies;
    spies["totalC"] = new QSignalSpy(model, SIGNAL(numTotalChanged(int)));
    spies["selectedC"] = new QSignalSpy(model, SIGNAL(numSelectedChanged(int)));
    setUpResultSpies(spies);

    model->fill();
    model->countItems();

    // all should be emitted just once
    foreach(QSignalSpy* spy, spies)
    KOMPARE(1, spy->size());

    assertSignalValue(spies.take("totalC"), 2);
    assertSignalValue(spies.take("selectedC"), 2);
    assertSignalValue(spies.take("successC"), 1);
    assertSignalValue(spies.take("fatalC"), 1);

    // the others are zero
    foreach(QSignalSpy* spy, spies)
    assertSignalValue(spy, 0);
}

void RunnerModelTest::assertSignalValue(QSignalSpy* spy, int expected)
{
    QVariant actual = spy->takeFirst().at(0);
    KOMPARE(QVariant(expected), actual);
    delete spy;
}

void RunnerModelTest::assertColumnHeader(const QVariant& expected, int index)
{
    KOMPARE_MSG(expected, model->headerData(index, Qt::Horizontal), "Incorrect column header caption");
}

void RunnerModelTest::assertDataAt(const QVariant& expected, int row, int column)
{
    QVariant actual = model->data(model->index(row, column), Qt::DisplayRole);
    //qDebug() << "actual >" << actual << "< ; expected >" << expected << "<";
    KOMPARE_MSG(expected, actual, QString("Expected: ") + QTest::toString(expected));
}

void RunnerModelTest::verifyRowContent(int index)
{
    QString rowStr = QString::number(index);
    assertDataAt(rowStr + '0', index, 0);
    assertDataAt(rowStr + '1', index, 1);
    assertDataAt(rowStr + '2', index, 2);

    KOMPARE(0, model->rowCount(model->index(index, 0))); // no children
    KOMPARE(3, model->columnCount(model->index(index, 0)));

}

void RunnerModelTest::setUpResultSpies(QMap<QString, QSignalSpy*>& spies)
{
    spies["successC"] = new QSignalSpy(model, SIGNAL(numSuccessChanged(int)));
    spies["infoC"]    = new QSignalSpy(model, SIGNAL(numInfosChanged(int)));
    spies["warnC"]    = new QSignalSpy(model, SIGNAL(numWarningsChanged(int)));
    spies["errorC"]   = new QSignalSpy(model, SIGNAL(numErrorsChanged(int)));
    spies["fatalC"]   = new QSignalSpy(model, SIGNAL(numFatalsChanged(int)));
    spies["exceptionC"] = new QSignalSpy(model, SIGNAL(numExceptionsChanged(int)));
}

QTEST_KDEMAIN(RunnerModelTest, GUI)
