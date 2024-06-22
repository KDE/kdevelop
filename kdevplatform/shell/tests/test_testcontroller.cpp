/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_testcontroller.h"
#include <testcontroller.h>
#include <QTest>
#include <QSignalSpy>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testproject.h>
#include <itestsuite.h>
#include <iproject.h>
#include <language/duchain/indexeddeclaration.h>

using namespace KDevelop;

const char* TestSuiteName = "TestTestSuite";
const char* TestSuiteNameTwo = "TestTestSuiteTwo";
const char* TestCaseNameOne = "TestTestCaseOne";
const char* TestCaseNameTwo = "TestTestCaseTwo";

Q_DECLARE_METATYPE(KDevelop::TestResult)

class FakeTestSuite : public KDevelop::ITestSuite
{
public:
    FakeTestSuite(const QString& name, IProject* project, const QStringList& cases = QStringList()) : m_name(name), m_project(project), m_cases(cases) {}
    ~FakeTestSuite() override {}

    IProject* project() const override {return m_project;}
    QString name() const override {return m_name;}
    QStringList cases() const override {return m_cases;}

    IndexedDeclaration declaration() const override;
    IndexedDeclaration caseDeclaration(const QString& testCase) const override;

    KJob* launchAllCases(TestJobVerbosity verbosity) override;
    KJob* launchCase(const QString& testCase, TestJobVerbosity verbosity) override;
    KJob* launchCases(const QStringList& testCases, TestJobVerbosity verbosity) override;

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
    return nullptr;
}

KJob* FakeTestSuite::launchCase(const QString& testCase, ITestSuite::TestJobVerbosity verbosity)
{
    Q_UNUSED(testCase);
    Q_UNUSED(verbosity);
    return nullptr;
}

KJob* FakeTestSuite::launchCases(const QStringList& testCases, ITestSuite::TestJobVerbosity verbosity)
{
    Q_UNUSED(testCases);
    Q_UNUSED(verbosity);
    return nullptr;
}

void TestTestController::emitTestResult(ITestSuite* suite, TestResult::TestCaseResult caseResult)
{
    TestResult result;
    result.suiteResult = caseResult;
    const auto testCases = suite->cases();
    for (const QString& testCase : testCases) {
        result.testCaseResults.insert(testCase, caseResult);
    }

    m_testController->notifyTestRunFinished(suite, result);
}

void TestTestController::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize( Core::NoUi );

    m_testController = Core::self()->testControllerInternal();
    m_project = new TestProject(Path(), this);

    qRegisterMetaType<KDevelop::ITestSuite*>("KDevelop::ITestSuite*");
    qRegisterMetaType<KDevelop::TestResult>("KDevelop::TestResult");
}

void TestTestController::cleanupTestCase()
{
    delete m_project;
    TestCore::shutdown();
}

void TestTestController::addSuite()
{
    FakeTestSuite suite(TestSuiteName, m_project);
    m_testController->addTestSuite(&suite);

    ITestSuite* found = m_testController->findTestSuite(m_project, TestSuiteName);

    QVERIFY(found);
    QCOMPARE(found->name(), QString(TestSuiteName));
    QCOMPARE(found->project(), m_project);

    m_testController->removeTestSuite(&suite);
}

void TestTestController::removeSuite()
{
    FakeTestSuite suite(TestSuiteName, m_project);
    m_testController->addTestSuite(&suite);

    QVERIFY(m_testController->findTestSuite(m_project, TestSuiteName));
    m_testController->removeTestSuite(&suite);

    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName), (ITestSuite*)nullptr);
    QVERIFY(m_testController->testSuites().isEmpty());
}

void TestTestController::replaceSuite()
{
    auto* suiteOne = new FakeTestSuite(TestSuiteName, m_project, QStringList() << TestCaseNameOne);
    m_testController->addTestSuite(suiteOne);

    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName)->name(), QString(TestSuiteName));
    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName)->cases().size(), 1);

    auto* suiteTwo = new FakeTestSuite(TestSuiteName, m_project, QStringList() << TestCaseNameOne << TestCaseNameTwo);
    m_testController->addTestSuite(suiteTwo);

    QCOMPARE(m_testController->testSuites().size(), 1);
    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName)->name(), QString(TestSuiteName));
    QCOMPARE(m_testController->findTestSuite(m_project, TestSuiteName)->cases().size(), 2);

    // TestController deletes the old suite when replacing it, so make sure we don't delete suiteOne manually

    m_testController->removeTestSuite(suiteTwo);
    delete suiteTwo;
}

void TestTestController::findByProject()
{
    IProject* otherProject = new TestProject(Path(), this);

    ITestSuite* suiteOne = new FakeTestSuite(TestSuiteName, m_project);
    ITestSuite* suiteTwo = new FakeTestSuite(TestSuiteName, otherProject);
    m_testController->addTestSuite(suiteOne);
    m_testController->addTestSuite(suiteTwo);

    QCOMPARE(m_testController->testSuites().size(), 2);
    QCOMPARE(m_testController->testSuitesForProject(m_project).size(), 1);

    QCOMPARE(m_testController->testSuitesForProject(m_project).at(0), suiteOne);

    m_testController->removeTestSuite(suiteOne);
    m_testController->removeTestSuite(suiteTwo);
    delete suiteOne;
    delete suiteTwo;

    delete otherProject;
}

void TestTestController::testResults()
{
    ITestSuite* suite = new FakeTestSuite(TestSuiteName, m_project);
    m_testController->addTestSuite(suite);

    QSignalSpy spy(m_testController, SIGNAL(testRunFinished(KDevelop::ITestSuite*,KDevelop::TestResult)));
    QVERIFY(spy.isValid());

    QList<TestResult::TestCaseResult> results;
    results << TestResult::Passed << TestResult::Failed << TestResult::Error << TestResult::Skipped << TestResult::NotRun;

    for (const TestResult::TestCaseResult result : std::as_const(results)) {
        emitTestResult(suite, result);
        QCOMPARE(spy.size(), 1);

        QVariantList arguments = spy.takeFirst();
        QCOMPARE(arguments.size(), 2);

        QVERIFY(arguments.first().canConvert<ITestSuite*>());
        QCOMPARE(arguments.first().value<ITestSuite*>(), suite);

        QVERIFY(arguments.at(1).canConvert<TestResult>());
        QCOMPARE(arguments.at(1).value<TestResult>().suiteResult, result);

        const auto testCases = suite->cases();
        for (const QString& testCase : testCases) {
            QCOMPARE(arguments.at(1).value<TestResult>().testCaseResults[testCase], result);
        }
    }

    QCOMPARE(spy.size(), 0);

    ITestSuite* suiteTwo = new FakeTestSuite(TestSuiteNameTwo, m_project);
    m_testController->addTestSuite(suiteTwo);

    // Verify that only one signal gets emitted even with more suites present
    emitTestResult(suiteTwo, TestResult::Passed);

    QCOMPARE(spy.size(), 1);

    m_testController->removeTestSuite(suite);
    m_testController->removeTestSuite(suiteTwo);
    delete suite;
    delete suiteTwo;
}


QTEST_GUILESS_MAIN(TestTestController)

#include "moc_test_testcontroller.cpp"
