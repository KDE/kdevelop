/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "referencetype.h"

#include "typeregister.h"
#include "typesystem.h"
#include "integraltype.h"
#include "structuretype.h"

namespace KDevelop {
REGISTER_TYPE(ReferenceType);

ReferenceType::ReferenceType(const ReferenceType& rhs) : AbstractType(copyData<ReferenceType>(*rhs.d_func()))
{
}

ReferenceType::ReferenceType(ReferenceTypeData& data) : AbstractType(data)
{
}

AbstractType* ReferenceType::clone() const
{
    return new ReferenceType(*this);
}

bool ReferenceType::equals(const AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!AbstractType::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const ReferenceType*>(_rhs));
    const auto* rhs = static_cast<const ReferenceType*>(_rhs);

    return d_func()->m_baseType == rhs->d_func()->m_baseType && d_func()->m_isRValue == rhs->d_func()->m_isRValue;
}

ReferenceType::ReferenceType()
    : AbstractType(createData<ReferenceType>())
{
}

ReferenceType::~ReferenceType()
{
}

AbstractType::Ptr ReferenceType::baseType() const
{
    return d_func()->m_baseType.abstractType();
}

void ReferenceType::setBaseType(const AbstractType::Ptr& type)
{
    d_func_dynamic()->m_baseType = IndexedType(type);
}

bool ReferenceType::isRValue() const
{
    return d_func()->m_isRValue;
}

void ReferenceType::setIsRValue(bool isRValue)
{
    d_func_dynamic()->m_isRValue = isRValue;
}

void ReferenceType::accept0(TypeVisitor* v) const
{
    if (v->visit(this))
        acceptType(d_func()->m_baseType.abstractType(), v);

    v->endVisit(this);
}

void ReferenceType::exchangeTypes(TypeExchanger* exchanger)
{
    d_func_dynamic()->m_baseType = IndexedType(exchanger->exchange(d_func()->m_baseType.abstractType()));
}

QString ReferenceType::toString() const
{
    AbstractType::Ptr base = baseType();
    QString baseString = (base ? base->toString() : QStringLiteral("<notype>"));
    const QLatin1String ampersands = d_func()->m_isRValue ? QLatin1String("&&") : QLatin1String("&");
    if (base.dynamicCast<IntegralType>() || base.dynamicCast<StructureType>())
        return AbstractType::toString(false) + baseString + ampersands;
    else
        return baseString + AbstractType::toString(true) + ampersands;
}

AbstractType::WhichType ReferenceType::whichType() const
{
    return TypeReference;
}

uint ReferenceType::hash() const
{
    return KDevHash(AbstractType::hash()) << d_func()->m_baseType.hash() << d_func()->m_isRValue;
}
}
