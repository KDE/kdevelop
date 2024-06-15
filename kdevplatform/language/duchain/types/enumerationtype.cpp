/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "enumerationtype.h"

#include "typesystemdata.h"
#include "typeregister.h"

namespace KDevelop {
//Because all these classes have no d-pointers, shallow copies are perfectly fine

REGISTER_TYPE(EnumerationType);

EnumerationType::EnumerationType(const EnumerationType& rhs)
    : EnumerationTypeBase(copyData<EnumerationType>(*rhs.d_func()))
{
}

EnumerationType::EnumerationType(EnumerationTypeData& data)
    : EnumerationTypeBase(data)
{
}

AbstractType* EnumerationType::clone() const
{
    return new EnumerationType(*this);
}

bool EnumerationType::equals(const AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!EnumerationTypeBase::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const EnumerationType*>(_rhs));

    // Nothing enumeration type-specific to compare
    return true;
}

EnumerationType::EnumerationType()
    : EnumerationTypeBase(createData<EnumerationType>())
{
    IntegralType::setDataType(TypeInt);
}

QString EnumerationType::toString() const
{
    return qualifiedIdentifier().toString();
}

size_t EnumerationType::hash() const
{
    return KDevHash(IntegralType::hash()) << IdentifiedType::hash();
}

AbstractType::WhichType EnumerationType::whichType() const
{
    return TypeEnumeration;
}
}
