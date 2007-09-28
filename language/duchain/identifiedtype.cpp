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

namespace KDevelop
{

class IdentifiedTypePrivate
{
public:
  DeclarationPointer m_declaration; //Self-invalidating
};

IdentifiedType::IdentifiedType(const IdentifiedType& rhs) : d(new IdentifiedTypePrivate(*rhs.d)) {
}

IdentifiedType::IdentifiedType()
  : d(new IdentifiedTypePrivate)
{
}

void IdentifiedType::clear() {
  d->m_declaration = DeclarationPointer();
}

bool IdentifiedType::equals(const IdentifiedType* rhs) const
{
  if( (bool)d->m_declaration != (bool)rhs->d->m_declaration )
    return false;
  if( d->m_declaration == rhs->d->m_declaration )
    return true;
  
  if( !d->m_declaration )
    return false;

  return d->m_declaration.data()->textRange() == rhs->d->m_declaration->textRange()
         && d->m_declaration.data()->url() == rhs->d->m_declaration->url();
}

QualifiedIdentifier IdentifiedType::identifier() const
{
  return d->m_declaration ? d->m_declaration->qualifiedIdentifier() : QualifiedIdentifier();
}

Declaration* IdentifiedType::declaration() const
{
  return d->m_declaration.data();
}

void IdentifiedType::setDeclaration(Declaration* declaration)
{
  d->m_declaration = declaration;
}

QString IdentifiedType::idMangled() const
{
  return identifier().mangled();
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
