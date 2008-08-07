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

#include "enumeratortype.h"

#include "typesystemdata.h"
#include "typeregister.h"

namespace KDevelop {

REGISTER_TYPE(EnumeratorType);

EnumeratorType::EnumeratorType(const EnumeratorType& rhs)
  : EnumeratorTypeBase(copyData<EnumeratorTypeData>(*rhs.d_func()))
{
}

EnumeratorType::EnumeratorType(EnumeratorTypeData& data)
  : EnumeratorTypeBase(data)
{
}

EnumeratorType::EnumeratorType()
  : EnumeratorTypeBase(createData<EnumeratorTypeData>())
{
  d_func_dynamic()->setTypeClassId<EnumeratorType>();
  IntegralType::setDataType(TypeInt);
  setModifiers(ConstModifier);
}

AbstractType* EnumeratorType::clone() const
{
  return new EnumeratorType(*this);
}

bool EnumeratorType::equals(const AbstractType* _rhs) const
{
  const EnumeratorType* rhs = fastCast<const EnumeratorType*>(_rhs);
  bool ret = false;

  if( !rhs )
    ret = false;

  else if( this == rhs )
    ret = true;

  else
    ret = IdentifiedType::equals(rhs) && ConstantIntegralType::equals(rhs);

  kDebug() << this << rhs << _rhs << ret << toString() << _rhs->toString();
  return ret;
}

uint EnumeratorType::hash() const
{
  return 27*(IdentifiedType::hash() + 13*ConstantIntegralType::hash());
}

AbstractType::WhichType EnumeratorType::whichType() const
{
  return TypeEnumerator;
}

QString EnumeratorType::toString() const
{
  return IdentifiedType::qualifiedIdentifier().toString();
}

}
