/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "arraytype.h"

#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop {
REGISTER_TYPE(ArrayType);

ArrayType::ArrayType(const ArrayType& rhs) : AbstractType(copyData<ArrayType>(*rhs.d_func()))
{
}

ArrayType::ArrayType(ArrayTypeData& data) : AbstractType(data)
{
}

AbstractType* ArrayType::clone() const
{
    return new ArrayType(*this);
}

bool ArrayType::equals(const AbstractType* _rhs) const
{
    if (!AbstractType::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const ArrayType*>(_rhs));
    const auto* rhs = static_cast<const ArrayType*>(_rhs);

    TYPE_D(ArrayType);
    if (d->m_dimension != rhs->d_func()->m_dimension)
        return false;

    return d->m_elementType == rhs->d_func()->m_elementType;
}

ArrayType::ArrayType()
    : AbstractType(createData<ArrayType>())
{
}

ArrayType::~ArrayType()
{
}

int ArrayType::dimension() const
{
    return d_func()->m_dimension;
}

void ArrayType::setDimension(int dimension)
{
    d_func_dynamic()->m_dimension = dimension;
}

AbstractType::Ptr ArrayType::elementType() const
{
    return d_func()->m_elementType.abstractType();
}

void ArrayType::setElementType(const AbstractType::Ptr& type)
{
    d_func_dynamic()->m_elementType = IndexedType(type);
}

QString ArrayType::toString() const
{
    if (d_func()->m_dimension == 0) {
        return QStringLiteral("%1[]").arg(elementType() ? elementType()->toString() : QStringLiteral("<notype>"));
    }
    return QStringLiteral("%1[%2]").arg(elementType() ? elementType()->toString() : QStringLiteral("<notype>")).arg(
        d_func()->m_dimension);
}

void ArrayType::accept0(TypeVisitor* v) const
{
    if (v->visit(this)) {
        acceptType(d_func()->m_elementType.abstractType(), v);
    }

    v->endVisit(this);
}

void ArrayType::exchangeTypes(TypeExchanger* exchanger)
{
    TYPE_D_DYNAMIC(ArrayType);
    d->m_elementType = IndexedType(exchanger->exchange(d->m_elementType.abstractType()));
}

AbstractType::WhichType ArrayType::whichType() const
{
    return TypeArray;
}

size_t ArrayType::hash() const
{
    return KDevHash(AbstractType::hash())
           << (elementType() ? elementType()->hash() : 0) << dimension();
}
}
