/* KDevelop xUnit plugin
 *
 * Copyright 2006 Ernst Huber <qxrunner@systest.ch>
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

#include "veritas/test.h"
#include <KDebug>

using Veritas::Test;
using Veritas::TestState;
using Veritas::TestResult;

const int Test::s_columnCount = 5;

Test::Test(const QList<QVariant>& data, Test* parent)
    : QObject(parent),
      m_itemData(data),
      m_state(Veritas::NoResult),
      m_result(new TestResult)
{
    // Make sure this item has as many columns as the parent.
    for (int i= m_itemData.count(); i < s_columnCount; i++) {
        m_itemData << "";
    }
    if (!data.empty()) {
        m_name = data.value(0).toString();
    } else {
        m_name.clear();
    }
    setSelected(true);
}

Test::Test(const QString& name, Test* parent)
    : QObject(parent),
      m_name(name),
      m_state(Veritas::NoResult),
      m_result(new TestResult)
{
    // Make sure this item has as many columns as the parent.
    for (int i=0; i < s_columnCount; i++) {
        m_itemData << QString();
    }
    setSelected(true);
}

QString Test::name() const
{
    return m_name;
}

Test::~Test()
{
    delete m_result;
    qDeleteAll(m_children);
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
    return qobject_cast<Test*>(QObject::parent());
}

Test* Test::child(int row) const
{
    return m_children.value(row);
}

Test* Test::childNamed(const QString& name) const
{
    if (!m_childMap.contains(name))
        return 0;
    return m_childMap[name];
}

void Test::addChild(Test* item)
{
    Test* t = qobject_cast<Test*>(item);
    m_children.append(t);
    m_childMap[t->name()] = t;
}

int Test::childCount() const
{
    return m_children.count();
}

int Test::row() const
{
    if (parent()) {
        return parent()->m_children.indexOf(const_cast<Test*>(this));
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
    foreach (Test* child, m_children) {
        child->setSelected(select);
    }
}

TestState Test::state() const
{
    return m_state;
}

void Test::setState(TestState result)
{
    m_state = result;
    if (m_result) {
        m_result->setState(result);
    }
}

TestResult* Test::result() const
{
    return m_result;
}

void Test::setResult(TestResult* res)
{
    if (m_result) delete m_result;
    m_result = res;
    if (res) {
        setData(2, res->message());
        setData(3, res->file().filePath());
        setData(4, res->line());
        setState(res->state());
    }
}

void Test::clear()
{
    // Initialize columns except column 0 which contains the item name.
    for (int i = 1; i < columnCount(); i++) {
        setData(i, "");
    }
    setState(Veritas::NoResult);
    if (m_result) delete m_result;
    m_result = new TestResult;
}

QList<Test*> Test::leafs() const
{
    QList<Test*> l;
    foreach(Test* t, m_children) {
        if (t->childCount() == 0) {
            l.append(t);
        } else {
            l += t->leafs();
        }
    }
    return l;
}

void Test::signalStarted()
{
    Q_ASSERT(index().isValid());
    emit started(index());
}

void Test::signalFinished()
{
    Q_ASSERT(index().isValid());
    emit finished(index());
}


#include "test.moc"
