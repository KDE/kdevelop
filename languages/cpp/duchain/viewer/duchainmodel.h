/*
 * KDevelop DUChain viewer
 *
 * Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
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

#ifndef DUCHAINMODEL_H
#define DUCHAINMODEL_H

#include <QAbstractItemModel>
#include <QCache>
#include <QSet>

#include <ktexteditor/cursor.h>

#include "duchainmodelbase.h"

class TopDUContext;
class DUChainViewPart;
class KDevDocument;

class DUChainModel : public QAbstractItemModel
{
  Q_OBJECT

  public:
    DUChainModel(DUChainViewPart* parent);
    virtual ~DUChainModel();

    void setTopContext(TopDUContext* context);

  public slots:
    void documentActivated( KDevDocument* document );

  public:
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

  private:
    class ProxyObject : public DUChainModelBase
    {
    public:
      ProxyObject(DUChainModelBase* _parent, DUChainModelBase* _object);

      DUChainModelBase* parent;
      DUChainModelBase* object;
    };


    template <typename T>
    QModelIndex createParentIndex(T* type) const
    {
      return createIndex(type->modelRow, 0, type);
    }

    template <typename T>
    DUChainModelBase* nextItem(QListIterator<T*>& it) const
    {
      if (it.hasPrevious())
        return it.peekPrevious();
      else
        return 0L;
    }

    template <typename T>
    KTextEditor::Cursor nextItem(QListIterator<T*>& it, bool initialise) const
    {
      if (initialise)
        if (it.hasNext())
          it.next();

      if (it.hasPrevious())
        return it.peekPrevious()->textRange().start();

      return KTextEditor::Cursor::invalid();
    }

    template <typename T>
    DUChainModelBase* item(QListIterator<T*>& it) const
    {
      Q_ASSERT(it.hasPrevious());
      DUChainModelBase* item = it.peekPrevious();
      if (it.hasNext())
        it.next();
      else
        // Make hasPrevious return false
        it.toFront();

      return item;
    }

    template <typename T>
    DUChainModelBase* proxyItem(DUChainModelBase* parent, QListIterator<T*> it) const
    {
      Q_ASSERT(it.hasPrevious());

      DUChainModelBase* item = new ProxyObject(parent, it.peekPrevious());
      m_proxyObjects.insert(item);
      if (it.hasNext())
        it.next();
      else
        // Make hasPrevious return false
        it.toFront();

      return item;
    }

    QList<DUChainModelBase*>* childItems(DUChainModelBase* parent) const;

    TopDUContext* m_chain;
    mutable QCache<DUChainModelBase*, QList<DUChainModelBase*> > m_objectCache;
    mutable QSet<DUChainModelBase*> m_proxyObjects;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on
