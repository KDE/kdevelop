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

#ifndef QXRUNNER_RUNNERPROXYMODELTEST_H
#define QXRUNNER_RUNNERPROXYMODELTEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace QxRunner
{
class RunnerModel;
class RunnerProxyModel;
}

class RunnerProxyModelTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void default_();
    //void deactivate(); TODO
    void enableColumns();
    void disableColumn();
    void errorHandling();

private:
    void assertDataAt(int row, int column, const QVariant& expected);
    void assertRowFiltered(int row);
    void setAllColumnsEnabled();
    void assertRowContains(int row, const QVariant& col1, const QVariant& col2, const QVariant& col3);

private:
    QxRunner::RunnerModel* source;
    QxRunner::RunnerProxyModel* proxy;
};

#endif // QXRUNNER_RUNNERPROXYMODELTEST_H
