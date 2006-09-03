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
#include "ducontext.h"

SymbolTable* SymbolTable::s_instance = 0;

SymbolTable::SymbolTable()
  : m_declarationMutex(new QReadWriteLock)
  , m_contextMutex(new QReadWriteLock)
{
  //m_declarations.reserve(2000);
}

SymbolTable::~ SymbolTable()
{
  delete m_declarationMutex;
  delete m_contextMutex;
}

SymbolTable* SymbolTable::self()
{
  if (!s_instance)
    s_instance = new SymbolTable;

  return s_instance;
}

void SymbolTable::addDeclaration(Declaration* declaration)
{
  QWriteLocker lock(m_declarationMutex);

  //kDebug() << k_funcinfo << "Adding declaration " << declaration->qualifiedIdentifier().toString(true) << " from " << declaration->textRange() << endl;

  m_declarations.insert(declaration->qualifiedIdentifier().toString(true), declaration);

  declaration->setInSymbolTable(true);
}

void SymbolTable::removeDeclaration(Declaration* declaration)
{
  QWriteLocker lock(m_declarationMutex);

  QString id = declaration->qualifiedIdentifier().toString(true);
  QMultiMap<QString, Declaration*>::Iterator it = m_declarations.find(id);
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
  QReadLocker lock(m_declarationMutex);

  return m_declarations.values(id.toString(true));
}

QList<Declaration*> SymbolTable::findDeclarationsBeginningWith(const QualifiedIdentifier& id) const
{
  QReadLocker lock(m_declarationMutex);

  QString idString = id.toString(true);
  QMultiMap<QString, Declaration*>::ConstIterator end = m_declarations.constEnd();
  QMultiMap<QString, Declaration*>::ConstIterator it = m_declarations.lowerBound(idString);

  bool forwards = it.key().startsWith(idString);
  if (!forwards)
    --it;

  QList<Declaration*> ret;
  for (; it != end && it.key().startsWith(idString); forwards ? ++it : --it)
    ret.append(it.value());

  return ret;
}

// kate: indent-width 2;

void SymbolTable::dumpStatistics() const
{
  QReadLocker lock(m_declarationMutex);
  QReadLocker lock2(m_contextMutex);

  kDebug() << k_funcinfo << "Definitions " << m_declarations.count() << ", Contexts " << m_contexts.count() << endl;

  // TODO: more data
}

QList< DUContext * > SymbolTable::findContexts(const QualifiedIdentifier & id) const
{
  QReadLocker lock(m_contextMutex);

  return m_contexts.values(id.toString(true));
}

void SymbolTable::addContext(DUContext * namedContext)
{
  QWriteLocker lock(m_contextMutex);

  m_contexts.insert(namedContext->scopeIdentifier(true).toString(true), namedContext);

  namedContext->setInSymbolTable(true);
}

void SymbolTable::removeContext(DUContext * namedContext)
{
  QWriteLocker lock(m_contextMutex);

  QString id = namedContext->scopeIdentifier(true).toString(true);
  QMultiHash<QString, DUContext*>::Iterator it = m_contexts.find(id);
  if (it != m_contexts.end())
    for (; it.key() == id; ++it)
      if (it.value() == namedContext) {
        m_contexts.erase(it);
        namedContext->setInSymbolTable(false);
        return;
      }

  kWarning() << k_funcinfo << "Could not find context matching " << id << endl;
}
