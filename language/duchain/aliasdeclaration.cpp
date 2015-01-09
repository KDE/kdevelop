/* This  is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "aliasdeclaration.h"

#include "ducontext.h"
#include "duchainregister.h"
#include "types/delayedtype.h"
#include <editor/rangeinrevision.h>

namespace KDevelop
{

REGISTER_DUCHAIN_ITEM(AliasDeclaration);

AliasDeclaration::AliasDeclaration(const AliasDeclaration& rhs) 
  : ClassMemberDeclaration(*new AliasDeclarationData(*rhs.d_func())) {
}

AliasDeclaration::AliasDeclaration(const RangeInRevision& range, DUContext* context)
  : ClassMemberDeclaration(*new AliasDeclarationData, range)
{
  d_func_dynamic()->setClassId(this);
  setKind(Alias);
  if( context )
    setContext( context );
}

AliasDeclaration::AliasDeclaration(AliasDeclarationData& data) : ClassMemberDeclaration(data) {
}


AliasDeclaration::~AliasDeclaration()
{
}

Declaration* AliasDeclaration::clonePrivate() const {
  return new AliasDeclaration(*this);
}

QString AliasDeclaration::toString() const {
  if( aliasedDeclaration().isValid() )
    return QStringLiteral("Alias %1 as %2").arg(aliasedDeclaration().declaration()->qualifiedIdentifier().toString()).arg(identifier().toString());
  else
    return QStringLiteral("Lost alias %1").arg(identifier().toString());
}

void AliasDeclaration::setAliasedDeclaration(const IndexedDeclaration& decl) {
  d_func_dynamic()->m_aliasedDeclaration = decl;
  Declaration* aliased = decl.data();
  if(aliased)
    Declaration::setAbstractType(aliased->abstractType());
}

IndexedDeclaration AliasDeclaration::aliasedDeclaration() const {
  return d_func()->m_aliasedDeclaration;
}

void AliasDeclaration::setAbstractType(AbstractType::Ptr type) {
  Declaration::setAbstractType(type);
}

}
