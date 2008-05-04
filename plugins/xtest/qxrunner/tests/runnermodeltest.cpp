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

#include <runnermodel.h>
#include <runneritem.h>

#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QMap>

#include "kasserts.h"
#include "runnermodeltest.h"

using QxRunner::RunnerModel;
using QxRunner::RunnerItem;

namespace RunnerModelTestNm
{

class StubRunnerItem : public RunnerItem
{
public:
    StubRunnerItem(const QList<QVariant>& data, RunnerItem* parent)
            : RunnerItem(data, parent) {}

    int run() {
         if (child(0)) {
             setResult(QxRunner::NoResult);  // Have nothing to do as a parent
         } else {
             setData(1, QString::number(row()) + QString("_1"));
             setData(2, QString::number(row()) + QString("_2"));
             setResult(QxRunner::RunSuccess);
         }
         return result();
    }
};

class RunnerModelImpl : public RunnerModel
{
public:
    RunnerModelImpl()
            : RunnerModel(NULL) {
    }

    void fill() {
        QList<QVariant> rootData;
        rootData << tr("col0") << tr("col1") << tr("col2");
        setRootItem(new StubRunnerItem(rootData, NULL));

        QList<QVariant> columnData;
        columnData << "00" << "01" << "02";
        StubRunnerItem* item1 = new StubRunnerItem(columnData, rootItem());
        item1->setResult(QxRunner::RunSuccess);
        rootItem()->appendChild(item1);

        columnData.clear();
        columnData << "10" << "11" << "12";
        StubRunnerItem* item2 = new StubRunnerItem(columnData, rootItem());
        item2->setResult(QxRunner::RunFatal);
        rootItem()->appendChild(item2);
    }

    QString name() const {
        return "";
    }

};

} // end RunnerModelTestNm

using RunnerModelTestNm::RunnerModelImpl;

void RunnerModelTest::init()
{
    model = new RunnerModelImpl();
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
    assertColumnHeader("col0", 0);
    assertColumnHeader("col1", 1);
    assertColumnHeader("col2", 2);

    // should contain the right stuff
    verifyRowContent(0);
    verifyRowContent(1);
}

// test command
void RunnerModelTest::changeItems()
{
    model->fill();
    KVERIFY_MSG(!model->setData(model->index(0,1), "new_content"), 
                "Only allowed to select/deselect, not change arbitrary data");
    KVERIFY_MSG(!model->setData(model->index(0,0), true, Qt::EditRole),
                "Only allowed to select/deselect, not change arbitrary data");

    KVERIFY_MSG(model->setData(model->index(0,0), false, Qt::CheckStateRole), 
                "Failed to set checked state, this is bad mkay.");
    assertItemChecked(0, false);
    KVERIFY_MSG(model->setData(model->index(0,0), true, Qt::CheckStateRole), 
                "Failed to set checked state, this is bad mkay.");
    assertItemChecked(0, true);
}

// test command
void RunnerModelTest::flags()
{
    model->fill();
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable), model->flags(model->index(0,0)));
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable), model->flags(model->index(0,1)));
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable), model->flags(model->index(0,2)));
}

// test command
void RunnerModelTest::changeChecked()
{
    model->fill();
    QModelIndex row0 = model->index(0, 0);
    assertItemChecked(0, true);
    model->setItemChecked(row0, false);
    assertItemChecked(0, false);
    model->setItemChecked(row0, true);
    assertItemChecked(0, true);
}

// test command
void RunnerModelTest::runItems()
{
    QMap<QString, QSignalSpy*> spies;
    spies["startedC"] = new QSignalSpy(model, SIGNAL(numStartedChanged(int)));
    spies["successC"] = new QSignalSpy(model, SIGNAL(numSuccessChanged(int)));
    spies["infoC"]    = new QSignalSpy(model, SIGNAL(numInfosChanged(int)));
    spies["warnC"]    = new QSignalSpy(model, SIGNAL(numWarningsChanged(int)));
    spies["errorC"]   = new QSignalSpy(model, SIGNAL(numErrorsChanged(int)));
    spies["fatalC"]   = new QSignalSpy(model, SIGNAL(numFatalsChanged(int)));
    spies["exceptionC"] = new QSignalSpy(model, SIGNAL(numExceptionsChanged(int)));

    model->runItems();

    foreach(QSignalSpy* spy, spies)
    {
        //qDebug() << *spy;
        KOMPARE(1, spy->size());
        KOMPARE(QVariant(0), spy->takeFirst().at(0));
    }

    foreach(QSignalSpy* spy, spies) delete spy;
}

void RunnerModelTest::assertColumnHeader(const QVariant& expected, int index)
{
    KOMPARE_MSG(expected, model->headerData(index, Qt::Horizontal), "Incorrect column header caption");
}

void RunnerModelTest::assertDataAt(const QVariant& expected, int row, int column)
{
    QVariant actual = model->data(model->index(row,column), Qt::DisplayRole);
    //qDebug() << "actual >" << actual << "< ; expected >" << expected << "<";
    KOMPARE_MSG(expected, actual, QString("Expected: ") + QTest::toString(expected));
}

void RunnerModelTest::assertItemChecked(int row, bool checked)
{
    bool actualCheckState = model->data(model->index(row, 0), Qt::CheckStateRole).toBool();
    KVERIFY_MSG(actualCheckState == checked, checked ? "Should be checked" : "Should not be checked");
}

void RunnerModelTest::verifyRowContent(int index)
{
    QString rowStr = QString::number(index);
    assertDataAt(rowStr + "0", index, 0);
    assertDataAt(rowStr + "1", index, 1);
    assertDataAt(rowStr + "2", index, 2);
    assertItemChecked(index, true);
}

QTEST_MAIN( RunnerModelTest );
