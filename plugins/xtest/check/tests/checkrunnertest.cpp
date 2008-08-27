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

#include "checkrunnertest.h"

#include <kasserts.h>
#include <qtest_kde.h>
#include "plugins/xtest/cppunit/register.h"

#include "testsuite.h"
#include "testroot.h"

#include "plugins/xtest/qtest/tests/runnertesthelper.h"

using Veritas::RunnerTestHelper;
using Check::TestSuite;
using Check::TestRoot;
using Check::Test::CheckRunnerTest;

void CheckRunnerTest::init()
{
    m_runner = new RunnerTestHelper;
    m_runner->initializeGUI();
}

void CheckRunnerTest::cleanup()
{
    m_runner->cleanupGUI();
    delete m_runner;
}

// helper
Veritas::Test* CheckRunnerTest::fetchRoot(const char* exe)
{
    Register<TestRoot, TestSuite> reg;
    QFileInfo executable(exe);
    reg.addFromExe(executable);
    reg.rootItem()->setExecutable(executable);
    return reg.rootItem();
}

// command
void CheckRunnerTest::empty()
{
    Veritas::Test* root = fetchRoot("./emptysuite");
    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList runnerItems;
    runnerItems << "0 my_suite"
                << "0 0 x"
                << "1 x";
    m_runner->verifyTestTree(runnerItems);

    QList<QStringList> results;
    m_runner->verifyResultItems(results);
}

QStringList sunnyDayTests()
{
    QStringList runnerItems;
    runnerItems
        << "0 root_suite"
        << "0 0 foo_test"
        << "0 0 0 foo_cmd1"
        << "0 0 1 foo_cmd2"
        << "0 0 2 x"
        << "0 1 bar_test"
        << "0 1 0 bar_cmd1"
        << "0 1 1 bar_cmd2"
        << "0 1 2 x"
        << "0 2 baz_test"
        << "0 2 0 baz_cmd1"
        << "0 2 1 x"
        << "0 3 x"
        << "1 x";
    return runnerItems;
}

QMap<QString, Veritas::TestState> sunnyDayTestStates()
{
    QMap<QString, Veritas::TestState> states;
    states["root_suite/foo_test/foo_cmd1"] = Veritas::RunSuccess;
    states["root_suite/foo_test/foo_cmd2"] = Veritas::RunSuccess;
    states["root_suite/bar_test/bar_cmd1"] = Veritas::RunSuccess;
    states["root_suite/bar_test/bar_cmd2"] = Veritas::RunError;
    states["root_suite/baz_test/baz_cmd1"] = Veritas::RunSuccess;
    return states;
}

// command
void CheckRunnerTest::sunnyDay()
{
    Veritas::Test* root = fetchRoot("./sunnysuite");
    m_runner->setRoot(root);
    m_runner->runTests();

    m_runner->verifyTestTree(sunnyDayTests());
    m_runner->verifyTestStates(sunnyDayTestStates(), root);
    QStringList result;
    result << "bar_cmd2" << "Assertion '0' failed" << "fake_sunnysuite.cpp" << "53";
    m_runner->verifyResultItems(QList<QStringList>() << result);
}

// command
void CheckRunnerTest::multiSuite()
{
    Veritas::Test* root = fetchRoot("./multisuite");
    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList topo;
    topo << "0 foo"
         << "0 0 foo_test"
         << "0 0 0 foo_cmd"
         << "0 0 1 x"
         << "1 bar"
         << "1 0 bar_test"
         << "1 0 0 bar_cmd"
         << "1 0 1 x"
         << "1 1 x"
         << "2 x";
    m_runner->verifyTestTree(topo);

    QList<QStringList> results;
    m_runner->verifyResultItems(results);
}

#include "checkrunnertest.moc"
QTEST_KDEMAIN(CheckRunnerTest, GUI)
