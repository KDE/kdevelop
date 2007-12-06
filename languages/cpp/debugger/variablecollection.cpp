/*
 * GDB Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "variablecollection.h"

#include <KLocale>
#include <KDebug>

#include "gdbcontroller.h"
#include "variableitem.h"

using namespace GDBDebugger;

VariableCollection::VariableCollection(GDBController* parent)
    : QAbstractItemModel(parent)
{
}

VariableCollection::~ VariableCollection()
{
    qDeleteAll(m_items);
}

void VariableCollection::addItem(VariableItem * item)
{
    m_items.append(item);
}

void VariableCollection::deleteItem(VariableItem * item)
{
    int index = m_items.indexOf(item);
    Q_ASSERT(index != -1);
    delete m_items.takeAt(index);
}

GDBController * GDBDebugger::VariableCollection::controller() const
{
    return static_cast<GDBController*>(const_cast<QObject*>(QObject::parent()));
}

int GDBDebugger::VariableCollection::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid())
        return m_items.count();

    VariableItem* item = itemForIndex(parent);
    if (!item)
        return 0;

    return item->children().count();
}

QModelIndex GDBDebugger::VariableCollection::index(int row, int column, const QModelIndex & parent) const
{
    if (row < 0 || column < 0 || column > VariableItem::ColumnLast)
        return QModelIndex();

    if (!parent.isValid()) {
        return createIndex(row, column, 0);
    }

    VariableItem* item = itemForIndex(parent);
    Q_ASSERT(item);

    if (row < item->children().count())
        return createIndex(row, column, item);

    return QModelIndex();
}

VariableItem * GDBDebugger::VariableCollection::itemForIndex(const QModelIndex & index) const
{
    if (index.internalPointer())
        return m_items.at(index.row());

    return static_cast<VariableItem*>(index.internalPointer())->parent();
}

VariableItem * GDBDebugger::VariableCollection::parentForIndex(const QModelIndex & index) const
{
    return static_cast<VariableItem*>(index.internalPointer());
}

int GDBDebugger::VariableCollection::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return VariableItem::ColumnLast + 1;
}

QVariant GDBDebugger::VariableCollection::data(const QModelIndex & index, int role) const
{
    if (VariableItem* item = itemForIndex(index))
        return item->data(index.column(), role);

    return QVariant();
}

Qt::ItemFlags GDBDebugger::VariableCollection::flags(const QModelIndex & index) const
{
    if (VariableItem* item = itemForIndex(index))
        return item->flags(index.column());

    return 0;
}

QVariant GDBDebugger::VariableCollection::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    switch (role) {
        case Qt::DisplayRole:
            switch (section) {
                case VariableItem::ColumnName:
                    return i18n("Variable");

                case VariableItem::ColumnValue:
                    return i18n("Value");

                case VariableItem::ColumnType:
                    return i18n("Type");
            }
            break;
    }

    return QVariant();
}

QModelIndex GDBDebugger::VariableCollection::parent(const QModelIndex & index) const
{
    VariableItem* parent = parentForIndex(index);
    if (!parent)
        return QModelIndex();

    VariableItem* grandParent = parent->parent();
    if (!grandParent)
        return createIndex(m_items.indexOf(parent), 0, 0);

    return createIndex(grandParent->children().indexOf(parent), 0, grandParent);
}

bool GDBDebugger::VariableCollection::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.column() != VariableItem::ColumnName)
        return false;

    VariableItem* item = itemForIndex(index);
    if (!item)
        return false;

    if (role == Qt::EditRole) {
        item->setVariableName(value.toString());
        return true;

    } else {
        kWarning() << "Unsupported set data role" << role;
    }

    return false;
}

#include "variablecollection.moc"
