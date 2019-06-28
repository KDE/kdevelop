/* KDevelop CMake Support
 *
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 * Copyright 2017 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "test_ctestfindsuites.h"

#include "testhelpers.h"
#include "cmakeutils.h"
#include "cmake-test-paths.h"

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
#include <QtTest>
#include <KJob>

Q_DECLARE_METATYPE(KDevelop::ITestSuite*)

using namespace KDevelop;

void waitForSuites(IProject* project, int count, int max)
{
    auto testController = ICore::self()->testController();
    for(int i = 0; testController->testSuitesForProject(project).size() < count && i < max * 10; ++i) {
        QSignalSpy spy(testController, &ITestController::testSuiteAdded);
        QVERIFY(spy.wait(5000));
    }
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
    waitForSuites(project, 5, 10);
    const QList<ITestSuite*> suites = ICore::self()->testController()->testSuitesForProject(project);

    QCOMPARE(suites.size(), 5);

    DUChainReadLocker locker(DUChain::lock());

    for (auto suite : suites) {
        QCOMPARE(suite->cases(), QStringList());
        QVERIFY(!suite->declaration().isValid());
        auto* ctestSuite = static_cast<CTestSuite*>(suite);
        const auto buildDir = Path(CMake::allBuildDirs(project).at(0));
        QString exeSubdir = buildDir.relativePath(ctestSuite->executable().parent());
        QCOMPARE(exeSubdir, ctestSuite->name() == "fail" ? QStringLiteral("bin") : QString() );
        QString willFail;
        const QString workingDirectory = ctestSuite->properties().value(QLatin1String("WORKING_DIRECTORY"), QString());
        if (ctestSuite->name() == QLatin1String("fail")) {
            willFail = QLatin1String("TRUE");
            QCOMPARE(workingDirectory, QLatin1String("/bar/baz"));
            QCOMPARE(ctestSuite->properties().value(QLatin1String("FOO"), QString()), QLatin1String("foo"));
            QCOMPARE(ctestSuite->properties().value(QLatin1String("BAR"), QString()), QLatin1String("TRUE"));
            QCOMPARE(ctestSuite->properties().value(QLatin1String("MULTILINE"), QString()), QLatin1String("this is \na multi\nline property"));
            QCOMPARE(ctestSuite->properties().value(QLatin1String("QUOTES"), QString()), QLatin1String("\"\\\\\"\\\\\\"));
        } else if (ctestSuite->name() == QLatin1String("test_three"))
            QCOMPARE(workingDirectory, QLatin1String("/foo"));
        else if (ctestSuite->name() == QLatin1String("test_three"))
            QCOMPARE(workingDirectory, QLatin1String("/foo"));
        else if (ctestSuite->name() == QLatin1String("test_five"))
            QCOMPARE(workingDirectory, QString(buildDir.path() + QLatin1String("/bin")));
        else
            QCOMPARE(workingDirectory, QString());
        QCOMPARE(ctestSuite->properties().value(QLatin1String("WILL_FAIL")), willFail);
    }
}

void TestCTestFindSuites::testQtTestCases()
{
    IProject* project = loadProject( "unit_tests_kde" );
    QVERIFY2(project, "Project was not opened");

    QSignalSpy spy(ICore::self()->testController(), &ITestController::testSuiteAdded);
    QVERIFY(spy.isValid());

    // Background parsing can take a long time, so we need a long timeout
    QVERIFY(spy.wait(30 * 1000));

    const QList<ITestSuite*> suites = ICore::self()->testController()->testSuitesForProject(project);
    QCOMPARE(suites.size(), 1);

    QStringList cases = {
        QStringLiteral("passingTestCase"),
        QStringLiteral("failingTestCase"),
        QStringLiteral("expectedFailTestCase"),
        QStringLiteral("unexpectedPassTestCase"),
        QStringLiteral("skippedTestCase"),
    };

    DUChainReadLocker locker(DUChain::lock());

    for (auto suite : suites) {
        QCOMPARE(suite->cases(), cases);
        QVERIFY(suite->declaration().isValid());

        const auto caseNames = suite->cases();
        for (const auto& caseName : caseNames) {
            QVERIFY(suite->caseDeclaration(caseName).isValid());
        }
    }
}

QTEST_MAIN(TestCTestFindSuites)
