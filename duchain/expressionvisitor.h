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

#ifndef EXPRESSIONVISITOR_H
#define EXPRESSIONVISITOR_H

#include <QStack>

#include <language/duchain/builders/dynamiclanguageexpressionvisitor.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/ducontext.h>

#include <qmljs/parser/qmljsast_p.h>

#include "duchainexport.h"

class KDEVQMLJSDUCHAIN_EXPORT ExpressionVisitor : public KDevelop::DynamicLanguageExpressionVisitor, public QmlJS::AST::Visitor
{
public:
    explicit ExpressionVisitor(KDevelop::DUContext* context);

    using Visitor::visit;
    using Visitor::endVisit;

protected:
    virtual bool visit(QmlJS::AST::NumericLiteral* node);
    virtual bool visit(QmlJS::AST::StringLiteral* node);
    virtual bool visit(QmlJS::AST::TrueLiteral* node);
    virtual bool visit(QmlJS::AST::FalseLiteral* node);

    virtual bool visit(QmlJS::AST::ArrayLiteral* node);
    virtual bool visit(QmlJS::AST::ObjectLiteral* node);
    virtual bool visit(QmlJS::AST::ArrayMemberExpression* node);
    virtual bool visit(QmlJS::AST::FieldMemberExpression* node);

    virtual bool visit(QmlJS::AST::BinaryExpression* node);
    virtual bool visit(QmlJS::AST::IdentifierExpression* node);
    virtual bool visit(QmlJS::AST::UiQualifiedId* node);

    virtual bool visit(QmlJS::AST::FunctionExpression* node);
    virtual bool visit(QmlJS::AST::CallExpression* node);

private:
    using KDevelop::DynamicLanguageExpressionVisitor::encounter;

    void encounterNothing();
    void encounter(KDevelop::IntegralType::CommonIntegralTypes type);
    void encounter(const QString &declaration, KDevelop::DUContext* context = nullptr);
    void encounterFieldMember(const QString &name);
    void encounterObjectAtLocation(const QmlJS::AST::SourceLocation &location);

};

#endif // EXPRESSIONVISITOR_H
