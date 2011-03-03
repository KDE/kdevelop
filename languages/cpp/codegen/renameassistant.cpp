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
#include <kmessagebox.h>

using namespace KDevelop;
using namespace Cpp;

typedef QMap<IndexedString, QList <RangeInRevision> > UsesList;

class RenameAction : public KDevelop::IAssistantAction {
public:
  RenameAction(Identifier oldDeclarationName, QString newDeclarationName, UsesList oldDeclarationUses)
  : m_oldDeclarationName(oldDeclarationName),
    m_newDeclarationName(newDeclarationName),
    m_oldDeclarationUses(oldDeclarationUses)
  { }

  virtual QString description() const {
    return i18n("Rename \"%1\" to \"%2\"", m_oldDeclarationName.toString(), m_newDeclarationName);
  }

  virtual void execute() {
    UsesList::iterator it;
    DocumentChangeSet changes;

    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    for (it = m_oldDeclarationUses.begin(); it != m_oldDeclarationUses.end(); ++it) {
      TopDUContext* topContext = DUChainUtils::standardContextForUrl(it.key().toUrl());
      if (!topContext) {
        //This would be abnormal
        kDebug() << "while renaming" << it.key().str() << "didn't produce a context";
        continue;
      }

      foreach (RangeInRevision range, it.value()) {
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
private:
  Identifier m_oldDeclarationName;
  QString m_newDeclarationName;
  UsesList m_oldDeclarationUses;
};

RenameAssistant::RenameAssistant(KTextEditor::View *view) :
m_documentUrl(IndexedString(view->document()->url())),
m_isUseful(false),
m_view(view)
{}

void RenameAssistant::reset() {
  m_oldDeclarationName = Identifier();
  m_newDeclarationRange.clear();
  m_oldDeclarationUses.clear();
  m_isUseful = false;
}

bool rangesConnect(const KTextEditor::Range &firstRange, const KTextEditor::Range &secondRange) {
  KTextEditor::Range expanded = KTextEditor::Range(
      secondRange.start().line(), secondRange.start().column() - 1,
      secondRange.end().line(), secondRange.end().column() + 1
  );
  return !firstRange.intersect(expanded).isEmpty();
}

Declaration* RenameAssistant::getValidDeclarationForChangedRange(KTextEditor::Range changed) {
  SimpleCursor cursor(changed.start());

  Declaration* declaration = DUChainUtils::itemUnderCursor(m_documentUrl.toUrl(), cursor);

  //If it's null we could be appending, but there's a case where appending gives a wrong decl
  //and not a null declaration ... "type var(init)", so check for that too
  if (!declaration || !rangesConnect(declaration->rangeInCurrentRevision().textRange(), changed))
    declaration = DUChainUtils::itemUnderCursor(m_documentUrl.toUrl(), SimpleCursor(cursor.line, --cursor.column));

  //In this case, we may either not have a decl at the cursor, or we got a decl, but are editing its use.
  //In either of those cases, give up and return 0
  if (!declaration || !rangesConnect(declaration->rangeInCurrentRevision().textRange(), changed))
    return 0;

  //Now we know we're editing a declaration, but some declarations we don't offer a rename for
  //basically that's any declaration that wouldn't be fully renamed just by renaming its uses().
  if (declaration->internalContext() || declaration->isForwardDeclaration()) {
    //make an exception for non-class functions
    if(!declaration->isFunctionDeclaration() || dynamic_cast<ClassFunctionDeclaration*>(declaration))
      return 0;
  }

  return declaration;
}

void RenameAssistant::textChanged(KTextEditor::Range invocationRange, QString removedText) {
  clearActions();

  if (!m_view)
    return;

  //If the inserted text isn't valid for a variable name, consider the editing ended
  QRegExp validDeclName("^[0-9a-zA-Z_]*$");
  if (removedText.isEmpty() && !validDeclName.exactMatch(m_view->document()->text(invocationRange))) {
    reset();
    return;
  }

  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());

  //If we've stopped editing m_newDeclarationRange, reset and see if there's another declaration being edited
  if (!m_newDeclarationRange.data() || !rangesConnect(m_newDeclarationRange->range().textRange(), invocationRange)) {
    reset();

    if (Declaration* declAtCursor = getValidDeclarationForChangedRange(invocationRange)) {
      QMap< IndexedString, QList<RangeInRevision> > declUses = declAtCursor->uses();
      if (declUses.size() > 0) {
        
        for(QMap< IndexedString, QList< RangeInRevision > >::iterator it = declUses.begin(); it != declUses.end(); ++it)
        {
          foreach(RangeInRevision range, it.value())
          {
            SimpleRange currentRange = declAtCursor->transformFromLocalRevision(range);
            if(currentRange.isEmpty() || m_view->document()->text(currentRange.textRange()) != declAtCursor->identifier().identifier().str())
              return; // One of the uses is invalid. Maybe the replacement has already been performed.
          }
        }
        
        m_oldDeclarationName = declAtCursor->identifier();
        m_oldDeclarationUses = declUses;

        KTextEditor::Range newRange = declAtCursor->rangeInCurrentRevision().textRange();
        if (removedText.isEmpty() && newRange.intersect(invocationRange).isEmpty())
          newRange = newRange.encompass(invocationRange); //if text was added to the ends, encompass it

        m_newDeclarationRange.attach(new PersistentMovingRange(newRange, m_documentUrl, true));
      }
      else  return; //new declaration is use-less
    }
    else return; //not editing a declaration
  }

  //Unfortunately this happens when you make a selection including one end of the decl's range and replace it
  if (removedText.isEmpty() && m_newDeclarationRange->range().textRange().intersect(invocationRange).isEmpty())
    m_newDeclarationRange.attach( new PersistentMovingRange(
        m_newDeclarationRange->range().textRange().encompass(invocationRange), m_documentUrl, true) );

  m_newDeclarationName = m_view->document()->text(m_newDeclarationRange->range().textRange());

  if (m_newDeclarationName == m_oldDeclarationName.toString())
    return;

  m_isUseful = true;
  addAction(IAssistantAction::Ptr(new RenameAction(m_oldDeclarationName, m_newDeclarationName, m_oldDeclarationUses)));
  emit actionsChanged();
}

#include "renameassistant.moc"
