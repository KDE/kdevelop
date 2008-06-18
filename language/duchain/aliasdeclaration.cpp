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
#include "declaration_p.h"

namespace KDevelop
{

class AliasDeclarationPrivate : public DeclarationPrivate
{
public:
  AliasDeclarationPrivate() {}
  AliasDeclarationPrivate( const AliasDeclarationPrivate& rhs )
      : DeclarationPrivate( rhs )
  {
    m_aliasedDeclaration = rhs.m_aliasedDeclaration;
  }
  DeclarationPointer m_aliasedDeclaration;
};

AliasDeclaration::AliasDeclaration(const AliasDeclaration& rhs) 
  : Declaration(*new AliasDeclarationPrivate(*rhs.d_func())) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

AliasDeclaration::AliasDeclaration(const HashedString& url, const SimpleRange& range, DUContext* context)
  : Declaration(*new AliasDeclarationPrivate, url, range)
{
  setKind(Alias);
  if( context )
    setContext( context );
}

AliasDeclaration::~AliasDeclaration()
{
}

Declaration* AliasDeclaration::clone() const {
  return new AliasDeclaration(*this);
}

QString AliasDeclaration::toString() const {
  if( aliasedDeclaration() )
    return QString("Alias %1 as %2").arg(aliasedDeclaration()->qualifiedIdentifier().toString()).arg(identifier().toString());
  else
    return QString("Lost alias %1").arg(identifier().toString());
}

void AliasDeclaration::setAliasedDeclaration(const DeclarationPointer& decl) {
  d_func()->m_aliasedDeclaration = decl;
}

DeclarationPointer AliasDeclaration::aliasedDeclaration() const {
  return d_func()->m_aliasedDeclaration;
}

void AliasDeclaration::setAbstractType(AbstractType::Ptr type) {
  if(type) {
    kDebug() << "tried to set type on AliasDeclaration" << type->toString();
  }
}

}
// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
