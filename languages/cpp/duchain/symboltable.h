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

#include "identifier.h"

class QReadWriteLock;

class Declaration;

/**
 * A global symbol table, which stores mangled identifiers for quick lookup.
 */
class SymbolTable
{
public:
  static SymbolTable* self();

  void dumpStatistics() const;

  void addDeclaration(Declaration* declaration);
  void removeDeclaration(Declaration* declaration);

  QList<Declaration*> findDeclarations(const QualifiedIdentifier& id) const;

private:
  SymbolTable();
  virtual ~SymbolTable();

  static SymbolTable* s_instance;

  QReadWriteLock *m_mutex;

  QMultiHash<QString, Declaration*> m_declarations;
};

#endif // SYMBOLTABLE_H

// kate: indent-width 2;
