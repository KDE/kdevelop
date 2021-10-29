/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "typealiastype.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop {
REGISTER_TYPE(TypeAliasType);

AbstractType* TypeAliasType::clone() const
{
    return new TypeAliasType(*this);
}

bool TypeAliasType::equals(const AbstractType* _rhs) const
{
    if (!fastCast<const TypeAliasType*>(_rhs))
        return false;
    const auto* rhs = static_cast<const TypeAliasType*>(_rhs);

    if (this == rhs)
        return true;

    if (AbstractType::equals(rhs) && IdentifiedType::equals(rhs)) {
        if (( bool )d_func()->m_type != ( bool )rhs->d_func()->m_type)
            return false;

        if (!d_func()->m_type)
            return true;

        return d_func()->m_type == rhs->d_func()->m_type;
    } else {
        return false;
    }
}
AbstractType::Ptr TypeAliasType::type() const
{
    return d_func()->m_type.abstractType();
}

void TypeAliasType::setType(const AbstractType::Ptr& type)
{
    d_func_dynamic()->m_type = IndexedType(type);
}

uint TypeAliasType::hash() const
{
    return KDevHash(AbstractType::hash()) << IdentifiedType::hash() << d_func()->m_type.hash();
}

QString TypeAliasType::toString() const
{
    QualifiedIdentifier id = qualifiedIdentifier();
    if (!id.isEmpty())
        return AbstractType::toString(false) + id.toString();

    if (type())
        return AbstractType::toString(false) + type()->toString();

    return QStringLiteral("typedef <notype>");
}

void TypeAliasType::accept0(KDevelop::TypeVisitor* v) const
{
    if (v->visit(this))
        acceptType(d_func()->m_type.abstractType(), v);

//     v->endVisit (this);
}

KDevelop::AbstractType::WhichType TypeAliasType::whichType() const
{
    return TypeAlias;
}

void TypeAliasType::exchangeTypes(KDevelop::TypeExchanger* exchanger)
{
    d_func_dynamic()->m_type = IndexedType(exchanger->exchange(d_func()->m_type.abstractType()));
}
}
