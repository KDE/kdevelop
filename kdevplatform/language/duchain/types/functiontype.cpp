/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "functiontype.h"

#include "typeregister.h"
#include "typesystem.h"

namespace KDevelop {
REGISTER_TYPE(FunctionType);

DEFINE_LIST_MEMBER_HASH(FunctionTypeData, m_arguments, IndexedType)

FunctionType::FunctionType(const FunctionType& rhs) : AbstractType(copyData<FunctionType>(*rhs.d_func()))
{
}

FunctionType::FunctionType(FunctionTypeData& data) : AbstractType(data)
{
}

AbstractType* FunctionType::clone() const
{
    return new FunctionType(*this);
}

bool FunctionType::equals(const AbstractType* _rhs) const
{
    if (this == _rhs)
        return true;

    if (!AbstractType::equals(_rhs))
        return false;

    Q_ASSERT(dynamic_cast<const FunctionType*>(_rhs));
    const auto* rhs = static_cast<const FunctionType*>(_rhs);

    TYPE_D(FunctionType);
    if (d->m_argumentsSize() != rhs->d_func()->m_argumentsSize())
        return false;

    if (( bool )rhs->d_func()->m_returnType != ( bool )d->m_returnType)
        return false;

    if (d->m_returnType != rhs->d_func()->m_returnType)
        return false;

    for (unsigned int a = 0; a < d->m_argumentsSize(); ++a)
        if (d->m_arguments()[a] != rhs->d_func()->m_arguments()[a])
            return false;

    return true;
}

FunctionType::FunctionType()
    : AbstractType(createData<FunctionType>())
{
}

FunctionType::~FunctionType()
{
}

void FunctionType::addArgument(const AbstractType::Ptr& argument, int index)
{
    if (index == -1)
        d_func_dynamic()->m_argumentsList().append(IndexedType(argument));
    else
        d_func_dynamic()->m_argumentsList().insert(index, IndexedType(argument));
}

void FunctionType::removeArgument(int i)
{
    d_func_dynamic()->m_argumentsList().remove(i);
}

void FunctionType::setReturnType(const AbstractType::Ptr& returnType)
{
    d_func_dynamic()->m_returnType = IndexedType(returnType);
}

AbstractType::Ptr FunctionType::returnType() const
{
    return d_func()->m_returnType.abstractType();
}

QList<AbstractType::Ptr> FunctionType::arguments() const
{
    ///@todo Don't do the conversion
    QList<AbstractType::Ptr> ret;
    ret.reserve(d_func()->m_argumentsSize());
    FOREACH_FUNCTION(const IndexedType &arg, d_func()->m_arguments)
    ret << arg.abstractType();
    return ret;
}

const IndexedType* FunctionType::indexedArguments() const
{
    return d_func()->m_arguments();
}

uint FunctionType::indexedArgumentsSize() const
{
    return d_func()->m_argumentsSize();
}

void FunctionType::accept0(TypeVisitor* v) const
{
    TYPE_D(FunctionType);
    if (v->visit(this)) {
        acceptType(d->m_returnType.abstractType(), v);

        for (unsigned int i = 0; i < d->m_argumentsSize(); ++i)
            acceptType(d->m_arguments()[i].abstractType(), v);
    }

    v->endVisit(this);
}

void FunctionType::exchangeTypes(TypeExchanger* exchanger)
{
    TYPE_D_DYNAMIC(FunctionType);
    for (uint i = 0; i < d->m_argumentsSize(); ++i)
        d->m_argumentsList()[i] = IndexedType(exchanger->exchange(d->m_arguments()[i].abstractType()));

    d->m_returnType = IndexedType(exchanger->exchange(d->m_returnType.abstractType()));
}

QString FunctionType::partToString(SignaturePart sigPart) const
{
    QString args;
    TYPE_D(FunctionType);
    if (sigPart == SignatureArguments || sigPart == SignatureWhole) {
        QStringList types;
        types.reserve(d->m_argumentsSize());
        FOREACH_FUNCTION(const IndexedType &type, d->m_arguments) {
            types.append(type ? type.abstractType()->toString() : QStringLiteral("<notype>"));
        }
        args += QLatin1Char('(') + types.join(QLatin1String(", ")) + QLatin1Char(')');
    }

    if (sigPart == SignatureArguments)
        return args;
    else if (sigPart == SignatureWhole)
        return QStringLiteral("function %1 %2").arg(returnType() ? returnType()->toString() : QStringLiteral(
                                                        "<notype>"), args);
    else if (sigPart == SignatureReturn)
        return returnType() ? returnType()->toString() : QString();

    return QStringLiteral("ERROR");
}

QString FunctionType::toString() const
{
    return partToString(SignatureWhole) + AbstractType::toString(true);
}

AbstractType::WhichType FunctionType::whichType() const
{
    return TypeFunction;
}

size_t FunctionType::hash() const
{
    KDevHash kdevhash(AbstractType::hash());
    kdevhash << d_func()->m_returnType.hash();

    FOREACH_FUNCTION(const IndexedType &t, d_func()->m_arguments) {
        kdevhash << t.hash();
    }

    return kdevhash;
}
}
