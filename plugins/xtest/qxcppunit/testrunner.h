/*!
 * \file  testrunner.h
 *
 * \brief Declares class TestRunner.
 */

#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include "qxcppunit_global.h"

#include <cppunit/Portability.h>
#include <cppunit/portability/CppUnitVector.h>

namespace CPPUNIT_NS {
	class Test;
}

namespace QxRunner {
	class Runner;
}

namespace QxCppUnit {

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
