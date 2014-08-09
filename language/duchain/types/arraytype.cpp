/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "arraytype.h"

#include "typerepository.h"
#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop
{

REGISTER_TYPE(ArrayType);

ArrayType::ArrayType(const ArrayType& rhs) : AbstractType(copyData<ArrayType>(*rhs.d_func())) {
}

ArrayType::ArrayType(ArrayTypeData& data) : AbstractType(data) {
}

AbstractType* ArrayType::clone() const {
  return new ArrayType(*this);
}

bool ArrayType::equals(const AbstractType* _rhs) const
{
  if (!AbstractType::equals(_rhs))
    return false;

  Q_ASSERT(fastCast<const ArrayType*>(_rhs));

  const ArrayType* rhs = static_cast<const ArrayType*>(_rhs);

  TYPE_D(ArrayType);
  if( d->m_dimension != rhs->d_func()->m_dimension )
    return false;

  return d->m_elementType == rhs->d_func()->m_elementType;
}

ArrayType::ArrayType()
  : AbstractType(createData<ArrayType>())
{
}

ArrayType::~ArrayType()
{
}

int ArrayType::dimension () const
{
  return d_func()->m_dimension;
}

void ArrayType::setDimension(int dimension)
{
  d_func_dynamic()->m_dimension = dimension;
}

AbstractType::Ptr ArrayType::elementType () const
{
  return d_func()->m_elementType.abstractType();
}

void ArrayType::setElementType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_elementType = type->indexed();
}

QString ArrayType::toString() const
{
  if (d_func()->m_dimension == 0) {
    return QString("%1[]").arg(elementType() ? elementType()->toString() : QString("<notype>"));
  }
  return QString("%1[%2]").arg(elementType() ? elementType()->toString() : QString("<notype>")).arg(d_func()->m_dimension);
}

void ArrayType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    {
      acceptType (d_func()->m_elementType.abstractType(), v);
    }

  v->endVisit (this);
}

void ArrayType::exchangeTypes( TypeExchanger* exchanger )
{
  TYPE_D_DYNAMIC(ArrayType);
  d->m_elementType = exchanger->exchange( d->m_elementType.abstractType() )->indexed();
}

AbstractType::WhichType ArrayType::whichType() const
{
  return TypeArray;
}

uint ArrayType::hash() const
{
  return KDevHash(AbstractType::hash())
             << (elementType() ? elementType()->hash() : 0) << dimension();
}

}
