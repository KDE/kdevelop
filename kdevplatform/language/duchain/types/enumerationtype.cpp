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

#include "enumerationtype.h"

#include "typesystemdata.h"
#include "typeregister.h"

namespace KDevelop {

//Because all these classes have no d-pointers, shallow copies are perfectly fine

REGISTER_TYPE(EnumerationType);

EnumerationType::EnumerationType(const EnumerationType& rhs)
  : EnumerationTypeBase(copyData<EnumerationType>(*rhs.d_func()))
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
  if( this == _rhs )
    return true;

  if (!EnumerationTypeBase::equals(_rhs))
    return false;

  Q_ASSERT(fastCast<const EnumerationType*>(_rhs));

  // Nothing enumeration type-specific to compare
  return true;
}

EnumerationType::EnumerationType()
  : EnumerationTypeBase(createData<EnumerationType>())
{
  IntegralType::setDataType(TypeInt);
}

QString EnumerationType::toString() const
{
  return qualifiedIdentifier().toString();
}

uint EnumerationType::hash() const
{
  return KDevHash(IntegralType::hash()) << IdentifiedType::hash();
}

AbstractType::WhichType EnumerationType::whichType() const
{
  return TypeEnumeration;
}

}
