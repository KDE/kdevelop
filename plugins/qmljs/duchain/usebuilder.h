/*
    SPDX-FileCopyrightText: 2013 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USEBUILDER_H
#define USEBUILDER_H

#include <language/duchain/builders/abstractusebuilder.h>
#include <util/stack.h>

#include "contextbuilder.h"

using UseBuilderBase = KDevelop::AbstractUseBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName, ContextBuilder>;

class KDEVQMLJSDUCHAIN_EXPORT UseBuilder : public UseBuilderBase
{
public:
    explicit UseBuilder(ParseSession* session);

protected:
    using Visitor::visit;
    using Visitor::preVisit;
    using Visitor::postVisit;

    bool preVisit(QmlJS::AST::Node* node) override;
    void postVisit(QmlJS::AST::Node* node) override;

    bool visit(QmlJS::AST::FieldMemberExpression* node) override;
    bool visit(QmlJS::AST::IdentifierExpression* node) override;
    bool visit(QmlJS::AST::UiQualifiedId* node) override;
    bool visit(QmlJS::AST::UiImport* node) override;
    bool visit(QmlJS::AST::UiPublicMember* node) override;
    bool visit(QmlJS::AST::UiScriptBinding* node) override;

private:
    void useForExpression(QmlJS::AST::Node* node, const KDevelop::RangeInRevision &range = KDevelop::RangeInRevision::invalid());

    void throwRecursionDepthError() override;

private:
    KDevelop::Stack<QmlJS::AST::Node*> m_nodesThatOpenedContexts;
};

#endif // USEBUILDER_H
