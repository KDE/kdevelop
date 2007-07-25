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

namespace KDevelop
{

class SymbolTablePrivate
{
public:
  QMultiMap<QString, Declaration*> m_declarations;
  QMultiHash<QString, DUContext*> m_contexts;
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

  //kDebug() << k_funcinfo << "Adding declaration " << declaration->qualifiedIdentifier().toString(true) << " from " << declaration->textRange() << endl;

  sdSymbolPrivate->m_declarations.insert(declaration->qualifiedIdentifier().toString(true), declaration);

  declaration->setInSymbolTable(true);
}

void SymbolTable::removeDeclaration(Declaration* declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  QString id = declaration->qualifiedIdentifier().toString(true);
  QMultiMap<QString, Declaration*>::Iterator it = sdSymbolPrivate->m_declarations.find(id);
  if (it != sdSymbolPrivate->m_declarations.end())
    for (; it.key() == id; ++it)
      if (it.value() == declaration) {
        sdSymbolPrivate->m_declarations.erase(it);
        declaration->setInSymbolTable(false);
        return;
      }

  kWarning() << k_funcinfo << "Could not find declaration matching " << id << endl;
}

QList<Declaration*> SymbolTable::findDeclarations(const QualifiedIdentifier& id) const
{
  ENSURE_CHAIN_READ_LOCKED

  return sdSymbolPrivate->m_declarations.values(id.toString(true));
}

QList<Declaration*> SymbolTable::findDeclarationsBeginningWith(const QualifiedIdentifier& id) const
{
  ENSURE_CHAIN_READ_LOCKED

  QList<Declaration*> ret;
  QString idString = id.toString(true);
  QMultiMap<QString, Declaration*>::ConstIterator end = sdSymbolPrivate->m_declarations.constEnd();
  QMultiMap<QString, Declaration*>::ConstIterator it = sdSymbolPrivate->m_declarations.lowerBound(idString);

  if (it != end) {
    bool forwards = it.key().startsWith(idString);
    if (!forwards) {
      end = sdSymbolPrivate->m_declarations.constBegin();
      --it;
    }
    for (; it != end && it.key().startsWith(idString); forwards ? ++it : --it)
      ret.append(it.value());
  }

  return ret;
}

// kate: indent-width 2;

void SymbolTable::dumpStatistics() const
{
  ENSURE_CHAIN_READ_LOCKED

  kDebug() << k_funcinfo << "Definitions " << sdSymbolPrivate->m_declarations.count() << ", Contexts " << sdSymbolPrivate->m_contexts.count() << endl;

  // TODO: more data
}

QList<DUContext*> SymbolTable::findContexts(const QualifiedIdentifier & id) const
{
  ENSURE_CHAIN_READ_LOCKED

  return sdSymbolPrivate->m_contexts.values(id.toString(true));
}

void SymbolTable::addContext(DUContext * namedContext)
{
  ENSURE_CHAIN_WRITE_LOCKED

  sdSymbolPrivate->m_contexts.insert(namedContext->scopeIdentifier(true).toString(true), namedContext);

  namedContext->setInSymbolTable(true);
}

void SymbolTable::removeContext(DUContext * namedContext)
{
  ENSURE_CHAIN_WRITE_LOCKED

  QString id = namedContext->scopeIdentifier(true).toString(true);
  QMultiHash<QString, DUContext*>::Iterator it = sdSymbolPrivate->m_contexts.find(id);
  if (it != sdSymbolPrivate->m_contexts.end())
    for (; it.key() == id; ++it)
      if (it.value() == namedContext) {
        sdSymbolPrivate->m_contexts.erase(it);
        namedContext->setInSymbolTable(false);
        return;
      }

  kWarning() << k_funcinfo << "Could not find context matching " << id << endl;
}
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
