/*
   Copyright 2010 Olivier de Gaalon <olivier.jg@gmail.com>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

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
#include "renamefileaction.h"
#include "../codegen/basicrefactoring.h"
#include "../codegen/documentchangeset.h"
#include "../duchain/duchain.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchainutils.h"
#include "../duchain/declaration.h"
#include "../duchain/functiondefinition.h"
#include "../duchain/classfunctiondeclaration.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <klocalizedstring.h>

using namespace KDevelop;

namespace {

bool rangesConnect(const KTextEditor::Range& firstRange, const KTextEditor::Range& secondRange)
{
    return !firstRange.intersect(secondRange + KTextEditor::Range(0, -1, 0, +1)).isEmpty();
}

Declaration* getDeclarationForChangedRange(KTextEditor::View* view, const KTextEditor::Range& changed)
{
    const KTextEditor::Cursor cursor(changed.start());
    Declaration* declaration = DUChainUtils::itemUnderCursor(view->document()->url(), cursor);

    //If it's null we could be appending, but there's a case where appending gives a wrong decl
    //and not a null declaration ... "type var(init)", so check for that too
    if (!declaration || !rangesConnect(declaration->rangeInCurrentRevision(), changed)) {
        declaration = DUChainUtils::itemUnderCursor(view->document()->url(), KTextEditor::Cursor(cursor.line(), cursor.column()-1));
    }

    //In this case, we may either not have a decl at the cursor, or we got a decl, but are editing its use.
    //In either of those cases, give up and return 0
    if (!declaration || !rangesConnect(declaration->rangeInCurrentRevision(), changed)) {
        return 0;
    }

    return declaration;
}

}

struct RenameAssistant::Private
{
    Private(RenameAssistant* qq)
        : q(qq)
        , m_isUseful(false)
        , m_renameFile(false)
    {
    }

    void reset()
    {
        q->doHide();
        q->clearActions();
        m_oldDeclarationName = Identifier();
        m_newDeclarationRange.reset();
        m_oldDeclarationUses.clear();
        m_isUseful = false;
        m_renameFile = false;
    }

    RenameAssistant* q;

    KDevelop::Identifier m_oldDeclarationName;
    QString m_newDeclarationName;
    KDevelop::PersistentMovingRange::Ptr m_newDeclarationRange;
    QVector<RevisionedFileRanges> m_oldDeclarationUses;

    bool m_isUseful;
    bool m_renameFile;
};

RenameAssistant::RenameAssistant(ILanguageSupport* supportedLanguage)
    : StaticAssistant(supportedLanguage)
    , d(new Private(this))
{
}

RenameAssistant::~RenameAssistant()
{
}

QString RenameAssistant::title() const
{
    return tr("Rename");
}

bool RenameAssistant::isUseful() const
{
    return d->m_isUseful;
}

void RenameAssistant::textChanged(KTextEditor::View* view, const KTextEditor::Range& invocationRange, const QString& removedText)
{
    clearActions();

    if (!supportedLanguage()->refactoring()) {
        qWarning() << "Refactoring not supported. Aborting.";
        return;
    }

    if (!view)
        return;

    //If the inserted text isn't valid for a variable name, consider the editing ended
    QRegExp validDeclName("^[0-9a-zA-Z_]*$");
    if (removedText.isEmpty() && !validDeclName.exactMatch(view->document()->text(invocationRange))) {
        d->reset();
        return;
    }

    const QUrl url = view->document()->url();
    const IndexedString indexedUrl(url);
    DUChainReadLocker lock;

    //If we've stopped editing m_newDeclarationRange or switched the view,
    // reset and see if there's another declaration being edited
    if (!d->m_newDeclarationRange.data() || !rangesConnect(d->m_newDeclarationRange->range(), invocationRange)
            || d->m_newDeclarationRange->document() != indexedUrl) {
        d->reset();

        Declaration* declAtCursor = getDeclarationForChangedRange(view, invocationRange);
        if (!declAtCursor) {
            // not editing a declaration
            return;
        }

        if (supportedLanguage()->refactoring()->shouldRenameUses(declAtCursor)) {
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
                KTextEditor::Range currentRange = declAtCursor->transformFromLocalRevision(range);
                if(currentRange.isEmpty() || view->document()->text(currentRange) != declAtCursor->identifier().identifier().str())
                    return; // One of the uses is invalid. Maybe the replacement has already been performed.
                }
            }
            d->m_oldDeclarationUses = RevisionedFileRanges::convert(declUses);
        } else if (supportedLanguage()->refactoring()->shouldRenameFile(declAtCursor)) {
            d->m_renameFile = true;
        } else {
            // not a valid declaration
            return;
        }

        d->m_oldDeclarationName = declAtCursor->identifier();
        KTextEditor::Range newRange = declAtCursor->rangeInCurrentRevision();
        if (removedText.isEmpty() && newRange.intersect(invocationRange).isEmpty()) {
            newRange = newRange.encompass(invocationRange); //if text was added to the ends, encompass it
        }

        d->m_newDeclarationRange = new PersistentMovingRange(newRange, indexedUrl, true);
    }

    //Unfortunately this happens when you make a selection including one end of the decl's range and replace it
    if (removedText.isEmpty() && d->m_newDeclarationRange->range().intersect(invocationRange).isEmpty()) {
        d->m_newDeclarationRange = new PersistentMovingRange(
            d->m_newDeclarationRange->range().encompass(invocationRange), indexedUrl, true);
    }

    d->m_newDeclarationName = view->document()->text(d->m_newDeclarationRange->range());
    if (d->m_newDeclarationName == d->m_oldDeclarationName.toString()) {
        d->reset();
        return;
    }

    if (d->m_renameFile && supportedLanguage()->refactoring()->newFileName(url, d->m_newDeclarationName) == url.fileName()) {
        // no change, don't do anything
        return;
    }

    d->m_isUseful = true;

    IAssistantAction::Ptr action;
    if (d->m_renameFile) {
        action = new RenameFileAction(supportedLanguage()->refactoring(), url, d->m_newDeclarationName);
    } else {
        action =new RenameAction(d->m_oldDeclarationName, d->m_newDeclarationName, d->m_oldDeclarationUses);
    }
    connect(action.data(), &IAssistantAction::executed, this, [&] { d->reset(); });
    addAction(action);
    emit actionsChanged();
}

#include "moc_renameassistant.cpp"
