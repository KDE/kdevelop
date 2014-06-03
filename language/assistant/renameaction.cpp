/*
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>
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

#include "renameaction.h"

#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/codegen/documentchangeset.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include <KMessageBox>
#include <KLocalizedString>

using namespace KDevelop;

QVector<RevisionedFileRanges> RevisionedFileRanges::convert(const QMap<IndexedString, QList<RangeInRevision> >& uses)
{
    QVector<RevisionedFileRanges> ret(uses.size());
    auto insertIt = ret.begin();
    for (auto it = uses.constBegin(); it != uses.constEnd(); ++it, ++insertIt)
    {
        insertIt->file = it.key();
        insertIt->ranges = it.value();

        DocumentChangeTracker* tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(it.key());
        if (tracker) {
            insertIt->revision = tracker->revisionAtLastReset();
        }
    }
    return ret;
}

struct RenameAction::Private
{
    Private() {}

    Identifier m_oldDeclarationName;
    QString m_newDeclarationName;
    QVector<RevisionedFileRanges> m_oldDeclarationUses;
};

RenameAction::RenameAction(const Identifier& oldDeclarationName, const QString& newDeclarationName,
                           const QVector<RevisionedFileRanges>& oldDeclarationUses)
    : d(new Private)
{
    d->m_oldDeclarationName = oldDeclarationName;
    d->m_newDeclarationName = newDeclarationName;
    d->m_oldDeclarationUses = oldDeclarationUses;
}

RenameAction::~RenameAction()
{
}

QString RenameAction::description() const
{
    return i18n("Rename \"%1\" to \"%2\"", d->m_oldDeclarationName.toString(), d->m_newDeclarationName);
}

QString RenameAction::newDeclarationName() const
{
    return d->m_newDeclarationName;
}

QString RenameAction::oldDeclarationName() const
{
    return d->m_oldDeclarationName.toString();
}

void RenameAction::execute()
{
    DocumentChangeSet changes;

    foreach(const RevisionedFileRanges& ranges, d->m_oldDeclarationUses) {
        foreach (const RangeInRevision &range, ranges.ranges) {
            SimpleRange currentRange;
            if (ranges.revision && ranges.revision->valid()) {
                currentRange = ranges.revision->transformToCurrentRevision(range);
            } else {
                currentRange = range.castToSimpleRange();
            }
            DocumentChange useRename(ranges.file, currentRange,
                                     d->m_oldDeclarationName.toString(), d->m_newDeclarationName);
            changes.addChange( useRename );
            changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
        }
    }

    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    if (!result) {
        KMessageBox::error(0, i18n("Failed to apply changes: %1", result.m_failureReason));
    }

    emit executed(this);
}
