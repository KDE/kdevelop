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

#include "referencetype.h"

#include "../indexedstring.h"
#include "../repositories/typerepository.h"
#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"
#include "integraltype.h"
#include "structuretype.h"

namespace KDevelop
{
REGISTER_TYPE(ReferenceType);

ReferenceType::ReferenceType(const ReferenceType& rhs) : AbstractType(copyData<ReferenceType>(*rhs.d_func())) {
}

ReferenceType::ReferenceType(ReferenceTypeData& data) : AbstractType(data) {
}

AbstractType* ReferenceType::clone() const {
  return new ReferenceType(*this);
}

bool ReferenceType::equals(const AbstractType* _rhs) const
{
  if( this == _rhs )
    return true;

  if (!AbstractType::equals(_rhs))
    return false;

  Q_ASSERT(fastCast<const ReferenceType*>(_rhs));

  const ReferenceType* rhs = static_cast<const ReferenceType*>(_rhs);

  return d_func()->m_baseType == rhs->d_func()->m_baseType;
}

ReferenceType::ReferenceType()
  : AbstractType(createData<ReferenceType>())
{
}

ReferenceType::~ReferenceType()
{
}

AbstractType::Ptr ReferenceType::baseType () const
{
  return d_func()->m_baseType.abstractType();
}

void ReferenceType::setBaseType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_baseType = type->indexed();
}

bool ReferenceType::isRValue() const
{
  return d_func()->m_isRValue;
}

void ReferenceType::setIsRValue(bool isRValue)
{
  d_func_dynamic()->m_isRValue = isRValue;
}

void ReferenceType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    acceptType (d_func()->m_baseType.abstractType(), v);

  v->endVisit (this);
}

void ReferenceType::exchangeTypes( TypeExchanger* exchanger )
{
  d_func_dynamic()->m_baseType = exchanger->exchange( d_func()->m_baseType.abstractType() )->indexed();
}

QString ReferenceType::toString() const
{
  AbstractType::Ptr base = baseType();
  QString baseString = (base ? base->toString() : "<notype>");
  const QString ampersands = d_func()->m_isRValue ? "&&" : "&";
  if(base.cast<IntegralType>() || base.cast<StructureType>())
    return AbstractType::toString(false) + baseString + ampersands;
  else
    return baseString + AbstractType::toString(true) + ampersands;
}

AbstractType::WhichType ReferenceType::whichType() const
{
  return TypeReference;
}

uint ReferenceType::hash() const
{
  return KDevHash(AbstractType::hash()) << d_func()->m_baseType.hash() << d_func()->m_isRValue;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
