/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
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

#include "test.h"
#include <KDebug>

using Veritas::Test;
using Veritas::TestState;
using Veritas::TestResult;

Test::Test(const QList<QVariant>& data, Test* parent)
    : m_parentItem(parent),
      m_itemData(data),
      m_state(Veritas::NoResult)
{
    // Make sure this item has as many columns as the parent.
    for (int i= m_itemData.count(); i < s_columnCount; i++) {
        m_itemData << "";
    }
    if (!data.empty())
        m_name = data.value(0).toString();
    else
        m_name = "";
    setSelected(true);
}

Test::Test(const QString& name, Test* parent)
    : m_parentItem(parent),
      m_name(name),
      m_state(Veritas::NoResult)
{
    // Make sure this item has as many columns as the parent.
    for (int i=0; i < s_columnCount; i++) {
        m_itemData << "";
    }
    setSelected(true);
}

QString Test::name() const
{
    return m_name;
}

Test::~Test()
{
    qDeleteAll(m_childItems);
}

int Test::run()
{
    return 0;
}

bool Test::shouldRun() const
{
    return false;
}

void Test::setIndex(const QModelIndex& index)
{
    m_index = index;
}
QModelIndex Test::index() const
{
    return m_index;
}

Test* Test::parent() const
{
    return m_parentItem;
}

Test* Test::child(int row) const
{
    return m_childItems.value(row);
}

Test* Test::childNamed(const QString& name) const
{
    if (!m_childMap.contains(name))
        return 0;
    return m_childMap[name];
}

void Test::addChild(ITest* item)
{
    Test* t = qobject_cast<Test*>(item);
    m_childItems.append(t);
    m_childMap[t->name()] = t;
}

int Test::childCount() const
{
    return m_childItems.count();
}

int Test::row() const
{
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<Test*>(this));
    }

    return 0;
}

int Test::columnCount() const
{
    Q_ASSERT(m_itemData.count() == s_columnCount);
    return s_columnCount;
}

QVariant Test::data(int column) const
{
    if (column == 0) {
        return name();
    } else {
        return m_itemData.value(column);
    }
    // Note: QList provides sensible default values if the column
    // number is out of range.
}

void Test::setData(int column, const QVariant& value)
{
    if (column == 0) {
        m_name = value.toString();
    } else if (column > 0 && column < columnCount()) {
        m_itemData.replace(column, value.toString());
    }
}

bool Test::selected() const
{
    return m_selected;
}

void Test::setSelected(bool select)
{
    m_selected = select;
    foreach (Test* child, m_childItems)
        child->setSelected(select);
}

TestState Test::state() const
{
    return m_state;
}

void Test::setState(TestState result)
{
    m_state = result;
}

TestResult Test::result() const
{
    TestResult res;
    res.setState(state());
    res.setMessage(data(2).toString());
    res.setFile(QFileInfo(data(3).toString()));
    res.setLine(data(4).toInt());
    return res;
}

void Test::setResult(const TestResult& res)
{
    setData(2, res.message());
    setData(3, res.file().filePath());
    setData(4, res.line());
    setState(res.state());
}

void Test::clear()
{
    // Initialize columns except column 0 which contains the item name.
    for (int i = 1; i < columnCount(); i++) {
        setData(i, "");
    }

    setState(Veritas::NoResult);
}

#include "test.moc"
