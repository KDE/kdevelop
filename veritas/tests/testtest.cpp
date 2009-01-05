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
#include <QString>
#include "testutils.h"

#include "../test.h"
#include "../internal/test_p.h"

using Veritas::Test;
using Veritas::TestTest;

namespace Veritas
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

using Veritas::TestFake;

void TestTest::init()
{
    columns = QList<QVariant>();
    column1 = "";
    column2 = "one";
    column3 = 3.0;
    columns << column1 << column2 << column3;
    m_root = new TestFake(columns);
}

void TestTest::cleanup()
{
    delete m_root;
}

// test command
void TestTest::simpleRoot()
{
    assertNrofChildren(m_root, 0);
    assertDefaultResult(m_root);
    KVERIFY_MSG(m_root->internal()->isChecked(), "Not selected. Item should be enabled by default");
    KOMPARE_MSG(0, m_root->row(), "No parent so should surely be the first row");
    KOMPARE(NULL, m_root->parent());
    KOMPARE("", m_root->name());
}

// test command
void TestTest::multipleColumns()
{
    KOMPARE(column1, m_root->internal()->data(0));
    KOMPARE(column2, m_root->internal()->data(1));
    KOMPARE(column3, m_root->internal()->data(2));
}

// test command
void TestTest::accessIllegalColumn()
{
    m_root->internal()->data(-1);
    m_root->internal()->data(3);
    m_root->internal()->data(10);
}

// test command
void TestTest::resetValue()
{
    m_root->internal()->clear();
    QVariant empty = "";

    assertDefaultResult(m_root);
    KOMPARE(empty, m_root->internal()->data(1));
    KOMPARE(empty, m_root->internal()->data(2));
}

// test command
void TestTest::appendChildren()
{
    QList<QVariant> child1Columns;
    child1Columns << "col1" << "col2" << "col3";
    TestFake* child1 = new TestFake(child1Columns, m_root);
    m_root->addChild(child1);

    TestFake* child2 = new TestFake("child2", m_root);
    m_root->addChild(child2);

    assertNrofChildren(m_root, 2);
    assertNrofChildren(child1, 0);
    assertNrofChildren(child2, 0);
    KOMPARE(m_root, child1->parent());
    KOMPARE(m_root, child2->parent());
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
void TestTest::assertDefaultResult(TestFake* item)
{
    KOMPARE(int(Veritas::NoResult), item->state());
}

// command
void TestTest::retrieveLeaves()
{
    QList<Test*> leafs = m_root->leafs();
    KOMPARE(0, leafs.size());

    // single lvl1 item
    TestFake* child1 = new TestFake("child1", m_root);
    m_root->addChild(child1);
    leafs = m_root->leafs();
    KOMPARE(1, leafs.size());
    KOMPARE("child1", leafs[0]->name());

    // two lvl1 items
    TestFake* child2 = new TestFake("child2", m_root);
    m_root->addChild(child2);
    leafs = m_root->leafs();
    KOMPARE(2, leafs.size());
    KOMPARE("child1", leafs[0]->name());
    KOMPARE("child2", leafs[1]->name());

    // add lvl2 item
    TestFake* child11 = new TestFake("child11", child1);
    child1->addChild(child11);
    leafs = m_root->leafs();
    KOMPARE(2, leafs.size());
    KOMPARE("child11", leafs[0]->name());
    KOMPARE("child2", leafs[1]->name());

    // nother lvl2
    TestFake* child12 = new TestFake("child12", child1);
    child1->addChild(child12);
    leafs = m_root->leafs();
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

// command
void TestTest::reparent()
{
    Test* test = new Test("foo", 0);
    KOMPARE(0, test->parent());
    Test* parent = new Test("parent", 0);
    test->setParent(parent);
    KOMPARE(parent, test->parent());

    delete parent;
}

// command
void TestTest::childByNameSunny()
{
    // exercise
    Test* foo = new Test("foo", m_root);
    m_root->addChild(foo);
    Test* bar = new Test("bar", m_root);
    m_root->addChild(bar);

    // verify
    KOMPARE(foo, m_root->childNamed("foo"));
    KOMPARE(bar, m_root->childNamed("bar"));
}

// command
void TestTest::childByNameNonExistantShouldReturnNull()
{
    KOMPARE(0, m_root->childNamed("non_existant_child"));
    
    Test* foo = new Test("foo", m_root);
    m_root->addChild(foo);

    KOMPARE(0, m_root->childNamed("non_existant_child"));
}

// command
void TestTest::addIdenticallyNamedChildShouldFail()
{
    Test* foo = new Test("foo", m_root);
    KVERIFY(m_root->addChild(foo));
    KVERIFY(!m_root->addChild(foo));
}

QTEST_KDEMAIN(TestTest, NoGUI)
