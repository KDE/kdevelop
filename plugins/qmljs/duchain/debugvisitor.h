/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DEBUGVISITOR_H
#define DEBUGVISITOR_H

#include <qmljs/parser/qmljsastvisitor_p.h>

#include "duchainexport.h"

class ParseSession;

class KDEVQMLJSDUCHAIN_EXPORT DebugVisitor : public QmlJS::AST::Visitor
{
public:
    explicit DebugVisitor(const ParseSession* session);

    void startVisiting(QmlJS::AST::Node* node);

protected:
    void postVisit(QmlJS::AST::Node* node) override;
    bool preVisit(QmlJS::AST::Node* node) override;
    void throwRecursionDepthError() override;

private:
    QString indent() const;
    enum Position {
        Start,
        End
    };
    void printNode(QmlJS::AST::Node* node, Position position);

    const ParseSession* m_session;
    uint m_depth;
};

#endif // DEBUGVISITOR_H
