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

#ifndef KDEVITEMMODEL_H
#define KDEVITEMMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtGui/QIcon>
#include <QtCore/QPair>

#include "kdevsharedptr.h"
#include "kdevexport.h"

class KDevItem;
class KDevItemGroup;
class KDevItemCollection;
class KDevItemModel;

class KDEVPLATFORM_EXPORT KDevItem : public KDevShared
{
public:
  KDevItem(KDevItemGroup *parent)
    : m_parent(parent) {}

  virtual ~KDevItem() {}

  /**
   * Retrieve this item's parent.
   * @return the parent of this item.
   */
  KDevItemGroup *parent() const { return m_parent; }

  /**
   * Set the parent of this item. The new parent is specified by
   * @p parent
   */
  void setParent(KDevItemGroup *parent) { m_parent = parent; }

  /**
   * Retrieve the group this item belongs to. If a null pointer
   * is returned, then this item does not have a group.
   * @return the group this item is a member of.
   */
  virtual KDevItemGroup *group() const { return 0; }

  /**
   * Retrieve the collection this item belongs to. If a null pointer
   * is returned, then this item is not part of a collection.
   * @return the collection this item belongs to
   * @todo describe the difference between a collection and a group
   */
  virtual KDevItemCollection *collection() const { return 0; }

  /**
   * Set the name of the item to the name specified by @p newName
   *
   * Implementations should ensure that the new name to be set is not an empty
   * string
   */
  virtual void setName( const QString& newName ) = 0;

  /** Get the name of the item */
  virtual QString name() const = 0;

  /** Get the icon for the item */
  virtual QIcon icon() const = 0;

  /** Get the tooltip for this item */
  virtual QString toolTip() const = 0;

  /** Get the what's this help for this item */
  virtual QString whatsThis() const = 0;

private:
  KDevItemGroup *m_parent;
};

/**
 * The KDevItemGroup class holds a group of KDevItems.
 */
class KDEVPLATFORM_EXPORT KDevItemGroup: public KDevItem
{
public:
  KDevItemGroup(KDevItemGroup *parent)
    : KDevItem(parent) {}

  virtual KDevItemGroup *group() const
  { return const_cast<KDevItemGroup*>(this); }

  /**
   * Get the number of items in this group
   */
  virtual int itemCount() const = 0;

  /**
   * Get the index of the item described by @p item
   *
   * Subclasses that implement KDevItemGroup should return -1 if the item is
   * not in this item group.
   */
  virtual int indexOf(KDevItem *item) const = 0;

  /**
   * Get the item at the index specified by @p index
   *
   * If there is no item at the specified index, the returned value will be
   * zero.
   * @return the item specified by @p index
   */
  virtual KDevItem *itemAt(int index) const = 0;
};

/**
 * Implements the KDevItemGroup interface and provides the KDevItemModel
 * with access to the group of items.
 */
class KDEVPLATFORM_EXPORT KDevItemCollection: public KDevItemGroup
{
public:
  explicit KDevItemCollection(const QString &name, KDevItemGroup *parent = 0)
    : KDevItemGroup(parent), m_name(name) {}

  virtual ~KDevItemCollection() {}

  virtual KDevItemCollection *collection() const { return const_cast<KDevItemCollection*>(this); }

  /** @copydoc KDevItem::setName( const QString& ) */
  virtual void setName( const QString& newName  )
  {
    Q_ASSERT( !newName.isEmpty() );
    m_name = newName;
  }

  /** @copydoc KDevItem::name() */
  virtual QString name() const { return m_name; }

  /** @copydoc KDevItem::icon() */
  virtual QIcon icon() const { return QIcon(); }

  /** @copydoc KDevItem::toolTip() */
  virtual QString toolTip() const { return QString(); }

  /** @copydoc KDevItem::whatsThis() */
  virtual QString whatsThis() const { return QString(); }

  /** Get the list of items in this collection */
  virtual const QList<KDevItem *> &items() const { return m_items; };

  /** @copydoc KDevItemGroup::itemCount */
  virtual int itemCount() const { return m_items.count(); }

  /** @copydoc KDevItemGroup::indexOf */
  virtual int indexOf(KDevItem *item) const { return m_items.indexOf(item); }

  /** @copydoc KDevItemGroup::itemAt */
  virtual KDevItem *itemAt(int index) const { return m_items.at(index); }

  /** Remove all items from the collection */
  virtual void clear() { m_items.clear(); }

  /**
   * Adds a KDevItem specified by @p item to the collection.
   *
   * The collection will become the parent of this item. Items that are added
   * should not already have a parent.
   */
  virtual void add(KDevItem *item)
  {
    Q_ASSERT(item != 0);
    Q_ASSERT(item->parent() == this || item->parent() == 0);

    if (item->parent() != this)
      item->setParent(this);

    m_items.append(item);
  }

  /**
   * Removes a KDevItem from the collection.
   *
   * The item at the index position @p index is removed from the collection.
   * @p index must be a valid index position.
   */
  virtual void remove(int index)
  {
    Q_ASSERT(index >= 0 );
    Q_ASSERT(index < m_items.count());
    m_items.removeAt(index);
  }

  /**
   * Replace an existing item in the collection with a different item.
   *
   * The item at the index position @p index is replaced with @p item. The
   * index specifed by @p index must be a valid index position.
   */
  virtual void replace(int index, KDevItem *item)
  {
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < m_items.count());
    m_items.replace(index, item);
  }

private:
  QString m_name;
  QList<KDevItem *> m_items;
};

/**
 * The generic KDevelop Model.
 *
 * If you need a model anywhere in KDevelop, then your model can inherit
 * from KDevItemModel and you can store your items in classes derived from
 * KDevItem to get some nice features.
 */
class KDEVPLATFORM_EXPORT KDevItemModel: public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit KDevItemModel(QObject *parent = 0);
  virtual ~KDevItemModel();

  virtual KDevItemCollection *root() const;

  void appendItem(KDevItem *item, KDevItemCollection *collection = 0);
  void removeItem(KDevItem *item);

  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex &index) const;

  virtual int rowCount(const QModelIndex &parent) const;
  virtual int columnCount(const QModelIndex &parent) const;

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

  virtual KDevItem *item(const QModelIndex &index) const;
  virtual QModelIndex indexOf(KDevItem *item) const;

public slots:
  void refresh();

protected:
  int positionOf(KDevItem *item) const;

private:
  mutable KDevItemCollection *m_collection;
};

#endif // KDEVITEMMODEL_H

// kate: space-indent on; indent-width 2; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
