/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "contextbuilder.h"
#include "expressionvisitor.h"
#include "parsesession.h"
#include "qmljsducontext.h"

using namespace KDevelop;

ContextBuilder::ContextBuilder()
: ContextBuilderBase()
, m_session(nullptr)
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

ContextBuilder::ExpressionType ContextBuilder::findType(QmlJS::AST::Node* node)
{
    ExpressionType ret;

    if (!node) {
        ret.type = AbstractType::Ptr(new IntegralType(IntegralType::TypeMixed));
        ret.isPrototype = false;

        return ret;
    }

    ExpressionVisitor visitor(currentContext());

    // Build every needed declaration in node, and then try to guess its type
    node->accept(this);
    node->accept(&visitor);

    ret.type = visitor.lastType();
    ret.declaration = visitor.lastDeclaration();
    ret.isPrototype = visitor.isPrototype();

    return ret;
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

    return new QmlJS::QmlJSTopDUContext(m_session->url(), range, file);
}

DUContext* ContextBuilder::newContext(const RangeInRevision& range)
{
    return new QmlJS::QmlJSNormalDUContext(range, currentContext());
}

void ContextBuilder::setParseSession(ParseSession* session)
{
    m_session = session;
}
