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

#ifndef KDEVELOP_ITESTCONTROLLER_H
#define KDEVELOP_ITESTCONTROLLER_H

#include "interfacesexport.h"

#include <QtCore/QList>
#include <QtCore/QObject>

class KJob;

namespace KDevelop {

class IProject;
class ITestSuite;

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
     * Create a job to reload all test suites.
     */
    virtual KJob* reloadTestSuites() = 0;
    virtual void notifyTestRunFinished(ITestSuite* suite) = 0;

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
    void testRunFinished(KDevelop::ITestSuite* suite) const;
};

}

Q_DECLARE_INTERFACE( KDevelop::ITestController, "org.kdevelop.ITestController")

#endif // KDEVELOP_ITESTCONTROLLER_H
