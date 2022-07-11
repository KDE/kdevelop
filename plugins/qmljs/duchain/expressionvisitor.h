/*
    SPDX-FileCopyrightText: 2013 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXPRESSIONVISITOR_H
#define EXPRESSIONVISITOR_H

#include <util/stack.h>

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
    bool visit(QmlJS::AST::NumericLiteral* node) override;
    bool visit(QmlJS::AST::StringLiteral* node) override;
    bool visit(QmlJS::AST::RegExpLiteral* node) override;
    bool visit(QmlJS::AST::TrueLiteral* node) override;
    bool visit(QmlJS::AST::FalseLiteral* node) override;

    bool visit(QmlJS::AST::ArrayPattern* node) override;
    bool visit(QmlJS::AST::ObjectPattern* node) override;
    bool visit(QmlJS::AST::ArrayMemberExpression* node) override;
    bool visit(QmlJS::AST::FieldMemberExpression* node) override;

    bool visit(QmlJS::AST::BinaryExpression* node) override;
    bool visit(QmlJS::AST::IdentifierExpression* node) override;
    bool visit(QmlJS::AST::UiQualifiedId* node) override;
    bool visit(QmlJS::AST::ThisExpression* node) override;

    bool visit(QmlJS::AST::FunctionExpression* node) override;
    bool visit(QmlJS::AST::CallExpression* node) override;
    bool visit(QmlJS::AST::NewMemberExpression* node) override;

    void postVisit(QmlJS::AST::Node* node) override;
    void throwRecursionDepthError() override;

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
    void encounterObjectAtLocation(const QmlJS::SourceLocation &location);
    void instantiateCurrentDeclaration();   /*!< @brief Encounter a StructureType whose declaration is currentDeclaration() */

private:
    int m_prototypeDepth;   // 2 = the current node is "prototype" or "__proto__". 1 = we have just closed this node. <= 0 : "__proto__" is not the last node (as in "foo.prototype.bar")
    KDevelop::Path m_currentDir;
};

#endif // EXPRESSIONVISITOR_H
