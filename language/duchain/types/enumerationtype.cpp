/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>

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

#include "enumerationtype.h"

#include "typesystemdata.h"
#include "typeregister.h"

namespace KDevelop {

//Because all these classes have no d-pointers, shallow copies are perfectly fine

REGISTER_TYPE(EnumerationType);

EnumerationType::EnumerationType(const EnumerationType& rhs)
  : EnumerationTypeBase(copyData<EnumerationTypeData>(*rhs.d_func()))
{
}

EnumerationType::EnumerationType(EnumerationTypeData& data)
  : EnumerationTypeBase(data)
{
}

AbstractType* EnumerationType::clone() const {
  return new EnumerationType(*this);
}

bool EnumerationType::equals(const AbstractType* _rhs) const
{
  const EnumerationType* rhs = fastCast<const EnumerationType*>(_rhs);
  bool ret = false;
  if( !rhs )
    ret = false;

  else if( this == rhs )
    ret = true;

  else
    ret = IdentifiedType::equals(rhs) && IntegralType::equals(rhs);

  kDebug() << this << rhs << _rhs << ret << toString() << _rhs->toString();

  return ret;
}

EnumerationType::EnumerationType()
  : EnumerationTypeBase(createData<EnumerationTypeData>())
{
  d_func_dynamic()->setTypeClassId<EnumerationType>();
  IntegralType::setDataType(TypeInt);
}

QString EnumerationType::toString() const
{
  return "enum " + qualifiedIdentifier().toString();
}

uint EnumerationType::hash() const
{
  return IdentifiedType::hash() + 57*IntegralType::hash();
}

AbstractType::WhichType EnumerationType::whichType() const
{
  return TypeEnumeration;
}

}
