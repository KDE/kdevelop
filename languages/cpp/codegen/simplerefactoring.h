/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef SIMPLEREFACTORING_H
#define SIMPLEREFACTORING_H

#include <interfaces/context.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/basicrefactoring.h>

namespace KDevelop {
class ContextMenuExtension;
class IndexedDeclaration;
class Declaration;
}

class SimpleRefactoring : public KDevelop::BasicRefactoring {
  Q_OBJECT

public:
  explicit SimpleRefactoring(QObject* parent = 0);
  void fillContextMenu(KDevelop::ContextMenuExtension& extension, KDevelop::Context* context);

  void startInteractiveRename(const KDevelop::IndexedDeclaration &decl);

  KDevelop::DocumentChangeSet::ChangeResult applyChangesToDeclarations(const QString& oldName,
                                                                       const QString& newName,
                                                                       KDevelop::DocumentChangeSet& changes,
                                                                       const QList<KDevelop::IndexedDeclaration>& declarations);

  /**
   * @return true if the declaration's file should be renamed if the declaration
   *         was renamed.
   */
  static bool shouldRenameFile(KDevelop::Declaration* declaration);

  /**
   * Add the change(s) related to renaming @p file to @p newName to @p changes and return the result.
   *
   * @param current The URL for the file you want to rename.
   * @param newName The new name of the file *without* the file extension.
   * @param changes The change set to add the rename changes to.
   */
  static KDevelop::DocumentChangeSet::ChangeResult addRenameFileChanges(const KUrl& current,
                                                                        const QString& newName,
                                                                        KDevelop::DocumentChangeSet* changes);

  static QString newFileName(const KUrl& current, const QString& newName);

  /**
   * Move the given inline declaration to the .cpp implementation file.
   *
   * @return empty string on success, error message otherwise.
   */
  static QString moveIntoSource(const KDevelop::IndexedDeclaration& decl);

public slots:
  void executeMoveIntoSourceAction();

private slots:
  void applyChangesDelayed();

private:
  KDevelop::DocumentChangeSet m_pendingChanges;
};

#endif
