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

#include "testbase.h"
#include <QDebug>

using QxCheck::TestBase;
using QxCheck::TestResult;

namespace
{
QList<QVariant> empty(const QString& name)
{
    return QList<QVariant>() << name << "" << "" << "" << "";
}
}

TestBase::TestBase()
        :  RunnerItem(empty("")), m_name(""), m_parent(0)
{
}

TestBase::TestBase(const QString& name, TestBase* parent)
        : RunnerItem(empty(name), parent), m_name(name), m_parent(parent)
{}

TestBase::TestBase(const QList<QVariant>& data)
        :  RunnerItem(data), m_name(""), m_parent(0)
{}

TestBase::~TestBase()
{}

QString TestBase::name() const
{
    return m_name;
}

TestBase* TestBase::owner()
{
    return m_parent;
}

void TestBase::addTest(TestBase* test)
{
    appendChild(test);
    m_children.push_back(test);
}

TestBase* TestBase::childAt(unsigned i)
{
    return m_children.value(i);
}

unsigned TestBase::childCount()
{
    return m_children.count();
}

TestResult TestBase::result_()
{
    TestResult res;
    res.setState(QxRunner::RunnerResult(result()));
    res.setMessage(data(2).toString());
    res.setFile(QFileInfo(data(3).toString()));
    res.setLine(data(4).toInt());
    return res;
}

void TestBase::setResult_(TestResult& res)
{
    setData(2, res.message());
    setData(3, res.file().filePath());
    setData(4, res.line());
    setResult(res.state());
}

TestBase* TestBase::findTestNamed(const QString& name)
{
    TestBase* child;
    for (int i = 0; i < childCount(); i++)
    {
        child= childAt(i);
        if (name == child->name())
            return child;
    }
    return 0;
}

#include "testbase.moc"
