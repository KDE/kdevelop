/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include <language/duchain/builders/abstractcontextbuilder.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/declaration.h>

#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsdocument.h>

#include "duchainexport.h"

class ParseSession;

using ContextBuilderBase = KDevelop::AbstractContextBuilder<QmlJS::AST::Node, QmlJS::AST::IdentifierPropertyName>;

class KDEVQMLJSDUCHAIN_EXPORT ContextBuilder : public ContextBuilderBase, public QmlJS::AST::Visitor
{
public:
    struct ExpressionType
    {
        KDevelop::AbstractType::Ptr type;
        KDevelop::DeclarationPointer declaration;
        bool isPrototype;
    };

public:
    ContextBuilder();

    void startVisiting(QmlJS::AST::Node* node) override;
    KDevelop::RangeInRevision editorFindRange(QmlJS::AST::Node* fromNode, QmlJS::AST::Node* toNode) override;
    KDevelop::QualifiedIdentifier identifierForNode(QmlJS::AST::IdentifierPropertyName* node) override;

    void setContextOnNode(QmlJS::AST::Node* node, KDevelop::DUContext* context) override;
    KDevelop::DUContext* contextFromNode(QmlJS::AST::Node* node) override;

    KDevelop::TopDUContext* newTopContext(const KDevelop::RangeInRevision& range,
                                          KDevelop::ParsingEnvironmentFile* file = nullptr) override;
    KDevelop::DUContext* newContext(const KDevelop::RangeInRevision& range) override;

    ExpressionType findType(QmlJS::AST::Node* node);
    void setParseSession(ParseSession* session);

protected:
    ParseSession* m_session;

};

#endif // CONTEXTBUILDER_H
