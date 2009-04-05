/*
 * KDevelop Class Browser
 *
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

#include "allclassesfolder.h"

#include <typeinfo>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/codemodel.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <QTimer>

using namespace KDevelop;
using namespace ClassModelNodes;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Contains a static list of classes within the namespace.
class ClassModelNodes::StaticNamespaceFolderNode : public Node
{
public:
  StaticNamespaceFolderNode(const KDevelop::QualifiedIdentifier& a_identifier, NodesModelInterface* a_model);

  /// Returns the qualified identifier for this node
  const KDevelop::QualifiedIdentifier& qualifiedIdentifier() const { return m_identifier; }

public: // Node overrides
  virtual bool getIcon(QIcon& a_resultIcon);
  virtual int getScore() const { return 1; }

private:
  /// The namespace identifier.
  KDevelop::QualifiedIdentifier m_identifier;
};

StaticNamespaceFolderNode::StaticNamespaceFolderNode(const KDevelop::QualifiedIdentifier& a_identifier, NodesModelInterface* a_model)
  : Node(a_identifier.last().toString(), a_model)
  , m_identifier(a_identifier)
{

}

bool StaticNamespaceFolderNode::getIcon(QIcon& a_resultIcon)
{
  static KIcon folderIcon("namespace");
  a_resultIcon = folderIcon;
  return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

DocumentClassesFolder::DocumentClassesFolder(const QString& a_displayName, NodesModelInterface* a_model)
  : DynamicFolderNode(a_displayName, a_model)
  , m_updateTimer( new QTimer(this) )
{
  connect( m_updateTimer, SIGNAL(timeout()), this, SLOT(updateChangedFiles()));
}

void DocumentClassesFolder::branchModified(KDevelop::DUContextPointer context)
{
  DUChainReadLocker readLock(DUChain::lock());

  if ( !context )
    return;

  // Queue the changed file.
  m_updatedFiles.insert(context->url());
}

void DocumentClassesFolder::branchRemoved(KDevelop::DUContextPointer context)
{
  DUChainReadLocker readLock(DUChain::lock());

  if ( !context )
    return;

  /// @todo
  kDebug() << "Del: " << context->url().toUrl() << " " << context->range().start.line << "-" << context->range().end.line;
}

void DocumentClassesFolder::updateChangedFiles()
{
  bool hadChanges = false;

  // re-parse changed documents.
  foreach( const IndexedString& file, m_updatedFiles )
  {
    // Make sure it's one of the monitored files.
    if ( m_openFiles.contains(file) )
      hadChanges |= updateDocument(file);
  }

  // Processed all files.
  m_updatedFiles.clear();

  // Sort if had changes.
  if ( hadChanges )
    recursiveSort();
}

void DocumentClassesFolder::nodeCleared()
{
  disconnect(DUChain::self()->notifier(), SIGNAL(branchModified(KDevelop::DUContextPointer)), this, SLOT(branchModified(KDevelop::DUContextPointer)));
  disconnect(DUChain::self()->notifier(), SIGNAL(branchRemoved(KDevelop::DUContextPointer)), this, SLOT(branchRemoved(KDevelop::DUContextPointer)));

  // Clear cached namespaces list (node was cleared).
  m_namespaces.clear();

  // Clear open files list
  m_openFiles.clear();

  // Stop the update timer.
  m_updateTimer->stop();
}

void DocumentClassesFolder::populateNode()
{
  // Get notification for file changes.
  connect(DUChain::self()->notifier(), SIGNAL(branchModified(KDevelop::DUContextPointer)), this, SLOT(branchModified(KDevelop::DUContextPointer)), Qt::QueuedConnection);
  connect(DUChain::self()->notifier(), SIGNAL(branchRemoved(KDevelop::DUContextPointer)), this, SLOT(branchRemoved(KDevelop::DUContextPointer)), Qt::QueuedConnection);

  // Start updates timer - this is the required delay.
  m_updateTimer->start(2000);
}

QList< KDevelop::IndexedString > DocumentClassesFolder::getAllOpenDocuments()
{
  return m_openFiles.keys();
}

ClassNode* DocumentClassesFolder::findClassNode(const IndexedQualifiedIdentifier& a_id)
{
  // Look in all the documents.
  foreach(const ClassNodeIDsMap& idsMap, m_openFiles)
  {
    ClassNodeIDsMap::const_iterator iter = idsMap.find(a_id);
    if ( iter !=  idsMap.end() )
      return iter.value();
  }

  // Not found.
  return 0;
}

void DocumentClassesFolder::closeDocument(const IndexedString& a_file)
{
  // Get list of nodes associated with this file and remove them.
  ClassNodeIDsMap values = m_openFiles.value(a_file);
  foreach( ClassNode* node, values )
    removeClassNode(node);

  // Clear the file from the list.
  m_openFiles.remove(a_file);
}

bool DocumentClassesFolder::updateDocument(const KDevelop::IndexedString& a_file)
{
  uint itemCount = 0;
  const CodeModelItem* items;
  CodeModel::self().items(a_file, itemCount, items);

  // List of declared namespaces in this file.
  QSet< QualifiedIdentifier > declaredNamespaces;

  // List of removed classes - it initially contains all the known classes, we'll eliminate them
  // one by one later on.
  ClassNodeIDsMap removedClasses = m_openFiles[a_file];

  bool documentChanged = false;

  for(uint i = 0; i < itemCount; ++i)
  {
    const CodeModelItem& item = items[i];

    // Don't insert unknown or forward declarations into the class browser
    if ( (item.kind & CodeModelItem::Unknown) || (item.kind & CodeModelItem::ForwardDeclaration) )
      continue;

    KDevelop::QualifiedIdentifier id = item.id.identifier();

    // Don't add empty identifiers.
    if ( id.count() == 0 )
      continue;

    // If it's a namespace, create it in the list.
    if ( item.kind & CodeModelItem::Namespace )
    {
      // This should create the namespace folder and add it to the cache.
      getNamespaceFolder(id);

      // Add to the locally created namespaces.
      declaredNamespaces.insert(id);
    }
    else if ( item.kind & CodeModelItem::Class )
    {
      // Ignore empty unnamed classes.
      if ( id.last().toString().isEmpty() )
        continue;
    
      // See if it matches our filter?
      if ( isClassFiltered(id) )
        continue;

      // Make sure this class is new.
      if ( removedClasses.contains(id) )
      {
        removedClasses.remove(id);
        continue;
      }

      // Where should we put this class?
      Node* parentNode = 0;

      // Check if it's namespaced and add it to the proper namespace.
      if ( id.count() > 1 )
      {
        // Look up the namespace in the cache.
        // If we fail to find it we assume that the parent context is a class
        // and in that case, when the parent class gets expanded, it will show it.
        NamespacesMap::iterator iter = m_namespaces.find(id.left(-1));
        if ( iter != m_namespaces.end() )
        {
          // Add to the namespace node.
          parentNode = iter.value();
        }
      }
      else
      {
        // Add to the main root.
        parentNode = this;
      }

      if ( parentNode != 0 )
      {
        // Make sure it's a new ID.
        ClassNode* newNode = new ClassNode(id, m_model);
        parentNode->addNode( newNode );

        // We have this ID - remove it.
        m_openFiles[a_file].insert(id, newNode);
        documentChanged = true;
      }
    }
  }

  // Remove empty namespaces from the list.
  // We need this because when a file gets unloaded, we unload the declared classes in it
  // and if a namespace has no class in it, it'll forever exist and no one will remove it
  // from the children list.
  foreach( QualifiedIdentifier id, declaredNamespaces )
    removeEmptyNamespace(id);

  // Remove erased classes.
  for ( ClassNodeIDsMap::iterator iter = removedClasses.begin();
        iter != removedClasses.end();
        ++iter )
  {
    removeClassNode(iter.value());
    m_openFiles[a_file].remove(iter.key());
    documentChanged = true;
  }

  return documentChanged;
}

void DocumentClassesFolder::parseDocument(const IndexedString& a_file)
{
  // Add the document to the list of open files - this means we monitor it.
  if ( !m_openFiles.contains(a_file) )
    m_openFiles.insert(a_file, ClassNodeIDsMap() );

  updateDocument(a_file);
}

void DocumentClassesFolder::removeClassNode(ClassModelNodes::ClassNode* a_node)
{
  // Get the parent namespace identifier.
  QualifiedIdentifier parentNamespaceIdentifier;
  if ( typeid(*a_node->getParent()) == typeid(StaticNamespaceFolderNode) )
  {
    parentNamespaceIdentifier =
      static_cast<StaticNamespaceFolderNode*>(a_node->getParent())->qualifiedIdentifier();
  }

  // Remove the node.
  a_node->removeSelf();

  // Remove empty namespace
  removeEmptyNamespace(parentNamespaceIdentifier);
}

void DocumentClassesFolder::removeEmptyNamespace(const QualifiedIdentifier& a_identifier)
{
  // Stop condition.
  if ( a_identifier.count() == 0 )
    return;

  // Look it up in the cache.
  NamespacesMap::iterator iter = m_namespaces.find(a_identifier);
  if ( iter != m_namespaces.end() )
  {
    if ( !(*iter)->hasChildren() )
    {
      // Remove this node and try to remove the parent node.
      QualifiedIdentifier parentIdentifier = (*iter)->qualifiedIdentifier().left(-1);
      (*iter)->removeSelf();
      m_namespaces.remove(a_identifier);
      removeEmptyNamespace(parentIdentifier);
    }
  }
}

StaticNamespaceFolderNode* DocumentClassesFolder::getNamespaceFolder(const KDevelop::QualifiedIdentifier& a_identifier)
{
  // Stop condition.
  if ( a_identifier.count() == 0 )
    return 0;

  // Look it up in the cache.
  NamespacesMap::iterator iter = m_namespaces.find(a_identifier);
  if ( iter == m_namespaces.end() )
  {
    // It's not in the cache - create folders up to it.
    Node* parentNode = getNamespaceFolder(a_identifier.left(-1));
    if ( parentNode == 0 )
      parentNode = this;

    // Create the new node.
    StaticNamespaceFolderNode* newNode =
      new StaticNamespaceFolderNode(a_identifier, m_model);
    parentNode->addNode( newNode );

    // Add it to the cache.
    m_namespaces.insert( a_identifier, newNode );

    // Return the result.
    return newNode;
  }
  else
    return *iter;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

AllClassesFolder::AllClassesFolder(NodesModelInterface* a_model)
  : DocumentClassesFolder("All projects classes", a_model)
{
}

void AllClassesFolder::nodeCleared()
{
  DocumentClassesFolder::nodeCleared();
  
  disconnect(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), this, SLOT(projectOpened(KDevelop::IProject*)));
  disconnect(ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)), this, SLOT(projectClosing(KDevelop::IProject*)));
}

void AllClassesFolder::populateNode()
{
  DocumentClassesFolder::populateNode();

  // Get notification for future project addition / removal.
  connect (ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), this, SLOT(projectOpened(KDevelop::IProject*)));
  connect (ICore::self()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)), this, SLOT(projectClosing(KDevelop::IProject*)));

  // Parse each existing project file
  foreach(IProject* project, ICore::self()->projectController()->projects())
  {
    // Run over all the files in the project.
    foreach(const IndexedString& file, project->fileSet())
      parseDocument(file);
  }
}

void AllClassesFolder::projectClosing(KDevelop::IProject* project)
{
  // Run over all the files in the project.
  foreach(const IndexedString& file, project->fileSet())
    closeDocument(file);
}

void AllClassesFolder::projectOpened(KDevelop::IProject* project)
{
  // Run over all the files in the project.
  foreach(const IndexedString& file, project->fileSet())
    parseDocument(file);

  recursiveSort();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FilteredAllClassesFolder::FilteredAllClassesFolder(NodesModelInterface* a_model)
  : AllClassesFolder(a_model)
{
  // Initial update (mostly to update the display name)
  updateFilterString(m_filterString);
}

void FilteredAllClassesFolder::updateFilterString(QString a_newFilterString)
{
  m_filterString = a_newFilterString;
  if ( m_filterString.isEmpty() )
    m_displayName = "All projects classes";
  else
    m_displayName = "Filtered classes for '" + m_filterString + "'";

#if 1 // Choose speed over correctness.
  // Close the node and re-open it should be quicker than reload each document
  // and remove indevidual nodes (at the cost of loosing the current selection).
  collapse();
  expand();
#else
  bool hadChanges = false;
  
  // Reload the documents.
  foreach( const IndexedString& file, getAllOpenDocuments() )
    hadChanges |= updateDocument(file);
    
  // Sort if we've updated documents. If nothing changed, the title changed so
  // mark the node as updated.
  if ( hadChanges )
    recursiveSort();
  else
  {
    m_model->nodesLayoutAboutToBeChanged(this);
    m_model->nodesLayoutChanged(this);
  }
#endif
}

bool FilteredAllClassesFolder::isClassFiltered(const KDevelop::QualifiedIdentifier& a_id)
{
  return !a_id.last().toString().contains(m_filterString, Qt::CaseInsensitive);
}


#include "allclassesfolder.moc"


// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
