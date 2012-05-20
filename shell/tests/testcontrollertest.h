#ifndef KDEVELOP_TESTCONTROLLERTEST_H
#define KDEVELOP_TESTCONTROLLERTEST_H

#include <QObject>


namespace KDevelop {

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
    
    void cleanupTestCase();
    
private:
    TestController* m_testController;
    IProject* m_project;
};

}

#endif // KDEVELOP_TESTCONTROLLERTEST_H
