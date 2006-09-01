/* This  is part of KDevelop
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

#include "symboltable.h"

#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "declaration.h"

SymbolTable* SymbolTable::s_instance = 0;

SymbolTable::SymbolTable()
  : m_mutex(new QReadWriteLock)
{
}

SymbolTable::~ SymbolTable()
{
  delete m_mutex;
}

SymbolTable* SymbolTable::self()
{
  if (!s_instance)
    s_instance = new SymbolTable;

  return s_instance;
}

void SymbolTable::addDeclaration(Declaration* declaration)
{
  QWriteLocker lock(m_mutex);

  //kDebug() << k_funcinfo << "Adding declaration " << declaration->qualifiedIdentifier().toString(true) << " from " << declaration->textRange() << endl;

  m_declarations.insert(declaration->qualifiedIdentifier().toString(true), declaration);

  declaration->setInSymbolTable(true);
}

void SymbolTable::removeDeclaration(Declaration* declaration)
{
  QWriteLocker lock(m_mutex);

  QString id = declaration->qualifiedIdentifier().toString(true);
  QMultiHash<QString, Declaration*>::Iterator it = m_declarations.find(id);
  if (it != m_declarations.end())
    for (; it.key() == id; ++it)
      if (it.value() == declaration) {
        m_declarations.erase(it);
        declaration->setInSymbolTable(false);
        return;
      }

  kWarning() << k_funcinfo << "Could not find declaration matching " << id << endl;
}

QList<Declaration*> SymbolTable::findDeclarations(const QualifiedIdentifier& id) const
{
  QReadLocker lock(m_mutex);

  return m_declarations.values(id.toString(true));
}

// kate: indent-width 2;
