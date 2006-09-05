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

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <QMultiHash>
#include <QMultiMap>
#include <QReadWriteLock>

#include "identifier.h"

class QReadWriteLock;

class Declaration;
class DUContext;

/**
 * A global symbol table, which stores mangled identifiers for quick lookup.
 *
 * \todo profiling, map vs hash etc...
 */
class SymbolTable
{
public:
  static SymbolTable* self();

  void dumpStatistics() const;

  // Declarations
  void addDeclaration(Declaration* declaration);
  void removeDeclaration(Declaration* declaration);

  QList<Declaration*> findDeclarations(const QualifiedIdentifier& id) const;
  QList<Declaration*> findDeclarationsBeginningWith(const QualifiedIdentifier& id) const;

  // Named Contexts (classes and namespaces)
  void addContext(DUContext* namedContext);
  void removeContext(DUContext* namedContext);

  QList<DUContext*> findContexts(const QualifiedIdentifier& id) const;

private:
  SymbolTable();

  static SymbolTable* s_instance;

  mutable QReadWriteLock m_declarationMutex;
  QMultiMap<QString, Declaration*> m_declarations;

  mutable QReadWriteLock m_contextMutex;
  QMultiHash<QString, DUContext*> m_contexts;
};

#endif // SYMBOLTABLE_H

// kate: indent-width 2;
