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

#ifndef KDEVPLATFORM_PLUGIN_DOCUMENTCLASSESFOLDER_H
#define KDEVPLATFORM_PLUGIN_DOCUMENTCLASSESFOLDER_H

#include "classmodelnode.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

namespace ClassModelNodes
{

class StaticNamespaceFolderNode;

/// This folder displays all the classes that relate to a list of documents.
class DocumentClassesFolder : public QObject, public DynamicFolderNode
{
  Q_OBJECT
public:
  DocumentClassesFolder(const QString& a_displayName, NodesModelInterface* a_model);

public: // Operations
  /// Find a class node in the lists by its id.
  ClassNode* findClassNode(const KDevelop::IndexedQualifiedIdentifier& a_id);

protected: // Documents list handling.
  /// Parse a single document for classes and add them to the list.
  void parseDocument(const KDevelop::IndexedString& a_file);

  /// Re-parse the given document - remove old declarations and add new declarations.
  bool updateDocument(const KDevelop::IndexedString& a_file);

  /// Close and remove all the nodes related to the specified document.
  void closeDocument(const KDevelop::IndexedString& a_file);

  /// Returns a list of documents we have monitored.
  QSet< KDevelop::IndexedString > getAllOpenDocuments();

protected: // Overridables
  /// Override this to filter the found classes.
  virtual bool isClassFiltered(const KDevelop::QualifiedIdentifier&) { return false; }
  
public: // Node overrides
  virtual void nodeCleared() override;
  virtual void populateNode() override;
  virtual bool hasChildren() const override { return true; }

private Q_SLOTS:
  // Files update.
  void updateChangedFiles();

private: // File updates related.
  /// List of updated files we check this list when update timer expires.
  QSet<KDevelop::IndexedString> m_updatedFiles;

  /// Timer for batch updates.
  QTimer* m_updateTimer;

private: // Opened class identifiers container definition.
  // An opened class item.
  struct OpenedFileClassItem
  {
    OpenedFileClassItem();
    OpenedFileClassItem(const KDevelop::IndexedString& a_file,
                        const KDevelop::IndexedQualifiedIdentifier& a_classIdentifier,
                        ClassNode* a_nodeItem);

    /// The file this class declaration comes from.
    KDevelop::IndexedString file;

    /// The identifier for this class.
    KDevelop::IndexedQualifiedIdentifier classIdentifier;

    /// An existing node item. It maybe 0 - meaning the class node is currently hidden.
    ClassNode* nodeItem;
  };

  // Index definitions.
  struct FileIndex {};
  struct ClassIdentifierIndex {};

  // Member types definitions.
  typedef boost::multi_index::member<
    OpenedFileClassItem,
    KDevelop::IndexedString,
    &OpenedFileClassItem::file> FileMember;
  typedef boost::multi_index::member<
    OpenedFileClassItem,
    KDevelop::IndexedQualifiedIdentifier,
    &OpenedFileClassItem::classIdentifier> ClassIdentifierMember;

  // Container definition.
  typedef boost::multi_index::multi_index_container<
    OpenedFileClassItem,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<FileIndex>,
        FileMember
      >,
      boost::multi_index::ordered_unique<
        boost::multi_index::tag<ClassIdentifierIndex>,
        ClassIdentifierMember
      >
    >
  > OpenFilesContainer;

  // Iterators definition.
  typedef OpenFilesContainer::index_iterator<FileIndex>::type FileIterator;
  typedef OpenFilesContainer::index_iterator<ClassIdentifierIndex>::type ClassIdentifierIterator;

  /// Maps all displayed classes and their referenced files.
  OpenFilesContainer m_openFilesClasses;

  /// Holds a set of open files.
  QSet< KDevelop::IndexedString > m_openFiles;

private:
  typedef QMap< KDevelop::IndexedQualifiedIdentifier, StaticNamespaceFolderNode* > NamespacesMap;
  /// Holds a map between an identifier and a namespace folder we hold.
  NamespacesMap m_namespaces;

  /// Recursively create a namespace folder for the specified identifier if it doesn't
  /// exist, cache it and return it (or just return it from the cache).
  StaticNamespaceFolderNode* getNamespaceFolder(const KDevelop::QualifiedIdentifier& a_identifier);

  /// Removes the given namespace identifier recursively if it's empty.
  void removeEmptyNamespace(const KDevelop::QualifiedIdentifier& a_identifier);

  /// Remove a single class node from the lists.
  void removeClassNode(ClassNode* a_node);
};

} // namespace ClassModelNodes

#endif // KDEVPLATFORM_PLUGIN_DOCUMENTCLASSESFOLDER_H
