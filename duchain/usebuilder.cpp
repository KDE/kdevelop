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

bool UseBuilder::visit(QmlJS::AST::UiPublicMember* node)
{
    // node->memberType can contain a type name (if node is a property), use it
    DeclarationPointer decl = QmlJS::getDeclaration(
        QualifiedIdentifier(node->memberType.toString()),
        currentContext()
    );

    newUse(
        node,
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
            node,
            range.isValid() ? range : m_session->locationsToRange(
                node->firstSourceLocation(),
                node->lastSourceLocation()
            ),
            visitor.lastDeclaration()
        );
    }
}
