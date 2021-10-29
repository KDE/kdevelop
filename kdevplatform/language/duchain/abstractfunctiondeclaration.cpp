/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "abstractfunctiondeclaration.h"

#include "types/functiontype.h"
#include "declaration.h"
#include "ducontext.h"
#include <serialization/indexedstring.h>

namespace KDevelop {
AbstractFunctionDeclaration::~AbstractFunctionDeclaration()
{
}

bool AbstractFunctionDeclaration::isVirtual() const
{
    return data()->m_isVirtual;
}

void AbstractFunctionDeclaration::setVirtual(bool isVirtual)
{
    dynamicData()->m_isVirtual = isVirtual;
}

bool AbstractFunctionDeclaration::isInline() const
{
    return data()->m_isInline;
}

void AbstractFunctionDeclaration::setInline(bool isInline)
{
    dynamicData()->m_isInline = isInline;
}

bool AbstractFunctionDeclaration::isExplicit() const
{
    return data()->m_isExplicit;
}

void AbstractFunctionDeclaration::setExplicit(bool isExplicit)
{
    dynamicData()->m_isExplicit = isExplicit;
}

void AbstractFunctionDeclaration::setFunctionSpecifiers(FunctionSpecifiers specifiers)
{
    dynamicData()->m_isInline = specifiers & InlineSpecifier;
    dynamicData()->m_isExplicit = specifiers & ExplicitSpecifier;
    dynamicData()->m_isVirtual = specifiers & VirtualSpecifier;
}

IndexedString AbstractFunctionDeclaration::defaultParameterForArgument(int index) const
{
    FunctionType::Ptr fType = dynamic_cast<const Declaration*>(this)->type<FunctionType>();
    if (fType && index >= 0 && index < fType->arguments().size()) {
        index -= (fType->arguments().size() - defaultParametersSize());
        if (index >= 0 && index < ( int )defaultParametersSize())
            return defaultParameters()[index];
    }

    return IndexedString();
}

void AbstractFunctionDeclaration::setInternalFunctionContext(DUContext* context)
{
    Q_ASSERT(!context || context->type() == DUContext::Function);
    dynamicData()->m_functionContext = context;
}
DUContext* AbstractFunctionDeclaration::internalFunctionContext() const
{
    return data()->m_functionContext.context();
}
}
