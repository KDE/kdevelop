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

#ifndef QXQTEST_QTESTBASE_H
#define QXQTEST_QTESTBASE_H

#include <qxrunner/runneritem.h>
#include "qtestresult.h"
#include <QList>
#include <QString>
#include <QFileInfo>


namespace QxQTest
{

/**
 * Base for QTestCase, QTestCommand, QTestSuite
 **/
class QTestBase : public QxRunner::RunnerItem
{
public:
    QTestBase();
    QTestBase(const QString&, QTestBase* parent);
    QTestBase(const QList<QVariant>& data); // root
    virtual ~QTestBase();

    QString name() const;
    QTestBase* owner();

    void addTest(QTestBase*);
    unsigned childCount();
    QTestResult result_();
    void setResult_(QTestResult&);

    // RunnerItem implementation. default to doing nadda
    virtual int run() { return 0; }
    virtual bool isRunnable() { return false; }

protected:
    QTestBase* childAt(unsigned i);

private:
    QString m_name;
    QTestBase* m_parent;
    QList<QTestBase*> m_children;
};

} // end namespace QxQTest

#endif // QXQTEST_QTESTBASE_H
