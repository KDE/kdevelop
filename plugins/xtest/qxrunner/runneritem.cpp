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

/*!
 * \file  runneritem.cpp
 *
 * \brief Implements class RunnerItem.
 */

#include "runneritem.h"
#include "runnermodel.h"

namespace QxRunner
{

RunnerItem::RunnerItem(const QList<QVariant>& data, RunnerItem* parent)
{
    m_parentItem = parent;
    m_itemData = data;

    // Make sure this item has as many columns as the parent.
    if (m_parentItem) {
        int parentColumns = m_parentItem->columnCount();
        int itemColumns =  m_itemData.count();

        for (int i = itemColumns; i < parentColumns; i++) {
            m_itemData.append("");
        }
    }

    setSelected(true);
    setResult(QxRunner::NoResult);
}

RunnerItem::~RunnerItem()
{
    qDeleteAll(m_childItems);
}

RunnerItem* RunnerItem::parent() const
{
    return m_parentItem;
}

RunnerItem* RunnerItem::child(int row) const
{

    return m_childItems.value(row);

    // Note: QList provides sensible default values if the row
    // number is out of range.
}

void RunnerItem::appendChild(RunnerItem* item)
{
    m_childItems.append(item);
}

int RunnerItem::childCount() const
{
    return m_childItems.count();
}

int RunnerItem::row() const
{
    if (m_parentItem) {
        return m_parentItem->m_childItems.indexOf(const_cast<RunnerItem*>(this));
    }

    return 0;
}

void RunnerItem::setModel(RunnerModel* model)
{
    m_model = model;
}


void RunnerItem::signalCompleted(QModelIndex index_)
{
/*  Appears to be illegal, Qt crashes
#0  0x00002ad575811641 in QCoreApplication::postEvent () from /usr/lib/libQtCore.so.4
#1  0x00002ad576d60271 in QxRunner::RunnerItem::signalCompleted (this=0x70e100) at /home/nix/KdeDev/kdevelop/plugins/xtest/qxrunner/runneritem.cpp:101
    if (m_model)
    {
        m_model->postItemCompleted(index());
    }
    else if (m_parentItem->m_model)
    {
        m_parentItem->m_model->postItemCompleted(index());
    }
    else
    {
        kDebug() << "Failed to post completion event for " << m_index;
    }*/
    emit completed(index_);
}

void RunnerItem::signalStarted(QModelIndex index_)
{
    emit started(index_);
}

int RunnerItem::columnCount() const
{
    return m_itemData.count();
}

QVariant RunnerItem::data(int column) const
{
    return m_itemData.value(column);

    // Note: QList provides sensible default values if the column
    // number is out of range.
}

void RunnerItem::setData(int column, const QVariant& value)
{
    if (column >= 0 && column < columnCount()) {
        m_itemData.replace(column, value.toString());
    }
}

bool RunnerItem::isSelected() const
{
    return m_selected;
}

void RunnerItem::setSelected(bool select)
{
    m_selected = select;
}

int RunnerItem::result() const
{
    return m_result;
}

void RunnerItem::setResult(int result)
{
    m_result = result;
}

void RunnerItem::clear()
{
    // Initialize columns except column 0 which contains the item name.
    for (int i = 1; i < columnCount(); i++) {
        setData(i, "");
    }

    setResult(QxRunner::NoResult);
}

} // namespace
