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

#include "qtestbase.h"
#include <QDebug>

using QxQTest::QTestBase;
using QxQTest::QTestResult;

namespace
{
QList<QVariant> empty(const QString& name)
{
    return QList<QVariant>() << name << "" << "" << "" << "";
}
}

QTestBase::QTestBase()
    :  RunnerItem(empty("")), m_name(""), m_parent(0)
{
}

QTestBase::QTestBase(const QString& name, QTestBase* parent)
    : RunnerItem(empty(name), parent), m_name(name), m_parent(parent)
{}

QTestBase::QTestBase(const QList<QVariant>& data)
    :  RunnerItem(data), m_name(""), m_parent(0)
 {
 }


QTestBase::~QTestBase()
{}

QString QTestBase::name() const
{
    return m_name;
}

QTestBase* QTestBase::owner()
{
    return m_parent;
}

void QTestBase::addTest(QTestBase* test)
{
    appendChild(test);
    m_children.push_back(test);
}

QTestBase* QTestBase::childAt(unsigned i)
{
    return m_children.value(i);
}

unsigned QTestBase::childCount()
{
    return m_children.count();
}

QTestResult QTestBase::result_()
{
    QTestResult res;
    res.setState(QxRunner::RunnerResult(result()));
    res.setMessage(data(2).toString());
    res.setFile(QFileInfo(data(3).toString()));
    res.setLine(data(4).toInt());
    return res;
}

void QTestBase::setResult_(QTestResult& res)
{
    setData(2, res.message());
    setData(3, res.file().filePath());
    setData(4, res.line());
    setResult(res.state());
}
