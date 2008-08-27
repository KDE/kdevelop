/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "cppunitrunnertest.h"

#include <kasserts.h>
#include <qtest_kde.h>
#include <veritas/testresult.h>
#include "register.h"

#include "testroot.h"
#include "testsuite.h"

#include "plugins/xtest/qtest/tests/runnertesthelper.h"

using Veritas::RunnerTestHelper;
using CppUnit::Test::CppUnitRunnerTest;

Q_DECLARE_METATYPE(QList<QStringList>)

void CppUnitRunnerTest::init()
{
    m_runner = new RunnerTestHelper;
    m_runner->initializeGUI();
}

void CppUnitRunnerTest::cleanup()
{
    delete m_runner;
}

// command
void CppUnitRunnerTest::empty()
{
    Veritas::Test* root = fetchRoot("./emptysuite");
    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList runnerItems;
    runnerItems << "0 x"; // an empty test-tree
    m_runner->verifyTestTree(runnerItems);

    QList<QStringList> results; // no test failures
    m_runner->verifyResultItems(results);
}

QStringList sunnyDayTests()
{
    QStringList runnerItems;
    runnerItems << "0 RootSuite"
                << "0 0 FooTest"
                << "0 0 0 testCmd1"
                << "0 0 1 testCmd2"
                << "0 0 2 x"
                << "0 1 BarTest"
                << "0 1 0 testCmd1"
                << "0 1 1 testCmd2"
                << "0 1 2 x"
                << "0 2 BazTest"
                << "0 2 0 testCmd1"
                << "0 2 1 x"
                << "0 3 x"
                << "1 x";
    return runnerItems;
}

QMap<QString, Veritas::TestState> sunnyDayTestStates()
{
    QMap<QString, Veritas::TestState> states;
    states["RootSuite/FooTest/testCmd1"] = Veritas::RunSuccess;
    states["RootSuite/FooTest/testCmd2"] = Veritas::RunSuccess;
    states["RootSuite/BarTest/testCmd1"] = Veritas::RunSuccess;
    states["RootSuite/BarTest/testCmd2"] = Veritas::RunError;
    states["RootSuite/BazTest/testCmd1"] = Veritas::RunSuccess;
    return states;
}

// command
void CppUnitRunnerTest::sunnyDay()
{
    Veritas::Test* root = fetchRoot("./sunnysuite");
    m_runner->setRoot(root);
    m_runner->runTests();

    m_runner->verifyTestTree(sunnyDayTests());
    m_runner->verifyTestStates(sunnyDayTestStates(), root);
    QStringList result0;
    result0 << "testCmd2" << "" << KUrl(__FILE__).upUrl().path() + "fake_sunnysuite.cpp" << "63";
    QList<QStringList> results;
    results << result0;
    m_runner->verifyResultItems(results);
}

// command
void CppUnitRunnerTest::multiSuite()
{
    Veritas::Test* root = fetchRoot("./multisuite");
    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList topo;
    topo << "0 FooSuite"
         << "0 0 FooTest"
         << "0 0 0 FooTest::fooCmd"
         << "0 0 1 x"
         << "0 1 x"
         << "1 BarSuite"
         << "1 0 BarTest"
         << "1 0 0 BarTest::barCmd"
         << "1 0 1 x"
         << "1 1 x"
         << "2 x";
    m_runner->verifyTestTree(topo);

    QList<QStringList> results;
    m_runner->verifyResultItems(results);
}

Veritas::Test* CppUnitRunnerTest::fetchRoot(const char* exe)
{
    Register<TestRoot, TestSuite> reg;
    reg.addFromExe(QFileInfo(exe));
    reg.rootItem()->setExecutable(QFileInfo(exe));
    return reg.rootItem();
}

#include "cppunitrunnertest.moc"
QTEST_KDEMAIN(CppUnitRunnerTest, GUI)
