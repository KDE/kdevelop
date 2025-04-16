/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_ctestfindsuites.h"

#include "testhelpers.h"
#include "cmakeutils.h"
#include "cmake-test-paths.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ibuildsystemmanager.h>
#include <interfaces/iprojectbuilder.h>
#include <testing/ctestsuite.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <project/projectmodel.h>

#include <QDir>
#include <QTest>
#include <KJob>

using namespace KDevelop;

void waitForSuites(IProject* project, int count, int secondsTimeout)
{
    auto testController = ICore::self()->testController();
    QTRY_COMPARE_WITH_TIMEOUT(testController->testSuitesForProject(project).size(), count, secondsTimeout * 1000);
}

void TestCTestFindSuites::initTestCase()
{
    AutoTestShell::init({"KDevCMakeManager", "KDevCMakeBuilder", "KDevMakeBuilder", "KDevStandardOutputView"});
    TestCore::initialize();

    qRegisterMetaType<ITestSuite*>();

    cleanup();
}

void TestCTestFindSuites::cleanup()
{
    const auto projects = ICore::self()->projectController()->projects() ;
    for (IProject* p : projects) {
        if (p && p->buildSystemManager() && p->buildSystemManager()->builder())
        {
            p->buildSystemManager()->builder()->prune(p)->exec();
        }
        ICore::self()->projectController()->closeProject(p);
    }
    QVERIFY(ICore::self()->projectController()->projects().isEmpty());
}

void TestCTestFindSuites::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCTestFindSuites::testCTestSuite()
{
    IProject* project = loadProject( "unit_tests" );
    QVERIFY2(project, "Project was not opened");
    waitForSuites(project, 5, 25);
    const QList<ITestSuite*> suites = ICore::self()->testController()->testSuitesForProject(project);

    QCOMPARE(suites.size(), 5);

    DUChainReadLocker locker(DUChain::lock());

    for (auto suite : suites) {
        QCOMPARE(suite->cases(), QStringList());
        auto* ctestSuite = static_cast<CTestSuite*>(suite);
        QVERIFY(suite->declaration().isValid());
        const auto buildDir = Path(CMake::allBuildDirs(project).at(0));
        QString exeSubdir = buildDir.relativePath(ctestSuite->executable().parent());
        QCOMPARE(exeSubdir, ctestSuite->name() == "fail" ? QStringLiteral("bin") : QString() );
        QString willFail;
        const QString workingDirectory = ctestSuite->properties().value(QLatin1String("WORKING_DIRECTORY"), QString());
        auto cursor = ctestSuite->declaration().data()->range().start;
        if (ctestSuite->name() == QLatin1String("fail")) {
            willFail = QLatin1String("TRUE");
            QCOMPARE(workingDirectory, QLatin1String("/bar/baz"));
            QCOMPARE(ctestSuite->properties().value(QLatin1String("FOO"), QString()), QLatin1String("foo"));
            QCOMPARE(ctestSuite->properties().value(QLatin1String("BAR"), QString()), QLatin1String("TRUE"));
            QCOMPARE(ctestSuite->properties().value(QLatin1String("MULTILINE"), QString()), QLatin1String("this is \na multi\nline property"));
            QCOMPARE(ctestSuite->properties().value(QLatin1String("QUOTES"), QString()), QLatin1String("\"\\\\\"\\\\\\"));
            QCOMPARE(cursor.line, 0);
        } else if (ctestSuite->name() == QLatin1String("test_three")) {
            QCOMPARE(workingDirectory, QLatin1String("/foo"));
            QCOMPARE(ctestSuite->arguments(), QStringList("3"));
            QCOMPARE(cursor.line, 2);
        } else if (ctestSuite->name() == QLatin1String("test_four")) {
            QCOMPARE(ctestSuite->arguments(), QStringList("4"));
            QCOMPARE(cursor.line, 2);
        } else if (ctestSuite->name() == QLatin1String("test_five")) {
            QCOMPARE(workingDirectory, QString(buildDir.path() + QLatin1String("/bin")));
            QCOMPARE(cursor.line, 2);
        } else {
            QCOMPARE(workingDirectory, QString());
            QCOMPARE(cursor.line, 0);
        }
        QCOMPARE(ctestSuite->properties().value(QLatin1String("WILL_FAIL")), willFail);
    }
}

void TestCTestFindSuites::testQtTestCases()
{
    IProject* project = loadProject( "unit_tests_kde" );
    QVERIFY2(project, "Project was not opened");

    waitForSuites(project, 2, 60);

    const QList<ITestSuite*> suites = ICore::self()->testController()->testSuitesForProject(project);
    QCOMPARE(suites.size(), 2);

    QMap<QString, int> caseLines = {
        {QStringLiteral("passingTestCase"), 9},
        {QStringLiteral("failingTestCase"), 14},
        {QStringLiteral("expectedFailTestCase"), 19},
        {QStringLiteral("unexpectedPassTestCase"), 25},
        {QStringLiteral("skippedTestCase"), 31}
    };

    DUChainReadLocker locker(DUChain::lock());

    for (auto suite : suites) {
        const auto caseNames = suite->cases();
        QCOMPARE(caseNames.count(), caseLines.count());
        QVERIFY(suite->declaration().isValid());

        for (const auto& caseName : caseNames) {
            auto decl = suite->caseDeclaration(caseName);
            QVERIFY(decl.isValid());
            auto it = caseLines.find(caseName);
            QVERIFY(it != caseLines.end());
            QCOMPARE(decl.data()->range().start.line, it.value());
        }
    }
}

QTEST_MAIN(TestCTestFindSuites)

#include "moc_test_ctestfindsuites.cpp"
