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
 * \file  testrunner.h
 *
 * \brief Declares class TestRunner.
 */

#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include "qxcppunitexport.h"

#include <QWidget>
#include <cppunit/Portability.h>
#include <cppunit/portability/CppUnitVector.h>

namespace CPPUNIT_NS
{
class Test;
}

namespace QxRunner
{
class Runner;
}

namespace QxCppUnit
{

class CppUnitModel;

/*!
 * \brief The TestRunner class starts a QxCppUnit application.
 *
 * This class creates a CppUnitModel and adds a CppUnitItem object for
 * each given CppUnit test to the model. It sets an application icon
 * and then shows the test runner main window on screen. This is the
 * only class of the library that is used by clients to build a GUI
 * test runner for CppUnit.
 */

class QXCPPUNIT_EXPORT TestRunner
{
public: // Operations

    /*!
     * Constructs a test runner. Creates the CppUnitModel instance.
     */
    TestRunner();

    /*!
     * Destroys this test runner.
     */
    virtual ~TestRunner();

    /*!
     * Appends the \a test to the list of CppUnit tests. \a test can be
     * one individual test or a test suite.
     */
    void addTest(CPPUNIT_NS::Test* test) const;

    /*!
     * Appends the \a tests to the list of CppUnit tests. An entry in
     * \a tests can be one individual test or a test suite.
     */
    void addTests(const CppUnitVector<CPPUNIT_NS::Test*>& tests) const;

    /*!
     * Sets the application icon and shows the main window.
     */
    void run();

    /*!
     * Don not start a full application, just construct the widget
     */
    QWidget* spawn();

private: // Operations

    // Copy and assignment not supported.
    TestRunner(const TestRunner&);
    TestRunner& operator=(const TestRunner&);

private: // Attributes

    QxRunner::Runner* m_runner;
    CppUnitModel*     m_model;
};

} // namespace

#endif // TESTRUNNER_H
