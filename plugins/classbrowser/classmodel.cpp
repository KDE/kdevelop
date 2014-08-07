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

#include "classmodel.h"
#include "classmodelnode.h"
#include "allclassesfolder.h"
#include "projectfolder.h"
#include <language/duchain/declaration.h>
#include <typeinfo>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

using namespace KDevelop;
using namespace ClassModelNodes;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

NodesModelInterface::~NodesModelInterface()
{
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ClassModel::ClassModel()
{
  m_topNode = new FolderNode("Top Node", this);

  m_allClassesNode = new FilteredAllClassesFolder(this);
  m_topNode->addNode( m_allClassesNode );

  connect(ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
          this, SLOT(removeProjectNode(KDevelop::IProject*)));
  connect(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
          this, SLOT(addProjectNode(KDevelop::IProject*)));

  foreach ( IProject* project, ICore::self()->projectController()->projects() ) {
    addProjectNode(project);
  }
}

ClassModel::~ClassModel()
{
  delete m_topNode;
}

void ClassModel::updateFilterString(QString a_newFilterString)
{
  m_allClassesNode->updateFilterString(a_newFilterString);
  foreach ( ClassModelNodes::FilteredProjectFolder* folder, m_projectNodes ) {
    folder->updateFilterString(a_newFilterString);
  }
}


void ClassModel::collapsed(const QModelIndex& index)
{
  Node* node = static_cast<Node*>(index.internalPointer());

  node->collapse();
}

void ClassModel::expanded(const QModelIndex& index)
{
  Node* node = static_cast<Node*>(index.internalPointer());

  node->expand();
}

QFlags< Qt::ItemFlag > ClassModel::flags(const QModelIndex&) const
{
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

int ClassModel::rowCount(const QModelIndex& parent) const
{
  Node* node = m_topNode;

  if ( parent.isValid() )
    node = static_cast<Node*>(parent.internalPointer());

  return node->getChildren().size();
}

QVariant ClassModel::data(const QModelIndex& index, int role) const
{
  if ( !index.isValid() )
    return QVariant();

  Node* node = static_cast<Node*>(index.internalPointer());

  if ( role == Qt::DisplayRole )
    return node->displayName();

  if ( role == Qt::DecorationRole )
  {
    QIcon icon = node->getCachedIcon();
    return icon.isNull() ? QVariant() : icon;
  }

  return QVariant();
}
    
QVariant ClassModel::headerData(int, Qt::Orientation, int role) const
{
  if ( role == Qt::DisplayRole )
    return "Class";

  return QVariant();
}

int ClassModel::columnCount(const QModelIndex&) const
{
  return 1;
}

bool ClassModel::hasChildren(const QModelIndex& parent) const
{
  if ( !parent.isValid() )
    return true;

  Node* node = static_cast<Node*>(parent.internalPointer());

  return node->hasChildren();
}

QModelIndex ClassModel::index(int row, int column, const QModelIndex& parent) const
{  
  if (row < 0 || column != 0)
    return QModelIndex();

  Node* node = m_topNode; 
  if ( parent.isValid() )
    node = static_cast<Node*>(parent.internalPointer());

  if ( row >= node->getChildren().size() )
    return QModelIndex();

  return index(node->getChildren()[row]);
}

QModelIndex ClassModel::parent(const QModelIndex& childIndex) const
{
  if ( !childIndex.isValid() )
    return QModelIndex();

  Node* childNode = static_cast<Node*>(childIndex.internalPointer());

  if ( childNode->getParent() == m_topNode )
    return QModelIndex();

  return index( childNode->getParent() );
}

QModelIndex ClassModel::index(ClassModelNodes::Node* a_node) const
{
  if (!a_node) {
    return QModelIndex();
  }

  // If no parent exists, we have an invalid index (root node or not part of a model).
  if ( a_node->getParent() == 0 )
    return QModelIndex();

  return createIndex(a_node->row(), 0, a_node);
}

KDevelop::DUChainBase* ClassModel::duObjectForIndex(const QModelIndex& a_index)
{
  if ( !a_index.isValid() )
    return 0;
    
  Node* node = static_cast<Node*>(a_index.internalPointer());

  if ( IdentifierNode* identifierNode = dynamic_cast<IdentifierNode*>(node) )
    return identifierNode->getDeclaration();

  // Non was found.
  return 0;
}

QModelIndex ClassModel::getIndexForIdentifier(const KDevelop::IndexedQualifiedIdentifier& a_id)
{
  ClassNode* node = m_allClassesNode->findClassNode(a_id);
  if ( node == 0 )
    return QModelIndex();

  return index(node);
}


void ClassModel::nodesLayoutAboutToBeChanged(ClassModelNodes::Node*)
{
  emit layoutAboutToBeChanged();
}

void ClassModel::nodesLayoutChanged(ClassModelNodes::Node*)
{
  QModelIndexList oldIndexList = persistentIndexList();
  QModelIndexList newIndexList;

  foreach(const QModelIndex& oldIndex, oldIndexList)
  {
    Node* node = static_cast<Node*>(oldIndex.internalPointer());
    if ( node )
    {
      // Re-map the index.
      newIndexList << createIndex(node->row(), 0, node);
    }
    else
      newIndexList << oldIndex;
  }

  changePersistentIndexList(oldIndexList, newIndexList);

  emit layoutChanged();
}

void ClassModel::nodesRemoved(ClassModelNodes::Node* a_parent, int a_first, int a_last)
{
  beginRemoveRows(index(a_parent), a_first, a_last);
  endRemoveRows();
}

void ClassModel::nodesAboutToBeAdded(ClassModelNodes::Node* a_parent, int a_pos, int a_size)
{
  beginInsertRows(index(a_parent), a_pos, a_pos + a_size - 1);
}

void ClassModel::nodesAdded(ClassModelNodes::Node*)
{
  endInsertRows();
}

void ClassModel::addProjectNode( IProject* project )
{
  m_projectNodes[project] = new ClassModelNodes::FilteredProjectFolder(this, project);
  nodesLayoutAboutToBeChanged(m_projectNodes[project]);
  m_topNode->addNode(m_projectNodes[project]);
  nodesLayoutChanged(m_projectNodes[project]);
}

void ClassModel::removeProjectNode( IProject* project )
{
  m_topNode->removeNode(m_projectNodes[project]);
  m_projectNodes.remove(project);
}

#include "classmodel.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
