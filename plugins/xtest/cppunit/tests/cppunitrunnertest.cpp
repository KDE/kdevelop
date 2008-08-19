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
#include <veritas/resultsmodel.h>
#include <veritas/runnerwindow.h>
#include <veritas/runnermodel.h>
#include <veritas/testresult.h>
#include "register.h"
#include "plugins/xtest/qtest/tests/ui_runnerwindow.h"

#include <KDebug>
#include <QBuffer>
#include <QDir>
#include <QTreeView>
#include <QModelIndex>
#include <QAbstractItemModel>

#include "testroot.h"
#include "testsuite.h"

using Veritas::RunnerWindow;
using Veritas::RunnerModel;
using Veritas::ResultsModel;
using Veritas::Test;
using CppUnit::TestSuite;
using CppUnit::TestRoot;
using CppUnit::Test::CppUnitRunnerTest;

Q_DECLARE_METATYPE(QList<QStringList>)

void CppUnitRunnerTest::init()
{
    QStringList resultHeaders;
    resultHeaders << i18n("Test Name") << i18n("Result") << i18n("Message")
                  << i18n("File Name") << i18n("Line Number");
    m_window = new RunnerWindow(new ResultsModel(resultHeaders));
}

void CppUnitRunnerTest::cleanup()
{
    delete m_window;
}

// command
void CppUnitRunnerTest::empty()
{
    initNrun("./emptysuite");

    QStringList runnerItems;
    runnerItems << "0 x";
    checkTests(runnerItems);

    QList<QStringList> results;
    checkResultItems(results);

    QMap<QString, QString> status;
    status["total"] = "0";
    status["selected"] = "0";
    status["run"] = "0";
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

QMap<QString,QString> sunnyDayStatus()
{
    QMap<QString,QString> status;
    status["total"] = "5";
    status["selected"] = "5";
    status["run"] = "5";
    status["success"] = ": 4";
    status["errors"] = ": 1";
    status["warnings"] = ": 0";
    return status;
}

// command
void CppUnitRunnerTest::sunnyDay()
{
    initNrun("./sunnysuite");
    checkTests(sunnyDayTests());

    QStringList result0;
    result0 << "testCmd2" << "" << QDir::currentPath() + "/fake_sunnysuite.cpp" << "63";
    QList<QStringList> results;
    results << result0;
    checkResultItems(results);
}

// command
void CppUnitRunnerTest::multiSuite()
{
    initNrun("./multisuite");

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
    checkTests(topo);

    QList<QStringList> results;
    checkResultItems(results);

}

// helper
void CppUnitRunnerTest::checkResultItems(QList<QStringList> expected)
{
    nrofMessagesEquals(expected.size());
    for (int i = 0; i < expected.size(); i++)
        checkResultItem(i, expected.value(i));
}

// helper
void CppUnitRunnerTest::checkResultItem(int num, const QStringList& item)
{
    m_resultModel = m_window->resultsView()->model();

    KOMPARE(item[0], m_resultModel->data(m_resultModel->index(num, 0))); // test name
    //KOMPARE(item[1], m_resultModel->data(m_resultModel->index(num, 1))); // status -> "Error"
    // commented out since this row is currently hidden
    KOMPARE(item[1], m_resultModel->data(m_resultModel->index(num, 2))); // failure message
    // KOMPARE(item[2], m_resultModel->data(m_resultModel->index(num, 3))); // filename
    // commented out since this is factually a file in the source-tree, for
    // which i can not get the proper location at runtime.
    KOMPARE(item[3], m_resultModel->data(m_resultModel->index(num, 4))); // line number
}

// helper
void CppUnitRunnerTest::nrofMessagesEquals(int num)
{
    m_resultModel = m_window->resultsView()->model();
    for (int i = 0; i < num; i++) {
        KVERIFY_MSG(m_resultModel->index(i, 0).isValid(),
                    QString("Expected ") + QString::number(num) + " items but got " + QString::number(i));
    }
    KVERIFY(!m_resultModel->index(num, 0).isValid());
}

// helper
void CppUnitRunnerTest::initNrun(const char* exe)
{
    Register<TestRoot, TestSuite> reg;
    reg.addFromExe(QFileInfo(exe));
    reg.rootItem()->setExecutable(QFileInfo(exe));
    RunnerModel* model = new RunnerModel;
    model->setRootItem(reg.rootItem());
    model->setExpectedResults(Veritas::RunError);
    //m_window = new RunnerWindow;
    m_window->setModel(model);
    //m_window->show();
    m_window->ui()->actionStart->trigger();
}

// helper
void CppUnitRunnerTest::checkTests(QStringList runnerItems)
{
    foreach(QString s, runnerItems) {
        QStringList spl = s.split(" ");
        int lvl0 = spl[0].toInt();
        int lvl1 = (spl.size() > 2) ? spl[1].toInt() : -1;
        int lvl2 = (spl.size() > 3) ? spl[2].toInt() : -1;
        QVariant exp = (spl.last() == "x") ? QVariant() : spl.last();
        checkTest(exp, lvl0, lvl1, lvl2);
    }
}

// helper
void CppUnitRunnerTest::checkTest(const QVariant& expected, int lvl0, int lvl1, int lvl2)
{
    QAbstractItemModel* runner = m_window->ui()->treeRunner->model();
    QModelIndex index = runner->index(lvl0, 0);
    if (lvl1 != -1) {
        index = runner->index(lvl1, 0, index);
        if (lvl2 != -1) {
            index = runner->index(lvl2, 0, index);
        }
    }
    KOMPARE(expected, runner->data(index));
}

#include "cppunitrunnertest.moc"
QTEST_KDEMAIN(CppUnitRunnerTest, GUI)
