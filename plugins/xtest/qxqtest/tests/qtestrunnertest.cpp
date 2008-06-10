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
using QxQTest::ut::QTestRunnerTest;

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
    "</suite>\n"
    "</root>\n";

// command
void QTestRunnerTest::test()
{
    RunnerWindow win;
    runUI(&win);
    checkStatusWidget(win.statusWidget());
    checkRunnerItems(win.ui().treeRunner->model());
    checkResultItems(win.ui().treeResults->model());
}

// data
void QTestRunnerTest::test_data()
{
    QTest::addColumn<QByteArray>("registrationXML");
    QTest::addColumn<QStringList>("runnerItems");
    QTest::addColumn<QList<QStringList> >("resultItems");

    QStringList tree;
    tree << "0 suite1"
    << "0 0 fakeqtest1"
    << "0 0 0 cmd1"
    << "0 0 1 cmd2"
    << "0 0 2 x"
    << "0 1 fakeqtest2"
    << "0 1 0 cmd1"
    << "0 1 1 cmd2"
    << "0 1 2 x"
    << "0 2 x"
    << "1 x";

    QStringList result0;
    result0 << "cmd2" << "Error" << "failure message" << "fakeqtest2.cpp" << "2";
    QList<QStringList> results;
    results << result0;

    QTest::newRow("sunny day") << regXml << tree << results;

}

// helper
void QTestRunnerTest::checkResultItems(QAbstractItemModel* results)
{
    m_resultItems = results;
    QFETCH(QList<QStringList> , resultItems);
    nrofMessagesEquals(results, resultItems.size());;
    for (int i = 0; i < resultItems.size(); i++)
        checkResultItem(i, resultItems.value(i));
}

// helper
void QTestRunnerTest::checkResultItem(int num, const QStringList& item)
{
    KOMPARE(item[0], m_resultItems->data(m_resultItems->index(num, 0))); // test name
    KOMPARE(item[1], m_resultItems->data(m_resultItems->index(num, 1))); // status -> "Error"
    KOMPARE(item[2], m_resultItems->data(m_resultItems->index(num, 2))); // failure message
    KOMPARE(item[3], m_resultItems->data(m_resultItems->index(num, 3))); // filename
    KOMPARE(item[4], m_resultItems->data(m_resultItems->index(num, 4))); // line number
}

// helper
void QTestRunnerTest::nrofMessagesEquals(QAbstractItemModel* results, int num)
{
    for (int i = 0; i < num; i++)
        KVERIFY(results->index(i, 0).isValid());
    KVERIFY(!results->index(num, 0).isValid());
}

// helper
void QTestRunnerTest::runUI(RunnerWindow* win)
{
    QFETCH(QByteArray, registrationXML);
    QBuffer buff(&registrationXML);
    QTestModel* model = new QTestModel();
    model->readTests(&buff);

    win->setModel(model);
    win->show();
    win->ui().actionStart->trigger();
//    if (!QTest::kWaitForSignal(win->runnerModel(), SIGNAL(allItemsCompleted()), 2000))
    if (!QTest::kWaitForSignal(win->runnerModel(), SIGNAL(allItemsCompleted())))
       QFAIL("Timeout while waiting for runner items to complete execution");
}

// helper
void QTestRunnerTest::checkStatusWidget(Ui::StatusWidget* status)
{
    // validate the status widget
    KOMPARE(QString("4"), status->labelNumTotal->text());
    KOMPARE(QString("4"), status->labelNumSelected->text());
    KOMPARE(QString("4"), status->labelNumRun->text());

    KOMPARE(QString(": 3"), status->labelNumSuccess->text());
    KOMPARE(QString(": 1"), status->labelNumErrors->text());
    KOMPARE(QString(": 0"), status->labelNumWarnings->text());

}

// helper
void QTestRunnerTest::checkRunnerItems(QAbstractItemModel* items)
{
    m_runnerItems = items;

// eg:
//     QStringList runn;
//     runn << "0 suite1"
//          << "0 0 fakeqtest1"
//          << "0 0 0 cmd1"
//          << "0 0 1 cmd2"
//          << "0 0 2 x"
//          << "0 1 fakeqtest2"
//          << "0 1 0 cmd1"
//          << "0 1 1 cmd2"
//          << "0 1 2 x"
//          << "0 2 x"
//          << "1 x";

    QFETCH(QStringList, runnerItems);
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
    QModelIndex index = m_runnerItems->index(lvl0, 0);
    if (lvl1 != -1) {
        index = m_runnerItems->index(lvl1, 0, index);
        if (lvl2 != -1) {
            index = m_runnerItems->index(lvl2, 0, index);
        }
    }
    KOMPARE(expected, m_runnerItems->data(index));

}


#include "qtestrunnertest.moc"
QTEST_KDEMAIN(QTestRunnerTest, GUI)
