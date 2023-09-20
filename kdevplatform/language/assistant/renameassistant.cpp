/*
    SPDX-FileCopyrightText: 2010 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "renameassistant.h"

#include "renameaction.h"
#include "renamefileaction.h"
#include <debug.h>
#include "../codegen/basicrefactoring.h"
#include "../duchain/duchain.h"
#include "../duchain/duchainlock.h"
#include "../duchain/duchainutils.h"
#include "../duchain/declaration.h"
#include "../duchain/functiondefinition.h"
#include "../duchain/classfunctiondeclaration.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguagesupport.h>

#include <KTextEditor/Document>

#include <KLocalizedString>

using namespace KDevelop;

namespace {
bool rangesConnect(const KTextEditor::Range& firstRange, const KTextEditor::Range& secondRange)
{
    return !firstRange.intersect(secondRange + KTextEditor::Range(0, -1, 0, +1)).isEmpty();
}

Declaration* declarationForChangedRange(KTextEditor::Document* doc, const KTextEditor::Range& changed)
{
    const KTextEditor::Cursor cursor(changed.start());
    Declaration* declaration = DUChainUtils::itemUnderCursor(doc->url(), cursor).declaration;

    //If it's null we could be appending, but there's a case where appending gives a wrong decl
    //and not a null declaration ... "type var(init)", so check for that too
    if (!declaration || !rangesConnect(declaration->rangeInCurrentRevision(), changed)) {
        declaration =
            DUChainUtils::itemUnderCursor(doc->url(),
                                          KTextEditor::Cursor(cursor.line(), cursor.column() - 1)).declaration;
    }

    //In this case, we may either not have a decl at the cursor, or we got a decl, but are editing its use.
    //In either of those cases, give up and return 0
    if (!declaration || !rangesConnect(declaration->rangeInCurrentRevision(), changed)) {
        return nullptr;
    }

    return declaration;
}
}

class KDevelop::RenameAssistantPrivate
{
public:
    explicit RenameAssistantPrivate(RenameAssistant* qq)
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
    KTextEditor::Cursor m_lastChangedLocation;
    QPointer<KTextEditor::Document> m_lastChangedDocument = nullptr;
};

RenameAssistant::RenameAssistant(ILanguageSupport* supportedLanguage)
    : StaticAssistant(supportedLanguage)
    , d_ptr(new RenameAssistantPrivate(this))
{
}

RenameAssistant::~RenameAssistant()
{
}

QString RenameAssistant::title() const
{
    return i18n("Rename");
}

bool RenameAssistant::isUseful() const
{
    Q_D(const RenameAssistant);

    return d->m_isUseful;
}

void RenameAssistant::textChanged(KTextEditor::Document* doc, const KTextEditor::Range& invocationRange,
                                  const QString& removedText)
{
    Q_D(RenameAssistant);

    clearActions();
    d->m_lastChangedLocation = invocationRange.end();
    d->m_lastChangedDocument = doc;

    if (!supportedLanguage()->refactoring()) {
        qCWarning(LANGUAGE) << "Refactoring not supported. Aborting.";
        return;
    }

    if (!doc)
        return;

    //If the inserted text isn't valid for a variable name, consider the editing ended
    QRegularExpression validDeclName(QStringLiteral("^[0-9a-zA-Z_]*$"));
    if (removedText.isEmpty() && !validDeclName.match(doc->text(invocationRange)).hasMatch()) {
        d->reset();
        return;
    }

    const QUrl url = doc->url();
    const IndexedString indexedUrl(url);
    DUChainReadLocker lock;

    //If we've stopped editing m_newDeclarationRange or switched the view,
    // reset and see if there's another declaration being edited
    if (!d->m_newDeclarationRange.data() || !rangesConnect(d->m_newDeclarationRange->range(), invocationRange)
        || d->m_newDeclarationRange->document() != indexedUrl) {
        d->reset();

        Declaration* declAtCursor = declarationForChangedRange(doc, invocationRange);
        if (!declAtCursor) {
            // not editing a declaration
            return;
        }

        if (supportedLanguage()->refactoring()->shouldRenameUses(declAtCursor)) {
            const auto declUses = declAtCursor->uses();
            if (declUses.isEmpty()) {
                // new declaration has no uses
                return;
            }

            for (auto& ranges : declUses) {
                for (const RangeInRevision range : ranges) {
                    KTextEditor::Range currentRange = declAtCursor->transformFromLocalRevision(range);
                    if (currentRange.isEmpty() ||
                        doc->text(currentRange) != declAtCursor->identifier().identifier().str()) {
                        return; // One of the uses is invalid. Maybe the replacement has already been performed.
                    }
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

    d->m_newDeclarationName = doc->text(d->m_newDeclarationRange->range()).trimmed();
    if (d->m_newDeclarationName == d->m_oldDeclarationName.toString()) {
        d->reset();
        return;
    }

    if (d->m_renameFile &&
        supportedLanguage()->refactoring()->newFileName(url, d->m_newDeclarationName) == url.fileName()) {
        // no change, don't do anything
        return;
    }

    d->m_isUseful = true;

    IAssistantAction::Ptr action;
    if (d->m_renameFile) {
        action = new RenameFileAction(supportedLanguage()->refactoring(), url, d->m_newDeclarationName);
    } else {
        action = new RenameAction(d->m_oldDeclarationName, d->m_newDeclarationName, d->m_oldDeclarationUses);
    }
    connect(action.data(), &IAssistantAction::executed, this, [this] {
        Q_D(RenameAssistant);
        d->reset();
    });
    addAction(action);
    emit actionsChanged();
}

KTextEditor::Range KDevelop::RenameAssistant::displayRange() const
{
    Q_D(const RenameAssistant);

    if (!d->m_lastChangedDocument) {
        return {};
    }
    auto range = d->m_lastChangedDocument->wordRangeAt(d->m_lastChangedLocation);
    qCDebug(LANGUAGE) << "range:" << range;
    return range;
}

#include "moc_renameassistant.cpp"
