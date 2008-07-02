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
  AbstractTypeData( const AbstractTypeData& /*rhs*/ );
  ///@todo make non-virtual again and make sure the correct destructors are called in other ways
  virtual ~AbstractTypeData();
  
  APPENDED_LISTS_STUB(AbstractTypeData)
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