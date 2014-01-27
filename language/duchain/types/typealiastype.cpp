/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "typealiastype.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop {

REGISTER_TYPE(TypeAliasType);

AbstractType* TypeAliasType::clone() const {
  return new TypeAliasType(*this);
}

bool TypeAliasType::equals(const AbstractType* _rhs) const
{
  if( !fastCast<const TypeAliasType*>(_rhs))
    return false;
  const TypeAliasType* rhs = static_cast<const TypeAliasType*>(_rhs);

  if( this == rhs )
    return true;

  if( AbstractType::equals(rhs) && IdentifiedType::equals(rhs) )
  {
    if( (bool)d_func()->m_type != (bool)rhs->d_func()->m_type )
      return false;

    if( !d_func()->m_type )
      return true;

    return d_func()->m_type == rhs->d_func()->m_type;

  } else {
    return false;
  }
}
AbstractType::Ptr TypeAliasType::type() const
{
  return d_func()->m_type.abstractType();
}

void TypeAliasType::setType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_type = type->indexed();
}

uint TypeAliasType::hash() const
{
  return KDevHash(AbstractType::hash()) << IdentifiedType::hash() << d_func()->m_type.hash();
}

QString TypeAliasType::toString() const
{
  QualifiedIdentifier id = qualifiedIdentifier();
  if (!id.isEmpty())
    return  AbstractType::toString(false) + id.toString();

  if (type())
    return AbstractType::toString(false) + type()->toString();

  return "typedef <notype>";
}

void TypeAliasType::accept0 (KDevelop::TypeVisitor *v) const
{
  if (v->visit (this))
    acceptType (d_func()->m_type.abstractType(), v);

//     v->endVisit (this);
}

KDevelop::AbstractType::WhichType TypeAliasType::whichType() const {
  return TypeAlias;
}

void TypeAliasType::exchangeTypes(KDevelop::TypeExchanger* exchanger) {
  d_func_dynamic()->m_type = exchanger->exchange( d_func()->m_type.abstractType() )->indexed();
}


}
