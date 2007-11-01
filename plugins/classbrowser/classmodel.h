/*
 * KDevelop Class Browser
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef CLASSMODEL_H
#define CLASSMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>

#include <ktexteditor/cursor.h>
#include <kurl.h>

#include "duchainbase.h"
#include "duchainobserver.h"

class ClassBrowserPart;

namespace KDevelop {
 class TopDUContext;
 class IDocument;
 class ParseJob;
 class DUContext;
}

class ClassModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  ClassModel(ClassBrowserPart* parent);
  virtual ~ClassModel();

public:
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex & index) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

private Q_SLOTS:
  // Definition use chain observer implementation
  /*void contextChanged(KDevelop::DUContextPointer context, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);
  void declarationChanged(KDevelop::DeclarationPointer declaration, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);
  void definitionChanged(KDevelop::DefinitionPointer definition, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);
  void useChanged(KDevelop::UsePointer use, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);*/
  void branchAdded(KDevelop::DUContextPointer context);

private:
  ClassBrowserPart* part() const;

  void contextAdded(KDevelop::DUContext* context);

  KDevelop::DUChainBasePointer* objectForIndex(const QModelIndex& index) const;
  QModelIndex createIndex(int row, int column, KDevelop::DUChainBase* object) const;
  QModelIndex createIndex(int row, int column, KDevelop::DUChainBasePointer* object) const;
  QModelIndex contextIndex(KDevelop::DUContext* context) const;

  template <class T>
  KDevelop::DUChainBasePointer* createPointer(T* object) const
  {
    if (m_knownObjects.contains(object))
      return m_knownObjects[object];

    KDevelop::DUChainBasePointer* p = new KDevelop::DUChainBasePointer(object);
    m_knownObjects.insert(object, p);

    return p;
  }

  QList<KDevelop::DUChainBasePointer*> m_topObjects;
  mutable QHash<KDevelop::DUChainBase*, KDevelop::DUChainBasePointer*> m_knownObjects;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
