/* KDevelop xUnit pluginQ
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

#ifndef QXQTEST_QTESTMODEL_H
#define QXQTEST_QTESTMODEL_H

#include <qxrunner/runnermodel.h>

class QIODevice;

namespace QxQTest
{

class QTestSuite;
class QTestCase;
class QTestBase;

class QTestModel : public QxRunner::RunnerModel
{
    Q_OBJECT

public:
    QTestModel(QObject* parent = 0);
    ~QTestModel();

    QString name() const;
    QString about() const;

    void readTests(QIODevice* dev);

private:
    void addSuite(QxQTest::QTestSuite* suite);
    void addCase(QxQTest::QTestCase* caze, QxRunner::RunnerItem* parent);
    QxRunner::RunnerItem* addTestItem(QxQTest::QTestBase*, QxRunner::RunnerItem*);
};

} // namespace QxQTest

#endif // QXQTEST_QTESTMODEL_H
