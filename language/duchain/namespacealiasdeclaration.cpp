/* This  is part of KDevelop
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "namespacealiasdeclaration.h"

#include "ducontext.h"
#include "duchainregister.h"
#include "persistentsymboltable.h"

namespace KDevelop
{

REGISTER_DUCHAIN_ITEM(NamespaceAliasDeclaration);

NamespaceAliasDeclaration::NamespaceAliasDeclaration(const NamespaceAliasDeclaration& rhs) 
  : Declaration(*new NamespaceAliasDeclarationData(*rhs.d_func())) {
}

NamespaceAliasDeclaration::NamespaceAliasDeclaration(const RangeInRevision& range, DUContext* context)
  : Declaration(*new NamespaceAliasDeclarationData, range)
{
  d_func_dynamic()->setClassId(this);
  setKind(NamespaceAlias);
  if( context )
    setContext( context );
}

NamespaceAliasDeclaration::NamespaceAliasDeclaration(NamespaceAliasDeclarationData& data) : Declaration(data) {
}


QualifiedIdentifier NamespaceAliasDeclaration::importIdentifier() const {
  return d_func()->m_importIdentifier.identifier();
}

void NamespaceAliasDeclaration::setImportIdentifier(const QualifiedIdentifier& id) {
  Q_ASSERT(!id.explicitlyGlobal());
  d_func_dynamic()->m_importIdentifier = id;
}

NamespaceAliasDeclaration::~NamespaceAliasDeclaration()
{
  if(persistentlyDestroying() && d_func()->m_inSymbolTable)
    unregisterAliasIdentifier();
}


void NamespaceAliasDeclaration::setInSymbolTable(bool inSymbolTable)
{
  if(d_func()->m_inSymbolTable && !inSymbolTable)
  {
    unregisterAliasIdentifier();
  }else if(!d_func()->m_inSymbolTable && inSymbolTable)
  {
    registerAliasIdentifier();
  }
  KDevelop::Declaration::setInSymbolTable(inSymbolTable);
}

void NamespaceAliasDeclaration::unregisterAliasIdentifier()
{
  if(indexedIdentifier() != globalIndexedImportIdentifier())
  {
    QualifiedIdentifier aliasId = qualifiedIdentifier();
    aliasId.push(globalIndexedAliasIdentifier());
    KDevelop::PersistentSymbolTable::self().removeDeclaration(aliasId, this);
  }
}

void NamespaceAliasDeclaration::registerAliasIdentifier()
{
  if(indexedIdentifier() != globalIndexedImportIdentifier())
  {
    QualifiedIdentifier aliasId = qualifiedIdentifier();
    aliasId.push(globalIndexedAliasIdentifier());
    KDevelop::PersistentSymbolTable::self().addDeclaration(aliasId, this);
  }
}

Declaration* NamespaceAliasDeclaration::clonePrivate() const {
  return new NamespaceAliasDeclaration(*this);
}

void NamespaceAliasDeclaration::setAbstractType(AbstractType::Ptr type) {
  //A namespace-import does not have a type, so ignore any set type
  Q_UNUSED(type);
}

QString NamespaceAliasDeclaration::toString() const {
  DUCHAIN_D(NamespaceAliasDeclaration);
  if( indexedIdentifier() != globalIndexedImportIdentifier() )
    return QStringLiteral("Import %1 as %2").arg(d->m_importIdentifier.identifier().toString()).arg(identifier().toString());
  else
    return QStringLiteral("Import %1").arg(d->m_importIdentifier.identifier().toString());
}

}
