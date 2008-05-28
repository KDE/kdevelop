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

#include "qtestsuite.h"

using QxQTest::QTestSuite;
using QxQTest::QTestBase;
using QxQTest::QTestCase;

QTestSuite::QTestSuite()
    : QTestBase("", 0), m_path(QFileInfo(""))
{
}

QTestSuite::QTestSuite(const QString& name, const QFileInfo& path, QTestBase* parent)
    : QTestBase(name, parent), m_path(path)
{
}

QTestSuite::~QTestSuite()
{
}

unsigned QTestSuite::nrofChildren()
{
    return m_children.count();
}

QFileInfo QTestSuite::path()
{
    return m_path;
}

void QTestSuite::setPath(const QFileInfo& path)
{
    m_path = path;
}

void QTestSuite::addTest(QTestCase* test)
{
    m_children.push_back(test);
}

QTestCase* QTestSuite::getTestAt(unsigned i)
{
    return m_children.value(i);
}

#include "qtestsuite.moc"
