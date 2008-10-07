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

#include "treetraversetest.h"
#include <QtTest/QTest>

#include "../internal/utils.h"
#include "../test.h"

using Veritas::TreeTraverseTest;

void TreeTraverseTest::init()
{
}

void TreeTraverseTest::cleanup()
{
}

namespace {

class Visit
{
public:
    void operator()(Veritas::Test* t) {
        visited << t;
    }
    QList<Veritas::Test*> visited;
};

}

void TreeTraverseTest::simpleTree()
{
    Test* root = new Test("root", 0);
    Test* child1 = new Test("child1", root);
    root->addChild(child1);
    Test* child2 = new Test("child2", root);
    root->addChild(child2);

    Visit v;
    traverseTree(root, v);

    QVERIFY(v.visited.contains(root));
    QVERIFY(v.visited.contains(child1));
    QVERIFY(v.visited.contains(child2));

    delete root;
}

void TreeTraverseTest::nill()
{
    Visit v;
    traverseTree(0, v);
    QVERIFY(v.visited.isEmpty());
}

void TreeTraverseTest::rootOnly()
{
    Visit v;
    Test* root = new Test("root", 0);
    traverseTree(root, v);
    QCOMPARE(1, v.visited.count());
    QCOMPARE(root, v.visited[0]);

    delete root;
}

QTEST_MAIN( TreeTraverseTest )
#include "treetraversetest.moc"
