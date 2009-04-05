/*
 * KDevelop Class Browser
 *
 * Copyright 2007-2009 Hamish Rodda <rodda@kde.org>
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

#ifndef CLASSMODELNODE_H
#define CLASSMODELNODE_H

#include "classmodel.h"
#include <qicon.h>
#include <language/duchain/identifier.h>
#include <language/duchain/duchainpointer.h>
#include "classmodelnodescontroller.h"

class QTimer;
class NodesModelInterface;

namespace KDevelop
{
  class ClassDeclaration;
  class ClassFunctionDeclaration;
  class ClassMemberDeclaration;
  class Declaration;
}

namespace ClassModelNodes
{

/// Base node class - provides basic functionality.
class Node
{
public:
  Node(const QString& a_displayName, NodesModelInterface* a_model);
  virtual ~Node();

public: // Operations
  /// Clear all the children from the node.
  void clear();

  /// Called by the model to collapse the node and remove sub-items if needed.
  virtual void collapse() {};

  /// Called by the model to expand the node and populate it with sub-nodes if needed.
  virtual void expand() {};

  /// Append a new child node to the list.
  void addNode(Node* a_child);

  /// Remove child node from the list and delete it.
  void removeNode(Node* a_child);

  /// Remove this node and delete it.
  void removeSelf() { m_parentNode->removeNode(this); }

  /// Called once the node has been populated to sort the entire tree / branch.
  void recursiveSort();

public: // Info retrieval
  /// Return the parent associated with this node.
  Node* getParent() const { return m_parentNode; }

  /// Get my index in the parent node
  int row();

  /// Return the display name for the node.
  QString displayName() const { return m_displayName; }

  /// Returns a list of child nodes
  const QList<Node*>& getChildren() const { return m_children; }

public: // overridables
  /// Return a score when sorting the nodes.
  virtual int getScore() const = 0;

  /// Return true if the node contains sub-nodes.
  virtual bool hasChildren() const { return !m_children.empty(); }

  /// We use this string when sorting items.
  virtual QString getSortableString() const { return m_displayName; }

  /// fill a_resultIcon with a display icon for the node.
  /// @param a_resultIcon returned icon.
  /// @return true if result was returned.
  virtual bool getIcon(QIcon& a_resultIcon) = 0;

private:
  Node* m_parentNode;

  /// Called once the node has been populated to sort the entire tree / branch.
  void recursiveSortInternal();

protected:
  typedef QList< Node* > NodesList;
  NodesList m_children;
  QString m_displayName;
  NodesModelInterface* m_model;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Base class for nodes that generate and populate their child nodes dynamically
class DynamicNode : public Node
{
public:
  DynamicNode(const QString& a_displayName, NodesModelInterface* a_model);

public: // Node overrides.
  virtual void collapse();
  virtual void expand();
  virtual bool hasChildren() const;

protected: // overridables
  /// Called by the framework when the node is about to be expanded
  /// it should be populated with sub-nodes if applicable.
  virtual void populateNode() {}

  /// Called after the nodes have been removed.
  /// It's for derived classes to clean cached data.
  virtual void nodeCleared() {}

private:
  bool m_populated;

  /// Populate the node and mark the flag.
  void performPopulateNode();
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Base class for nodes associated with a @ref KDevelop::QualifiedIdentifier
class IdentifierNode : public DynamicNode
{
public:
  IdentifierNode(const KDevelop::QualifiedIdentifier& a_identifier, NodesModelInterface* a_model);
  IdentifierNode(const QString& a_displayName, const KDevelop::QualifiedIdentifier& a_identifier, NodesModelInterface* a_model);
  IdentifierNode(const QString& a_displayName, KDevelop::Declaration* a_decl, NodesModelInterface* a_model);

public:
  /// Lookup a child node by it's identifier.
  /// @return 0 if none was found for the identifier or the node pointer.
  static IdentifierNode* findNode(Node* a_parentNode, const KDevelop::IndexedQualifiedIdentifier& a_identifier);

  /// Returns the qualified identifier for this node by going through the tree
  KDevelop::QualifiedIdentifier qualifiedIdentifier() const;

public: // Node overrides
  virtual bool getIcon(QIcon& a_resultIcon);

public: // Overridables
  /// Return the associated declaration
  /// @note DU CHAIN MUST BE LOCKED FOR READ
  virtual KDevelop::Declaration* getDeclaration();

private:
  KDevelop::IndexedQualifiedIdentifier m_identifier;
  QIcon m_cachedIcon;
  KDevelop::DeclarationPointer m_cachedDeclaration;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides display for a single class.
class ClassNode : public IdentifierNode, public ClassModelNodeDocumentChangedInterface
{
public:
  ClassNode(const KDevelop::QualifiedIdentifier& a_identifier, NodesModelInterface* a_model);
  virtual ~ClassNode();

public: // Node overrides
  virtual int getScore() const { return 3; }
  virtual void populateNode();
  virtual void nodeCleared();
  virtual bool hasChildren() const { return true; }

protected: // ClassModelNodeDocumentChangedInterface overrides
  virtual void documentChanged(const KDevelop::IndexedString& a_file);

private:
  typedef QMap< uint, Node* > SubIdentifiersMap;
  /// Set of known sub-identifiers. It's used for updates check.
  SubIdentifiersMap m_subIdentifiers;

  /// We use this variable to know if we've registered for change notification or not.
  KDevelop::IndexedString m_cachedUrl;

  /// Updates the node to reflect changes in the declaration.
  /// @note DU CHAIN MUST BE LOCKED FOR READ
  /// @return true if something was updated.
  bool updateClassDeclarations();
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides a display for a single class function.
class FunctionNode : public IdentifierNode
{
public:
  FunctionNode(KDevelop::ClassFunctionDeclaration* a_decl, NodesModelInterface* a_model);

public: // Node overrides
  virtual int getScore() const { return 4; }
  virtual QString getSortableString() const { return m_sortableString; }

private:
  QString m_sortableString;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides display for a single class variable.
class VariableNode : public IdentifierNode
{
public:
  VariableNode(KDevelop::ClassMemberDeclaration* a_decl, NodesModelInterface* a_model);

public: // Node overrides
  virtual int getScore() const { return 5; }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides a folder node with a static list of nodes.
class FolderNode : public Node
{
public:
  FolderNode(const QString& a_displayName, NodesModelInterface* a_model);

public: // Node overrides
  virtual bool getIcon(QIcon& a_resultIcon);
  virtual int getScore() const { return 1; }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides a folder node with a dynamic list of nodes.
class DynamicFolderNode : public DynamicNode
{
public:
  DynamicFolderNode(const QString& a_displayName, NodesModelInterface* a_model);

public: // Node overrides
  virtual bool getIcon(QIcon& a_resultIcon);
  virtual int getScore() const { return 1; }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Special folder - the parent is assumed to be a ClassNode.
/// It then displays the base classes for the class it sits in.
class BaseClassesFolderNode : public DynamicFolderNode
{
public:
  BaseClassesFolderNode(NodesModelInterface* a_model);

public: // Node overrides
  virtual void populateNode();
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Special folder - the parent is assumed to be a ClassNode.
/// It then displays list of derived classes from the parent class.
class DerivedClassesFolderNode : public DynamicFolderNode
{
public:
  DerivedClassesFolderNode(NodesModelInterface* a_model);

public: // Node overrides
  virtual void populateNode();
};

} // namespace classModelNodes

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
