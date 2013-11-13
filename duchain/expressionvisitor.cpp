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

using namespace KDevelop;

AbstractType::Ptr findType(QmlJS::AST::Node* node)
{
    ExpressionVisitor visitor;
    QmlJS::AST::Node::accept(node, &visitor);
    return visitor.lastType();
}

void ExpressionVisitor::endVisit(QmlJS::AST::ArrayLiteral* node)
{
    Q_UNUSED(node)
    m_lastType.push(AbstractType::Ptr(new IntegralType(IntegralType::TypeArray)));
}

void ExpressionVisitor::endVisit(QmlJS::AST::FalseLiteral* node)
{
    Q_UNUSED(node)
    m_lastType.push(AbstractType::Ptr(new IntegralType(IntegralType::TypeBoolean)));
}

void ExpressionVisitor::endVisit(QmlJS::AST::NumericLiteral* node)
{
    if (QString::number(node->value).contains('.')) {
        m_lastType.push(AbstractType::Ptr(new IntegralType(IntegralType::TypeDouble)));
    } else {
        m_lastType.push(AbstractType::Ptr(new IntegralType(IntegralType::TypeInt)));
    }
}

void ExpressionVisitor::endVisit(QmlJS::AST::StringLiteral* node)
{
    Q_UNUSED(node)
    m_lastType.push(AbstractType::Ptr(new IntegralType(IntegralType::TypeString)));
}

void ExpressionVisitor::endVisit(QmlJS::AST::TrueLiteral* node)
{
    Q_UNUSED(node)
    m_lastType.push(AbstractType::Ptr(new IntegralType(IntegralType::TypeBoolean)));
}

AbstractType::Ptr ExpressionVisitor::lastType()
{
    return ( m_lastType.isEmpty() ?
        AbstractType::Ptr(new IntegralType(IntegralType::TypeMixed)) :
        m_lastType.last() );
}
