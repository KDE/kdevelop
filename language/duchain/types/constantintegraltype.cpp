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

#include "constantintegraltype.h"

#include "typesystemdata.h"
#include "typeregister.h"

namespace KDevelop {

REGISTER_TYPE(ConstantIntegralType);

ConstantIntegralType::ConstantIntegralType(const ConstantIntegralType& rhs)
  : IntegralType(copyData<ConstantIntegralType>(*rhs.d_func()))
{
}

ConstantIntegralType::ConstantIntegralType(ConstantIntegralTypeData& data)
  : IntegralType(data)
{
}

ConstantIntegralType::ConstantIntegralType(uint type)
  : IntegralType(createData<ConstantIntegralType>())
{
  setDataType(type);
  setModifiers(ConstModifier);
}

qint64 ConstantIntegralType::plainValue() const
{
  return d_func()->m_value;
}

AbstractType* ConstantIntegralType::clone() const
{
  return new ConstantIntegralType(*this);
}

bool ConstantIntegralType::equals(const AbstractType* _rhs) const
{
  if( this == _rhs )
    return true;

  if (!IntegralType::equals(_rhs))
    return false;

  Q_ASSERT(fastCast<const ConstantIntegralType*>(_rhs));

  const ConstantIntegralType* rhs = static_cast<const ConstantIntegralType*>(_rhs);

  return d_func()->m_value == rhs->d_func()->m_value;
}

QString ConstantIntegralType::toString() const
{
  QString ret;

  switch(dataType()) {
    case TypeNone:
      ret += "none";
      break;
    case TypeChar:
      ret += QString("char");
      break;
    case TypeWchar_t:
      ret += QString("wchar_t");
      break;
    case TypeChar16_t:
      ret += QString("char16_t");
      break;
    case TypeChar32_t:
      ret += QString("char32_t");
      break;
    case TypeBoolean:
      ret += d_func()->m_value ? "true" : "false";
      break;
    case TypeInt:
      ret += (modifiers() & UnsignedModifier) ? QString("unsigned") : QString("int");
      break;
    case TypeFloat:
      ret += QString("float");
      break;
    case TypeDouble:
      ret += QString("double");
      break;
    case TypeVoid:
      ret += "void";
      break;
    default:
      ret += "<unknown_value>";
      break;
  }

  const QString valueAsString = plainValueAsString();
  if (!valueAsString.isEmpty()) {
    ret += QString("(%1)").arg(valueAsString);
  }

  return ret;
}

QString ConstantIntegralType::plainValueAsString() const
{
  switch(dataType()) {
    case TypeNone:
      return "none";
    case TypeChar:
      return QString::number((char)d_func()->m_value);
    case TypeWchar_t:
      return QString::number((wchar_t)d_func()->m_value);
    case TypeChar16_t:
      return QString::number((char16_t)d_func()->m_value);
    case TypeChar32_t:
      return QString::number((char32_t)d_func()->m_value);
    case TypeBoolean:
      return d_func()->m_value ? "true" : "false";
    case TypeInt:
      return (modifiers() & UnsignedModifier) ?
        QString::number((uint)d_func()->m_value) :
        QString::number((int)d_func()->m_value);
    case TypeFloat:
      return QString::number(value<float>());
    case TypeDouble:
      return QString::number(value<double>());
    default:
      return QString();
  }
}

uint ConstantIntegralType::hash() const
{
  return KDevHash(IntegralType::hash()) << d_func()->m_value;
}

template<>
KDEVPLATFORMLANGUAGE_EXPORT
void ConstantIntegralType::setValueInternal<qint64>(qint64 value) {
  if((modifiers() & UnsignedModifier)) {
    kDebug() << "setValue(signed) called on unsigned type";
  }
  d_func_dynamic()->m_value = value;
}

template<>
KDEVPLATFORMLANGUAGE_EXPORT
void ConstantIntegralType::setValueInternal<quint64>(quint64 value) {
  if(!(modifiers() & UnsignedModifier)) {
    kDebug() << "setValue(unsigned) called on not unsigned type";
  }
  d_func_dynamic()->m_value = (qint64)value;
}

template<>
KDEVPLATFORMLANGUAGE_EXPORT
void ConstantIntegralType::setValueInternal<float>(float value) {
  if(dataType() != TypeFloat) {
    kDebug() << "setValue(float) called on non-float type";
  }
  memcpy(&d_func_dynamic()->m_value, &value, sizeof(float));
}

template<>
KDEVPLATFORMLANGUAGE_EXPORT
void ConstantIntegralType::setValueInternal<double>(double value) {
  if(dataType() != TypeDouble) {
    kDebug() << "setValue(double) called on non-double type";
  }
  memcpy(&d_func_dynamic()->m_value, &value, sizeof(double));
}

}
