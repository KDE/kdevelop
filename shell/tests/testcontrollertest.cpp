#include "testcontrollertest.h"
#include <testcontroller.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testproject.h>
#include <itestsuite.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <language/duchain/indexeddeclaration.h>

#include <qtest_kde.h>

using namespace KDevelop;

const char* TestSuiteName = "TestTestSuite";
const char* TestCaseNameOne = "TestTestCaseOne";
const char* TestCaseNameTwo = "TestTestCaseTwo";

class FakeTestSuite : public KDevelop::ITestSuite
{
public:
    FakeTestSuite(const QString& name, IProject* project, const QStringList& cases = QStringList()) : m_name(name), m_project(project), m_cases(cases) {}
    virtual ~FakeTestSuite() {}
    
    virtual IProject* project() const {return m_project;}
    virtual QString name() const {return m_name;}
    virtual QStringList cases() const {return m_cases;}
    
    virtual IndexedDeclaration declaration() const;
    virtual IndexedDeclaration caseDeclaration(const QString& testCase) const;
    
    virtual KJob* launchAllCases(TestJobVerbosity verbosity);
    virtual KJob* launchCase(const QString& testCase, TestJobVerbosity verbosity);
    virtual KJob* launchCases(const QStringList& testCases, TestJobVerbosity verbosity);
    
private:
    QString m_name;
    IProject* m_project;
    QStringList m_cases;
};

IndexedDeclaration FakeTestSuite::declaration() const
{
    return IndexedDeclaration();
}

IndexedDeclaration FakeTestSuite::caseDeclaration(const QString& testCase) const
{
    Q_UNUSED(testCase);
    return IndexedDeclaration();
}

KJob* FakeTestSuite::launchAllCases(ITestSuite::TestJobVerbosity verbosity)
{
    Q_UNUSED(verbosity);
    return 0;
}

KJob* FakeTestSuite::launchCase(const QString& testCase, ITestSuite::TestJobVerbosity verbosity)
{
    Q_UNUSED(testCase);
    Q_UNUSED(verbosity);
    return 0;
}

KJob* FakeTestSuite::launchCases(const QStringList& testCases, ITestSuite::TestJobVerbosity verbosity)
{
    Q_UNUSED(testCases);
    Q_UNUSED(verbosity);
    return 0;
}

void TestControllerTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize( Core::NoUi );
    
    m_testController = Core::self()->testControllerInternal();
    m_project = new TestProject(this);
}

void TestControllerTest::cleanupTestCase()
{
    delete m_project;
    TestCore::shutdown();
}

void TestControllerTest::addSuite()
{
    FakeTestSuite suite(TestSuiteName, m_project);
    m_testController->addTestSuite(&suite);
    
    ITestSuite* found = m_testController->findTestSuite(m_project, TestSuiteName);
    
    QVERIFY(found);
    QCOMPARE(found->name(), QString(TestSuiteName));
    QCOMPARE(found->project(), m_project);
    
    m_testController->removeTestSuite(&suite);
}

void TestControllerTest::removeSuite()
{
    FakeTestSuite suite(TestSuiteName, m_project);
    m_testController->addTestSuite(&suite);
    
    QVERIFY(m_testController->findTestSuite(m_project, TestSuiteName));
    m_testController->removeTestSuite(&suite);
    
    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName), (ITestSuite*)0);
    QVERIFY(m_testController->testSuites().isEmpty());
}

void TestControllerTest::replaceSuite()
{
    FakeTestSuite* suiteOne = new FakeTestSuite(TestSuiteName, m_project, QStringList() << TestCaseNameOne);
    m_testController->addTestSuite(suiteOne);
    
    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName)->name(), QString(TestSuiteName));
    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName)->cases().size(), 1);
    
    FakeTestSuite* suiteTwo = new FakeTestSuite(TestSuiteName, m_project, QStringList() << TestCaseNameOne << TestCaseNameTwo);
    m_testController->addTestSuite(suiteTwo);
    
    QCOMPARE(m_testController->testSuites().size(), 1);
    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName)->name(), QString(TestSuiteName));
    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName)->cases().size(), 2);
    
    // TestController deletes the old suite when replacing it, so make sure we don't delete suiteOne manually
    
    m_testController->removeTestSuite(suiteTwo);
    delete suiteTwo;
}

void TestControllerTest::findByProject()
{
    IProject* otherProject = new TestProject(this);
    
    ITestSuite* suiteOne = new FakeTestSuite(TestSuiteName, m_project);
    ITestSuite* suiteTwo = new FakeTestSuite(TestSuiteName, otherProject);
    m_testController->addTestSuite(suiteOne);    
    m_testController->addTestSuite(suiteTwo);
    
    QCOMPARE(m_testController->testSuites().size(), 2);
    QCOMPARE(m_testController->testSuitesForProject(m_project).size(), 1);
    
    QCOMPARE(m_testController->testSuitesForProject(m_project).first(), suiteOne);
    
    m_testController->removeTestSuite(suiteOne);
    m_testController->removeTestSuite(suiteTwo);
    delete suiteOne;
    delete suiteTwo;
    
    delete otherProject;
}


QTEST_KDEMAIN(TestControllerTest, NoGUI)
