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

#ifndef CPP_RENAMEACTION_H
#define CPP_RENAMEACTION_H

#include <interfaces/iassistant.h>
#include <language/duchain/identifier.h>
#include <language/editor/rangeinrevision.h>
#include <language/backgroundparser/documentchangetracker.h>

using namespace KDevelop;
namespace Cpp {

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
struct RevisionedFileRanges
{
  IndexedString file;
  RevisionReference revision;
  QList<RangeInRevision> ranges;
  static QVector<RevisionedFileRanges> convert(const QMap<IndexedString, QList<RangeInRevision> >& uses);
};

class RenameAction : public IAssistantAction
{
public:
  RenameAction(const Identifier &oldDeclarationName, const QString &newDeclarationName,
               const QVector<RevisionedFileRanges> &oldDeclarationUses);
  virtual QString description() const;
  virtual QString newDeclarationName() const;
  virtual QString oldDeclarationName() const;
  virtual void execute();
private:
  Identifier m_oldDeclarationName;
  QString m_newDeclarationName;
  QVector<RevisionedFileRanges> m_oldDeclarationUses;
};

}

Q_DECLARE_TYPEINFO(Cpp::RevisionedFileRanges, Q_MOVABLE_TYPE);

#endif
