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

#ifndef QXCPPUNIT_TESTBASE_H
#define QXCPPUNIT_TESTBASE_H

#include <qxrunner/runneritem.h>
#include <QList>
#include <QString>
#include <QFileInfo>

#include "testresult.h"

namespace QxCppUnit
{

/**
 * Base for QTestCase, QTestCommand, QTestSuite
 **/
class TestBase : public QxRunner::RunnerItem
{
Q_OBJECT

public:
    TestBase();
    TestBase(const QString&, TestBase* parent);
    TestBase(const QList<QVariant>& data); // root
    virtual ~TestBase();

    QString name() const;
    TestBase* owner();

    void addTest(TestBase*);
    unsigned childCount();
    TestResult result_();
    void setResult_(TestResult&);

    TestBase* findTestNamed(const QString& name);

    // RunnerItem implementation. default to doing nadda
    virtual int run() {
        return 0;
    }
    virtual bool isRunnable() {
        return false;
    }

protected:
    TestBase* childAt(unsigned i);

private:
    QString m_name;
    TestBase* m_parent;
    QList<TestBase*> m_children;
};

} // end namespace QxCppUnit

#endif // QXCPPUNIT_TESTBASE_H
