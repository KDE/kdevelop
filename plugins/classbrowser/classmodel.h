/*
 * KDevelop Class Browser
 *
 * Copyright 2007-2008 Hamish Rodda <rodda@kde.org>
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

#include "language/duchain/duchainbase.h"
#include "language/duchain/duchainobserver.h"
#include "language/duchain/identifier.h"
#include "language/duchain/duchainpointer.h"
#include "language/duchain/codemodel.h"

class ClassBrowserPlugin;

namespace KDevelop {
 class TopDUContext;
 class IDocument;
 class ParseJob;
 class DUContext;
 class IProject;
}

/**
 * \short An item model which represents all of the declarations loaded by %KDevelop.
 *
 * The class model is complex for several reasons:
 * 1) It has to be able to display all declarations, even if the definition-use chains
 *    for those declarations are not currently loaded.  This is accomplished by using
 *    the KDevelop::CodeModel.
 * 2) It has to update itself whenever the definition-use chain changes.  Thus, it waits
 *    for notifications from KDevelop::DUChainNotifier, looks up the nodes which
 *    correspond to the changed duchain items, and modifies the node tree.
 * 3) Two separate code paths are required to implement the above two approaches to populating
 *    the class model.
 * 4) It does all this in a lazy way, to maximize performance.
 * 5) It provides some useful extras, such as sorting, searching for declarations, and filtering.
 */
class ClassModel : public QAbstractItemModel
{
  Q_OBJECT
  friend class Node;

public:
  class Node;

  ClassModel(ClassBrowserPlugin* parent);
  virtual ~ClassModel();

  Node* objectForIndex(const QModelIndex& index) const;
  KDevelop::DUChainBasePointer duObjectForIndex(const QModelIndex& index) const;

  QModelIndex indexForObject(Node* node) const;

  Node* objectForIdentifier(const KDevelop::QualifiedIdentifier& identifier) const;
  Node* objectForIdentifier(const KDevelop::IndexedQualifiedIdentifier& identifier) const;

  KDevelop::Declaration* declarationForObject(const KDevelop::DUChainBasePointer& pointer) const;
  KDevelop::Declaration* definitionForObject(const KDevelop::DUChainBasePointer& pointer) const;

  void setFilterByProject(bool filterByProject);

public Q_SLOTS:
  void setFilterDocument(KDevelop::IDocument* document);
  void searchStringChanged(const QString& string);

public:
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex & index) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

private Q_SLOTS:
  // Definition use chain observer implementation
  void branchAdded(KDevelop::DUContextPointer context);
  void branchModified(KDevelop::DUContextPointer context);
  void branchRemoved(KDevelop::DUContextPointer context, KDevelop::DUContextPointer parent);

  // Project watching
  void projectOpened(KDevelop::IProject* project);
  void projectClosing(KDevelop::IProject* project);

  // File watching
  void documentOpened(KDevelop::IDocument* doc);
  void documentClosed(KDevelop::IDocument* doc);

private:
  ClassBrowserPlugin* plugin() const;

  void resetModel();

  static QVariant data(Node* node, int role = Qt::DisplayRole);

  void initialize();

  void startLoading();
  void finishLoading();

  void referenceFile(const KDevelop::IndexedString& file);
  void dereferenceFile(const KDevelop::IndexedString& file);

  // General refresh method
  void refreshNode(Node* const node, KDevelop::DUContext* context = 0, QList<Node*>* resultChildren = 0) const;
  // Code model refresh method, defaults to top level
  void refreshNodes(const KDevelop::IndexedString& file, int level = 1, const KDevelop::QualifiedIdentifier& from = KDevelop::QualifiedIdentifier()) const;
  static void getDuObject(Node* node);

  Node* discover(Node* node) const;
  KDevelop::DUContext* trueParent(KDevelop::DUContext* parent) const;

  void branchChanged(KDevelop::DUContext* context);

  // Code model item constructor
  Node* createPointer(const KDevelop::QualifiedIdentifier& id, Node* parent) const;
  Node* createPointer(KDevelop::Declaration* object, Node* parent) const;
  Node* createPointer(KDevelop::DUContext* context, Node* parent) const;

  void nodeDeleted(Node* node);

  // returns true if object should not be displayed
  bool filterObject(KDevelop::DUChainBase* object) const;

  static bool orderItems(ClassModel::Node* p1, ClassModel::Node* p2);

  mutable Node* m_topNode;
  Node* m_globalFunctions;
  Node* m_globalVariables;
  mutable QHash<KDevelop::IndexedQualifiedIdentifier, Node*> m_objects;
  QHash<KDevelop::IndexedString, int> m_displayedFiles;

  KDevelop::IDocument* m_filterDocument;
  bool m_filterProject;
  bool m_loading;
  QString m_searchString;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

