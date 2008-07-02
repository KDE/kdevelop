/*
 * This file is part of KDevelop
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
#include <cppunit/extensions/HelperMacros.h>
#include <cppunitwrapper.h>
#include <vector>

// "0 FooSuite"
// "0 0 FooTest"
// "0 0 0 fooCmd"
// "0 0 1 x"
// "0 1 x"
// "1 BarSuite"
// "1 0 BarTest"
// "1 0 0 barCmd"
// "1 0 1 x"
// "1 1 x"
// "2 x"

using namespace CppUnit;

class FooTest : public TestFixture
{
CPPUNIT_TEST_SUITE( FooTest );
    CPPUNIT_TEST( fooCmd );
CPPUNIT_TEST_SUITE_END();

public:
    void fooCmd() {}
};

class BarTest : public TestFixture
{
CPPUNIT_TEST_SUITE( BarTest );
    CPPUNIT_TEST( barCmd );
CPPUNIT_TEST_SUITE_END();

public:
    void barCmd() {}
};

std::vector<TestSuite*> suite()
{
    TestSuite* fs = new TestSuite("FooSuite");
    fs->addTest(FooTest::suite());
    TestSuite* bs = new TestSuite("BarSuite");
    bs->addTest(BarTest::suite());

    std::vector<TestSuite*> suites;
    suites.push_back(fs);
    suites.push_back(bs);
    return suites;
}

CPPUNIT_VERITAS_MAIN_(suite())
