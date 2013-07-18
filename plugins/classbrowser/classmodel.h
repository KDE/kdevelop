/*
 * KDevelop Class Browser
 *
 * Copyright 2007-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
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

#ifndef KDEVPLATFORM_PLUGIN_CLASSMODEL_H
#define KDEVPLATFORM_PLUGIN_CLASSMODEL_H

#include <QtCore/QAbstractItemModel>
#include "classmodelnode.h"

class ClassBrowserPlugin;

namespace KDevelop
{
  class TopDUContext;
  class IDocument;
  class ParseJob;
  class DUContext;
  class IProject;
  class DUChainBase;
  class IndexedQualifiedIdentifier;
}

namespace ClassModelNodes
{
  class Node;
  class FilteredAllClassesFolder;
  class FilteredProjectFolder;
  class FolderNode;
  class IdentifierNode;
}

/// The model interface accessible from the nodes.
class NodesModelInterface
{
public:
  virtual ~NodesModelInterface();

public:
  virtual void nodesLayoutAboutToBeChanged(ClassModelNodes::Node* a_parent) = 0;
  virtual void nodesLayoutChanged(ClassModelNodes::Node* a_parent) = 0;
  virtual void nodesRemoved(ClassModelNodes::Node* a_parent, int a_first, int a_last) = 0;
  virtual void nodesAboutToBeAdded(ClassModelNodes::Node* a_parent, int a_pos, int a_size) = 0;
  virtual void nodesAdded(ClassModelNodes::Node* a_parent) = 0;
};

/**
 * @short A model that holds a convinient representation of the defined class in the project
 * 
 * This model doesn't have much code in it, it mostly acts as a glue between the different
 * nodes and the tree view.
 *
 * The nodes are defined in the namespace @ref ClassModelNodes
 */
class ClassModel : public QAbstractItemModel, public NodesModelInterface
{
  Q_OBJECT
public:
  ClassModel();
  virtual ~ClassModel();

public:
  /// Retrieve the DU object related to the specified index.
  /// @note DUCHAINS READER LOCK MUST BE TAKEN!
  KDevelop::DUChainBase* duObjectForIndex(const QModelIndex& a_index);

  /// Call this to retrieve the index for the node associated with the specified id.
  QModelIndex getIndexForIdentifier(const KDevelop::IndexedQualifiedIdentifier& a_id);

  /// Return the model index associated with the given node.
  QModelIndex index(ClassModelNodes::Node* a_node) const;

public Q_SLOTS:
  /// Call this to update the filter string for the search results folder.
  void updateFilterString(QString a_newFilterString);

  /// removes the project-specific node
  void removeProjectNode(KDevelop::IProject* project);
  /// adds the project-specific node
  void addProjectNode(KDevelop::IProject* project);

private: // NodesModelInterface overrides
  virtual void nodesLayoutAboutToBeChanged(ClassModelNodes::Node* a_parent);
  virtual void nodesLayoutChanged(ClassModelNodes::Node* a_parent);
  virtual void nodesRemoved(ClassModelNodes::Node* a_parent, int a_first, int a_last);
  virtual void nodesAboutToBeAdded(ClassModelNodes::Node* a_parent, int a_pos, int a_size);
  virtual void nodesAdded(ClassModelNodes::Node* a_parent);

private:
  /// Main level node - it's usually invisible.
  ClassModelNodes::Node* m_topNode;
  ClassModelNodes::FilteredAllClassesFolder* m_allClassesNode;
  QMap<KDevelop::IProject*, ClassModelNodes::FilteredProjectFolder*> m_projectNodes;

public Q_SLOTS:
  /// This slot needs to be attached to collapsed signal in the tree view.
  void collapsed(const QModelIndex& index);
  /// This slot needs to be attached to expanded signal in the tree view.
  void expanded(const QModelIndex& index);

public: // QAbstractItemModel overrides
  virtual QFlags< Qt::ItemFlag > flags(const QModelIndex&) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;

  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
};


#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on

