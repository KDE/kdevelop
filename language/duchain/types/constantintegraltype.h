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

#ifndef KDEVPLATFORM_CONSTANTINTEGRALTYPE_H
#define KDEVPLATFORM_CONSTANTINTEGRALTYPE_H

#include "integraltype.h"
#include "typesystemdata.h"

namespace KDevelop
{

template<typename T>
T constant_value(const qint64* realval)
{
  T value;
  memcpy(&value, realval, sizeof(T));
  return value;
}

class KDEVPLATFORMLANGUAGE_EXPORT ConstantIntegralType : public IntegralType
{
public:
  ConstantIntegralType(const ConstantIntegralType& rhs);

  ConstantIntegralType(ConstantIntegralTypeData& data);

  ConstantIntegralType(uint type = TypeNone);

  typedef TypePtr<ConstantIntegralType> Ptr;

    /**The types and modifiers are not changed!
   * The values are casted internally to the local representation, so you can lose precision.
   * */
  template<class ValueType>
  void setValue(ValueType value) {
    if(AbstractType::modifiers() & UnsignedModifier)
      setValueInternal<quint64>(value);
    else if(IntegralType::dataType() == TypeFloat)
      setValueInternal<float>(value);
    else if(IntegralType::dataType() == TypeDouble)
      setValueInternal<double>(value);
    else
      setValueInternal<qint64>(value);
  }

  /**
   * For booleans, the value is 1 for true, and 0 for false.
   * All signed values should be retrieved and set through value(),
   *
   * */
  template<class ValueType>
  ValueType value() const {
    if(modifiers() & UnsignedModifier) {
      return constant_value<quint64>(&d_func()->m_value);
    } else if(dataType() == TypeFloat) {
      return constant_value<float>(&d_func()->m_value);
    } else if(dataType() == TypeDouble) {
      return constant_value<double>(&d_func()->m_value);
    } else {
      return constant_value<qint64>(&d_func()->m_value);
    }
  }

  qint64 plainValue() const;

  virtual QString toString() const;

  QString valueAsString() const;

  virtual bool equals(const KDevelop::AbstractType* rhs) const;

  virtual KDevelop::AbstractType* clone() const;

  virtual uint hash() const;

  enum {
    Identity = 14
  };

  typedef ConstantIntegralTypeData Data;

protected:
  TYPE_DECLARE_DATA(ConstantIntegralType);

private:
  //Sets the value without casting
  template<class ValueType>
  void setValueInternal(ValueType value);
};

template<>
inline ConstantIntegralType* fastCast<ConstantIntegralType*>(AbstractType* from) {
  if(!from || from->whichType() != KDevelop::AbstractType::TypeIntegral)
    return 0;
  else
    return dynamic_cast<ConstantIntegralType*>(from);
}

}


#endif // KDEVPLATFORM_CONSTANTINTEGRALTYPE_H
