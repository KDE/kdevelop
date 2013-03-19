/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
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

#include "enumeratortype.h"

#include "typesystemdata.h"
#include "typeregister.h"

namespace KDevelop {

REGISTER_TYPE(EnumeratorType);

EnumeratorType::EnumeratorType(const EnumeratorType& rhs)
  : EnumeratorTypeBase(copyData<EnumeratorType>(*rhs.d_func()))
{
}

EnumeratorType::EnumeratorType(EnumeratorTypeData& data)
  : EnumeratorTypeBase(data)
{
}

EnumeratorType::EnumeratorType()
  : EnumeratorTypeBase(createData<EnumeratorType>())
{
  IntegralType::setDataType(TypeInt);
  setModifiers(ConstModifier);
}

AbstractType* EnumeratorType::clone() const
{
  return new EnumeratorType(*this);
}

bool EnumeratorType::equals(const AbstractType* _rhs) const
{
  if( this == _rhs )
    return true;

  if (!EnumeratorTypeBase::equals(_rhs))
    return false;

  Q_ASSERT(fastCast<const EnumeratorType*>(_rhs));

  // Nothing Enumerator-type specific to compare
  return true;
}

uint EnumeratorType::hash() const
{
  return KDevHash(ConstantIntegralType::hash()) << IdentifiedType::hash();
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
