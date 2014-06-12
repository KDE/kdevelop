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

void UseBuilder::useForExpression(QmlJS::AST::Node* node, const KDevelop::RangeInRevision &range)
{
    // ExpressionVisitor can find the type and corresponding declaration of many
    // kinds of expressions (identifiers, field members, special identifiers like
    // this or parent, etc).
    ExpressionVisitor visitor(contextOnNode(node));

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

DUContext* UseBuilder::contextOnNode(QmlJS::AST::Node* node) const
{
    DUChainReadLocker lock;

    return topContext()->findContextAt(
        m_session->locationToRange(node->firstSourceLocation()).start
    );
}
