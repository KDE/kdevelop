/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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

void DynamicLanguageExpressionVisitor::encounter(AbstractType::Ptr type, DeclarationPointer declaration)
{
    m_lastType = encounterPreprocess(type);
    m_lastDeclaration = declaration;
}

void DynamicLanguageExpressionVisitor::encounterLvalue(DeclarationPointer lvalueDeclaration)
{
    m_lastDeclaration = lvalueDeclaration;
    if ( lvalueDeclaration ) {
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

// kate: space-indent on; indent-width 4;
