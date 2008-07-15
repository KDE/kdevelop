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

EnumeratorTypeData::EnumeratorTypeData()
  : m_value(0)
{
}

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

qint64 EnumeratorType::plainValue() const
{
  return d_func()->m_value;
}

AbstractType* EnumeratorType::clone() const
{
  return new EnumeratorType(*this);
}

bool EnumeratorType::equals(const AbstractType* _rhs) const
{
  const EnumeratorType* rhs = fastCast<const EnumeratorType*>(_rhs);

  if( !rhs )
    return false;

  if( this == rhs )
    return true;

  return IdentifiedType::equals(rhs) && d_func()->m_value == rhs->d_func()->m_value && IntegralType::equals(rhs);
}

QString EnumeratorType::toString() const
{
  QString ret = "enum ";

  switch(dataType()) {
    case TypeNone:
      ret += "none";
    case TypeChar:
      ret += QString("%1").arg((char)d_func()->m_value);
    case TypeWchar_t:
      ret += QString("%1").arg((wchar_t)d_func()->m_value);
    case TypeBoolean:
      ret += d_func()->m_value ? "true" : "false";
    case TypeInt:
      ret += (modifiers() & UnsignedModifier) ? QString("%1u").arg((uint)d_func()->m_value) : QString("%1").arg((int)d_func()->m_value);
    case TypeFloat:
      ret += QString("%1").arg( value<float>() );
    case TypeDouble:
      ret += QString("%1").arg( value<double>() );
    case TypeVoid:
      ret += "void";
    default:
      ret += "<unknown_value>";
  }

  return ret;
}

uint EnumeratorType::hash() const
{
  uint hashValue = IntegralType::hash() + 47 * (uint)d_func()->m_value;
  return 27*IdentifiedType::hash() + 13*hashValue;
}

template<>
void EnumeratorType::setValueInternal<qint64>(qint64 value) {
  if((modifiers() & UnsignedModifier))
    kDebug() << "setValue(signed) called on unsigned type";
  d_func_dynamic()->m_value = value;
}

template<>
void EnumeratorType::setValueInternal<quint64>(quint64 value) {
  if((modifiers() & UnsignedModifier))
    kDebug() << "setValue(unsigned) called on not unsigned type";
  d_func_dynamic()->m_value = (qint64)value;
}

template<>
void EnumeratorType::setValueInternal<float>(float value) {
  if(dataType() != TypeFloat)
    kDebug() << "setValue(float) called on non-float type";
  memcpy(&d_func_dynamic()->m_value, &value, sizeof(float));
}

template<>
void EnumeratorType::setValueInternal<double>(double value) {
  if(dataType() != TypeDouble)
    kDebug() << "setValue(double) called on non-double type";
  memcpy(&d_func_dynamic()->m_value, &value, sizeof(double));
}

}
