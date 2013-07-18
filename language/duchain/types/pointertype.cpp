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

#include "pointertype.h"

#include "../indexedstring.h"
#include "../repositories/typerepository.h"
#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop
{

REGISTER_TYPE(PointerType);

PointerType::PointerType(const PointerType& rhs) : AbstractType(copyData<PointerType>(*rhs.d_func())) {
}

PointerType::PointerType(PointerTypeData& data) : AbstractType(data) {
}

AbstractType* PointerType::clone() const {
  return new PointerType(*this);
}

bool PointerType::equals(const AbstractType* _rhs) const
{
  if( this == _rhs )
    return true;

  if (!AbstractType::equals(_rhs))
    return false;

  Q_ASSERT(fastCast<const PointerType*>(_rhs));

  const PointerType* rhs = static_cast<const PointerType*>(_rhs);

  return d_func()->m_baseType == rhs->d_func()->m_baseType;
}

PointerType::PointerType()
  : AbstractType(createData<PointerType>())
{
}

void PointerType::accept0 (TypeVisitor *v) const
{
  if (v->visit (this))
    acceptType (d_func()->m_baseType.abstractType(), v);

  v->endVisit (this);
}

void PointerType::exchangeTypes( TypeExchanger* exchanger ) {
  d_func_dynamic()->m_baseType = exchanger->exchange( d_func()->m_baseType.abstractType() )->indexed();
}

PointerType::~PointerType()
{
}

AbstractType::Ptr PointerType::baseType () const
{
  return d_func()->m_baseType.abstractType();
}

void PointerType::setBaseType(AbstractType::Ptr type)
{
  d_func_dynamic()->m_baseType = type->indexed();
}

QString PointerType::toString() const
{
  QString baseString = (baseType() ? baseType()->toString() : "<notype>");
  return QString("%1*").arg(baseString) + AbstractType::toString(true);
}

AbstractType::WhichType PointerType::whichType() const
{
  return TypePointer;
}

uint PointerType::hash() const
{
  return KDevHash(AbstractType::hash()) << d_func()->m_baseType.hash();
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
