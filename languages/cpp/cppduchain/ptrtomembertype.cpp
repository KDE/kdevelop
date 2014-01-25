/* This file is part of KDevelop
 *    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
 *    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
 *    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *    Copyright 2010 Floris Ruijter <flo.ruijt@hotmail.com> , adaption of pointertype by above-mentioned authors
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License version 2 as published by the Free Software Foundation.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */
#include "ptrtomembertype.h"

#include <language/duchain/indexedstring.h>
#include <language/duchain/repositories/typerepository.h>
#include <language/duchain/types/typesystemdata.h>
#include <language/duchain/types/typeregister.h>
#include <language/duchain/types/typesystem.h>
#include <language/duchain/types/typesystemdata.h>

namespace Cpp {
/// Private data structure for PtrToMemberType
using namespace KDevelop;
class KDEVCPPDUCHAIN_EXPORT PtrToMemberTypeData : public KDevelop::PointerTypeData {
public:
/// Constructor
  PtrToMemberTypeData()
  {}
/// Copy constructor. \param rhs data to copy
  PtrToMemberTypeData ( const PtrToMemberTypeData& rhs )
      : PointerTypeData ( rhs )
      , m_classType ( rhs.m_classType ) {}
/// Type of data at which the pointer points
  KDevelop::IndexedType m_classType;
};

REGISTER_TYPE ( PtrToMemberType );

PtrToMemberType::PtrToMemberType ( const PtrToMemberType& rhs )
    : PointerType ( copyData<PtrToMemberType> ( *rhs.d_func() ) ) {
}

PtrToMemberType::PtrToMemberType ( PtrToMemberTypeData& data )
    : PointerType ( data ) {
}

PtrToMemberType::PtrToMemberType()
    : PointerType ( createData<PtrToMemberType>() ) {
}

AbstractType* PtrToMemberType::clone() const {
  return new PtrToMemberType ( *this );
}


bool PtrToMemberType::equals ( const AbstractType* _rhs ) const {
  if ( this == _rhs )
    return true;

  if ( !PointerType::equals ( _rhs ) )
    return false;

  Q_ASSERT ( KDevelop::fastCast<const PtrToMemberType*> ( _rhs ) );

  const PtrToMemberType* rhs = static_cast<const PtrToMemberType*> ( _rhs );

  return d_func()->m_classType == rhs->d_func()->m_classType;
}

void PtrToMemberType::accept0 ( TypeVisitor *v ) const {
  BaseType::accept0 ( v );
  if ( v->visit ( this ) )
    acceptType ( d_func()->m_classType.abstractType(), v );
  v->endVisit ( this );
}

void PtrToMemberType::exchangeTypes ( TypeExchanger* exchanger ) {
  BaseType::exchangeTypes ( exchanger );
  d_func_dynamic()->m_classType = exchanger->exchange ( d_func()->m_classType.abstractType() )->indexed();
}

PtrToMemberType::~PtrToMemberType() {
}

AbstractType::Ptr PtrToMemberType::classType() const {
  return d_func()->m_classType.abstractType();
}

void PtrToMemberType::setClassType ( AbstractType::Ptr type ) {
  d_func_dynamic()->m_classType = type->indexed();
}

QString PtrToMemberType::toString() const {
  QString baseString  = ( baseType()  ? baseType()->toString()  : "<notype>" );
  QString classString = ( classType() ? classType()->toString() : "<notype>" );
  return QString ( "%1 %2::*" ).arg ( baseString,classString ) + AbstractType::toString ( true );
}

AbstractType::WhichType PtrToMemberType::whichType() const {
  return TypePointer; //a bit of a kludge, ptr-to-member is unique.
}

uint PtrToMemberType::hash() const {
  return PointerType::hash() + d_func()->m_classType.hash() * 17  ;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
