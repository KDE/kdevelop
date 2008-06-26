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
#include <test.h>
#include <qtest_kde.h>
#include <kasserts.h>
#include <QString>

using Veritas::Test;
using Veritas::ut::TestTest;

namespace Veritas
{
namespace ut
{
// Test is abstract so stub the run method
class ConcreteTest : public Test
{
public:
    ConcreteTest(const QList<QVariant>& data, Test* parent = 0)
            : Test(data, parent) {}
    ConcreteTest(const QString& name, Test* parent = 0)
            : Test(name, parent) {}
    int run() { return 0; }
};
}
}

using Veritas::ut::ConcreteTest;

void TestTest::init()
{
    columns = QList<QVariant>();
    column1 = "";
    column2 = "one";
    column3 = 3.0;
    columns << column1 << column2 << column3;
    root = new ConcreteTest(columns);
}

void TestTest::cleanup()
{
    delete root;
}

// test command
void TestTest::simpleRoot()
{
    assertNrofChildren(root, 0);
    assertNrofColumns(root, Test::s_columnCount);
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
    assertNrofColumns(root, Test::s_columnCount);
    KOMPARE(empty, root->data(1));
    KOMPARE(empty, root->data(2));
}

// test command
void TestTest::appendChildren()
{
    QList<QVariant> child1Columns;
    child1Columns << "col1" << "col2" << "col3";
    ConcreteTest* child1 = new ConcreteTest(child1Columns, root);
    root->addChild(child1);

    ConcreteTest* child2 = new ConcreteTest("child2", root);
    root->addChild(child2);

    assertNrofChildren(root, 2);
    assertNrofChildren(child1, 0);
    assertNrofChildren(child2, 0);
    KOMPARE(root, child1->parent());
    KOMPARE(root, child2->parent());
    assertNrofColumns(child1, Test::s_columnCount);
    assertNrofColumns(child2, Test::s_columnCount);
    KOMPARE(0, child1->row());
    KOMPARE(1, child2->row());
    KOMPARE("child2", child2->name());
}

QString toString(int expected, int actual)
{
    return QString(" expecting: ") + QString::number(expected) +
           QString(" got: ") + QString::number(actual);
}

void TestTest::assertNrofChildren(ConcreteTest* item, int nrof)
{
    int actual = item->childCount();
    KOMPARE_MSG(nrof, actual, QString("Incorrect number of child items; ") + toString(nrof, actual));
}

void TestTest::assertNrofColumns(ConcreteTest* item, int nrof)
{
    int actual = item->columnCount();
    KOMPARE_MSG(nrof, actual, QString("Not the right number of columns; ") + toString(nrof, actual));
}

void TestTest::assertDefaultResult(ConcreteTest* item)
{
    KOMPARE(int(Veritas::NoResult), item->state());
}

QTEST_KDEMAIN(TestTest, NoGUI)
