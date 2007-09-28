/*
 * KDevelop DUChain viewer
 *
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>

#include <ktexteditor/cursor.h>
#include <kurl.h>

#include "duchainbase.h"
#include "duchainobserver.h"

class DUChainViewPart;

namespace KDevelop {
 class TopDUContext;
 class IDocument;
 class ParseJob;
}

class ProxyObject : public KDevelop::DUChainBase
{
public:
  ProxyObject(KDevelop::DUChainBase* _parent, KDevelop::DUChainBase* _object);

  KDevelop::DUChainBase* parent;
  KDevelop::DUChainBase* object;
};

class DUChainModel : public QAbstractItemModel, public KDevelop::DUChainObserver
{
  Q_OBJECT

public:
  DUChainModel(DUChainViewPart* parent);
  virtual ~DUChainModel();

  void setTopContext(KDevelop::TopDUContext* context);

public Q_SLOTS:
  void documentActivated(KDevelop::IDocument* document);
  void parseJobFinished(KDevelop::ParseJob* job);

public:
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex & index) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
  //virtual bool hasChildren(const QModelIndex & parent = QModelIndex()) const;

  // Definition use chain observer implementation
  virtual void contextChanged(KDevelop::DUContext* context, Modification change, Relationship relationship, KDevelop::DUChainBase* relatedObject = 0);
  virtual void declarationChanged(KDevelop::Declaration* declaration, Modification change, Relationship relationship, KDevelop::DUChainBase* relatedObject = 0);
  virtual void definitionChanged(KDevelop::Definition* definition, Modification change, Relationship relationship, KDevelop::DUChainBase* relatedObject = 0);
  virtual void useChanged(KDevelop::Use* use, Modification change, Relationship relationship, KDevelop::DUChainBase* relatedObject = 0);

private slots:
  void doubleClicked ( const QModelIndex & index );
    
private:
  DUChainViewPart* part() const;
  
  KDevelop::DUChainBase* objectForIndex(const QModelIndex& index) const;
  int findInsertIndex(QList<KDevelop::DUChainBase*>& list, KDevelop::DUChainBase* object) const;

  template <typename T>
  QModelIndex createParentIndex(T* type) const
  {
    return createIndex(m_modelRow[type], 0, type);
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
  KDevelop::DUChainBase* item(QListIterator<T*>& it) const
  {
    Q_ASSERT(it.hasPrevious());
    KDevelop::DUChainBase* item = it.peekPrevious();
    if (it.hasNext())
      it.next();
    else
      // Make hasPrevious return false
      it.toFront();

    return item;
  }

  template <typename T>
  KDevelop::DUChainBase* proxyItem(KDevelop::DUChainBase* parent, QListIterator<T*>& it) const
  {
    KDevelop::DUChainBase* target = item(it);
    KDevelop::DUChainBase* proxy = new ProxyObject(parent, target);
    m_proxyObjects.insert(target, proxy);
    return proxy;
  }

  QList<KDevelop::DUChainBase*>* childItems(KDevelop::DUChainBase* parent) const;

  KDevelop::TopDUContext* m_chain;
  KUrl m_document;
  mutable QMutex m_mutex;
  mutable QHash<KDevelop::DUChainBase*, QList<KDevelop::DUChainBase*>* > m_objectLists;
  mutable QHash<KDevelop::DUChainBase*, int > m_modelRow;
  mutable QHash<KDevelop::DUChainBase*, KDevelop::DUChainBase*> m_proxyObjects;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
