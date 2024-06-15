/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "unsuretype.h"
#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop {
REGISTER_TYPE(UnsureType);
DEFINE_LIST_MEMBER_HASH(UnsureTypeData, m_types, IndexedType)

UnsureType::UnsureType(const KDevelop::UnsureType& rhs) : AbstractType(copyData<UnsureType>(*rhs.d_func()))
{
}

UnsureType::UnsureType() : AbstractType(createData<UnsureType>())
{
}

void UnsureType::accept0(KDevelop::TypeVisitor* v) const
{
    FOREACH_FUNCTION(const IndexedType &type, d_func()->m_types) {
        AbstractType::Ptr t = type.abstractType();
        v->visit(t.data());
    }
}

KDevelop::AbstractType* UnsureType::clone() const
{
    return new UnsureType(*this);
}

QString UnsureType::toString() const
{
    QStringList typeNames;
    typeNames.reserve(d_func()->m_typesSize());
    FOREACH_FUNCTION(const IndexedType &type, d_func()->m_types) {
        AbstractType::Ptr t = type.abstractType();
        if (t)
            typeNames.append(t->toString());
        else
            typeNames.append(QStringLiteral("none"));
    }
    QString ret = QLatin1String("unsure (") + typeNames.join(QLatin1String(", ")) + QLatin1Char(')');

    return ret;
}

bool UnsureType::equals(const KDevelop::AbstractType* rhs) const
{
    const auto* rhsU = dynamic_cast<const UnsureType*>(rhs);
    if (!rhsU)
        return false;
    if (d_func()->typeClassId != rhsU->d_func()->typeClassId)
        return false;
    if (d_func()->m_typesSize() != rhsU->d_func()->m_typesSize())
        return false;

    for (uint a = 0; a < d_func()->m_typesSize(); ++a)
        if (d_func()->m_types()[a] != rhsU->d_func()->m_types()[a])
            return false;

    return KDevelop::AbstractType::equals(rhs);
}

bool UnsureType::contains(const KDevelop::AbstractType* type) const
{
    const IndexedType indexed(type);

    FOREACH_FUNCTION(const IndexedType& t, d_func()->m_types) {
        if (indexed == t) {
            return true;
        }
    }

    return false;
}

size_t UnsureType::hash() const
{
    KDevHash kdevhash(AbstractType::hash());
    FOREACH_FUNCTION(const IndexedType &type, d_func()->m_types)
    kdevhash << type.hash();
    return kdevhash << d_func()->m_typesSize();
}

KDevelop::AbstractType::WhichType UnsureType::whichType() const
{
    return TypeUnsure;
}

void UnsureType::exchangeTypes(KDevelop::TypeExchanger* exchanger)
{
    for (uint a = 0; a < d_func()->m_typesSize(); ++a) {
        AbstractType::Ptr from = d_func()->m_types()[a].abstractType();
        AbstractType::Ptr exchanged = exchanger->exchange(from);
        if (exchanged != from)
            d_func_dynamic()->m_typesList()[a] = exchanged->indexed();
    }

    KDevelop::AbstractType::exchangeTypes(exchanger);
}

void UnsureType::addType(const KDevelop::IndexedType& type)
{
    if (!d_func_dynamic()->m_typesList().contains(type)) {
        d_func_dynamic()->m_typesList().append(type);
    }
}

void UnsureType::removeType(const KDevelop::IndexedType& type)
{
    d_func_dynamic()->m_typesList().removeOne(type);
}

const KDevelop::IndexedType* UnsureType::types() const
{
    return d_func()->m_types();
}

uint UnsureType::typesSize() const
{
    return d_func()->m_typesSize();
}

UnsureType::UnsureType(KDevelop::UnsureTypeData& data) : AbstractType(data)
{
}
}
