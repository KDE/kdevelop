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

#include "debugvisitor.h"

#include "parsesession.h"

#include <qmljs/parser/qmljsast_p.h>

using namespace KDevelop;

QTextStream qout(stdout);

DebugVisitor::DebugVisitor(ParseSession* session)
: m_session(session)
, m_depth(0)
{
}

void DebugVisitor::startVisiting(QmlJS::AST::Node* node)
{
    QmlJS::AST::Node::accept(node, this);
}

bool DebugVisitor::preVisit(QmlJS::AST::Node* node)
{
    qout << indent() << node->kind << " " << m_session->stringForLocation(node->firstSourceLocation()) << endl;
    ++m_depth;
    return true;
}

void DebugVisitor::postVisit(QmlJS::AST::Node* node)
{
    Q_ASSERT(m_depth);
    --m_depth;
    qout << indent() << node->kind << " " << m_session->stringForLocation(node->lastSourceLocation()) << endl;
}

QString DebugVisitor::indent() const
{
    return QString().fill(' ', m_depth * 2);
}
