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

#ifndef KDEVPLATFORM_PLUGIN_CLASSMODELNODESCONTROLLER_H
#define KDEVPLATFORM_PLUGIN_CLASSMODELNODESCONTROLLER_H

#include <QObject>
#include <language/duchain/indexedstring.h>
#include <language/duchain/ducontext.h>

class QTimer;

class ClassModelNodeDocumentChangedInterface
{
public:
  virtual ~ClassModelNodeDocumentChangedInterface();

  /// Called when the registered document is changed.
  virtual void documentChanged(const KDevelop::IndexedString& a_file) = 0;
};

/// This class provides notifications for updates between the different nodes
/// and the various kdevelop sub-systems (such as notification when a DUChain gets
/// updated).
class ClassModelNodesController : public QObject
{
  Q_OBJECT
public:
  ClassModelNodesController();
  ~ClassModelNodesController();

  static ClassModelNodesController& self();

  /// Register the given class node to receive notifications about its top context changes.
  void registerForChanges(const KDevelop::IndexedString& a_file, ClassModelNodeDocumentChangedInterface* a_node);
  /// Unregister the given class node from further notifications.
  void unregisterForChanges(const KDevelop::IndexedString& a_file, ClassModelNodeDocumentChangedInterface* a_node);

private Q_SLOTS:
  // Files update.
  void branchModified(KDevelop::DUContextPointer context);
  void updateChangedFiles();

private: // File updates related.
  /// List of updated files we check this list when update timer expires.
  QSet<KDevelop::IndexedString> m_updatedFiles;

  /// Timer for batch updates.
  QTimer* m_updateTimer;

  typedef QMultiMap< KDevelop::IndexedString, ClassModelNodeDocumentChangedInterface* > FilesMap;
  /// Maps between monitored files and their class nodes.
  FilesMap m_filesMap;
};

#endif
