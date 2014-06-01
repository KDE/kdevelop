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

#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/functiontype.h>

#include "helper.h"
#include "parsesession.h"

using namespace KDevelop;

ExpressionVisitor::ExpressionVisitor(DUContext* context)
: DynamicLanguageExpressionVisitor(context)
{
}

/*
 * Literals
 */
bool ExpressionVisitor::visit(QmlJS::AST::NumericLiteral* node)
{
    int num_int_digits = (int)std::log10(node->value) + 1;

    encounter(
        num_int_digits == (int)node->literalToken.length ?
            IntegralType::TypeInt :
            IntegralType::TypeDouble
    );
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::StringLiteral*)
{
    encounter(IntegralType::TypeString);
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::TrueLiteral*)
{
    encounter(IntegralType::TypeBoolean);
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::FalseLiteral*)
{
    encounter(IntegralType::TypeBoolean);
    return false;
}

/*
 * Object and arrays
 */
bool ExpressionVisitor::visit(QmlJS::AST::ArrayLiteral*)
{
    encounter(AbstractType::Ptr(new IntegralType(IntegralType::TypeArray)));
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::ObjectLiteral*)
{
    encounter(AbstractType::Ptr(new StructureType));
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::FieldMemberExpression* node)
{
    // Find the type of the base, and if this type has a declaration, use
    // its inner context to get the type of the field member
    node->base->accept(this);

    DeclarationPointer declaration = lastDeclaration();
    DUContext* context = QmlJS::getInternalContext(declaration);

    if (context) {
        encounter(node->name.toString(), context);
    } else {
        encounter(AbstractType::Ptr());
    }

    return false;
}

/*
 * Identifiers and common expressions
 */
bool ExpressionVisitor::visit(QmlJS::AST::BinaryExpression* node)
{
    switch (node->op) {
    case QSOperator::BitAnd:
    case QSOperator::BitOr:
    case QSOperator::BitXor:
    case QSOperator::LShift:
    case QSOperator::RShift:
    case QSOperator::URShift:
        encounter(IntegralType::TypeInt);
        break;
    case QSOperator::And:
    case QSOperator::Equal:
    case QSOperator::Ge:
    case QSOperator::Gt:
    case QSOperator::In:
    case QSOperator::InstanceOf:
    case QSOperator::Le:
    case QSOperator::Lt:
    case QSOperator::Or:
    case QSOperator::StrictEqual:
    case QSOperator::StrictNotEqual:
        encounter(IntegralType::TypeBoolean);
        break;
    default:
        break;
    }

    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::IdentifierExpression* node)
{
    encounter(node->name.toString());
    return false;
}

/*
 * Functions
 */
bool ExpressionVisitor::visit(QmlJS::AST::FunctionExpression* node)
{
    DUChainReadLocker lock;
    QmlJS::AST::SourceLocation location = node->lbraceToken;

    // Find the anonymous declaration corresponding to the function. This is
    // the owner of the current context (function expressions create new contexts)
    Declaration* dec = m_context->topContext()->findContextAt(
        CursorInRevision(location.startLine-1, location.startColumn)
    )->owner();

    if (dec && dec->abstractType()) {
        encounter(dec->abstractType());
    }

    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::CallExpression* node)
{
    // Find the type of the function called
    node->base->accept(this);

    FunctionType::Ptr func = FunctionType::Ptr::dynamicCast(m_lastType);

    if (func && func->returnType()) {
        encounter(func->returnType());
    }

    return false;
}

void ExpressionVisitor::encounter(IntegralType::CommonIntegralTypes type)
{
    encounter(AbstractType::Ptr(new IntegralType(type)));
}

void ExpressionVisitor::encounter(const QString& declaration, KDevelop::DUContext* context)
{
    const QualifiedIdentifier name(declaration);
    DeclarationPointer dec = QmlJS::getDeclaration(name, context ? context : m_context);

    if (dec && dec->abstractType()) {
        encounterLvalue(dec);
    }
}
