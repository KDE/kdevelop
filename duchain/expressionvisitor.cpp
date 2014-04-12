/*************************************************************************************
 *  Copyright (C) 2013 by Andrea Scarpino <scarpino@kde.org>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "expressionvisitor.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/functiontype.h>

#include "helper.h"
#include "parsesession.h"

using namespace KDevelop;

ExpressionVisitor::ExpressionVisitor(DUContext* context) :
    m_context(context)
{
}

/*
 * Literals
 */
bool ExpressionVisitor::visit(QmlJS::AST::NumericLiteral* node)
{
    int num_int_digits = (int)std::log10(node->value) + 1;

    setType(
        num_int_digits == (int)node->literalToken.length ?
            IntegralType::TypeInt :
            IntegralType::TypeDouble
    );
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::StringLiteral*)
{
    setType(IntegralType::TypeString);
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::TrueLiteral*)
{
    setType(IntegralType::TypeBoolean);
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::FalseLiteral*)
{
    setType(IntegralType::TypeBoolean);
    return false;
}

/*
 * Object and arrays
 */
bool ExpressionVisitor::visit(QmlJS::AST::ArrayLiteral*)
{
    setType(AbstractType::Ptr(new IntegralType(IntegralType::TypeArray)));
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::ObjectLiteral*)
{
    setType(AbstractType::Ptr(new StructureType));
    return false;
}

/*
 * Identifiers and common expressions
 */
bool ExpressionVisitor::visit(QmlJS::AST::IdentifierExpression* node)
{
    setType(node->name.toString());
    return false;
}

/*
 * Functions
 */
bool ExpressionVisitor::visit(QmlJS::AST::CallExpression* node)
{
    // Find the type of the function called
    node->base->accept(this);

    FunctionType::Ptr func = FunctionType::Ptr::dynamicCast(m_lastType);

    if (func && func->returnType()) {
        setType(func->returnType());
    }

    return false;
}

void ExpressionVisitor::setType(AbstractType::Ptr type)
{
    m_lastType = type;
}

void ExpressionVisitor::setType(IntegralType::CommonIntegralTypes type)
{
    m_lastType = AbstractType::Ptr(new IntegralType(type));
}

void ExpressionVisitor::setType(const QString& declaration)
{
    const QualifiedIdentifier name(declaration);
    DeclarationPointer dec = QmlJS::getDeclaration(name, DUContextPointer(m_context));

    if (dec && dec->abstractType()) {
        setType(dec->abstractType());
    } else {
        m_lastType = NULL;
    }
}

AbstractType::Ptr ExpressionVisitor::lastType() const
{
    return m_lastType ?
        m_lastType :
        AbstractType::Ptr(new IntegralType(IntegralType::TypeMixed));
}
