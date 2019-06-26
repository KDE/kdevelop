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
