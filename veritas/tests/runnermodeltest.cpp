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

#include <QString>
#include <QStringList>
#include <QSignalSpy>
#include <QMap>
#include <qtest_kde.h>

#include "../test.h"
#include "../internal/testexecutor.h"

#include "kasserts.h"
#include "modelcreation.h"


using Veritas::RunnerModel;
using Veritas::Test;
using Veritas::TestStub;
using Veritas::createRunnerModelStub;
using Veritas::RunnerModelStub;
using Veritas::RunnerModelTest;

Q_DECLARE_METATYPE(QModelIndex)

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
    KOMPARE(1, model->columnCount());
    KOMPARE(Veritas::AllStates, model->expectedResults());
}

// test command
void RunnerModelTest::appendResults()
{
    model->fill1();

    // size check
    KOMPARE(2, model->rowCount());
    KOMPARE(1, model->columnCount());

    verifyRowContent(0);
    verifyRowContent(1);
}

// test command
void RunnerModelTest::changeItems()
{
    model->fill1();
    KVERIFY_MSG(!model->setData(model->index(0, 1), "new_content"),
                "Only allowed to select/deselect, not change arbitrary data");
    KVERIFY_MSG(!model->setData(model->index(0, 0), true, Qt::EditRole),
                "Only allowed to select/deselect, not change arbitrary data");
}

// test command
void RunnerModelTest::flags()
{
    model->fill1();
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable), model->flags(model->index(0, 0)));
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable), model->flags(model->index(0, 1)));
    KOMPARE((Qt::ItemIsEnabled | Qt::ItemIsSelectable), model->flags(model->index(0, 2)));
}

namespace
{
Test* takeTestFromSpy(QSignalSpy* spy)
{
    return static_cast<Test*>(spy->takeFirst()[0].value<QModelIndex>().internalPointer());
}

/*! Fill the model with the test tree:
 *  root
 *   `- parent
 *        `- child 
 *  Only the child test is set to be run. */
void fillModel(RunnerModel* model)
{
    TestStub* root = new TestStub("root", 0);
    root->m_shouldRun = false;
    TestStub* parent = new TestStub("parent", root);
    parent->m_shouldRun = false;
    root->addChild(parent);
    TestStub* child = new TestStub("child", parent);
    child->m_shouldRun = true;
    parent->addChild(child);
    model->setRootItem(root);
}
}

// helper
void RunnerModelTest::executeItems(RunnerModel* model)
{
    TestExecutor* exec = new TestExecutor;
    exec->setRoot(model->rootItem());
    QSignalSpy s(exec, SIGNAL(allDone()));
    exec->go();
    Q_ASSERT(s.count() == 1);
    delete exec;
}

// test command
void RunnerModelTest::runItems()
{
    QMap<QString, QSignalSpy*> spies;
    spies["startedC"] = new QSignalSpy(model, SIGNAL(numStartedChanged(int)));
    setUpResultSpies(spies);
    fillModel(model);

    executeItems(model);

    assertSignalValue(spies.take("startedC"), 1);
    assertSignalValue(spies.take("successC"), 1);

    QMutableMapIterator<QString, QSignalSpy*> it(spies);
    while(it.hasNext()) {
        it.next();
        QSignalSpy* spy = it.value();
        QString description = it.key();
        it.remove();
        KOMPARE_MSG(0, spy->size(), description);
        delete spy;
    }
}

//command
void RunnerModelTest::dataChangedSignalsOnRun()
{
    bool fill = false;
    RunnerModelStub* model = new RunnerModelStub(fill);
    fillModel(model); // adds a test named 'parent' and a child-test named 'child'.

    QSignalSpy* dataChanged = new QSignalSpy(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    executeItems(model);

    // dataChanged must have been emitted with both the parent's and the child's index
    // the child since it has been run
    // the parent since it's state will have changed after the child has been run.

    KOMPARE(2, dataChanged->count());

    Test* emitted1 = takeTestFromSpy(dataChanged);
    Test* emitted2 = takeTestFromSpy(dataChanged);
    QStringList actualNames = QStringList() << emitted1->name() << emitted2->name();

    KVERIFY(actualNames.contains("child"));
    KVERIFY(actualNames.contains("parent"));

    delete model;
    delete dataChanged;
}

//test command
void RunnerModelTest::errorHandling()
{
    QVariant illegal;
    KOMPARE(illegal, model->data(QModelIndex()));
    KOMPARE(illegal, model->data(model->index(0, 1), Qt::CheckStateRole));
    KOMPARE(illegal, model->data(model->index(0, 0), Qt::CheckStateRole));

    KOMPARE(false, model->setData(QModelIndex(), QVariant(), Qt::DisplayRole));
    KOMPARE(0, model->flags(QModelIndex()));

    KOMPARE(QModelIndex(), model->parent(QModelIndex()));
}

//test command
void RunnerModelTest::countItems()
{
    QMap<QString, QSignalSpy*> spies;
    spies["totalC"] = new QSignalSpy(model, SIGNAL(numTotalChanged(int)));
    spies["selectedC"] = new QSignalSpy(model, SIGNAL(numSelectedChanged(int)));
    setUpResultSpies(spies);

    model->fill1();
    model->countItems();

    // all should be emitted just once
    foreach(QSignalSpy* spy, spies) {
        KOMPARE(1, spy->size());
    }

    assertSignalValue(spies.take("totalC"), 2);
    assertSignalValue(spies.take("selectedC"), 2);
    assertSignalValue(spies.take("successC"), 1);
    assertSignalValue(spies.take("fatalC"), 1);

    // the others are zero
    foreach(QSignalSpy* spy, spies) {
        assertSignalValue(spy, 0);
    }
}

// helper
void RunnerModelTest::assertSignalValue(QSignalSpy* spy, int expected)
{
    QVariant actual = spy->takeFirst().at(0);
    KOMPARE(QVariant(expected), actual);
    delete spy;
}

// helper
void RunnerModelTest::assertDataAt(const QVariant& expected, int row, int column)
{
    QVariant actual = model->data(model->index(row, column), Qt::DisplayRole);
    //qDebug() << "actual >" << actual << "< ; expected >" << expected << "<";
    KOMPARE_MSG(expected, actual, 
        QString("Expected: %1 at row %2 & col %3").arg(expected.toString()).arg(row).arg(column));
}

// helper
void RunnerModelTest::verifyRowContent(int index)
{
    QString rowStr = QString::number(index);
    assertDataAt(rowStr + '0', index, 0);

    KOMPARE(0, model->rowCount(model->index(index, 0))); // no children
    KOMPARE(1, model->columnCount(model->index(index, 0)));

}

// helper
void RunnerModelTest::setUpResultSpies(QMap<QString, QSignalSpy*>& spies)
{
    spies["successC"] = new QSignalSpy(model, SIGNAL(numSuccessChanged(int)));
    spies["infoC"]    = new QSignalSpy(model, SIGNAL(numInfosChanged(int)));
    spies["warnC"]    = new QSignalSpy(model, SIGNAL(numWarningsChanged(int)));
    spies["errorC"]   = new QSignalSpy(model, SIGNAL(numErrorsChanged(int)));
    spies["fatalC"]   = new QSignalSpy(model, SIGNAL(numFatalsChanged(int)));
    spies["exceptionC"] = new QSignalSpy(model, SIGNAL(numExceptionsChanged(int)));
}

// command
void RunnerModelTest::updateViewLastItem()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    model->fill2();
    QSignalSpy* s = new QSignalSpy(model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)));
    QModelIndex secondTopIndex = model->index(1,0);
    model->updateView(secondTopIndex); // this should emit datachanged item2->child21

    KOMPARE(1, s->size());
    QList<QVariant> arg = s->takeFirst();

    QModelIndex arg1 = arg[0].value<QModelIndex>();
    KOMPARE(arg1, secondTopIndex);
    QModelIndex arg2 = arg[1].value<QModelIndex>();
    KOMPARE(arg2, secondTopIndex.child(0,0));

    delete s;
}

QTEST_KDEMAIN(RunnerModelTest, GUI)
