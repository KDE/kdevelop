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
#include <veritas/resultsmodel.h>
#include <veritas/runnerwindow.h>
#include <veritas/runnermodel.h>
#include "plugins/xtest/cppunit/register.h"
#include "plugins/xtest/qtest/tests/ui_runnerwindow.h"

#include <KDebug>
#include <QBuffer>
#include <QDir>
#include <QTreeView>
#include <QModelIndex>
#include <QAbstractItemModel>

#include "testsuite.h"
#include "testroot.h"

using Veritas::RunnerWindow;
using Veritas::RunnerModel;
using Veritas::ResultsModel;

using Check::TestSuite;
using Check::TestRoot;
using Check::it::CheckRunnerTest;

Q_DECLARE_METATYPE(QList<QStringList>)

void CheckRunnerTest::init()
{
    QStringList resultHeaders;
    resultHeaders << i18n("Test Name") << i18n("Result") << i18n("Message")
                  << i18n("File Name") << i18n("Line Number");
    m_window = new RunnerWindow(new ResultsModel(resultHeaders));
}

void CheckRunnerTest::cleanup()
{
    delete m_window;
}

// command
void CheckRunnerTest::empty()
{
    initNrun("./emptysuite");

    QStringList runnerItems;
    runnerItems << "0 my_suite"
                << "0 0 x"
                << "1 x";
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
void CheckRunnerTest::sunnyDay()
{
    initNrun("./sunnysuite");
    checkTests(sunnyDayTests());

    QStringList result0;
    result0 << "bar_cmd2" << "Assertion '0' failed" << "/fake_sunnysuite.cpp" << "53";
    QList<QStringList> results;
    results << result0;
    checkResultItems(results);
}

// command
void CheckRunnerTest::multiSuite()
{
    initNrun("./multisuite");
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
    checkTests(topo);

    QList<QStringList> results;
    checkResultItems(results);

}

// helper
void CheckRunnerTest::checkResultItems(QList<QStringList> expected)
{
    nrofMessagesEquals(expected.size());
    for (int i = 0; i < expected.size(); i++)
        checkResultItem(i, expected.value(i));
}

// helper
void CheckRunnerTest::checkResultItem(int num, const QStringList& item)
{
    m_resultModel = m_window->resultsView()->model();
;
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
void CheckRunnerTest::nrofMessagesEquals(int num)
{
    m_resultModel = m_window->resultsView()->model();
    for (int i = 0; i < num; i++) {
        KVERIFY_MSG(m_resultModel->index(i, 0).isValid(),
                    QString("Expected ") + QString::number(num) + " items but got " + QString::number(i));
    }
    KVERIFY(!m_resultModel->index(num, 0).isValid());
}

// helper
void CheckRunnerTest::initNrun(const char* exe)
{
    Register<TestRoot, TestSuite> reg;
    QFileInfo executable(exe);
    reg.addFromExe(executable);
    reg.rootItem()->setExecutable(executable);
    RunnerModel* model = new RunnerModel;
    model->setRootItem(reg.rootItem());
    //m_window = new RunnerWindow;
    m_window->setModel(model);
    //m_window->show();
    m_window->ui()->actionStart->trigger();
}

// helper
void CheckRunnerTest::checkTests(QStringList runnerItems)
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
void CheckRunnerTest::checkTest(const QVariant& expected, int lvl0, int lvl1, int lvl2)
{
    QAbstractItemModel* runner = m_window->ui()->treeRunner->model();
    QModelIndex index = runner->index(lvl0, 0);
    if (lvl1 != -1) {
        index = runner->index(lvl1, 0, index);
        if (lvl2 != -1) {
            index = runner->index(lvl2, 0, index);
        }
    }
    KOMPARE_MSG(expected, runner->data(index),
                QString("lvl0 ") + QString::number(lvl0)  +
                QString(" lvl1 ") + QString::number(lvl1) +
                QString(" lvl2 ") + QString::number(lvl2));
}

#include "checkrunnertest.moc"
QTEST_KDEMAIN(CheckRunnerTest, GUI)
