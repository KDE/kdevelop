/*
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "indexedtype.h"

#include "typerepository.h"
#include <serialization/referencecounting.h>

namespace KDevelop {
IndexedType::IndexedType(const AbstractType* type)
    : m_index(TypeRepository::indexForType(type))
{
    if (m_index && shouldDoDUChainReferenceCounting(this))
        TypeRepository::increaseReferenceCount(m_index, this);
}

IndexedType::IndexedType(uint index) : m_index(index)
{
    if (m_index && shouldDoDUChainReferenceCounting(this))
        TypeRepository::increaseReferenceCount(m_index, this);
}

IndexedType::IndexedType(const IndexedType& rhs) : m_index(rhs.m_index)
{
    if (m_index && shouldDoDUChainReferenceCounting(this))
        TypeRepository::increaseReferenceCount(m_index, this);
}

IndexedType::~IndexedType()
{
    if (m_index && shouldDoDUChainReferenceCounting(this))
        TypeRepository::decreaseReferenceCount(m_index, this);
}

IndexedType& IndexedType::operator=(const IndexedType& rhs)
{
    if (m_index && shouldDoDUChainReferenceCounting(this))
        TypeRepository::decreaseReferenceCount(m_index, this);

    m_index = rhs.m_index;

    if (m_index && shouldDoDUChainReferenceCounting(this))
        TypeRepository::increaseReferenceCount(m_index, this);

    return *this;
}

AbstractType::Ptr IndexedType::abstractType() const
{
    if (!m_index)
        return AbstractType::Ptr();
    return TypeRepository::typeForIndex(m_index);
}
}
