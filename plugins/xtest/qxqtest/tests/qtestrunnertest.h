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

#ifndef QXQTEST_QTESTRUNNERTEST_H
#define QXQTEST_QTESTRUNNERTEST_H

#include <QtTest/QTest>

class QAbstractItemModel;
namespace Ui
{
class StatusWidget;
}
namespace QxRunner
{
class RunnerWindow;
}

namespace QxQTest
{
namespace it {

class QTestRunnerTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void empty();
    void sunnyDay();
    void runTwice();

private:
    void initNrun(QByteArray&);
    void checkRunnerItems(QStringList items);
    void checkRunnerItem(const QVariant& expected, int lvl0, int lvl1 = -1, int lvl2 = -1);
    void checkStatusWidget(QMap<QString, QString> labels);
    void checkResultItems(QList<QStringList> expected);
    void checkResultItem(int num, const QStringList& item);
    void nrofMessagesEquals(int num);

private:
    QxRunner::RunnerWindow* m_window;
    QAbstractItemModel* m_runnerItems;
    QAbstractItemModel* m_resultItems;
};

}
}


#endif // QXQTEST_QTESTRUNNERTEST_H
