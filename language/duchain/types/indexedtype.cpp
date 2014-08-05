/* This file is part of KDevelop
   Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "indexedtype.h"

#include "../repositories/typerepository.h"
#include "../referencecounting.h"

namespace KDevelop
{

IndexedType::IndexedType(uint index) : m_index(index) {
  if(m_index && shouldDoDUChainReferenceCounting(this))
    TypeRepository::increaseReferenceCount(m_index, this);
}

IndexedType::IndexedType(const IndexedType& rhs) : m_index(rhs.m_index) {
  if(m_index && shouldDoDUChainReferenceCounting(this))
    TypeRepository::increaseReferenceCount(m_index, this);
}

IndexedType::~IndexedType() {
  if(m_index && shouldDoDUChainReferenceCounting(this))
    TypeRepository::decreaseReferenceCount(m_index, this);
}

IndexedType& IndexedType::operator=(const IndexedType& rhs) {
  
  if(m_index && shouldDoDUChainReferenceCounting(this))
    TypeRepository::decreaseReferenceCount(m_index, this);

  m_index = rhs.m_index;
  
  if(m_index && shouldDoDUChainReferenceCounting(this))
    TypeRepository::increaseReferenceCount(m_index, this);
  
  return *this;
}

AbstractType::Ptr IndexedType::abstractType() const {
  if(!m_index)
    return AbstractType::Ptr();
  return TypeRepository::typeForIndex(m_index);
}
}
