/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "pointertype.h"

#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop {
REGISTER_TYPE(PointerType);

PointerType::PointerType(const PointerType& rhs) : AbstractType(copyData<PointerType>(*rhs.d_func()))
{
}

PointerType::PointerType(PointerTypeData& data) : AbstractType(data)
{
}

AbstractType* PointerType::clone() const
{
    return new PointerType(*this);
}

bool PointerType::equals(const AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!AbstractType::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const PointerType*>(_rhs));
    const auto* rhs = static_cast<const PointerType*>(_rhs);

    return d_func()->m_baseType == rhs->d_func()->m_baseType;
}

PointerType::PointerType()
    : AbstractType(createData<PointerType>())
{
}

void PointerType::accept0(TypeVisitor* v) const
{
    if (v->visit(this))
        acceptType(d_func()->m_baseType.abstractType(), v);

    v->endVisit(this);
}

void PointerType::exchangeTypes(TypeExchanger* exchanger)
{
    d_func_dynamic()->m_baseType = IndexedType(exchanger->exchange(d_func()->m_baseType.abstractType()));
}

PointerType::~PointerType()
{
}

AbstractType::Ptr PointerType::baseType() const
{
    return d_func()->m_baseType.abstractType();
}

void PointerType::setBaseType(const AbstractType::Ptr& type)
{
    d_func_dynamic()->m_baseType = IndexedType(type);
}

QString PointerType::toString() const
{
    QString baseString = (baseType() ? baseType()->toString() : QStringLiteral("<notype>"));
    return baseString + QLatin1Char('*') + AbstractType::toString(true);
}

AbstractType::WhichType PointerType::whichType() const
{
    return TypePointer;
}

size_t PointerType::hash() const
{
    return KDevHash(AbstractType::hash()) << d_func()->m_baseType.hash();
}
}
