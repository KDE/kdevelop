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

#ifndef XTEST_SAMPLE
#define XTEST_SAMPLE

#include <cppunit/Test.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>

#include "qxcppunit/testrunner.h"

namespace sample
{

using CppUnit::Test;
using CppUnit::TestSuite;
using CppUnit::TestFixture;
using QxCppUnit::TestRunner;

class FooTest : public TestFixture
{
public:
    void testFoo1() {};
    void testFoo2() {};
    void testFoo3() {};
    static Test* suite() {
        TestSuite* suite = new CppUnit::TestSuite("FooTest");
        typedef CppUnit::TestCaller<FooTest> fooCaller;
        suite->addTest(new fooCaller("testFoo1", &FooTest::testFoo1));
        suite->addTest(new fooCaller("testFoo2", &FooTest::testFoo2));
        suite->addTest(new fooCaller("testFoo3", &FooTest::testFoo3));
        return suite;
    }
};

class BarTest : public TestFixture
{
public:
    void testBar() {};
    static Test* suite() {
        TestSuite* suite = new CppUnit::TestSuite("BarTest");
        typedef CppUnit::TestCaller<BarTest> barCaller;
        suite->addTest(new barCaller("testBar", &BarTest::testBar));
        return suite;
    }
};

class BazTest : public TestFixture
{
public:
    void foo() {};
    void fuz() { CPPUNIT_ASSERT(0); };
    void bar() {};
    void baz() {};
    void rim() {};
    void raf() {};

    static Test* suite() {
        TestSuite* suite = new CppUnit::TestSuite("BazTest");
        typedef CppUnit::TestCaller<BazTest> bazCaller;
        suite->addTest(new bazCaller("foo", &BazTest::foo));
        suite->addTest(new bazCaller("fuz", &BazTest::fuz));
        suite->addTest(new bazCaller("bar", &BazTest::bar));
        suite->addTest(new bazCaller("baz", &BazTest::baz));
        suite->addTest(new bazCaller("rim", &BazTest::rim));
        suite->addTest(new bazCaller("raf", &BazTest::raf));
        return suite;
    }
};

QWidget* testRunnerWidget()
{
    TestRunner* runner = new TestRunner();
    runner->addTest(FooTest::suite());
    runner->addTest(BarTest::suite());
    runner->addTest(BazTest::suite());
    return runner->spawn();
}

}

#endif // XTEST_SAMPLE_H
