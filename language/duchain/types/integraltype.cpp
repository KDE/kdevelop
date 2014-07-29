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

#include "integraltype.h"

#include "../indexedstring.h"
#include "typesystemdata.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop
{

REGISTER_TYPE(IntegralType);

IntegralType::IntegralType(const IntegralType& rhs) : AbstractType(copyData<IntegralType>(*rhs.d_func())) {
}

IntegralType::IntegralType(IntegralTypeData& data) : AbstractType(data) {
}

uint IntegralType::dataType() const
{
  return d_func()->m_dataType;
}

void IntegralType::setDataType(uint dataType)
{
  d_func_dynamic()->m_dataType = dataType;
}

AbstractType* IntegralType::clone() const {
  return new IntegralType(*this);
}

bool IntegralType::equals(const AbstractType* _rhs) const
{
  if( this == _rhs )
    return true;

  if (!AbstractType::equals(_rhs))
    return false;

  Q_ASSERT(fastCast<const IntegralType*>(_rhs));

  const IntegralType* rhs = static_cast<const IntegralType*>(_rhs);

  return d_func()->m_dataType == rhs->d_func()->m_dataType;
}

IntegralType::IntegralType(uint type)
  : AbstractType(createData<IntegralType>())
{
  d_func_dynamic()->setTypeClassId<IntegralType>();
  setDataType(type);
}

IntegralType::~IntegralType()
{
}

QString IntegralType::toString() const
{
  TYPE_D(IntegralType);

  QString name;

  switch (d->m_dataType) {
    case TypeChar:
      name = "char";
      break;
    case TypeChar16_t:
      name = "char16_t";
      break;
    case TypeChar32_t:
      name = "char32_t";
      break;
    case TypeWchar_t:
      name = "wchar_t";
      break;
    case TypeBoolean:
      name = "bool";
      break;
    case TypeInt:
      name = "int";
      break;
    case TypeFloat:
      name = "float";
      break;
    case TypeDouble:
      name = "double";
      break;
    case TypeVoid:
      name = "void";
      break;
    case TypeMixed:
      name = "mixed";
      break;
    case TypeString:
      name = "string";
      break;
    case TypeArray:
      name = "array";
      break;
    case TypeNull:
      name = "null";
      break;
    default:
      name = "<unknown>";
      break;
  }

  if (modifiers() & UnsignedModifier)
    name.prepend("unsigned ");
  else if (modifiers() & SignedModifier)
    name.prepend("signed ");

  if (modifiers() & ShortModifier)
    name.prepend("short ");
  else if (modifiers() & LongLongModifier)
    name.prepend("long long ");
  else if (modifiers() & LongModifier)
    name.prepend("long ");

  return AbstractType::toString() + name;
}

void IntegralType::accept0(TypeVisitor *v) const
{
  v->visit (this);
}

AbstractType::WhichType IntegralType::whichType() const
{
  return TypeIntegral;
}

uint IntegralType::hash() const
{
  return KDevHash(AbstractType::hash()) << d_func()->m_dataType;
}

}
