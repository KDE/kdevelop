/*
    Unit tests for TestController.
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef KDEVPLATFORM_TESTCONTROLLERTEST_H
#define KDEVPLATFORM_TESTCONTROLLERTEST_H

#include <QObject>
#include <itestcontroller.h>

namespace KDevelop {

class ITestSuite;
class IProject;
class TestController;

class TestControllerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void addSuite();
    void removeSuite();
    void replaceSuite();

    void findByProject();
    void testResults();

    void cleanupTestCase();

private:
    void emitTestResult(ITestSuite* suite, TestResult::TestCaseResult caseResult);

    TestController* m_testController;
    IProject* m_project;
};

}

#endif // KDEVPLATFORM_TESTCONTROLLERTEST_H
