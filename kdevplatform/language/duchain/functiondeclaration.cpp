/*
    SPDX-FileCopyrightText: 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "functiondeclaration.h"
#include "ducontext.h"
#include "duchainregister.h"
#include "types/functiontype.h"
#include <debug.h>

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(FunctionDeclaration);

DEFINE_LIST_MEMBER_HASH(FunctionDeclarationData, m_defaultParameters, IndexedString)

FunctionDeclaration::FunctionDeclaration(FunctionDeclarationData& data) : FunctionDeclarationBase(data)
{
}

FunctionDeclaration::FunctionDeclaration(FunctionDeclarationData& data,
                                         const RangeInRevision& range) : FunctionDeclarationBase(data, range)
{
}

FunctionDeclaration::FunctionDeclaration(const FunctionDeclaration& rhs) : FunctionDeclarationBase(*new FunctionDeclarationData(
            *rhs.d_func()))
{
}

FunctionDeclaration::FunctionDeclaration(const RangeInRevision& range, DUContext* context)
    : FunctionDeclarationBase(*new FunctionDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}

FunctionDeclaration::~FunctionDeclaration()
{
}

Declaration* FunctionDeclaration::clonePrivate() const
{
    return new FunctionDeclaration(*this);
}

bool FunctionDeclaration::isFunctionDeclaration() const
{
    return true;
}

void FunctionDeclaration::setAbstractType(AbstractType::Ptr type)
{
    if (type && !type.dynamicCast<FunctionType>()) {
        qCDebug(LANGUAGE) << "wrong type attached to function declaration:" << type->toString();
    }
    Declaration::setAbstractType(type);
}

QString FunctionDeclaration::toString() const
{
    AbstractType::Ptr type = abstractType();
    if (!type)
        return Declaration::toString();

    auto function = type.dynamicCast<FunctionType>();
    if (function) {
        return QStringLiteral("%1 %2 %3").arg(function->partToString(FunctionType::SignatureReturn),
                                              identifier().toString(),
                                              function->partToString(FunctionType::SignatureArguments));
    } else {
        return Declaration::toString();
    }
}

uint FunctionDeclaration::additionalIdentity() const
{
    if (abstractType())
        return abstractType()->hash();
    else
        return 0;
}

const IndexedString* FunctionDeclaration::defaultParameters() const
{
    return d_func()->m_defaultParameters();
}

unsigned int FunctionDeclaration::defaultParametersSize() const
{
    return d_func()->m_defaultParametersSize();
}

void FunctionDeclaration::addDefaultParameter(const IndexedString& str)
{
    d_func_dynamic()->m_defaultParametersList().append(str);
}

void FunctionDeclaration::clearDefaultParameters()
{
    d_func_dynamic()->m_defaultParametersList().clear();
}
}
