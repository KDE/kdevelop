/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
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

#include "identifiedtype.h"
#include "declaration.h"
#include "duchainpointer.h"
#include "declarationid.h"

namespace KDevelop
{

class IdentifiedTypePrivate
{
public:
  DeclarationId m_id;
};

IdentifiedType::IdentifiedType(const IdentifiedType& rhs) : d(new IdentifiedTypePrivate(*rhs.d)) {
}

IdentifiedType::IdentifiedType()
  : d(new IdentifiedTypePrivate)
{
}

void IdentifiedType::clear() {
  d->m_id = DeclarationId();
}

bool IdentifiedType::equals(const IdentifiedType* rhs) const
{
  if( d->m_id == rhs->d->m_id )
    return true;
  else
    return false;
}

// QualifiedIdentifier IdentifiedType::identifier() const
// {
//   return d->m_id ? d->m_id->qualifiedIdentifier() : QualifiedIdentifier();
// }

QualifiedIdentifier IdentifiedType::qualifiedIdentifier() const {
  return d->m_id.qualifiedIdentifier();
}

uint IdentifiedType::hash() const {
  return d->m_id.hash();
}

DeclarationId IdentifiedType::declarationId() const {
  return d->m_id;
}

void IdentifiedType::setDeclarationId(const DeclarationId& id) {
  d->m_id = id;
}

Declaration* IdentifiedType::declaration(const TopDUContext* top) const
{
  return d->m_id.getDeclaration(top);
}

void IdentifiedType::setDeclaration(Declaration* declaration)
{
  if(declaration)
    d->m_id = declaration->id();
  else
    d->m_id = DeclarationId();
}

// QString IdentifiedType::idMangled() const
// {
//   return identifier().mangled();
// }

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
