/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_ITESTCONTROLLER_H
#define KDEVPLATFORM_ITESTCONTROLLER_H

#include "interfacesexport.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QStringList>

class KJob;

namespace KDevelop {

struct TestResult;

class IProject;
class ITestSuite;


/**
 * The result of a single unit test run
 **/
struct KDEVPLATFORMINTERFACES_EXPORT TestResult
{
    /**
     * Enumeration of possible test case results
     **/
    enum TestCaseResult
    {
        NotRun, ///< The test case was not selected for running.
        Skipped, ///< The test case was skipped.
        Passed, ///< The test case was run and passed.
        Failed, ///< The test case was run and failed.
        ExpectedFail, ///< The test case was expected to fail, and did.
        UnexpectedPass, ///< The test case was expected to fail, but passed.
        Error, ///< There was an error while trying to run the test case.
    };

    /**
     * The individual results of all test cases.
     **/
    QHash<QString, TestCaseResult> testCaseResults;

    /**
     * The total result of the entire suite.
     *
     * This is usually the worst outcome of the individual test cases,
     * but can be different especially when dealing with errors.
     */
    TestCaseResult suiteResult;
};

class KDEVPLATFORMINTERFACES_EXPORT ITestController : public QObject
{
    Q_OBJECT
public:
    explicit ITestController(QObject* parent = 0);
    virtual ~ITestController();

    /**
     * Add a new test suite.
     *
     * No ownership is taken, the caller stays responsible for the suite.
     *
     * If a suite with the same project and same name exists, the old one will be removed and deleted.
     */
    virtual void addTestSuite(ITestSuite* suite) = 0;
    /**
     * Remove a test suite from the controller.
     *
     * This does not delete the test suite.
     */
    virtual void removeTestSuite(ITestSuite* suite) = 0;

    /**
     * Returns the list of all known test suites.
     */
    virtual QList<ITestSuite*> testSuites() const = 0;
    /**
     * Find a test suite in @p project with @p name.
     */
    virtual ITestSuite* findTestSuite(IProject* project, const QString& name) const = 0;
    /**
     * Return the list of all test suites in @p project.
     */
    virtual QList<ITestSuite*> testSuitesForProject(IProject* project) const = 0;

    /**
     * Notify the controller that a test run for @p suite was finished with result @p result
     */
    virtual void notifyTestRunFinished(ITestSuite* suite, const KDevelop::TestResult& result) = 0;

    /**
     * Notify the controller that a test run for @p suite was started
     */
    virtual void notifyTestRunStarted(KDevelop::ITestSuite* suite, const QStringList& test_cases) = 0;

Q_SIGNALS:
    /**
     * Emitted whenever a new test suite gets added.
     */
    void testSuiteAdded(KDevelop::ITestSuite* suite) const;
    /**
     * Emitted whenever a test suite gets removed.
     */
    void testSuiteRemoved(KDevelop::ITestSuite* suite) const;
    /**
     * Emitted after a test suite was run.
     */
    void testRunFinished(KDevelop::ITestSuite* suite, const KDevelop::TestResult& result) const;
     /**
     * Emitted when a test suite starts.
     */
    void testRunStarted(KDevelop::ITestSuite* suite, const QStringList& test_cases) const;
};

}

Q_DECLARE_INTERFACE( KDevelop::ITestController, "org.kdevelop.ITestController")

#endif // KDEVPLATFORM_ITESTCONTROLLER_H
