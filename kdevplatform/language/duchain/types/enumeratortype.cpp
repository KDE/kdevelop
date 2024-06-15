/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "enumeratortype.h"

#include "typesystemdata.h"
#include "typeregister.h"

namespace KDevelop {
REGISTER_TYPE(EnumeratorType);

EnumeratorType::EnumeratorType(const EnumeratorType& rhs)
    : EnumeratorTypeBase(copyData<EnumeratorType>(*rhs.d_func()))
{
}

EnumeratorType::EnumeratorType(EnumeratorTypeData& data)
    : EnumeratorTypeBase(data)
{
}

EnumeratorType::EnumeratorType()
    : EnumeratorTypeBase(createData<EnumeratorType>())
{
    IntegralType::setDataType(TypeInt);
    setModifiers(ConstModifier);
}

AbstractType* EnumeratorType::clone() const
{
    return new EnumeratorType(*this);
}

bool EnumeratorType::equals(const AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!EnumeratorTypeBase::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const EnumeratorType*>(_rhs));

    // Nothing Enumerator-type specific to compare
    return true;
}

size_t EnumeratorType::hash() const
{
    return KDevHash(ConstantIntegralType::hash()) << IdentifiedType::hash();
}

AbstractType::WhichType EnumeratorType::whichType() const
{
    return TypeEnumerator;
}

QString EnumeratorType::toString() const
{
    return IdentifiedType::qualifiedIdentifier().toString();
}
}
