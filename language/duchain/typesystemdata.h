/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#ifndef TYPESYSTEMDATA_H
#define TYPESYSTEMDATA_H

#include "appendedlist.h"
#include "typesystem.h"
#include "indexedstring.h"
#include "../languageexport.h"

namespace KDevelop {

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(FunctionTypeData, m_arguments, IndexedType)

/**
 * Private data structure for AbstractType.
 *
 * Inherit from this for custom type private data.
 */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractTypeData
{
public:
  /// Constructor.
  AbstractTypeData();
  /**
   * Copy constructor.
   *
   * While cloning, the dynamic/constant attribute alternates(The copy of dynamic data is constant, and the copy of constant data is dynamic)
   * This means that when copying dynamic data, the size of the allocated buffer must be big enough to hold the appended lists.
   * the AbstractType::copyData function cares about that.
   *
   * \param rhs data to copy.
   */
  AbstractTypeData( const AbstractTypeData& rhs );
  /// Destructor.
  ~AbstractTypeData();

  /**
   * Internal setup for the data structure.
   *
   * This must be called from actual class that belongs to this data(not parent classes), and the class must have the
   * "Identity" enumerator with a unique identity. Do NOT call this in copy-constructors!
   */
  template<class T>
  void setTypeClassId() {
    typeClassId = T::Identity;
  }

  /// Remember which type this data was created for. \sa setTypeClassId()
  uint typeClassId;

  /// Remember whether this type is in a TypeRepository. Not used for comparison or hashes.
  bool inRepository : 1;

  APPENDED_LISTS_STUB(AbstractTypeData)

  /// Returns the data size of this class.
  uint classSize() const;

private:
  AbstractTypeData& operator=(const AbstractTypeData&);
};

/// Private data structure for IntegralType
class KDEVPLATFORMLANGUAGE_EXPORT IntegralTypeData : public AbstractTypeData
{
public:
  /// Constructor
  IntegralTypeData();
  /// Copy constructor. \param rhs data to copy
  IntegralTypeData( const IntegralTypeData& rhs );
  /// Name of the integral type.
  IndexedString m_name;
};

/// Private data structure for PointerType
class KDEVPLATFORMLANGUAGE_EXPORT PointerTypeData : public AbstractTypeData
{
public:
  /// Constructor
  PointerTypeData();
  /// Copy constructor. \param rhs data to copy
  PointerTypeData( const PointerTypeData& rhs );
  /// Type of data at which the pointer points
  IndexedType m_baseType;
};

/// Private data structure for ReferenceType
class KDEVPLATFORMLANGUAGE_EXPORT ReferenceTypeData : public AbstractTypeData
{
public:
  /// Constructor
  ReferenceTypeData();
  /// Copy constructor. \param rhs data to copy
  ReferenceTypeData( const ReferenceTypeData& rhs );
  /// Type of data which is referenced
  IndexedType m_baseType;
};

DECLARE_LIST_MEMBER_HASH(FunctionTypeData, m_arguments, IndexedType)

/// Private data structure for FunctionType
class KDEVPLATFORMLANGUAGE_EXPORT FunctionTypeData : public AbstractTypeData
{
public:
  /// Constructor
  FunctionTypeData();
  /// Copy constructor. \param rhs data to copy
  FunctionTypeData( const FunctionTypeData& rhs );
  /// Destructor
  ~FunctionTypeData();

  /// Function return type
  IndexedType m_returnType;

  START_APPENDED_LISTS_BASE(FunctionTypeData, AbstractTypeData);

  APPENDED_LIST_FIRST(FunctionTypeData, IndexedType, m_arguments);

  END_APPENDED_LISTS(FunctionTypeData, m_arguments);
  private:
    void operator=(const FunctionTypeData& rhs);
};

/// Private data structure for StructureType
class KDEVPLATFORMLANGUAGE_EXPORT StructureTypeData : public AbstractTypeData
{
public:
  /// Constructor
  StructureTypeData();
  /// Copy constructor. \param rhs data to copy
  StructureTypeData( const StructureTypeData& rhs );
};

/// Private data structure for ArrayType
class KDEVPLATFORMLANGUAGE_EXPORT ArrayTypeData : public AbstractTypeData
{
public:
  /// Constructor
  ArrayTypeData();
  /// Copy constructor. \param rhs data to copy
  ArrayTypeData( const ArrayTypeData& rhs );
  /// Dimension of the array
  int m_dimension;
  /// Element type of the array
  IndexedType m_elementType;
};

/// Private data structure for DelayedType
class KDEVPLATFORMLANGUAGE_EXPORT DelayedTypeData : public AbstractTypeData
{
public:
  /// Constructor
  DelayedTypeData();
  /// Copy constructor. \param rhs data to copy
  DelayedTypeData( const DelayedTypeData& rhs );
  /// Identifier of the delayed type
  IndexedTypeIdentifier m_identifier;
  /// Type of delay in resolving the type
  DelayedType::Kind m_kind;
};

}
#endif
