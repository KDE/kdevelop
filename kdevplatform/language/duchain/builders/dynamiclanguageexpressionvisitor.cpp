/*
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dynamiclanguageexpressionvisitor.h"

#include "language/duchain/types/integraltype.h"
#include "language/duchain/declaration.h"

namespace KDevelop {
DynamicLanguageExpressionVisitor::DynamicLanguageExpressionVisitor(DynamicLanguageExpressionVisitor* parent)
    : m_context(parent->m_context)
    , m_parentVisitor(parent)
{
    Q_ASSERT(m_context);
}

DynamicLanguageExpressionVisitor::DynamicLanguageExpressionVisitor(const DUContext* context)
    : m_context(context)
    , m_parentVisitor(nullptr)
{
    Q_ASSERT(m_context);
}

AbstractType::Ptr DynamicLanguageExpressionVisitor::unknownType() const
{
    return AbstractType::Ptr(new IntegralType(IntegralType::TypeMixed));
}

void DynamicLanguageExpressionVisitor::encounter(const AbstractType::Ptr& type, const DeclarationPointer& declaration)
{
    m_lastType = encounterPreprocess(type);
    m_lastDeclaration = declaration;
}

void DynamicLanguageExpressionVisitor::encounterLvalue(const DeclarationPointer& lvalueDeclaration)
{
    m_lastDeclaration = lvalueDeclaration;
    if (lvalueDeclaration) {
        m_lastType = lvalueDeclaration->abstractType();
    }
}

void DynamicLanguageExpressionVisitor::encounterUnknown()
{
    // setting it to 0 avoids constructing the mixed type;
    // that is constructed when lastType() is called
    m_lastType = nullptr;
}

AbstractType::Ptr DynamicLanguageExpressionVisitor::encounterPreprocess(AbstractType::Ptr type)
{
    return type;
}
} // namespace KDevelop
