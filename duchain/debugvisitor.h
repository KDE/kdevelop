/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2012 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef DEBUGVISITOR_H
#define DEBUGVISITOR_H

#include <qmljs/parser/qmljsastvisitor_p.h>

#include "duchainexport.h"

class ParseSession;

class KDEVQMLJSDUCHAIN_EXPORT DebugVisitor : public QmlJS::AST::Visitor
{
public:
    DebugVisitor(const ParseSession* session);

    void startVisiting(QmlJS::AST::Node* node);

    static QString stringForAstKind(int kind);

protected:
    virtual void postVisit(QmlJS::AST::Node* node);
    virtual bool preVisit(QmlJS::AST::Node* node);

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
