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

#include <runneritem.h>
#include <QString>

#include "kasserts.h"
#include "runneritemtest.h"

using QxRunner::RunnerItem;

// RunnerItem is abstract so stub the run method
class ConcreteRunnerItem : public RunnerItem
{
public:
    ConcreteRunnerItem(const QList<QVariant>& data, RunnerItem* parent = 0)
        : RunnerItem(data, parent)
    {}
    int run() { return 0; }
};

void RunnerItemTest::init()
{
    columns = QList<QVariant>();
    column1 = 0;
    column2 = "one";
    column3 = 3.0;
    columns << column1 << column2 << column3;
    root = new ConcreteRunnerItem(columns);
}

void RunnerItemTest::cleanup()
{
    delete root;
}

// test command
void RunnerItemTest::simpleRoot()
{
    assertNrofChildren(root, 0);
    assertNrofColumns(root, 3);
    assertDefaultResult(root);
    KVERIFY_MSG(root->isSelected(), "Not selected. Item should be enabled by default");
    KOMPARE_MSG(0, root->row(), "No parent so should surely be the first row");
    KOMPARE(NULL, root->parent());
}

// test command
void RunnerItemTest::multipleColumns()
{
    KOMPARE(column1, root->data(0));
    KOMPARE(column2, root->data(1));
    KOMPARE(column3, root->data(2));
}

// test command
void RunnerItemTest::accessIllegalColumn()
{
    root->data(-1);
    root->data(3);
    root->data(10);
}

// test command
void RunnerItemTest::resetValue()
{
    root->setResult(2);
    root->clear();
    QVariant empty = "";

    assertDefaultResult(root);
    assertNrofColumns(root, 3);
    KOMPARE(empty, root->data(1));
    KOMPARE(empty, root->data(2));
}

// test command
void RunnerItemTest::appendChildren()
{
    QList<QVariant> child1Columns;
    child1Columns << "col1" << "col2" << "col3";
    ConcreteRunnerItem* child1 = new ConcreteRunnerItem(child1Columns, root);
    root->appendChild(child1);

    QList<QVariant> child2Columns;
    child2Columns << "something";
    ConcreteRunnerItem* child2 = new ConcreteRunnerItem(child2Columns, root);
    root->appendChild(child2);

    assertNrofChildren(root, 2);
    assertNrofChildren(child1, 0);
    assertNrofChildren(child2, 0);
    KOMPARE(root, child1->parent());
    KOMPARE(root, child2->parent());
    assertNrofColumns(child1, 3);
    assertNrofColumns(child2, 3);
    KOMPARE(0, child1->row());
    KOMPARE(1, child2->row());
}

QString toString(int expected, int actual)
{
    return QString(" expecting: ") + QString::number(expected) +
           QString(" got: ") + QString::number(actual); 
}

void RunnerItemTest::assertNrofChildren(ConcreteRunnerItem* item, int nrof)
{
    int actual = item->childCount();
    KOMPARE_MSG(nrof, actual, QString("Incorrect number of child items; ") + toString(nrof, actual));
}

void RunnerItemTest::assertNrofColumns(ConcreteRunnerItem* item, int nrof)
{
    int actual = item->columnCount();
    KOMPARE_MSG(nrof, actual, QString("Not the right number of columns; ") + toString(nrof, actual));
}

void RunnerItemTest::assertDefaultResult(ConcreteRunnerItem* item)
{
    KOMPARE(int(QxRunner::NoResult), item->result());
}

QTEST_MAIN( RunnerItemTest );
