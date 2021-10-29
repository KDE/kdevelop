/*
    SPDX-FileCopyrightText: 2013 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "usebuilder.h"
#include "expressionvisitor.h"

#include "helper.h"
#include "parsesession.h"

using namespace KDevelop;

UseBuilder::UseBuilder(ParseSession* session)
: UseBuilderBase()
{
    m_session = session;
    m_nodesThatOpenedContexts.push(nullptr);    // One push here, a thousand isEmpty() calls avoided later
}

bool UseBuilder::preVisit(QmlJS::AST::Node* node)
{
    DUContext* ctx = contextFromNode(node);

    if (ctx && currentContext() != ctx) {
        openContext(ctx);
        m_nodesThatOpenedContexts.push(node);
    }

    return true;
}

void UseBuilder::postVisit(QmlJS::AST::Node* node)
{
    if (m_nodesThatOpenedContexts.top() == node) {
        closeContext();
        m_nodesThatOpenedContexts.pop();
    }
}

bool UseBuilder::visit(QmlJS::AST::FieldMemberExpression* node)
{
    useForExpression(node, m_session->locationToRange(node->identifierToken));
    return UseBuilderBase::visit(node);
}

bool UseBuilder::visit(QmlJS::AST::IdentifierExpression* node)
{
    useForExpression(node);
    return UseBuilderBase::visit(node);
}

bool UseBuilder::visit(QmlJS::AST::UiQualifiedId* node)
{
    useForExpression(node);
    return false;
}

bool UseBuilder::visit(QmlJS::AST::UiImport* node)
{
    Q_UNUSED(node);
    return false;   // Don't highlight the identifiers that appear in import statements
}

bool UseBuilder::visit(QmlJS::AST::UiScriptBinding* node)
{
    QString propertyName = node->qualifiedId->name.toString();

    if (propertyName == QLatin1String("name") ||
        propertyName == QLatin1String("type") ||
        propertyName == QLatin1String("exports") ||
        propertyName == QLatin1String("prototype")) {
        // Ignore plugin.qmltypes-specific property names. They appear a huge
        // number of time and are never declared anywhere.
        return false;
    }

    return true;
}

bool UseBuilder::visit(QmlJS::AST::UiPublicMember* node)
{
    // node->memberType can contain a type name (if node is a property), use it
    DeclarationPointer decl = QmlJS::getDeclaration(
        QualifiedIdentifier(node->memberTypeName().toString()),
        currentContext()
    );

    newUse(
        m_session->locationToRange(node->typeToken),
        decl
    );

    return true;
}

void UseBuilder::useForExpression(QmlJS::AST::Node* node, const KDevelop::RangeInRevision &range)
{
    // ExpressionVisitor can find the type and corresponding declaration of many
    // kinds of expressions (identifiers, field members, special identifiers like
    // this or parent, etc).
    ExpressionVisitor visitor(currentContext());

    node->accept(&visitor);

    if (visitor.lastDeclaration()) {
        newUse(
            range.isValid() ? range : m_session->locationsToRange(
                node->firstSourceLocation(),
                node->lastSourceLocation()
            ),
            visitor.lastDeclaration()
        );
    }
}
