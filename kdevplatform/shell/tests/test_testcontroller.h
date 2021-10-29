/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_TESTCONTROLLER_H
#define KDEVPLATFORM_TEST_TESTCONTROLLER_H

#include <QObject>

#include <itestcontroller.h>

namespace KDevelop {

class ITestSuite;
class IProject;
class TestController;

class TestTestController : public QObject
{
    Q_OBJECT

private Q_SLOTS:
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

#endif // KDEVPLATFORM_TEST_TESTCONTROLLER_H
