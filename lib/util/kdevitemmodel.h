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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KDEVITEMMODEL_H
#define KDEVITEMMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtGui/QIcon>

class KDevItem;
class KDevItemGroup;
class KDevItemCollection;

class KDevItem
{
public:
  KDevItem(KDevItemGroup *parent)
    : m_parent(parent) {}

  virtual ~KDevItem() {}

  KDevItemGroup *parent() const { return m_parent; }
  void setParent(KDevItemGroup *parent) { m_parent = parent; }

  virtual KDevItemGroup *group() const { return 0; }
  virtual KDevItemCollection *collection() const { return 0; }

  virtual QString name() const = 0;
  virtual QIcon icon() const = 0;
  virtual QString toolTip() const = 0;
  virtual QString whatsThis() const = 0;

private:
  KDevItemGroup *m_parent;
};

class KDevItemGroup: public KDevItem
{
public:
  KDevItemGroup(KDevItemGroup *parent)
    : KDevItem(parent) {}

  virtual KDevItemGroup *group() const
  { return const_cast<KDevItemGroup*>(this); }

  virtual int itemCount() const = 0;
  virtual int indexOf(KDevItem *item) const = 0;
  virtual KDevItem *itemAt(int index) const = 0;
};

class KDevItemCollection: public KDevItemGroup
{
public:
  KDevItemCollection(const QString &name, KDevItemGroup *parent = 0)
    : KDevItemGroup(parent), m_name(name) {}

  virtual ~KDevItemCollection() { clear(); }

  virtual KDevItemCollection *collection() const { return const_cast<KDevItemCollection*>(this); }

  virtual QString name() const { return m_name; }
  virtual QIcon icon() const { return QIcon(); }
  virtual QString toolTip() const { return QString(); }
  virtual QString whatsThis() const { return QString(); }

  virtual int itemCount() const { return m_items.count(); }
  virtual int indexOf(KDevItem *item) const { return m_items.indexOf(item); }
  virtual KDevItem *itemAt(int index) const { return m_items.at(index); }

  void clear() { qDeleteAll(m_items); m_items.clear(); }

  void add(KDevItem *item)
  {
    Q_ASSERT(item != 0);
    Q_ASSERT(item->parent() == this || item->parent() == 0);

    if (item->parent() != this)
      item->setParent(this);

    m_items.append(item);
  }

  void remove(int index) { m_items.removeAt(index); }

private:
  QString m_name;
  QList<KDevItem *> m_items;
};

class KDevItemModel: public QAbstractItemModel
{
  Q_OBJECT
public:
  KDevItemModel(QObject *parent = 0);
  virtual ~KDevItemModel();

  KDevItemCollection *collection() const;

  void appendItem(KDevItem *item, KDevItemCollection *collection);
  void removeItem(KDevItem *item);

  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  virtual QModelIndex parent(const QModelIndex &index) const;

  virtual int rowCount(const QModelIndex &parent) const;
  virtual int columnCount(const QModelIndex &parent) const;

  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

  KDevItem *item(const QModelIndex &index) const;
  virtual QModelIndex indexOf(KDevItem *item) const;

public slots:
  void refresh();

protected:
  int positionOf(KDevItem *item) const;

private:
  KDevItemCollection m_collection;
};

#endif // KDEVITEMMODEL_H
