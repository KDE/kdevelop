/*
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

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

#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/codegen/documentchangeset.h>
#include <KMessageBox>
#include <KLocalizedString>

#include "renameaction.h"

using namespace KDevelop;
using namespace Cpp;

RenameAction::RenameAction(const Identifier &oldDeclarationName, const QString &newDeclarationName, const UsesList &oldDeclarationUses)
  : m_oldDeclarationName(oldDeclarationName),
    m_newDeclarationName(newDeclarationName),
    m_oldDeclarationUses(oldDeclarationUses)
  { }

QString RenameAction::description() const {
  return i18n("Rename \"%1\" to \"%2\"", m_oldDeclarationName.toString(), m_newDeclarationName);
}

QString RenameAction::newDeclarationName() const
{
  return m_newDeclarationName;
}

QString RenameAction::oldDeclarationName() const
{
  return m_oldDeclarationName.toString();
}

void RenameAction::execute() {
  UsesList::iterator it;
  DocumentChangeSet changes;

  DUChainReadLocker lock;
  for (it = m_oldDeclarationUses.begin(); it != m_oldDeclarationUses.end(); ++it) {
    TopDUContext* topContext = DUChainUtils::standardContextForUrl(it.key().toUrl());
    if (!topContext) {
      //This would be abnormal
      kDebug() << "while renaming" << it.key().str() << "didn't produce a context";
      continue;
    }

    foreach (const RangeInRevision &range, it.value()) {
      DocumentChange useRename
          (it.key(), topContext->transformFromLocalRevision(range), m_oldDeclarationName.toString(), m_newDeclarationName);
      changes.addChange( useRename );
      changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    }
  }
  lock.unlock();

  DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
  if(!result)
    KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
}
