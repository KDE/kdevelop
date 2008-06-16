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
#include <qxrunner/runnerwindow.h>
#include <qtestmodel.h>
#include <QBuffer>
#include <QTreeView>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <ui_statuswidget.h>

using QxRunner::RunnerWindow;
using QxQTest::QTestModel;
using QxQTest::it::QTestRunnerTest;

Q_DECLARE_METATYPE(QList<QStringList>)

QByteArray regXml =
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

void QTestRunnerTest::init()
{
    m_window = new RunnerWindow();
}

void QTestRunnerTest::cleanup()
{
    delete m_window;
}

// command
void QTestRunnerTest::empty()
{
    QByteArray reg =
            "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
            "<root dir=\"\">\n"
            "</root>\n";
    initNrun(reg);

    QStringList runnerItems;
    runnerItems << "0 x";
    checkRunnerItems(runnerItems);

    QList<QStringList> results;
    checkResultItems(results);

    QMap<QString, QString> status;
    status["total"] = "0";
    status["selected"] = "0";
    status["run"] = "0";
}

QStringList sunnyDayRunnerItems()
{
    QStringList runnerItems;
    runnerItems << "0 suite1"
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
void QTestRunnerTest::sunnyDay()
{
    initNrun(regXml);
    checkRunnerItems(sunnyDayRunnerItems());

    QStringList result0;
    result0 << "cmd2" << "failure message" << "fakeqtest2.cpp" << "2";
    QList<QStringList> results;
    results << result0;
    checkResultItems(results);

    QMap<QString, QString> status;
    checkStatusWidget(sunnyDayStatus());
}

// command
void QTestRunnerTest::runTwice()
{
    initNrun(regXml);
    m_window->ui().actionStart->trigger();
    if (!QTest::kWaitForSignal(m_window->runnerModel(), SIGNAL(allItemsCompleted()), 2000))
        QFAIL("Timeout while waiting for runner items to complete execution");

    checkRunnerItems(sunnyDayRunnerItems());

    QStringList result0;
    result0 << "cmd2" << "failure message" << "fakeqtest2.cpp" << "2";
    QList<QStringList> results;
    results << result0;
    checkResultItems(results);

    checkStatusWidget(sunnyDayStatus());
}

// helper
void QTestRunnerTest::checkResultItems(QList<QStringList> expected)
{
    nrofMessagesEquals(expected.size());;
    for (int i = 0; i < expected.size(); i++)
        checkResultItem(i, expected.value(i));
}

// helper
void QTestRunnerTest::checkResultItem(int num, const QStringList& item)
{
    QAbstractItemModel* results = m_window->ui().treeResults->model();
    KOMPARE(item[0], results->data(results->index(num, 0))); // test name
    //KOMPARE(item[1], results->data(results->index(num, 1))); // status -> "Error"
    KOMPARE(item[1], results->data(results->index(num, 2))); // failure message
    KOMPARE(item[2], results->data(results->index(num, 3))); // filename
    KOMPARE(item[3], results->data(results->index(num, 4))); // line number
}

// helper
void QTestRunnerTest::nrofMessagesEquals(int num)
{
    QAbstractItemModel* results = m_window->ui().treeResults->model();
    for (int i = 0; i < num; i++)
        KVERIFY(results->index(i, 0).isValid());
    KVERIFY(!results->index(num, 0).isValid());
}

// helper
void QTestRunnerTest::initNrun(QByteArray& regXml)
{
    QBuffer buff(&regXml);
    QTestModel* model = new QTestModel();
    model->readTests(&buff);

    m_window->setModel(model);
    m_window->show();
    m_window->ui().actionStart->trigger();
    if (!QTest::kWaitForSignal(m_window->runnerModel(), SIGNAL(allItemsCompleted()), 2000))
//    if (!QTest::kWaitForSignal(win->runnerModel(), SIGNAL(allItemsCompleted())))
       QFAIL("Timeout while waiting for runner items to complete execution");
}

// helper
void QTestRunnerTest::checkStatusWidget(QMap<QString, QString> labels)
{
    Ui::StatusWidget* status = m_window->statusWidget();

    // validate the status widget
    KOMPARE(labels["total"], status->labelNumTotal->text());
    KOMPARE(labels["selected"], status->labelNumSelected->text());
    KOMPARE(labels["run"], status->labelNumRun->text());

    if (labels.contains("success"))
        KOMPARE(labels["success"], status->labelNumSuccess->text());
    if (labels.contains("errors"))
        KOMPARE(labels["errors"], status->labelNumErrors->text());
    if (labels.contains("warnings"))
        KOMPARE(labels["warnings"], status->labelNumWarnings->text());
}

// helper
void QTestRunnerTest::checkRunnerItems(QStringList runnerItems)
{
    foreach(QString s, runnerItems) {
        QStringList spl = s.split(" ");
        int lvl0 = spl[0].toInt();
        int lvl1 = (spl.size() > 2) ? spl[1].toInt() : -1;
        int lvl2 = (spl.size() > 3) ? spl[2].toInt() : -1;
        QVariant exp = (spl.last() == "x") ? QVariant() : spl.last();
        checkRunnerItem(exp, lvl0, lvl1, lvl2);
    }
}

// helper
void QTestRunnerTest::checkRunnerItem(const QVariant& expected, int lvl0, int lvl1, int lvl2)
{
    QAbstractItemModel* runner = m_window->ui().treeRunner->model();
    QModelIndex index = runner->index(lvl0, 0);
    if (lvl1 != -1) {
        index = runner->index(lvl1, 0, index);
        if (lvl2 != -1) {
            index = runner->index(lvl2, 0, index);
        }
    }
    KOMPARE(expected, runner->data(index));
}

#include "qtestrunnertest.moc"
QTEST_KDEMAIN(QTestRunnerTest, GUI)
