/*
   Copyright 2010 Olivier de Gaalon <olivier.jg@gmail.com>

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

#include "renameassistant.h"
#include "renameaction.h"
#include "simplerefactoring.h"
#include "renamefileaction.h"
#include <language/duchain/duchainutils.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <klocalizedstring.h>

using namespace KDevelop;
using namespace Cpp;

RenameAssistant::RenameAssistant(KTextEditor::View *view)
: m_view(view)
, m_isUseful(false)
, m_renameFile(false)
{

}

void RenameAssistant::reset() {
  m_oldDeclarationName = Identifier();
  m_newDeclarationRange.clear();
  m_oldDeclarationUses.clear();
  m_isUseful = false;
  m_renameFile = false;
}

bool rangesConnect(const KTextEditor::Range &firstRange, const KTextEditor::Range &secondRange) {
  KTextEditor::Range expanded = KTextEditor::Range(
      secondRange.start().line(), secondRange.start().column() - 1,
      secondRange.end().line(), secondRange.end().column() + 1
  );
  return !firstRange.intersect(expanded).isEmpty();
}

Declaration* RenameAssistant::getDeclarationForChangedRange(const KTextEditor::Range& changed) const
{
  SimpleCursor cursor(changed.start());

  Declaration* declaration = DUChainUtils::itemUnderCursor(m_view->document()->url(), cursor);

  //If it's null we could be appending, but there's a case where appending gives a wrong decl
  //and not a null declaration ... "type var(init)", so check for that too
  if (!declaration || !rangesConnect(declaration->rangeInCurrentRevision().textRange(), changed))
    declaration = DUChainUtils::itemUnderCursor(m_view->document()->url(), SimpleCursor(cursor.line, --cursor.column));

  //In this case, we may either not have a decl at the cursor, or we got a decl, but are editing its use.
  //In either of those cases, give up and return 0
  if (!declaration || !rangesConnect(declaration->rangeInCurrentRevision().textRange(), changed))
    return 0;

  return declaration;
}

bool RenameAssistant::shouldRenameUses(Declaration* declaration) const
{
  //Now we know we're editing a declaration, but some declarations we don't offer a rename for
  //basically that's any declaration that wouldn't be fully renamed just by renaming its uses().
  if (declaration->internalContext() || declaration->isForwardDeclaration()) {
    //make an exception for non-class functions
    if(!declaration->isFunctionDeclaration() || dynamic_cast<ClassFunctionDeclaration*>(declaration))
      return false;
  }
  return true;
}

void RenameAssistant::textChanged(const KTextEditor::Range& invocationRange, const QString& removedText) {
  clearActions();

  if (!m_view)
    return;

  //If the inserted text isn't valid for a variable name, consider the editing ended
  QRegExp validDeclName("^[0-9a-zA-Z_]*$");
  if (removedText.isEmpty() && !validDeclName.exactMatch(m_view->document()->text(invocationRange))) {
    reset();
    return;
  }

  const KUrl url = m_view->document()->url();
  const IndexedString indexedUrl(url);
  DUChainReadLocker lock;

  //If we've stopped editing m_newDeclarationRange, reset and see if there's another declaration being edited
  if (!m_newDeclarationRange.data() || !rangesConnect(m_newDeclarationRange->range().textRange(), invocationRange)) {
    reset();

    Declaration* declAtCursor = getDeclarationForChangedRange(invocationRange);
    if (!declAtCursor) {
      // not editing a declaration
      return;
    }
    if (shouldRenameUses(declAtCursor)) {
      QMap< IndexedString, QList<RangeInRevision> > declUses = declAtCursor->uses();
      if (declUses.isEmpty()) {
        // new declaration is use-less
        return;
      }
      for(QMap< IndexedString, QList< RangeInRevision > >::const_iterator it = declUses.constBegin();
          it != declUses.constEnd(); ++it)
      {
        foreach(const RangeInRevision& range, it.value())
        {
          SimpleRange currentRange = declAtCursor->transformFromLocalRevision(range);
          if(currentRange.isEmpty() || m_view->document()->text(currentRange.textRange()) != declAtCursor->identifier().identifier().str())
            return; // One of the uses is invalid. Maybe the replacement has already been performed.
        }
      }
      m_oldDeclarationUses = RevisionedFileRanges::convert(declUses);
    } else if (SimpleRefactoring::shouldRenameFile(declAtCursor)) {
      m_renameFile = true;
    } else {
      // not a valid declaration
      return;
    }

    m_oldDeclarationName = declAtCursor->identifier();
    KTextEditor::Range newRange = declAtCursor->rangeInCurrentRevision().textRange();
    if (removedText.isEmpty() && newRange.intersect(invocationRange).isEmpty()) {
      newRange = newRange.encompass(invocationRange); //if text was added to the ends, encompass it
    }

    m_newDeclarationRange.attach(new PersistentMovingRange(newRange, indexedUrl, true));
  }

  //Unfortunately this happens when you make a selection including one end of the decl's range and replace it
  if (removedText.isEmpty() && m_newDeclarationRange->range().textRange().intersect(invocationRange).isEmpty()) {
    m_newDeclarationRange.attach( new PersistentMovingRange(
        m_newDeclarationRange->range().textRange().encompass(invocationRange), indexedUrl, true) );
  }

  m_newDeclarationName = m_view->document()->text(m_newDeclarationRange->range().textRange());

  if (m_newDeclarationName == m_oldDeclarationName.toString()) {
    return;
  }

  if (m_renameFile && SimpleRefactoring::newFileName(url, m_newDeclarationName) == url.fileName()) {
    // no change, don't do anything
    return;
  }

  m_isUseful = true;

  IAssistantAction::Ptr action;

  if (m_renameFile) {
    action.attach(new RenameFileAction(url, m_newDeclarationName));
  } else {
    action.attach(new RenameAction(m_oldDeclarationName, m_newDeclarationName,
                                   m_oldDeclarationUses));
  }
  connect(action.data(), SIGNAL(executed(IAssistantAction*)), SLOT(reset()));
  addAction(action);
  emit actionsChanged();
}

#include "renameassistant.moc"
