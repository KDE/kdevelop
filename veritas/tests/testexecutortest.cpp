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

#include "testexecutortest.h"


#include <qtest_kde.h>
#include <KDebug>
#include <QModelIndex>
#include <QMetaType>
#include <QSignalSpy>

#include "kasserts.h"

#include "../test.h"
#include "../testexecutor.h"

using Veritas::Test;
using Veritas::TestExecutor;
using Veritas::TestExecutorTest;

namespace Veritas
{

class TestStub : public Test
{
public:
    TestStub(const QString& name, Test* parent) : Test(name, parent), m_shouldRun(false) {}
    virtual ~TestStub() {}

    int run() { emit executionFinished(); kDebug() << "run " << name(); return 0; }
    bool shouldRun() const { return m_shouldRun; }
    bool m_shouldRun;
};

}

using Veritas::TestStub;


void TestExecutorTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

void TestExecutorTest::init()
{
    m_executor = new TestExecutor;
    m_allDoneSpy = new QSignalSpy(m_executor, SIGNAL(allDone()));
    root = 0;
    m_garbage.clear();
}

void TestExecutorTest::assertAllDone()
{
    KOMPARE_MSG(1, m_allDoneSpy->count(), "All done signal not emitted.");
}

void TestExecutorTest::cleanup()
{
    delete m_allDoneSpy;
    delete m_executor;
    if (root) delete root;
    qDeleteAll(m_garbage);
}

QSignalSpy* TestExecutorTest::createSpy(Test* t)
{
    QSignalSpy* s = new QSignalSpy(t, SIGNAL(executionFinished()));
    m_garbage << s;
    return s;
}

TestStub* TestExecutorTest::createEnabledTest(const QString& name, Test* parent, QSignalSpy*& s)
{
    TestStub* test = new TestStub(name, parent);
    if (parent) parent->addChild(test);
    test->m_shouldRun = true;
    s = createSpy(test);
    return test;
}

TestStub* TestExecutorTest::createTest(const QString& name, Test* parent, QSignalSpy*& s)
{
    TestStub* test = new TestStub(name, parent);
    if (parent) parent->addChild(test);
    s = createSpy(test);
    return test;
}

void TestExecutorTest::assertRun(QSignalSpy* s, Test* t)
{
    // If this test got run, the executionFinished signal should
    // have been emitted. Hench spy->count() == 1
    KOMPARE_MSG(1, s->count(), t->name() + " should have been run.");
}

void TestExecutorTest::assertRunTwice(QSignalSpy* s, Test* t)
{
    // If this test got run, the executionFinished signal should
    // have been emitted. Hench spy->count() == 1
    KOMPARE_MSG(2, s->count(),
        t->name() + " should have been run twice. (was : " +
        QString::number(s->count()) + ")");
}

void TestExecutorTest::assertNotRun(QSignalSpy* s, Test* t)
{
    // No executionFinished signal should have been emitted. 
    // Hench spy->count() == 0
    KOMPARE_MSG(0, s->count(), t->name() + " should not have been run.");
}

void TestExecutorTest::rootOnly()
{
    // setup
    QSignalSpy* s;
    root = createEnabledTest("Root", 0, s);
    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50); // asynchronous ...

    // verify
    assertRun(s, root);
    assertAllDone();
}

void TestExecutorTest::noneShouldRun()
{
    // root
    //   child
    //      child11

    // setup
    TestStub *child, *child11;
    QSignalSpy *rs, *cs, *c11s;

    root    = createTest("root", 0, rs);
    child   = createTest("child", root, cs);
    child11 = createTest("child11", child, c11s);

    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50); // asynchronous ...

    // verify
    assertNotRun(rs, root);
    assertNotRun(cs, child);
    assertNotRun(c11s, child11);
    assertAllDone();
}

void TestExecutorTest::levelOneAggregate()
{
    // root
    //   child [should run]
    //      child11
    //      child12

    // setup
    TestStub *child, *child11, *child12;
    QSignalSpy *rs, *cs, *c11s, *c12s;

    root    = createTest("root", 0, rs);
    child   = createEnabledTest("child", root, cs);
    child11 = createTest("child11", child, c11s);
    child12 = createTest("child12", child, c12s);

    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50); // asynchronous ...

    // verify
    assertRun(cs, child);
    assertNotRun(rs, root);
    assertNotRun(c11s, child11);
    assertNotRun(c12s, child12);
    assertAllDone();
}

void TestExecutorTest::levelTwoAggregate()
{
    // root
    //   child1
    //      child2 [should run]
    //         child3

    // setup
    TestStub *child1, *child2, *child3;
    QSignalSpy *rs, *c1s, *c2s, *c3s;

    root    = createTest("root", 0, rs);
    child1  = createTest("child1", root, c1s);
    child2  = createEnabledTest("child2", child1, c2s);
    child3  = createTest("child3", child2, c3s);

    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50); // asynchronous ...

    // verify
    assertRun(c2s, child2);
    assertNotRun(rs,  root);
    assertNotRun(c1s, child1);
    assertNotRun(c3s, child3);
    assertAllDone();
}

void TestExecutorTest::multipleAggregates()
{
    // root
    //   child1
    //      child11 [should run]
    //      child12 [should run]
    //   child2
    //      child21 [should run]

    // setup
    TestStub *child1, *child11, *child12, *child2, *child21;
    QSignalSpy *rs, *c1s, *c11s, *c12s, *c2s, *c21s;

    root    = createTest("root", 0, rs);
    child1  = createTest("child1", root, c1s);
    child11 = createEnabledTest("child11", child1, c11s);
    child12 = createEnabledTest("child12", child1, c12s);
    child2  = createTest("child2", root, c2s);
    child21 = createEnabledTest("child21", child2, c21s);

    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50); // asynchronous ...

    // verify
    assertRun(c11s, child11);
    assertRun(c12s, child12);
    assertRun(c21s, child21);
    assertNotRun(c1s, child1);
    assertNotRun(c2s, child2);
    assertNotRun(rs,  root);
    assertAllDone();
}


void TestExecutorTest::unbalancedAggregates()
{
    // root
    //   child1 [should run]
    //   child2
    //      child21 [should run]
    //      child22 [should run]

    // setup
    TestStub *child1, *child2, *child21, *child22;
    QSignalSpy *rs, *c1s, *c2s, *c21s, *c22s;

    root    = createTest("root", 0, rs);
    child1  = createEnabledTest("child1", root, c1s);
    child2  = createTest("child2", root, c2s);
    child21 = createEnabledTest("child21", child2, c21s);
    child22 = createEnabledTest("child22", child2, c22s);

    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50); // asynchronous ...

    // verify
    assertRun(c1s,  child1);
    assertRun(c21s, child21);
    assertRun(c22s, child22);
    assertNotRun(c2s, child2);
    assertAllDone();
}

TestStub* TestExecutorTest::createDeselectedTest(const QString& name, Test* parent, QSignalSpy*& s)
{
    TestStub* t = createEnabledTest(name, parent, s);
    t->unCheck();
    return t;
}

void TestExecutorTest::noneSelected()
{
    // root
    //   child1 [should run] - deselcted
    //      child11
    //   child2 [should run] - deselected
    //      child21

    // setup
    TestStub *child1, *child11, *child2, *child21;
    QSignalSpy *rs, *c1s, *c2s, *c11s, *c21s;

    root    = createTest("root", 0, rs);
    child1  = createDeselectedTest("child1", root, c1s);
    child11 = createTest("child11", child1, c11s);
    child2  = createDeselectedTest("child2", root, c2s);
    child21 = createDeselectedTest("chld21", child2, c21s);

    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50); // asynchronous ...

    // verify
    assertNotRun(rs,   root);
    assertNotRun(c1s,  child1);
    assertNotRun(c11s, child11);
    assertNotRun(c2s,  child2);
    assertNotRun(c21s, child21);
    assertAllDone();
}

void TestExecutorTest::deselectedTests()
{
    // root
    //   child1 [should run]
    //   child2 [should run] - deselected
    //   child3 [should run] - deselected
    //   child4 [should run]

    // setup
    TestStub *child1, *child2, *child3, *child4;
    QSignalSpy *rs, *c1s, *c2s, *c3s, *c4s;

    root    = createTest("root", 0, rs);
    child1  = createEnabledTest("child1", root, c1s);
    child2  = createDeselectedTest("child2", root, c2s);
    child3  = createDeselectedTest("child3", root, c3s);
    child4  = createEnabledTest("child4", root, c4s);

    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50); // asynchronous ...

    // verify
    assertRun(c1s, child1);
    assertRun(c4s, child4);
    assertNotRun(rs,  root);
    assertNotRun(c2s, child2);
    assertNotRun(c3s, child3);
    assertAllDone();
}

void TestExecutorTest::runTwice()
{
    // root
    //   child1
    //      child11 [should run]
    //      child12 [should run]
    //   child2
    //      child21 [should run]

    // setup
    TestStub *child1, *child11, *child12, *child2, *child21;
    QSignalSpy *rs, *c1s, *c11s, *c12s, *c2s, *c21s;

    root    = createTest("root", 0, rs);
    child1  = createTest("child1", root, c1s);
    child11 = createEnabledTest("child11", child1, c11s);
    child12 = createEnabledTest("child12", child1, c12s);
    child2  = createTest("child2", root, c2s);
    child21 = createEnabledTest("child21", child2, c21s);

    m_executor->setRoot(root);

    // exercise
    m_executor->go();
    QTest::qWait(50);

    // need to refresh spies, since executor disconnects signals ...
    rs   = createSpy(root);
    c1s  = createSpy(child1);
    c11s = createSpy(child11);
    c12s = createSpy(child12);
    c2s  = createSpy(child2);
    c21s = createSpy(child21);

    m_executor->go();
    QTest::qWait(50);

    // verify
    assertRun(c11s, child11);
    assertRun(c12s, child12);
    assertRun(c21s, child21);
    assertNotRun(c1s, child1);
    assertNotRun(c2s, child2);
    assertNotRun(rs,  root);
}

#include "testexecutortest.moc"
QTEST_KDEMAIN(TestExecutorTest, NoGUI)

