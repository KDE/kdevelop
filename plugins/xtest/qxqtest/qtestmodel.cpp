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

#include "qtestmodel.h"
#include "qtestitem.h"
#include "qtestcase.h"
#include "qtestsuite.h"
#include "qtestcommand.h"
#include "qtestregister.h"
#include "qtestbase.h"
#include <QIODevice>
#include <QDebug>

using QxQTest::QTestModel;
using QxQTest::QTestItem;
using QxQTest::QTestBase;
using QxQTest::QTestSuite;
using QxQTest::QTestCase;
using QxQTest::QTestBase;
using QxQTest::QTestCommand;
using QxRunner::RunnerModel;
using QxRunner::RunnerItem;

QTestModel::QTestModel(QObject* parent)
        : RunnerModel(parent)
{
    // Data for column headers is stored in the root item.
    QList<QVariant> rootData;
    rootData << tr("Test Name") << tr("Result") << tr("Message")
             << tr("File Name") << tr("Line Number");

    setRootItem(new QTestItem(rootData));
    setExpectedResults(QxRunner::RunWarning | QxRunner::RunError);
}

QTestModel::~QTestModel()
{
}

QString QTestModel::name() const
{
    return tr("QxQTest");
}

QString QTestModel::about() const
{
    QString version("4.4");
    QString aboutModel = tr("for QTestLib") + " " + version;
    return aboutModel;
}

void QTestModel::readTests(QIODevice* dev)
{
    QTestRegister reg;
    reg.addFromXml(dev);
    for (unsigned i=0; i<reg.testSuiteCount(); i++)
    {
        QTestSuite* suite = reg.takeSuite(i);
        addSuite(suite);
    }
}

// TODO push-up the nrofChildren & getTestAt methods 
//      so the follwing mtds can be merged

void QTestModel::addSuite(QTestSuite* suite)
{
    RunnerItem* item = addTestItem(suite, rootItem());
    for (unsigned i=0; i<suite->testCaseCount(); i++)
        addCase(suite->testAt(i), item);
}

void QTestModel::addCase(QTestCase* caze, RunnerItem* parent)
{
    RunnerItem* item = addTestItem(caze, parent);
    for (unsigned i=0; i<caze->testCommandCount(); i++)
        addTestItem(caze->testAt(i), item);
}

RunnerItem* QTestModel::addTestItem(QTestBase* test, RunnerItem* parent)
{
    QList<QVariant> columnData;
    columnData << test->name();
    QTestItem* item = new QTestItem(columnData, parent, test);
    parent->appendChild(item);
    return item;
}

#include "qtestmodel.moc"
