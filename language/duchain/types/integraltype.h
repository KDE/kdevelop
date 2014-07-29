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

#ifndef KDEVPLATFORM_INTEGRALTYPE_H
#define KDEVPLATFORM_INTEGRALTYPE_H

#include "abstracttype.h"
#include "typesystemdata.h"

namespace KDevelop
{
class IntegralTypeData;
class IndexedString;

/**
 * \short A type representing inbuilt data types.
 *
 * IntegralType is used to represent types which are native to a programming languge,
 * such as (e.g.) int, float, double, char, bool etc.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IntegralType: public AbstractType
{
public:
  typedef TypePtr<IntegralType> Ptr;

  /**
   * Enumeration of frequently used integral types.
   * If your language has another integral type not listed here,
   * you can create custom types staring from TypeLanguageSpecific.
   */
  enum CommonIntegralTypes {
    TypeVoid,
    TypeNone,
    TypeNull,
    TypeChar,
    TypeBoolean,
    TypeByte,
    TypeSbyte,
    TypeShort,
    TypeInt,
    TypeLong,
    TypeFloat,
    TypeDouble,
    TypeWchar_t,
    TypeString,
    TypeMixed,
    TypeChar16_t,
    TypeChar32_t,
    TypeLanguageSpecific = 200
  };

  /// Default constructor
  IntegralType(uint type = TypeNone);
  /// Copy constructor. \param rhs type to copy
  IntegralType(const IntegralType& rhs);
  /// Constructor using raw data. \param data internal data.
  IntegralType(IntegralTypeData& data);
  /// Destructor
  virtual ~IntegralType();

  /**
   * Access the integral type
   *
   * \returns the type's modifiers.
   */
  uint dataType() const;

  /**
   * Set the type's modifiers.
   *
   * \param modifiers modifiers of this type.
   */
  void setDataType(uint dataType);

  /**
   * TODO: think of a way to make @c toString work properly for custom data types
   *       right now you need to create a custom type and overload it...
   */
  virtual QString toString() const;

  virtual uint hash() const;

  virtual WhichType whichType() const;

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  enum {
    Identity = 2
  };

  typedef IntegralTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const;

  TYPE_DECLARE_DATA(IntegralType)
};

template<>
inline IntegralType* fastCast<IntegralType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeIntegral)
    return 0;
  else
    return static_cast<IntegralType*>(from);
}

}

#endif
