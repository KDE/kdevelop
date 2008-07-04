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

class KDEVPLATFORMLANGUAGE_EXPORT AbstractTypeData
{
public:
  AbstractTypeData();
  //While cloning, the dynamic/constant attribute alternates(The copy of dynamic data is constant, and the copy of constant data is dynamic)
  //This means that when copying dynamic data, the size of the allocated buffer must be big enough to hold the appended lists.
  //the AbstractType::copyData function cares about hat.
  AbstractTypeData( const AbstractTypeData& /*rhs*/ );
  ~AbstractTypeData();

  //This must be called from actual class that belongs to this data(not parent classes), and the class must have the
  //"Identity" enumerator with a unique identity. Do NOT call this in copy-constructors!
  template<class T>
  void setTypeClassId() {
    typeClassId = T::Identity;
  }

  uint typeClassId;
  bool inRepository : 1; //Not used for comparison or hashes.

  APPENDED_LISTS_STUB(AbstractTypeData)

  uint classSize() const;
  private:
  AbstractTypeData& operator=(const AbstractTypeData&);
};

class KDEVPLATFORMLANGUAGE_EXPORT IntegralTypeData : public AbstractTypeData
{
public:
  IntegralTypeData();
  IntegralTypeData( const IntegralTypeData& rhs );
  IndexedString m_name;
};

class KDEVPLATFORMLANGUAGE_EXPORT PointerTypeData : public AbstractTypeData
{
public:
  PointerTypeData();
  PointerTypeData( const PointerTypeData& rhs );
  IndexedType m_baseType;
};

class KDEVPLATFORMLANGUAGE_EXPORT ReferenceTypeData : public AbstractTypeData
{
public:
  ReferenceTypeData();
  ReferenceTypeData( const ReferenceTypeData& rhs );
  IndexedType m_baseType;
};

DECLARE_LIST_MEMBER_HASH(FunctionTypeData, m_arguments, IndexedType)

class KDEVPLATFORMLANGUAGE_EXPORT FunctionTypeData : public AbstractTypeData
{
public:
  FunctionTypeData();
  ~FunctionTypeData();
  FunctionTypeData( const FunctionTypeData& rhs );

  IndexedType m_returnType;

  START_APPENDED_LISTS_BASE(FunctionTypeData, AbstractTypeData);

  APPENDED_LIST_FIRST(FunctionTypeData, IndexedType, m_arguments);

  END_APPENDED_LISTS(FunctionTypeData, m_arguments);
  private:
    void operator=(const FunctionTypeData& rhs);
};

class KDEVPLATFORMLANGUAGE_EXPORT StructureTypeData : public AbstractTypeData
{
public:
  StructureTypeData();
  StructureTypeData( const StructureTypeData& rhs );
};

class KDEVPLATFORMLANGUAGE_EXPORT ArrayTypeData : public AbstractTypeData
{
public:
  ArrayTypeData();
  ArrayTypeData( const ArrayTypeData& rhs );
  int m_dimension;
  IndexedType m_elementType;
};

class KDEVPLATFORMLANGUAGE_EXPORT DelayedTypeData : public AbstractTypeData
{
public:
  DelayedTypeData();
  DelayedTypeData( const DelayedTypeData& rhs );
  IndexedTypeIdentifier m_identifier;
  DelayedType::Kind m_kind;
};

}
#endif
