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

KDevItemModel::KDevItemModel(QObject *parent)
  : QAbstractItemModel(parent),
  m_collection( 0L )
{
}

KDevItemModel::~KDevItemModel()
{
}

void KDevItemModel::appendItem(KDevItem *item, KDevItemCollection *collection)
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

void KDevItemModel::removeItem(KDevItem *item)
{
  Q_ASSERT(item != 0 && item->parent() != 0);
  Q_ASSERT(item->parent()->collection() != 0);

  KDevItemCollection *parent_collection = item->parent()->collection();

  int row = positionOf(item);
  beginRemoveRows(indexOf(parent_collection), row, row);
  parent_collection->remove(row);
  endRemoveRows();
}

QModelIndex KDevItemModel::indexOf(KDevItem *item) const
{
  Q_ASSERT(item != 0);

  if (!item->parent())
    return createIndex(0, 0, root());

  return createIndex(positionOf(item), 0, item);
}

int KDevItemModel::positionOf(KDevItem *item) const
{
  Q_ASSERT(item->parent() != 0);

  return item->parent()->indexOf(item);
}

KDevItem *KDevItemModel::item(const QModelIndex &index) const
{
  return reinterpret_cast<KDevItem*>(index.internalPointer());
}

KDevItemCollection *KDevItemModel::root() const
{
  if (!m_collection)
    m_collection = new KDevItemCollection( QString::fromUtf8("<root>"), 0 );
  return const_cast<KDevItemCollection*>(m_collection);
}

QModelIndex KDevItemModel::index(int row, int column, const QModelIndex &parent) const
{
  KDevItem *parent_item = item(parent);
  if (!parent_item)
    parent_item = root();

  Q_ASSERT(parent_item != 0);
  Q_ASSERT(parent_item->group() != 0);
  //Q_ASSERT(row < parent_item->group()->itemCount());
  if ( row >= parent_item->group()->itemCount() )
    return QModelIndex();

  return createIndex(row, column, parent_item->group()->itemAt(row));
}

QModelIndex KDevItemModel::parent(const QModelIndex &index) const
{
  if (KDevItem *kdev_item = item(index))
    {
      if (kdev_item->parent())
        if (kdev_item->parent()->parent())
          return createIndex(positionOf(kdev_item->parent()), 0,
                             kdev_item->parent());
    }

  return QModelIndex();
}

int KDevItemModel::rowCount(const QModelIndex &parent) const
{
  KDevItem *parent_item = item(parent);
  if (!parent_item)
    parent_item = root();

  Q_ASSERT(parent_item != 0);

  if (KDevItemGroup *group = parent_item->group())
    return group->itemCount();

  return 0;
}

int KDevItemModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QVariant KDevItemModel::data(const QModelIndex &index, int role) const
{
  if (KDevItem *kdev_item = item(index))
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

bool KDevItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  Q_UNUSED(index);
  Q_UNUSED(value);
  Q_UNUSED(role);
  return false;
}

void KDevItemModel::refresh()
{
  emit reset();
}

#include "kdevitemmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
