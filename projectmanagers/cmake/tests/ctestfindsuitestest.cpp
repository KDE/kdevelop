/* KDevelop CMake Support
 *
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
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

#include "ctestfindsuitestest.h"
#include "testhelpers.h"
#include "cmake-test-paths.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <testing/ctestsuite.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <qtest_kde.h>

#define WAIT_FOR_SUITES(n, max)    \
for(int i = 0; ICore::self()->testController()->testSuitesForProject(project).size() < n && i < max*10; ++i) { QTest::qWait(100); }

QTEST_KDEMAIN( CTestFindSuitesTest, GUI )

using namespace KDevelop;

void CTestFindSuitesTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();
    DUChain::self()->disablePersistentStorage();
}

void CTestFindSuitesTest::cleanupTestCase()
{
    Core::self()->shutdown();
}

void CTestFindSuitesTest::testCTestSuite()
{
    IProject* project = parseProject( "unit_tests" );
    QVERIFY2(project, "Project was not opened");
    WAIT_FOR_SUITES(5, 10)
    QList<ITestSuite*> suites = ICore::self()->testController()->testSuitesForProject(project);
    
    QCOMPARE(suites.size(), 5);
    
    DUChainReadLocker locker(DUChain::lock());
    
    foreach (ITestSuite* suite, suites)
    {
        QCOMPARE(suite->cases(), QStringList());
        QVERIFY(!suite->declaration().isValid());
        CTestSuite* ctest = (CTestSuite*)(suite);
        QString exeSubdir = KUrl::relativeUrl(project->folder(), ctest->executable().directory());
        QCOMPARE(exeSubdir, ctest->name() == "fail" ? QString("build/bin") : QString("build") );
    }
}

void CTestFindSuitesTest::testQtTestSuite()
{
    IProject* project = parseProject( "unit_tests_kde" );
    QVERIFY2(project, "Project was not opened");
    WAIT_FOR_SUITES(2, 10)
    QList<ITestSuite*> suites = ICore::self()->testController()->testSuitesForProject(project);
    
    QCOMPARE(suites.size(), 2);
    ITestSuite* suite = suites.first();
    QCOMPARE(suite->cases().size(), 5);

    DUChainReadLocker locker(DUChain::lock());
    QVERIFY(suite->declaration().isValid());

    foreach (ITestSuite* suite, suites)
    {
        CTestSuite* ctest = (CTestSuite*)(suite);
        QString exeSubdir = KUrl::relativeUrl(project->folder(), ctest->executable().directory());
        QCOMPARE(exeSubdir, ctest->name() == "unittestskde-nonstd-location" ? QString("build/bin") : QString("build") );
    }

    foreach (const QString& testCase, suite->cases())
    {
        QVERIFY(suite->caseDeclaration(testCase).isValid());
    }
}

IProject* CTestFindSuitesTest::parseProject( const QString& name)
{
    const TestProjectPaths paths = projectPaths(name);
    defaultConfigure(paths);
    
    ICore::self()->projectController()->openProject(paths.projectFile);
    
    IProject* project = ICore::self()->projectController()->findProjectByName(name);
    int t = 0;
    const int timeout = 100;
    while (!project && t < 30000)
    {
        t += timeout;
        QTest::kWaitForSignal(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), timeout);
        project = ICore::self()->projectController()->findProjectByName(name);
    }
        
    return project;
}

#include "ctestfindsuitestest.moc"

