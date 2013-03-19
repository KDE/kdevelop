/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#include "unsuretype.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop {

REGISTER_TYPE(UnsureType);
DEFINE_LIST_MEMBER_HASH(UnsureTypeData, m_types, IndexedType)

UnsureType::UnsureType(const KDevelop::UnsureType& rhs): AbstractType(copyData<UnsureType>(*rhs.d_func())) {
}

UnsureType::UnsureType() : AbstractType(createData<UnsureType>()) {
}

void UnsureType::accept0(KDevelop::TypeVisitor* v) const {
  FOREACH_FUNCTION(const IndexedType& type, d_func()->m_types) {
    AbstractType::Ptr t = type.abstractType();
    v->visit(t.unsafeData());
  }
}

KDevelop::AbstractType* UnsureType::clone() const {
  return new UnsureType(*this);
}

QString UnsureType::toString() const {
  QString ret = "unsure (";
  bool first = true;
  FOREACH_FUNCTION(const IndexedType& type, d_func()->m_types) {
    if(!first)
      ret += ", ";
    first = false;
    
    AbstractType::Ptr t = type.abstractType();
    if(t)
      ret += t->toString();
    else
      ret += "none";
  }
  ret += ')';
  
  return ret;
}

bool UnsureType::equals(const KDevelop::AbstractType* rhs) const {
  const UnsureType* rhsU = dynamic_cast<const UnsureType*>(rhs);
  if(!rhsU)
    return false;
  if(d_func()->typeClassId != rhsU->d_func()->typeClassId)
    return false;
  if(d_func()->m_typesSize() != rhsU->d_func()->m_typesSize())
    return false;
  
  for(uint a = 0; a < d_func()->m_typesSize(); ++a)
    if(d_func()->m_types()[a] != rhsU->d_func()->m_types()[a])
      return false;
  
  return KDevelop::AbstractType::equals(rhs);
}

uint UnsureType::hash() const {
  KDevHash kdevhash(AbstractType::hash());
  FOREACH_FUNCTION(const IndexedType& type, d_func()->m_types)
    kdevhash << type.hash();
  return kdevhash << d_func()->m_typesSize();
}

KDevelop::AbstractType::WhichType UnsureType::whichType() const {
  return TypeUnsure;
}

void UnsureType::exchangeTypes(KDevelop::TypeExchanger* exchanger) {
  for(uint a = 0; a < d_func()->m_typesSize(); ++a) {
    AbstractType::Ptr from = d_func()->m_types()[a].abstractType();
    AbstractType::Ptr exchanged = exchanger->exchange(from);
    if(exchanged != from)
      d_func_dynamic()->m_typesList()[a] = exchanged->indexed();
  }
  
  KDevelop::AbstractType::exchangeTypes(exchanger);
}

void UnsureType::addType(KDevelop::IndexedType type) {
  if ( !d_func_dynamic()->m_typesList().contains(type) ) {
    d_func_dynamic()->m_typesList().append(type);
  }
}

void UnsureType::removeType(KDevelop::IndexedType type) {
  d_func_dynamic()->m_typesList().removeOne(type);
}

const KDevelop::IndexedType* UnsureType::types() const {
  return d_func()->m_types();
}

uint UnsureType::typesSize() const {
  return d_func()->m_typesSize();
}

UnsureType::UnsureType(KDevelop::UnsureTypeData& data): AbstractType(data) {
}

}
