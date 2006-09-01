/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef TOPDUCONTEXT_H
#define TOPDUCONTEXT_H

#include "ducontext.h"

/**
 * The top context in a definition-use chain for one source file.
 *
 * Will be used later to manage a few things.
 *
 * \todo move the registration with DUChain here
 */
class TopDUContext : public DUContext
{
public:
  TopDUContext(KTextEditor::Range* range);
  virtual ~TopDUContext();

  bool hasUses() const;
  void setHasUses(bool hasUses);

  bool imports(TopDUContext* origin, int depth = 0) const;

private:
  virtual Declaration* findDeclarationInternal(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<UsingNS*>* usingNamespaces, bool inImportedContext) const;

  QList<Declaration*> checkDeclarations(const QList<Declaration*>& declarations, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType) const;

  bool m_hasUses;
};

#endif // TOPDUCONTEXT_H

// kate: indent-width 2;
