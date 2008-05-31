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

#include "qtestsuitetest.h"

#include <kasserts.h>
#include <qtest_kde.h>
#include <qtestcase.h>

using QxQTest::QTestSuite;
using QxQTest::QTestCase;

namespace
{
QFileInfo someExe()
{
    return QFileInfo("some_exe.exe");
}
}

void QTestSuiteTest::init()
{
    m_path = QFileInfo("/path/to/module");
    m_name = "s1";
    m_suite = new QTestSuite(m_name, m_path, 0);
}

void QTestSuiteTest::cleanup()
{
    delete m_suite;
}

void QTestSuiteTest::construct()
{
    KOMPARE(m_suite->name(), m_name);
    KOMPARE(m_suite->path(), m_path);
}

void QTestSuiteTest::addChildCase()
{
    KOMPARE(m_suite->childCount(), 0);

    QTestCase t("t1", someExe(), m_suite);
    m_suite->addTest(&t);

    KOMPARE(m_suite->childCount(), 1);
    KOMPARE(m_suite->testAt(0), &t);
}

void QTestSuiteTest::addChildCases()
{
    QTestCase t1("t1", someExe(), m_suite);
    QTestCase t2("t2", someExe(), m_suite);
    m_suite->addTest(&t1);
    m_suite->addTest(&t2);

    KOMPARE(m_suite->childCount(), 2);
    KOMPARE(m_suite->testAt(0), &t1);
    KOMPARE(m_suite->testAt(1), &t2);
}

#include "qtestsuitetest.moc"
QTEST_KDEMAIN(QTestSuiteTest, NoGUI)
