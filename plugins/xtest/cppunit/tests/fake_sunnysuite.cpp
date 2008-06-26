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

#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCaller.h>
#include <cppunitwrapper.h>

// "0 RootSuite"
// "0 0 FooTest"
// "0 0 0 testCmd1"
// "0 0 1 testCmd2"
// "0 0 2 x"
// "0 1 BarTest"
// "0 1 0 testCmd1"
// "0 1 1 testCmd2"
// "0 1 2 x"
// "0 2 BazTest"
// "0 2 0 testCmd1"
// "0 2 1 x"
// "0 3 x"
// "1 x"

using namespace CppUnit;

class FooTest : public TestFixture
{
public:
    void testCmd1() {}
    void testCmd2() {}

    static Test* suite() {
        TestSuite* suite = new CppUnit::TestSuite("FooTest");
        typedef TestCaller<FooTest> fooCaller;
        suite->addTest(new fooCaller("testCmd1", &FooTest::testCmd1));
        suite->addTest(new fooCaller("testCmd2", &FooTest::testCmd2));
        return suite;
    }
};

class BarTest : public TestFixture
{
public:
    void testCmd1() {}
    void testCmd2() {
        CPPUNIT_ASSERT(false);
    }

    static Test* suite() {
        TestSuite* suite = new CppUnit::TestSuite("BarTest");
        typedef TestCaller<BarTest> barCaller;
        suite->addTest(new barCaller("testCmd1", &BarTest::testCmd1));
        suite->addTest(new barCaller("testCmd2", &BarTest::testCmd2));
        return suite;
    }
};

#include <QThread>

class Sleep : public QThread
{
public:
    void run() {}
    static void waitAbit() {
        QThread::usleep(500000);    // 0.5 sec
    }
};

class BazTest : public TestFixture
{
public:
    void testCmd1() { Sleep::waitAbit(); }

    static Test* suite() {
        TestSuite* suite = new CppUnit::TestSuite("BazTest");
        typedef TestCaller<BazTest> bazCaller;
        suite->addTest(new bazCaller("testCmd1", &BazTest::testCmd1));
        return suite;
    }
};

Test* suite()
{
    TestSuite* suite = new TestSuite("RootSuite");
    suite->addTest(FooTest::suite());
    suite->addTest(BarTest::suite());
    suite->addTest(BazTest::suite());
    return suite;
}

CPPUNIT_XTEST_MAIN(suite())
