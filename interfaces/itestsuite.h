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

#ifndef KDEVPLATFORM_ITESTSUITE_H
#define KDEVPLATFORM_ITESTSUITE_H

#include "interfacesexport.h"

#include <QtCore/QStringList>

class KJob;
class KUrl;

namespace KDevelop {

class IndexedDeclaration;
class IProject;

/**
 * A unit test suite class.
 **/
class KDEVPLATFORMINTERFACES_EXPORT ITestSuite
{

public:

    /**
     * Specifies how much output a test job should produce
     **/
    enum TestJobVerbosity
    {
        Silent, ///< No toolview is raised by the job
        Verbose ///< The job raises an output toolview
    };

    /**
     * Destructor
     **/
    virtual ~ITestSuite();

    /**
     * @returns the display name of this suite. It has to be unique within a project.
     **/
    virtual QString name() const = 0;
    /**
     * @returns the list of all test cases in this suite.
     **/
    virtual QStringList cases() const = 0;
    /**
     * Get the project to which this test suite belongs.
     * Since all suites must have a project associated,
     * this function should never return 0.
     *
     * @returns the test suite's project.
     **/
    virtual IProject* project() const = 0;

    /**
     * Return a job that will execute all the test cases in this suite.
     *
     * The implementation of this class is responsible for creating the job
     * and interpreting its results. After the job is finished, the test results
     * should be made available to the result() function.
     *
     * Starting the job is up to the caller, usually by registering it with
     * the run controller.
     **/
    virtual KJob* launchAllCases(TestJobVerbosity verbosity) = 0;

    /**
     * @param testCases list of test cases to run
     * @returns a KJob that will run the specified @p testCases.
     * @sa launchAllCases()
     **/
    virtual KJob* launchCases(const QStringList& testCases, TestJobVerbosity verbosity) = 0;

    /**
     * @param testCase the test case to run
     * @returns a KJob that will run only @p testCase.
     * @sa launchAllCases()
     **/
    virtual KJob* launchCase(const QString& testCase, TestJobVerbosity verbosity) = 0;

    /**
     * The location in source code where the test suite is declared.
     * If no such declaration can be found, an invalid declaration is returned.
     **/
    virtual IndexedDeclaration declaration() const = 0;

    /**
     * The location in source code where the test case @p testCase is declared.
     * If no such declaration can be found, an invalid declaration is returned.
     *
     * This function may also return declarations for setup and teardown functions,
     * even though these functions are not included in cases().
     *
     * @param testCase the test case
     **/
    virtual IndexedDeclaration caseDeclaration(const QString& testCase) const = 0;
};

}

#endif // KDEVPLATFORM_ITESTSUITE_H
