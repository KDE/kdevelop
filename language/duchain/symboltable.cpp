/* This  is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include <kglobal.h>

#include "duchain.h"
#include "duchainlock.h"
#include "declaration.h"
#include "ducontext.h"

//#define DEBUG

#ifdef DEBUG
//If debug is defined, the code X will be executed. Else it will not be compiled.
#define ifDebug(X) X 
#else
#define ifDebug(X)
#endif


namespace KDevelop
{

typedef QMultiHash<QualifiedIdentifier, Declaration*> DeclarationMap;
typedef QMultiHash<QualifiedIdentifier, DUContext*> ContextMap;

class SymbolTablePrivate
{
public:
  DeclarationMap m_declarations;
  ContextMap m_contexts;
  SymbolTable m_instance;
};

K_GLOBAL_STATIC(SymbolTablePrivate, sdSymbolPrivate)

SymbolTable::SymbolTable()
{
  //m_declarations.reserve(2000);
}

SymbolTable* SymbolTable::self()
{
  return &sdSymbolPrivate->m_instance;
}

void SymbolTable::addDeclaration(Declaration* declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  ifDebug( kDebug(9505) << k_funcinfo << "Adding declaration" << declaration->qualifiedIdentifier().toString() << " with hash " <<  declaration->qualifiedIdentifier().hash(); )

  sdSymbolPrivate->m_declarations.insert(declaration->qualifiedIdentifier(), declaration);

  declaration->setInSymbolTable(true);
}

void SymbolTable::removeDeclaration(Declaration* declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  QualifiedIdentifier id = declaration->qualifiedIdentifier();
  DeclarationMap::Iterator it = sdSymbolPrivate->m_declarations.find(id);
  if (it != sdSymbolPrivate->m_declarations.end())
    for (; it.key() == id; ++it)
      if (it.value() == declaration) {
        sdSymbolPrivate->m_declarations.erase(it);
        declaration->setInSymbolTable(false);
        return;
      }

  kWarning() << k_funcinfo << "Could not find declaration matching" << id ;
}

QList<Declaration*> SymbolTable::findDeclarations(const QualifiedIdentifier& id) const
{
  ENSURE_CHAIN_READ_LOCKED
  ifDebug( kDebug(9505) << k_funcinfo << "Searching declaration " << id.toString() << " with hash " <<  id.hash(); )

  return sdSymbolPrivate->m_declarations.values(id);
}

// QList<Declaration*> SymbolTable::findDeclarationsBeginningWith(const QualifiedIdentifier& id) const
// {
//   ENSURE_CHAIN_READ_LOCKED
// 
//   QList<Declaration*> ret;
//   DeclarationMap::ConstIterator end = sdSymbolPrivate->m_declarations.constEnd();
//   DeclarationMap::ConstIterator it = sdSymbolPrivate->m_declarations.lowerBound(id);
// 
//   if (it != end) {
//     bool forwards = it.key().startsWith(idString);
//     if (!forwards) {
//       end = sdSymbolPrivate->m_declarations.constBegin();
//       --it;
//     }
//     for (; it != end && it.key().startsWith(idString); forwards ? ++it : --it)
//       ret.append(it.value());
//   }
// 
//   return ret;
// }

// kate: indent-width 2;

void SymbolTable::dumpStatistics() const
{
  ENSURE_CHAIN_READ_LOCKED

  ifDebug( kDebug(9505) << k_funcinfo << "Definitions" << sdSymbolPrivate->m_declarations.count() << ", Contexts" << sdSymbolPrivate->m_contexts.count(); )

  // TODO: more data
}

QList<DUContext*> SymbolTable::findContexts(const QualifiedIdentifier & id) const
{
  ENSURE_CHAIN_READ_LOCKED
  ifDebug( kDebug(9505) << k_funcinfo << "Searching context " << id.toString() << " with hash " <<  id.hash(); )

  return sdSymbolPrivate->m_contexts.values(id);
}

void SymbolTable::addContext(DUContext * namedContext)
{
  ENSURE_CHAIN_WRITE_LOCKED
  ifDebug( kDebug(9505) << k_funcinfo << "Adding context " << namedContext->scopeIdentifier(true).toString() << " with hash " <<  namedContext->scopeIdentifier(true).hash(); )

  sdSymbolPrivate->m_contexts.insert(namedContext->scopeIdentifier(true), namedContext);

  namedContext->setInSymbolTable(true);
}

void SymbolTable::removeContext(DUContext * namedContext)
{
  ENSURE_CHAIN_WRITE_LOCKED

  QualifiedIdentifier id = namedContext->scopeIdentifier(true);
  ContextMap::Iterator it = sdSymbolPrivate->m_contexts.find(id);
  if (it != sdSymbolPrivate->m_contexts.end())
    for (; it.key() == id; ++it)
      if (it.value() == namedContext) {
        sdSymbolPrivate->m_contexts.erase(it);
        namedContext->setInSymbolTable(false);
        return;
      }

  kWarning() << k_funcinfo << "Could not find context matching" << id ;
}
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
