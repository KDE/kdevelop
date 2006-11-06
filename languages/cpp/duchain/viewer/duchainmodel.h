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
#include <QHash>
#include <QSet>

#include <ktexteditor/cursor.h>

#include "duchainbase.h"
#include "duchainobserver.h"

class TopDUContext;
class DUChainViewPart;
class KDevDocument;

class ProxyObject : public DUChainBase
{
public:
  ProxyObject(DUChainBase* _parent, DUChainBase* _object);

  DUChainBase* parent;
  DUChainBase* object;
};

class DUChainModel : public QAbstractItemModel, public DUChainObserver
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
    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;

    // Definition use chain observer implementation
    virtual void contextChanged(DUContext* context, Modification change, Relationship relationship, DUChainBase* relatedObject = 0);
    virtual void declarationChanged(Declaration* declaration, Modification change, Relationship relationship, DUChainBase* relatedObject = 0);
    virtual void definitionChanged(Definition* definition, Modification change, Relationship relationship, DUChainBase* relatedObject = 0);
    virtual void useChanged(Use* use, Modification change, Relationship relationship, DUChainBase* relatedObject = 0);

  private:
    DUChainBase* objectForIndex(const QModelIndex& index) const;
    int findInsertIndex(QList<DUChainBase*>& list, DUChainBase* object) const;

    template <typename T>
    QModelIndex createParentIndex(T* type) const
    {
      return createIndex(type->modelRow, 0, type);
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
    DUChainBase* item(QListIterator<T*>& it) const
    {
      Q_ASSERT(it.hasPrevious());
      DUChainBase* item = it.peekPrevious();
      if (it.hasNext())
        it.next();
      else
        // Make hasPrevious return false
        it.toFront();

      return item;
    }

    template <typename T>
    DUChainBase* proxyItem(DUChainBase* parent, QListIterator<T*>& it) const
    {
      DUChainBase* target = item(it);
      DUChainBase* proxy = new ProxyObject(parent, target);
      m_proxyObjects.insert(target, proxy);
      return proxy;
    }

    QList<DUChainBase*>* childItems(DUChainBase* parent) const;

    TopDUContext* m_chain;
    mutable QMutex m_mutex;
    mutable QHash<DUChainBase*, QList<DUChainBase*>* > m_objectLists;
    mutable QHash<DUChainBase*, DUChainBase*> m_proxyObjects;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on
