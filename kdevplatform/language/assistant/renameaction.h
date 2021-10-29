/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_RENAMEACTION_H
#define KDEVPLATFORM_RENAMEACTION_H

#include <interfaces/iassistant.h>
#include <language/editor/rangeinrevision.h>
#include <language/backgroundparser/documentchangetracker.h>
#include <language/languageexport.h>

namespace KDevelop {
class Identifier;
class RenameActionPrivate;

/**
 * A HACK to circumvent the bad RangeInRevision API without rewriting everything.
 *
 * The RangeInRevision is actually just blindly assuming that it belongs to
 * the revision the file was parsed in the last time. But if the file is
 * reparsed in between (due to changes) the ranges might be wrong. Due to that
 * we must store the ranges and their actual revision... Stupid!
 *
 * See also: https://bugs.kde.org/show_bug.cgi?id=295707
 */
struct KDEVPLATFORMLANGUAGE_EXPORT RevisionedFileRanges
{
    KDevelop::IndexedString file;
    KDevelop::RevisionReference revision;
    QVector<KDevelop::RangeInRevision> ranges;

    static QVector<RevisionedFileRanges> convert(const QMap<KDevelop::IndexedString,
                                                     QVector<KDevelop::RangeInRevision>>& uses);
};

class KDEVPLATFORMLANGUAGE_EXPORT RenameAction
    : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    RenameAction(const KDevelop::Identifier& oldDeclarationName, const QString& newDeclarationName,
                 const QVector<RevisionedFileRanges>& oldDeclarationUses);
    ~RenameAction() override;

    QString description() const override;
    void execute() override;

    QString newDeclarationName() const;
    QString oldDeclarationName() const;

private:
    const QScopedPointer<class RenameActionPrivate> d_ptr;
    Q_DECLARE_PRIVATE(RenameAction)
};
}

Q_DECLARE_TYPEINFO(KDevelop::RevisionedFileRanges, Q_MOVABLE_TYPE);

#endif
