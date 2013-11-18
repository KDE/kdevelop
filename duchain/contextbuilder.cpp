/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
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

#include "contextbuilder.h"
#include "expressionvisitor.h"
#include "parsesession.h"

using namespace KDevelop;

ContextBuilder::ContextBuilder()
: ContextBuilderBase()
, m_session(0)
, m_mapAst(false)
, m_editor(new Editor(&m_session))
{
}

RangeInRevision ContextBuilder::editorFindRange(QmlJS::AST::Node* fromNode, QmlJS::AST::Node* toNode)
{
    return m_session->editorFindRange(fromNode, toNode);
}

QualifiedIdentifier ContextBuilder::identifierForNode(QmlJS::AST::IdentifierPropertyName* node)
{
    return QualifiedIdentifier(node->id.toString());
}

AbstractType::Ptr ContextBuilder::findType(QmlJS::AST::Node* node)
{
    ExpressionVisitor visitor(currentContext());
    QmlJS::AST::Node::accept(node, &visitor);
    return visitor.lastType();
}

void ContextBuilder::setContextOnNode(QmlJS::AST::Node* node, DUContext* context)
{
    m_session->setContextOnNode(node, context);
}

DUContext* ContextBuilder::contextFromNode(QmlJS::AST::Node* node)
{
    return m_session->contextFromNode(node);
}

void ContextBuilder::startVisiting(QmlJS::AST::Node* node)
{
    QmlJS::AST::Node::accept(node, this);
}

TopDUContext* ContextBuilder::newTopContext(const RangeInRevision& range, ParsingEnvironmentFile* file)
{
    if (!file) {
        file = new ParsingEnvironmentFile(m_session->url());
        /// identify environment files from this language plugin
        file->setLanguage(m_session->languageString());
    }
    return ContextBuilderBase::newTopContext(range, file);
}

void ContextBuilder::setParseSession(ParseSession* session)
{
    m_session = session;
}

Editor* ContextBuilder::editor() const
{
    return m_editor.data();
}

bool ContextBuilder::visit(QmlJS::AST::FunctionDeclaration* node)
{
    return visitFunction(node);
}

bool ContextBuilder::visit(QmlJS::AST::FunctionExpression* node)
{
    return visitFunction(node);
}

bool ContextBuilder::visitFunction(QmlJS::AST::FunctionExpression* node)
{
    const QualifiedIdentifier functionName(node->name.toString());

    const RangeInRevision pRange(m_session->locationToRange(node->lparenToken).end,
                                 m_session->locationToRange(node->rparenToken).start);
    DUContext* parameters = openContext(node, pRange, DUContext::Function, functionName);
    QmlJS::AST::Node::accept(node->formals, this);
    closeContext();

    const RangeInRevision bRange(m_session->locationToRange(node->lbraceToken).end,
                                 m_session->locationToRange(node->rbraceToken).start);
    DUContext* body = openContext(node, bRange, DUContext::Other, functionName);
    if (compilingContexts()) {
        DUChainWriteLocker lock;
        body->addImportedParentContext(parameters);
    }
    QmlJS::AST::Node::accept(node->body, this);
    closeContext();

    // return false, we visited the children manually
    return false;
}

bool ContextBuilder::visit(QmlJS::AST::UiObjectInitializer* node)
{
    const RangeInRevision range(m_session->locationToRange(node->lbraceToken).end,
                                m_session->locationToRange(node->rbraceToken).start);
    openContext(node, range, DUContext::Class);

    return true;
}

void ContextBuilder::endVisit(QmlJS::AST::UiObjectInitializer* /*node*/)
{
    closeContext();
}
