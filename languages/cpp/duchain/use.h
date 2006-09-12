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

#ifndef DEFINITIONUSE_H
#define DEFINITIONUSE_H

#include "kdevdocumentrangeobject.h"
#include "duchainbase.h"

#include <QReadWriteLock>

class DUContext;
class Declaration;

/**
 * Represents a single variable definition in a definition-use chain.
 */
class Use : public DUChainBase, public KDevDocumentRangeObject
{
  friend class Declaration;
  friend class ForwardDeclaration;

public:
  Use(KTextEditor::Range* range, DUContext* context);
  virtual ~Use();

  DUContext* context() const;
  void setContext(DUContext* context);

  /**
   * Retrieve the declaration for this use.
   *
   * \note Does not require the chain to be locked.
   */
  Declaration* declaration() const;

  bool isOrphan() const;

private:
  /**
   * Set the declaration for this use.
   *
   * \note Does not require the chain to be locked.
   */
  void setDeclaration(Declaration* definition);

  DUContext* m_context;
  mutable QReadWriteLock m_declarationLock;
  Declaration* m_declaration;
};

#endif // DEFINITIONUSE_H

// kate: indent-width 2;
