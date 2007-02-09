/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdevitemmodel.h"

#include <QtCore/QVector>

namespace KDevelop
{

ItemModel::ItemModel(QObject *parent)
  : QAbstractItemModel(parent),
  m_collection( 0L )
{
}

ItemModel::~ItemModel()
{
}

void ItemModel::appendItem(Item *item, ItemCollection *collection)
{
  QModelIndex parent;

  if (!collection)
    collection = root();

  parent = indexOf(collection);
  int row = collection->itemCount();

  beginInsertRows(parent, row, row);
  collection->add(item);
  endInsertRows();
}

void ItemModel::removeItem(Item *item)
{
  Q_ASSERT(item != 0 && item->parent() != 0);
  Q_ASSERT(item->parent()->collection() != 0);

  ItemCollection *parent_collection = item->parent()->collection();

  int row = positionOf(item);
  beginRemoveRows(indexOf(parent_collection), row, row);
  parent_collection->remove(row);
  endRemoveRows();
}

QModelIndex ItemModel::indexOf(Item *item) const
{
  Q_ASSERT(item != 0);

  if (!item->parent())
    return createIndex(0, 0, root());

  return createIndex(positionOf(item), 0, item);
}

int ItemModel::positionOf(Item *item) const
{
  Q_ASSERT(item->parent() != 0);

  return item->parent()->indexOf(item);
}

Item *ItemModel::item(const QModelIndex &index) const
{
  return reinterpret_cast<Item*>(index.internalPointer());
}

ItemCollection *ItemModel::root() const
{
  if (!m_collection)
    m_collection = new ItemCollection( QString::fromUtf8("<root>"), 0 );
  return const_cast<ItemCollection*>(m_collection);
}

QModelIndex ItemModel::index(int row, int column, const QModelIndex &parent) const
{
  Item *parent_item = item(parent);
  if (!parent_item)
    parent_item = root();

  Q_ASSERT(parent_item != 0);
  Q_ASSERT(parent_item->group() != 0);
  //Q_ASSERT(row < parent_item->group()->itemCount());
  if ( row >= parent_item->group()->itemCount() )
    return QModelIndex();

  return createIndex(row, column, parent_item->group()->itemAt(row));
}

QModelIndex ItemModel::parent(const QModelIndex &index) const
{
  if (Item *kdev_item = item(index))
    {
      if (kdev_item->parent())
        if (kdev_item->parent()->parent())
          return createIndex(positionOf(kdev_item->parent()), 0,
                             kdev_item->parent());
    }

  return QModelIndex();
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
  Item *parent_item = item(parent);
  if (!parent_item)
    parent_item = root();

  Q_ASSERT(parent_item != 0);

  if (ItemGroup *group = parent_item->group())
    return group->itemCount();

  return 0;
}

int ItemModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
  if (Item *kdev_item = item(index))
    {
      switch (role)
        {
          case Qt::DisplayRole: return kdev_item->name();
          case Qt::DecorationRole: return kdev_item->icon();
          case Qt::ToolTipRole: return kdev_item->toolTip();
          case Qt::WhatsThisRole: return kdev_item->whatsThis();
          default: break;
        }
    }

  return QVariant();
}

bool ItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  Q_UNUSED(index);
  Q_UNUSED(value);
  Q_UNUSED(role);
  return false;
}

void ItemModel::refresh()
{
  emit reset();
}

}
#include "kdevitemmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
