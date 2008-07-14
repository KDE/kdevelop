/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
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

#ifndef INTEGRALTYPE_H
#define INTEGRALTYPE_H

#include "language/duchain/types/abstracttype.h"

namespace KDevelop
{
class IntegralTypeData;
class IndexedString;

/**
  * Enumeration of frequently used integral types.
  * If your language has another integral type not listed here,
  * you can create custom types staring from TypeLanguageSpecific.
  */
namespace CommonIntegralTypes {
  enum {
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
    TypeLanguageSpecific = 200
  };
}

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

  /// Default constructor
  IntegralType();
  /// Copy constructor. \param rhs type to copy
  IntegralType(const IntegralType& rhs);
  /// Constructor for named integral types. \param name name of this type
  IntegralType(const IndexedString& name);
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

  /// Access the name of this type. \returns the type's name.
  const IndexedString& name() const;

  /// Set the name of this type.  \param name the type's name.
  void setName(const IndexedString& name);

  /// Equivalence operator. \param other other integral type to compare. \returns true if types are equal, otherwise false
  bool operator == (const IntegralType &other) const;

  /// Not equal operator. \param other other integral type to compare. \returns false if types are equal, otherwise false
  bool operator != (const IntegralType &other) const;

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

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
