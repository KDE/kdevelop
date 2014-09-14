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

#include <util/path.h>

#include <qmljs/parser/qmljsast_p.h>

#include "duchainexport.h"

class KDEVQMLJSDUCHAIN_EXPORT ExpressionVisitor : public KDevelop::DynamicLanguageExpressionVisitor, public QmlJS::AST::Visitor
{
public:
    explicit ExpressionVisitor(KDevelop::DUContext* context);

    /**
     * Return whether the expression ends with a prototype member.
     *
     * Example of such expressions are:
     *
     * @code
     * Class.prototype
     * Module.Class.prototype
     * object.__proto__
     * @endcode
     *
     * These expressions don't point to a prototype:
     *
     * @code
     * Class.prototype.method
     * object.__proto__.member
     * @endcode
     */
    bool isPrototype() const;

    using Visitor::visit;
    using Visitor::endVisit;

protected:
    virtual bool visit(QmlJS::AST::NumericLiteral* node);
    virtual bool visit(QmlJS::AST::StringLiteral* node);
    virtual bool visit(QmlJS::AST::RegExpLiteral* node);
    virtual bool visit(QmlJS::AST::TrueLiteral* node);
    virtual bool visit(QmlJS::AST::FalseLiteral* node);

    virtual bool visit(QmlJS::AST::ArrayLiteral* node);
    virtual bool visit(QmlJS::AST::ObjectLiteral* node);
    virtual bool visit(QmlJS::AST::ArrayMemberExpression* node);
    virtual bool visit(QmlJS::AST::FieldMemberExpression* node);

    virtual bool visit(QmlJS::AST::BinaryExpression* node);
    virtual bool visit(QmlJS::AST::IdentifierExpression* node);
    virtual bool visit(QmlJS::AST::UiQualifiedId* node);
    virtual bool visit(QmlJS::AST::ThisExpression* node);

    virtual bool visit(QmlJS::AST::FunctionExpression* node);
    virtual bool visit(QmlJS::AST::CallExpression* node);
    virtual bool visit(QmlJS::AST::NewMemberExpression* node);

    virtual void postVisit(QmlJS::AST::Node* node);

private:
    using KDevelop::DynamicLanguageExpressionVisitor::encounter;

    void encounterNothing();
    void encounter(KDevelop::IntegralType::CommonIntegralTypes type);

    void encounter(const QString &declaration, KDevelop::DUContext* context = nullptr);
    bool encounterParent(const QString& declaration);       // "parent" QML identifier
    bool encounterDeclarationInContext(const KDevelop::QualifiedIdentifier& id,
                                       KDevelop::DUContext* context);
    bool encounterDeclarationInNodeModule(const KDevelop::QualifiedIdentifier& id,
                                          const QString& module);
    bool encounterGlobalDeclaration(const KDevelop::QualifiedIdentifier& id);

    void encounterFieldMember(const QString &name);
    void encounterObjectAtLocation(const QmlJS::AST::SourceLocation &location);
    void instantiateCurrentDeclaration();   /*!< @brief Encounter a StructureType whose declaration is currentDeclaration() */

private:
    int m_prototypeDepth;   // 2 = the current node is "prototype" or "__proto__". 1 = we have just closed this node. <= 0 : "__proto__" is not the last node (as in "foo.prototype.bar")
    KDevelop::Path m_currentDir;
};

#endif // EXPRESSIONVISITOR_H
