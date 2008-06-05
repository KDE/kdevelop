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

#ifndef QXQTEST_QTESTTITEM_H
#define QXQTEST_QTESTITEM_H

#include <qxrunner/runneritem.h>

#include <QList>
#include <QVariant>
//#include <QProcess>
#include <kprocess.h>

namespace QxQTest
{
class QTestBase;
class QTestCommand;

class QTestItem : public QxRunner::RunnerItem
{
public:
    QTestItem(const QList<QVariant>& data, QxRunner::RunnerItem* parent = 0, QTestBase* test = 0);
    virtual ~QTestItem();
    int run();

private: // state
    QTestBase* m_test;

private: // operations
    void startProcess(QTestCommand* cmd, KProcess* proc);
    int parseOutput(KProcess* proc);
};

} // end namespace QxQTest

#endif // QXQTEST_QTESTITEM_H
