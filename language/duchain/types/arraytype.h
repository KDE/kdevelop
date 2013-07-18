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

#ifndef KDEVPLATFORM_ARRAYTYPE_H
#define KDEVPLATFORM_ARRAYTYPE_H

#include "abstracttype.h"

namespace KDevelop
{
class ArrayTypeData;

class KDEVPLATFORMLANGUAGE_EXPORT ArrayType : public AbstractType
{
public:
  typedef TypePtr<ArrayType> Ptr;

  /// Default constructor
  ArrayType();
  /// Copy constructor. \param rhs type to copy
  ArrayType(const ArrayType& rhs);
  /// Constructor using raw data. \param data internal data.
  ArrayType(ArrayTypeData& data);
  /// Destructor
  virtual ~ArrayType();

  virtual AbstractType* clone() const;

  virtual bool equals(const AbstractType* rhs) const;

  /**
   * Retrieve the dimension of this array type. Multiple-dimensioned
   * arrays will have another array type as their elementType().
   *
   * \returns the dimension of the array, or zero if the array is dimensionless (eg. int[])
   */
  int dimension () const;

  /**
   * Set this array type's dimension.
   * If @p dimension is zero, the array is considered dimensionless (eg. int[]).
   *
   * \param dimension new dimension, set to zero for a dimensionless type (eg. int[])
   */
  void setDimension(int dimension);

  /**
   * Retrieve the element type of the array, e.g. "int" for int[3].
   *
   * \returns the element type.
   */
  AbstractType::Ptr elementType () const;

  /**
   * Set the element type of the array, e.g. "int" for int[3].
   *
   * \returns the element type.
   */
  void setElementType(AbstractType::Ptr type);

  virtual QString toString() const;

  virtual uint hash() const;

  virtual WhichType whichType() const;

  virtual void exchangeTypes( TypeExchanger* exchanger );

  enum {
    Identity = 7
  };

  typedef ArrayTypeData Data;

protected:
  virtual void accept0 (TypeVisitor *v) const;

  TYPE_DECLARE_DATA(ArrayType)
};

template<>
inline ArrayType* fastCast<ArrayType*>(AbstractType* from) {
  if(!from || from->whichType() != AbstractType::TypeArray)
    return 0;
  else
    return static_cast<ArrayType*>(from);
}

}

#endif // KDEVPLATFORM_TYPESYSTEM_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
