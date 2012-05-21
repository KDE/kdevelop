#ifndef KDEVELOP_TESTCONTROLLERTEST_H
#define KDEVELOP_TESTCONTROLLERTEST_H

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

#endif // KDEVELOP_TESTCONTROLLERTEST_H
