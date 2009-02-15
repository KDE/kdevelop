/*
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

/*! Example CppUnit testcase intended to showcase the KDevelop4 xTest runner */

#ifndef MONEYTEST_H
#define MONEYTEST_H

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>

class MoneyTest : public CppUnit::TestFixture
{
public:
    void testCreate();
    void testNegative();
    void testZero();

    static CppUnit::Test* suite();
};

#endif // MONEYTEST_H
