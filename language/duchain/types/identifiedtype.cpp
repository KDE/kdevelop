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
#include "../declaration.h"
#include "../duchainpointer.h"
#include "../declarationid.h"

namespace KDevelop
{

IdentifiedType::~IdentifiedType()
{
}

void IdentifiedType::clear() {
  idData()->m_id = DeclarationId();
}

bool IdentifiedType::equals(const IdentifiedType* rhs) const
{
  if( idData()->m_id == rhs->idData()->m_id )
    return true;
  else
    return false;
}

// QualifiedIdentifier IdentifiedType::identifier() const
// {
//   return idData()->m_id ? idData()->m_iidData()->qualifiedIdentifier() : QualifiedIdentifier();
// }

QualifiedIdentifier IdentifiedType::qualifiedIdentifier() const {
  return idData()->m_id.qualifiedIdentifier();
}

uint IdentifiedType::hash() const {
  return idData()->m_id.hash();
}

DeclarationId IdentifiedType::declarationId() const {
  return idData()->m_id;
}

void IdentifiedType::setDeclarationId(const DeclarationId& id) {
  idData()->m_id = id;
}

Declaration* IdentifiedType::declaration(const TopDUContext* top) const
{
  return idData()->m_id.getDeclaration(top);
}

void IdentifiedType::setDeclaration(Declaration* declaration)
{
  if(declaration)
    idData()->m_id = declaration->id();
  else
    idData()->m_id = DeclarationId();
}

// QString IdentifiedType::idMangled() const
// {
//   return identifier().mangled();
// }

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
