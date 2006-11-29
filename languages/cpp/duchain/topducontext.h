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
#include <kdevexport.h>
#include <QMutex>

/**
 * The top context in a definition-use chain for one source file.
 *
 * Implements SymbolTable lookups and locking for the chain.
 *
 * \todo move the registration with DUChain here
 */
class KDEVCPPLANGUAGE_EXPORT TopDUContext : public DUContext
{
public:
  TopDUContext(KTextEditor::Range* range);
  virtual ~TopDUContext();

  TopDUContext* topContext() const;

  /// Returns true if this object is being deleted, otherwise false.
  inline bool deleting() const { return m_deleting; }

  bool hasUses() const;
  void setHasUses(bool hasUses);

  /**
   * Determine if this chain imports another chain.
   *
   * \note you must be holding a read but not a write chain lock when you access this function.
   */
  bool imports(TopDUContext* origin, const KTextEditor::Cursor& position) const;

protected:
  virtual void findDeclarationsInternal(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<UsingNS*>& usingNamespaces, QList<Declaration*>& ret, bool inImportedContext) const;

  void findDeclarationsInNamespaces(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<UsingNS*>& usingNamespaces, QList<Declaration*>& ret) const;

  QList<UsingNS*> findNestedNamespaces(const KTextEditor::Cursor& position, UsingNS* ns) const;

  QList<Declaration*> checkDeclarations(const QList<Declaration*>& declarations, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType) const;

  virtual void findContextsInternal(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, QList<UsingNS*>& usingNS, QList<DUContext*>& ret, bool inImportedContext = false) const;

  void findContextsInNamespaces(ContextType contextType, const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, QList< UsingNS * >& usingNS, QList<DUContext*>& ret) const;

  void checkContexts(ContextType contextType, const QList<DUContext*>& contexts, const KTextEditor::Cursor& position, QList<DUContext*>& ret) const;

private:
  bool imports(TopDUContext* origin, int depth) const;

  mutable QReadWriteLock m_chainLock;
  bool m_hasUses  : 1;
  bool m_deleting : 1;
};

#endif // TOPDUCONTEXT_H

// kate: indent-width 2;
