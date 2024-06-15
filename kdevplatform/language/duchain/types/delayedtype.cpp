/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "delayedtype.h"

#include "typeregister.h"
#include "typesystem.h"

#include <QHash>

namespace KDevelop {
REGISTER_TYPE(DelayedType);

DelayedType::DelayedType(DelayedTypeData& data) : AbstractType(data)
{
}

AbstractType* DelayedType::clone() const
{
    return new DelayedType(*this);
}

bool DelayedType::equals(const AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!AbstractType::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const DelayedType*>(_rhs));
    const auto* rhs = static_cast<const DelayedType*>(_rhs);

    return d_func()->m_identifier == rhs->d_func()->m_identifier && rhs->d_func()->m_kind == d_func()->m_kind;
}

AbstractType::WhichType DelayedType::whichType() const
{
    return AbstractType::TypeDelayed;
}

QString DelayedType::toString() const
{
    return AbstractType::toString(false) + identifier().toString();
}

DelayedType::Kind DelayedType::kind() const
{
    return d_func()->m_kind;
}

void DelayedType::setKind(Kind kind)
{
    d_func_dynamic()->m_kind = kind;
}

DelayedType::DelayedType()
    : AbstractType(createData<DelayedType>())
{
}

DelayedType::DelayedType(const DelayedType& rhs) : AbstractType(copyData<DelayedType>(*rhs.d_func()))
{
}

DelayedType::~DelayedType()
{
}

void DelayedType::setIdentifier(const IndexedTypeIdentifier& identifier)
{
    d_func_dynamic()->m_identifier = identifier;
}

IndexedTypeIdentifier DelayedType::identifier() const
{
    return d_func()->m_identifier;
}

void DelayedType::accept0(KDevelop::TypeVisitor* v) const
{
    v->visit(this);
/*    v->endVisit(this);*/
}

inline size_t qHash(DelayedType::Kind kind)
{
    return ::qHash(static_cast<quint8>(kind));
}

size_t DelayedType::hash() const
{
    return KDevHash(AbstractType::hash()) << d_func()->m_identifier.hash() << d_func()->m_kind;
}
}
