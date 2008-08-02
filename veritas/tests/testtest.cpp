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

#include "testtest.h"
#include <qtest_kde.h>
#include <QString>

#include "kasserts.h"

#include "../test.h"

using Veritas::Test;
using Veritas::ut::TestTest;

namespace Veritas
{
namespace ut
{
// Test is abstract so stub the run method
class TestFake : public Test
{
public:
    TestFake(const QList<QVariant>& data, Test* parent = 0)
            : Test(data, parent) {}
    TestFake(const QString& name, Test* parent = 0)
            : Test(name, parent) {}

public slots:
    int run() { return 0; }
};
}
}

using Veritas::ut::TestFake;

#define TEST_COLUMN_COUNT 5

void TestTest::init()
{
    columns = QList<QVariant>();
    column1 = "";
    column2 = "one";
    column3 = 3.0;
    columns << column1 << column2 << column3;
    root = new TestFake(columns);
}

void TestTest::cleanup()
{
    delete root;
}

// test command
void TestTest::simpleRoot()
{
    assertNrofChildren(root, 0);
    assertNrofColumns(root, TEST_COLUMN_COUNT);
    assertDefaultResult(root);
    KVERIFY_MSG(root->selected(), "Not selected. Item should be enabled by default");
    KOMPARE_MSG(0, root->row(), "No parent so should surely be the first row");
    KOMPARE(NULL, root->parent());
    KOMPARE("", root->name());
}

// test command
void TestTest::multipleColumns()
{
    KOMPARE(column1, root->data(0));
    KOMPARE(column2, root->data(1));
    KOMPARE(column3, root->data(2));
}

// test command
void TestTest::accessIllegalColumn()
{
    root->data(-1);
    root->data(3);
    root->data(10);
}

// test command
void TestTest::resetValue()
{
    root->setState(Veritas::NoResult);
    root->clear();
    QVariant empty = "";

    assertDefaultResult(root);
    assertNrofColumns(root, TEST_COLUMN_COUNT);
    KOMPARE(empty, root->data(1));
    KOMPARE(empty, root->data(2));
}

// test command
void TestTest::appendChildren()
{
    QList<QVariant> child1Columns;
    child1Columns << "col1" << "col2" << "col3";
    TestFake* child1 = new TestFake(child1Columns, root);
    root->addChild(child1);

    TestFake* child2 = new TestFake("child2", root);
    root->addChild(child2);

    assertNrofChildren(root, 2);
    assertNrofChildren(child1, 0);
    assertNrofChildren(child2, 0);
    KOMPARE(root, child1->parent());
    KOMPARE(root, child2->parent());
    assertNrofColumns(child1, TEST_COLUMN_COUNT);
    assertNrofColumns(child2, TEST_COLUMN_COUNT);
    KOMPARE(0, child1->row());
    KOMPARE(1, child2->row());
    KOMPARE("child2", child2->name());
}

QString toString(int expected, int actual)
{
    return QString(" expecting: ") + QString::number(expected) +
           QString(" got: ") + QString::number(actual);
}

// helper
void TestTest::assertNrofChildren(TestFake* item, int nrof)
{
    int actual = item->childCount();
    KOMPARE_MSG(nrof, actual, QString("Incorrect number of child items; ") + toString(nrof, actual));
}

// helper
void TestTest::assertNrofColumns(TestFake* item, int nrof)
{
    int actual = item->columnCount();
    KOMPARE_MSG(nrof, actual, QString("Not the right number of columns; ") + toString(nrof, actual));
}

// helper
void TestTest::assertDefaultResult(TestFake* item)
{
    KOMPARE(int(Veritas::NoResult), item->state());
}

// command
void TestTest::retrieveLeaves()
{
    QList<Test*> leafs = root->leafs();
    KOMPARE(0, leafs.size());

    // single lvl1 item
    TestFake* child1 = new TestFake("child1", root);
    root->addChild(child1);
    leafs = root->leafs();
    KOMPARE(1, leafs.size());
    KOMPARE("child1", leafs[0]->name());

    // two lvl1 items
    TestFake* child2 = new TestFake("child2", root);
    root->addChild(child2);
    leafs = root->leafs();
    KOMPARE(2, leafs.size());
    KOMPARE("child1", leafs[0]->name());
    KOMPARE("child2", leafs[1]->name());

    // add lvl2 item
    TestFake* child11 = new TestFake("child11", child1);
    child1->addChild(child11);
    leafs = root->leafs();
    KOMPARE(2, leafs.size());
    KOMPARE("child11", leafs[0]->name());
    KOMPARE("child2", leafs[1]->name());

    // nother lvl2
    TestFake* child12 = new TestFake("child12", child1);
    child1->addChild(child12);
    leafs = root->leafs();
    KOMPARE(3, leafs.size());
    KOMPARE("child11", leafs[0]->name());
    KOMPARE("child12", leafs[1]->name());
    KOMPARE("child2", leafs[2]->name());

    // check lvl1's leafs
    leafs = child1->leafs();
    KOMPARE(2, leafs.size());
    KOMPARE("child11", leafs[0]->name());
    KOMPARE("child12", leafs[1]->name());
}

QTEST_KDEMAIN(TestTest, NoGUI)
