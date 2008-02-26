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

class DUChainViewPlugin;

namespace KDevelop {
 class TopDUContext;
 class IDocument;
 class ParseJob;
}

class ProxyObject : public KDevelop::DUChainBase
{
public:
  ProxyObject(KDevelop::DUChainBase* _parent, KDevelop::DUChainBase* _object);

  virtual void preDelete() {};
  
  KDevelop::DUChainBase* parent;
  KDevelop::DUChainBase* object;
};

class DUChainModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  DUChainModel(DUChainViewPlugin* parent);
  virtual ~DUChainModel();

  void setTopContext(KDevelop::TopDUContextPointer context);

  KDevelop::DUChainBasePointer* objectForIndex(const QModelIndex& index) const;

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

private slots:
  void doubleClicked ( const QModelIndex & index );

  // Watch definition use chain changes
  void branchAdded(KDevelop::DUContextPointer context);

private:
  DUChainViewPlugin* plugin() const;

  int findInsertIndex(QList<KDevelop::DUChainBasePointer*>& list, KDevelop::DUChainBase* object) const;

  QModelIndex createParentIndex(KDevelop::DUChainBasePointer* type) const;

  template <typename T>
  KTextEditor::Cursor nextItem(QVectorIterator<T*>& it, bool initialise) const
  {
    if (initialise)
      if (it.hasNext())
        it.next();

    if (it.hasPrevious())
      return it.peekPrevious()->range().textRange().start();

    return KTextEditor::Cursor::invalid();
  }

  template <typename T>
  KDevelop::DUChainBase* item(QVectorIterator<T*>& it) const
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
  KDevelop::DUChainBase* proxyItem(KDevelop::DUChainBase* parent, QVectorIterator<T*>& it) const
  {
    KDevelop::DUChainBase* target = item(it);
    KDevelop::DUChainBase* proxy = new ProxyObject(parent, target);
    m_proxyObjects.insert(target, createPointerForObject(proxy));
    return proxy;
  }

  QList<KDevelop::DUChainBasePointer*>* childItems(KDevelop::DUChainBasePointer* parent) const;
  KDevelop::DUChainBasePointer* pointerForObject(KDevelop::DUChainBase* object) const;
  KDevelop::DUChainBasePointer* createPointerForObject(KDevelop::DUChainBase* object) const;

  KDevelop::TopDUContextPointer m_chain;
  KUrl m_document;
  mutable QHash<KDevelop::DUChainBase*, KDevelop::DUChainBasePointer*> m_knownObjects;
  mutable QHash<KDevelop::DUChainBasePointer*, QList<KDevelop::DUChainBasePointer*>* > m_objectLists;
  mutable QHash<KDevelop::DUChainBasePointer*, int > m_modelRow;
  mutable QHash<KDevelop::DUChainBase*, KDevelop::DUChainBasePointer*> m_proxyObjects;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
