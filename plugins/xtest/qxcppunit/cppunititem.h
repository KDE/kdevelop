/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
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
 
/*!
 * \file  cppunititem.h
 *
 * \brief Declares class CppUnitItem.
 */

#ifndef CPPUNITITEM_H
#define CPPUNITITEM_H

#include <qxrunner/runneritem.h>
#include <cppunit/Test.h>
#include <cppunit/TestListener.h>

using namespace QxRunner;

namespace QxCppUnit
{

/*!
 * \brief The CppUnitItem class executes a CppUnit test case.
 *
 * The CppUnitItem class registers itself as a test listener and runs
 * a CppUnit test. In case of failure the item columns contain the
 * failure information, whereby the textual representation for a
 * warning is set to 'failure' which complies with CppUnit speech.
 * The class is designed to execute one individual test and not a test
 * suite.
 */

class CppUnitItem : public RunnerItem, private CPPUNIT_NS::TestListener
{
public: // Operations

    /*!
     * Constructs a CppUnit item for one individual \a test with the
     * test name in \a data and the given \a parent.
     */
    CppUnitItem(const QList<QVariant>& data, RunnerItem* parent = 0,
                CPPUNIT_NS::Test* test = 0);

    /*!
     * Destroys this CppUnit item.
     */
    ~CppUnitItem();

    /*!
     * Runs the test and returns the test result code which is of type
     * QxRunner::RunnerResult.
     */
    int run();

private: // Operations

    /*!
     * Called when a failure occurs while running the test. Records
     * failure information in the item.
     */
    void addFailure(const CPPUNIT_NS::TestFailure& failure);

private: // Attributes

    CPPUNIT_NS::Test* m_test;
};

} // namespace

#endif // CPPUNITITEM_H
