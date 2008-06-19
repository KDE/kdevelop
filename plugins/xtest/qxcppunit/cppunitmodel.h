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
 * \file  cppunitmodel.h
 *
 * \brief Declares class CppUnitModel.
 */

#ifndef CPPUNITMODEL_H
#define CPPUNITMODEL_H

#include <qxrunner/runnermodel.h>
#include <cppunit/Test.h>

namespace QxCppUnit
{

/*!
 * \brief The CppUnitModel class maintains CppUnit data and executes
 *        CppUnit unit tests.
 *
 * This class stores CppUnit test data using CppUnitItem objects. The
 * CppUnit tests referenced by the CppUnitItem objects can be executed.
 * Test results are prepared for representation in views.
 *
 * The expected results from this model are:
 *
 *
 * - QxRunner::RunSuccess - CppUnit test case passed
 * - QxRunner::RunWarning - CppUnit assertion failed
 * - QxRunner::RunError   - CppUnit reported an error
 */

class CppUnitModel : public QxRunner::RunnerModel
{
    Q_OBJECT

public: // Operations

    CppUnitModel(QObject* parent = 0);
    ~CppUnitModel();

    void readTests(const QFileInfo& exe);

    QString name() const;
    QString about() const;

    /*!
     * Appends the \a test to the list of CppUnit tests in the model.
     * \a test can be one individual test or a test suite.
     */
    void addTest(CPPUNIT_NS::Test* test) const;

private: // Operations

    /*!
     * Helper method to append a CppUnitItem object for the \a test
     * to the given \a parent. If \a test is a suite then recursively
     * a CppUnitItem object for each test in the suite is appended.
     */
    void addTestItem(CPPUNIT_NS::Test* test, QxRunner::RunnerItem* parent) const;
};

} // namespace

#endif // CPPUNITMODEL_H
