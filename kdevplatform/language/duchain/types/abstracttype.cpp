/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "abstracttype.h"

#include "typeregister.h"
#include "typesystem.h"
#include <debug.h>

namespace KDevelop {
//REGISTER_TYPE(AbstractType);

void AbstractType::makeDynamic()
{
    if (d_ptr->m_dynamic)
        return;
    AbstractType::Ptr newType(clone()); //While cloning, all the data is cloned as well. So we use that mechanism and steal the cloned data.
    Q_ASSERT(newType->equals(this));
    AbstractTypeData* oldData = d_ptr;
    d_ptr = newType->d_ptr;
    newType->d_ptr = oldData;
    Q_ASSERT(d_ptr->m_dynamic);
}

AbstractType::AbstractType(AbstractTypeData& dd)
    : d_ptr(&dd)
{
}

quint32 AbstractType::modifiers() const
{
    return d_func()->m_modifiers;
}

void AbstractType::setModifiers(quint32 modifiers)
{
    d_func_dynamic()->m_modifiers = modifiers;
}

int64_t AbstractType::sizeOf() const
{
    return d_func()->m_sizeOf;
}

void AbstractType::setSizeOf(int64_t sizeOf)
{
    d_func_dynamic()->m_sizeOf = sizeOf;
}

int64_t AbstractType::alignOf() const
{
    if (d_func()->m_alignOfExponent == AbstractTypeData::MaxAlignOfExponent) {
        return -1;
    } else {
        return Q_INT64_C(1) << d_func()->m_alignOfExponent;
    }
}

void AbstractType::setAlignOf(int64_t alignedTo)
{
    if (alignedTo <= 0) {
        d_func_dynamic()->m_alignOfExponent = AbstractTypeData::MaxAlignOfExponent;
        return;
    }

    unsigned int alignOfExponent = 0;
    while (alignedTo >>= 1)
        alignOfExponent++;
    d_func_dynamic()->m_alignOfExponent = alignOfExponent;
}

AbstractType::AbstractType()
    : d_ptr(&createData<AbstractType>())
{
}

AbstractType::~AbstractType()
{
    if (!d_ptr->inRepository) {
        TypeSystem::self().callDestructor(d_ptr);
        delete[] ( char* )d_ptr;
    }
}

void AbstractType::accept(TypeVisitor* v) const
{
    if (v->preVisit(this))
        this->accept0(v);

    v->postVisit(this);
}

void AbstractType::acceptType(AbstractType::Ptr type, TypeVisitor* v)
{
    if (!type)
        return;

    type->accept(v);
}

AbstractType::WhichType AbstractType::whichType() const
{
    return TypeAbstract;
}

void AbstractType::exchangeTypes(TypeExchanger* /*exchanger */)
{
}

IndexedType AbstractType::indexed() const
{
    return IndexedType(this);
}

bool AbstractType::equals(const AbstractType* rhs) const
{
    //qCDebug(LANGUAGE) << this << rhs << modifiers() << rhs->modifiers();
    return d_func()->typeClassId == rhs->d_func()->typeClassId && d_func()->m_modifiers == rhs->d_func()->m_modifiers
        && d_func()->m_sizeOf == rhs->d_func()->m_sizeOf
        && d_func()->m_alignOfExponent == rhs->d_func()->m_alignOfExponent;
}

bool AbstractType::contains(const AbstractType* type) const
{
    return equals(type);
}

size_t AbstractType::hash() const
{
    return KDevHash() << d_func()->typeClassId << d_func()->m_modifiers << d_func()->m_sizeOf
                      << d_func()->m_alignOfExponent;
}

QString AbstractType::toString() const
{
    return toString(false);
}

QString AbstractType::toString(bool spaceOnLeft) const
{
    // TODO complete
    QString modifiersStr;

    if (modifiers() & ConstModifier) {
        modifiersStr.append(QStringLiteral("const"));
    }

    if (modifiers() & VolatileModifier) {
        if (!modifiersStr.isEmpty())
            modifiersStr.append(QStringLiteral(" "));
        modifiersStr.append(QStringLiteral("volatile"));
    }

    if (modifiers() & AtomicModifier) {
        if (!modifiersStr.isEmpty())
            modifiersStr.append(QStringLiteral(" "));
        modifiersStr.append(QStringLiteral("_Atomic"));
    }

    if (!modifiersStr.isEmpty()) {
        if (spaceOnLeft)
            modifiersStr.prepend(QStringLiteral(" "));
        else
            modifiersStr.append(QStringLiteral(" "));
    }

    return modifiersStr;
}
}
