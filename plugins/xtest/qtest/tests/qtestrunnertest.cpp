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

#include "qtestrunnertest.h"
#include <kasserts.h>
#include <qtest_kde.h>

#include <KDebug>
#include <QBuffer>

#include "../xmlregister.h"
#include "../qtestcase.h"

#include <veritas/test.h>
#include <veritas/testresult.h>

#include "runnertesthelper.h"

using Veritas::RunnerTestHelper;
using QTest::Test::QTestRunnerTest;

Q_DECLARE_METATYPE(QList<QStringList>)

void QTestRunnerTest::init()
{
    m_runner = new RunnerTestHelper;
    m_runner->initializeGUI();
}

void QTestRunnerTest::cleanup()
{
    m_runner->cleanupGUI();
    delete m_runner;
}

// command
void QTestRunnerTest::empty()
{
    QByteArray reg =
            "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
            "<root dir=\"\">\n"
            "</root>\n";
    Veritas::Test* root = fetchRoot(reg);

    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList runnerItems;
    runnerItems << "0 x";
    m_runner->verifyTestTree(runnerItems);

    QList<QStringList> results;
    m_runner->verifyResultItems(results);
}

QByteArray sunnyDayXml =
    "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
    "<root dir=\"\">\n"
    "<suite name=\"suite1\" dir=\"suite1\">\n"
    "<case name=\"fakeqtest1\" exe=\"fakeqtest1\">\n"
    "<command name=\"cmd1\" />\n"
    "<command name=\"cmd2\" />\n"
    "</case>\n"
    "<case name=\"fakeqtest2\" exe=\"fakeqtest2\">\n"
    "<command name=\"cmd1\" />\n"
    "<command name=\"cmd2\" />\n"
    "</case>"
    "<case name=\"fakeqtest3\" exe=\"fakeqtest3\">\n"
    "<command name=\"cmd1\" />\n"
    "</case>\n"
    "</suite>\n"
    "</root>\n";

QStringList sunnyDayTests()
{
    QStringList runnerItems;
    runnerItems
            << "0 suite1"
            << "0 0 fakeqtest1"
            << "0 0 0 cmd1"
            << "0 0 1 cmd2"
            << "0 0 2 x"
            << "0 1 fakeqtest2"
            << "0 1 0 cmd1"
            << "0 1 1 cmd2"
            << "0 1 2 x"
            << "0 2 fakeqtest3"
            << "0 2 0 cmd1"
            << "0 2 1 x"
            << "0 3 x"
            << "1 x";
    return runnerItems;
}

QList<QStringList> sunnyDayResultItems()
{
    QStringList result;
    result << "cmd2" << "failure message" << "fakeqtest2.cpp" << "2";
    return QList<QStringList>() << result;
}

QMap<QString, Veritas::TestState> sunnyDayTestStates()
{
    QMap<QString, Veritas::TestState> states;
    states["suite1/fakeqtest1/cmd1"] = Veritas::RunSuccess;
    states["suite1/fakeqtest1/cmd2"] = Veritas::RunSuccess;
    states["suite1/fakeqtest2/cmd1"] = Veritas::RunSuccess;
    states["suite1/fakeqtest2/cmd2"] = Veritas::RunError;
    states["suite1/fakeqtest3/cmd1"] = Veritas::RunSuccess;
    return states;
}

// command
void QTestRunnerTest::sunnyDay()
{
    Veritas::Test* root = fetchRoot(sunnyDayXml);
    m_runner->setRoot(root);
    m_runner->runTests();

    m_runner->verifyTestTree(sunnyDayTests());
    m_runner->verifyResultItems(sunnyDayResultItems());
    m_runner->verifyTestStates(sunnyDayTestStates(), root);
    assertAllFilesClosed(root);
}

// command
void QTestRunnerTest::runTwice()
{
    Veritas::Test* root = fetchRoot(sunnyDayXml);

    RunnerTestHelper* m_runner = new RunnerTestHelper;
    m_runner->initializeGUI();
    m_runner->setRoot(root);
    m_runner->runTests();
    m_runner->runTests();

    m_runner->verifyTestTree(sunnyDayTests());
    m_runner->verifyResultItems(sunnyDayResultItems());
    assertAllFilesClosed(root);
}

// helper
Veritas::Test* QTestRunnerTest::fetchRoot(QByteArray& testRegistrationXml)
{
    QBuffer buff(&testRegistrationXml);
    XmlRegister reg;
    reg.setSource(&buff);
    reg.reload();
    return reg.root();
}

// helper
void QTestRunnerTest::assertAllFilesClosed(Veritas::Test* root)
{
    QTest::qWait(75);
    // QTestCases are on lvl2 in the test-tree
    for(int i=0; i<root->childCount(); i++) {
        Veritas::Test* suite = root->child(i);
        for (int i=0; i<suite->childCount(); i++) {
            Veritas::Test* test = suite->child(i);
            QTestCase* caze = qobject_cast<QTestCase*>(test);
            KVERIFY_MSG(caze, "Not a QTestCase on lvl2 of the test tree? (cast failed)");
            KVERIFY_MSG(caze->fto_outputFileClosed(), caze->name());
        }
    }
}

#include "qtestrunnertest.moc"
QTEST_KDEMAIN(QTestRunnerTest, GUI)
