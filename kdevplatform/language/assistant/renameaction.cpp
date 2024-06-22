/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "renameaction.h"

#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/codegen/documentchangeset.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>
// KF
#include <KLocalizedString>

using namespace KDevelop;

QVector<RevisionedFileRanges> RevisionedFileRanges::convert(const QMap<IndexedString, QVector<RangeInRevision>>& uses)
{
    QVector<RevisionedFileRanges> ret(uses.size());
    auto insertIt = ret.begin();
    for (auto it = uses.constBegin(); it != uses.constEnd(); ++it, ++insertIt) {
        insertIt->file = it.key();
        insertIt->ranges = it.value();

        DocumentChangeTracker* tracker =
            ICore::self()->languageController()->backgroundParser()->trackerForUrl(it.key());
        if (tracker) {
            insertIt->revision = tracker->revisionAtLastReset();
        }
    }

    return ret;
}

class KDevelop::RenameActionPrivate
{
public:
    Identifier m_oldDeclarationName;
    QString m_newDeclarationName;
    QVector<RevisionedFileRanges> m_oldDeclarationUses;
};

RenameAction::RenameAction(const Identifier& oldDeclarationName, const QString& newDeclarationName,
                           const QVector<RevisionedFileRanges>& oldDeclarationUses)
    : d_ptr(new RenameActionPrivate)
{
    Q_D(RenameAction);

    d->m_oldDeclarationName = oldDeclarationName;
    d->m_newDeclarationName = newDeclarationName.trimmed();
    d->m_oldDeclarationUses = oldDeclarationUses;
}

RenameAction::~RenameAction()
{
}

QString RenameAction::description() const
{
    Q_D(const RenameAction);

    return i18n("Rename \"%1\" to \"%2\"", d->m_oldDeclarationName.toString(), d->m_newDeclarationName);
}

QString RenameAction::newDeclarationName() const
{
    Q_D(const RenameAction);

    return d->m_newDeclarationName;
}

QString RenameAction::oldDeclarationName() const
{
    Q_D(const RenameAction);

    return d->m_oldDeclarationName.toString();
}

void RenameAction::execute()
{
    Q_D(RenameAction);

    DocumentChangeSet changes;

    for (const RevisionedFileRanges& ranges : std::as_const(d->m_oldDeclarationUses)) {
        for (const RangeInRevision range : ranges.ranges) {
            KTextEditor::Range currentRange;
            if (ranges.revision && ranges.revision->valid()) {
                currentRange = ranges.revision->transformToCurrentRevision(range);
            } else {
                currentRange = range.castToSimpleRange();
            }
            DocumentChange useRename(ranges.file, currentRange,
                d->m_oldDeclarationName.toString(), d->m_newDeclarationName);
            changes.addChange(useRename);
            changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
        }
    }

    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    if (!result) {
        auto* message = new Sublime::Message(i18n("Failed to apply changes: %1", result.m_failureReason), Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }

    emit executed(this);
}

#include "moc_renameaction.cpp"
