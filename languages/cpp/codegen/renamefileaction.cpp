/*
  Copyright 2012 Milian Wolff <mail@milianw.de>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "renamefileaction.h"

#include "simplerefactoring.h"
#include <shell/documentcontroller.h>
#include <interfaces/icore.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <KDialog>
#include <KDebug>

using namespace KDevelop;
using namespace Cpp;

RenameFileAction::RenameFileAction(const KUrl& file, const QString& newName)
: m_file(file)
, m_newName(newName)
{

}

QString RenameFileAction::description() const
{
  return i18n("Rename file from \"%1\" to \"%2\".",
              m_file.fileName(), SimpleRefactoring::newFileName(m_file, m_newName));
}

void RenameFileAction::execute()
{
  // save document to prevent unwanted dialogs
  IDocument* doc = ICore::self()->documentController()->documentForUrl(m_file);
  if (!doc) {
    kWarning() << "could find no document for url:" << m_file;
    return;
  }
  if (!ICore::self()->documentController()->saveSomeDocuments(QList<IDocument*>() << doc, IDocument::Silent)) {
    return;
  }
  // rename document
  DocumentChangeSet changes;
  DocumentChangeSet::ChangeResult result = SimpleRefactoring::addRenameFileChanges(m_file, m_newName, &changes);
  if (result) {
    result = changes.applyAllChanges();
  }
  if(!result) {
    KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
  }
  emit executed(this);
}
