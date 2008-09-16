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


#include "selectionstoretest.h"
#include <QtTest/QTest>

#include "../test.h"
#include "../selectionstore.h"
#include "kasserts.h"

using Veritas::SelectionStoreTest;

void SelectionStoreTest::init()
{
}

void SelectionStoreTest::cleanup()
{
}

// TODO refactor this mess

void SelectionStoreTest::rootOnly()
{
    SelectionStore* s = new SelectionStore;
    Test* root = new Test("root", 0);
    root->check();
    s->saveState(root);
    KVERIFY(!s->wasDeselected(root));

    root->unCheck();
    s->saveState(root);
    KVERIFY(s->wasDeselected(root));
}

void SelectionStoreTest::newObject()
{
    SelectionStore* s = new SelectionStore;
    Test* root = new Test("root", 0);
    root->unCheck();
    s->saveState(root);

    Test* root2 = new Test("root", 0);
    KVERIFY(s->wasDeselected(root2));
}

void SelectionStoreTest::saveMultiple()
{
    SelectionStore* s = new SelectionStore;
    Test* test1 = new Test("test1", 0);
    Test* test2 = new Test("test2", 0);
    Test* test3 = new Test("test3", 0);

    test1->unCheck();
    test2->check();
    test3->unCheck();

    s->saveState(test1);
    s->saveState(test2);
    s->saveState(test3);

    KVERIFY(s->wasDeselected(test1));
    KVERIFY(!s->wasDeselected(test2));
    KVERIFY(s->wasDeselected(test3));
}

void SelectionStoreTest::testTree()
{
    SelectionStore* s = new SelectionStore;
    Test* root = new Test("test1", 0);
    Test* child1 = new Test("test2", root);
    Test* child2 = new Test("test3", root);

    child1->check();
    child2->unCheck();

    s->saveState(child1);
    s->saveState(child2);

    KVERIFY(!s->wasDeselected(child1));
    KVERIFY(s->wasDeselected(child2));
}

void SelectionStoreTest::testTreeWithIdenticalNames()
{
    SelectionStore* s = new SelectionStore;
    Test* root1 = new Test("test1", 0);
    Test* child1 = new Test("child", root1);
    Test* root2 = new Test("root2", 0);
    Test* child2 = new Test("child", root2);

    child1->check();
    child2->unCheck();

    s->saveState(child1);
    s->saveState(child2);

    KVERIFY(!s->wasDeselected(child1));
    KVERIFY(s->wasDeselected(child2));
}

void SelectionStoreTest::saveRecursive()
{
    SelectionStore* s = new SelectionStore;
    Test* root = new Test("test1", 0);
    Test* child1 = new Test("test2", root);
    Test* child2 = new Test("test3", root);
    Test* child21 = new Test("test21", child2);
    root->addChild(child1);
    root->addChild(child2);
    child2->addChild(child21);

    root->check();
    child1->check();
    child2->unCheck();
    child21->unCheck();

    s->saveTree(root);

    root = new Test("test1", 0);
    child1 = new Test("test2", root);
    child2 = new Test("test3", root);
    child21 = new Test("test21", child2);

    KVERIFY(!s->wasDeselected(root));
    KVERIFY(!s->wasDeselected(child1));
    KVERIFY(s->wasDeselected(child2));
    KVERIFY(s->wasDeselected(child21));
}


void SelectionStoreTest::restoreRecursive()
{
    SelectionStore* s = new SelectionStore;
    Test* root = new Test("test1", 0);
    Test* child1 = new Test("test2", root);
    root->addChild(child1);
    Test* child2 = new Test("test3", root);
    root->addChild(child2);
    Test* child21 = new Test("test21", child2);
    child2->addChild(child21);

    root->check();
    child1->check();
    child2->unCheck();
    child21->unCheck();

    s->saveTree(root);

    // reset the tree
    root = new Test("test1", 0);
    child1 = new Test("test2", root);
    root->addChild(child1);
    child2 = new Test("test3", root);
    root->addChild(child2);
    child21 = new Test("test21", child2);
    child2->addChild(child21);

    // insert some more
    Test* child3 = new Test("child3", root);
    root->addChild(child3);
    Test* child22 = new Test("child22", child2);
    child2->addChild(child22);

    s->restoreTree(root);

    KVERIFY(root->isChecked());
    KVERIFY(child1->isChecked());
    KVERIFY(!child2->isChecked());
    KVERIFY(!child21->isChecked());
    KVERIFY(!child22->isChecked());
    KVERIFY(child3->isChecked());
}

void SelectionStoreTest::ignoreRoot()
{
    // The invisible root of the test-tree should not be saved/restored.

    SelectionStore* s = new SelectionStore;
    Test* root = new Test("root", 0);
    root->check();
    s->saveTree(root);

    Test* root2 = new Test("root", 0);
    s->restoreTree(root2);
    KVERIFY(root2->isChecked());

    root->unCheck();
    s->saveTree(root);

    root2 = new Test("root", 0);
    s->restoreTree(root2);
    KVERIFY(root2->isChecked());
}

QTEST_MAIN( SelectionStoreTest )
#include "selectionstoretest.moc"
