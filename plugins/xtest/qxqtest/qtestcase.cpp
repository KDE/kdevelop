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

#include "qtestcase.h"
#include "qtestsuite.h"
#include <QDir>

using QxQTest::QTestCase;
using QxQTest::QTestBase;
using QxQTest::QTestCommand;
using QxQTest::QTestSuite;

QTestCase::QTestCase()
    : QTestBase("", 0), m_exe(QFileInfo(""))
{
}

QTestCase::QTestCase(const QString& name, const QFileInfo& exe, QTestBase* parent)
    : QTestBase(name, parent), m_exe(exe)
{
}

QTestCase::~QTestCase()
{}

QFileInfo QTestCase::executable()
{
    QFileInfo exe(m_exe);
    QTestBase* suite = parent();
    if(suite != 0 && qobject_cast<QTestSuite*>(suite) != 0)
    {
        QDir path = QDir(qobject_cast<QTestSuite*>(suite)->path().filePath());
        exe.setFile(path, m_exe.filePath());
    }
    return exe;
}

void QTestCase::setExecutable(const QFileInfo& exe)
{
    m_exe = exe;
}

QTestCommand* QTestCase::testAt(unsigned i)
{
    return qobject_cast<QTestCommand*>(childAt(i));
}

#include "qtestcase.moc"
