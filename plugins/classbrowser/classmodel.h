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

#include <duchainbase.h>
#include <duchainobserver.h>
#include <identifier.h>
#include <duchainpointer.h>

class ClassBrowserPlugin;

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
  ClassModel(ClassBrowserPlugin* parent);
  virtual ~ClassModel();

  class Node : public KDevelop::DUChainBasePointer
  {
    public:
      Node(KDevelop::DUChainBase* p, Node* parent) : KDevelop::DUChainBasePointer(p), m_parent(parent) {}

      Node* parent() const { return m_parent; }

      const QList<KDevelop::DUContextPointer>& namespaceContexts() const { return m_namespaceContexts; }
      void addNamespaceContext(const KDevelop::DUContextPointer& context) { m_namespaceContexts.append(context); }

    private:
      Node* m_parent;
      QList<KDevelop::DUContextPointer> m_namespaceContexts;
  };

  Node* objectForIndex(const QModelIndex& index) const;

  KDevelop::Declaration* declarationForObject(const KDevelop::DUChainBasePointer& pointer) const;
  KDevelop::Declaration* definitionForObject(const KDevelop::DUChainBasePointer& pointer) const;

  void setFilterByProject(bool filterByProject);

public Q_SLOTS:
  void setFilterDocument(KDevelop::IDocument* document);

public:
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex & index) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  //virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

private Q_SLOTS:
  // Definition use chain observer implementation
  void branchAdded(KDevelop::DUContextPointer context);
  void branchRemoved(KDevelop::DUContextPointer context, KDevelop::DUContextPointer parent);

private:
  ClassBrowserPlugin* plugin() const;

  void resetModel();

  static QVariant data(Node* node, int role = Qt::DisplayRole);

  void contextAdded(Node* parent, KDevelop::DUContext* context);
  void contextRemoved(Node* parent, KDevelop::DUContext* context);

  void addTopLevelToList(KDevelop::DUContext* context, QList<Node*>* list, Node* parent, bool first = true) const;

  QList<Node*>* childItems(Node* parent) const;
  KDevelop::DUContext* trueParent(KDevelop::DUContext* parent) const;

  Node* pointer(KDevelop::DUChainBase* object) const;
  Node* createPointer(KDevelop::DUChainBase* object, Node* parent) const;
  Node* createPointer(KDevelop::DUContext* context, Node* parent) const;

  // returns true if object should not be displayed
  bool filterObject(KDevelop::DUChainBase* object) const;

  static bool orderItems(ClassModel::Node* p1, ClassModel::Node* p2);

  mutable QList<Node*>* m_topList;
  mutable QHash<KDevelop::DUChainBase*, Node*> m_knownObjects;
  mutable QHash<Node*, QList<Node*>* > m_lists;
  mutable QHash<KDevelop::QualifiedIdentifier, Node*> m_namespaces;
  mutable QMap<KUrl, bool> m_inProject;

  KDevelop::IDocument* m_filterDocument;
  bool m_filterProject;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
