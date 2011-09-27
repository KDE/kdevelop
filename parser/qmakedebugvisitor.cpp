/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "qmakedebugvisitor.h"

#include <kdebug.h>

#include "qmakeparser.h"
#include "qmakeast.h"
#include "kdev-pg-token-stream.h"

namespace QMake
{

DebugVisitor::DebugVisitor(QMake::Parser* parser)
    : m_parser(parser), indent(0)
{
}

QString DebugVisitor::getTokenInfo(qint64 idx)
{
    qint64 line,col;
    QMake::Parser::Token token = m_parser->tokenStream->at(idx);
    m_parser->tokenStream->startPosition(idx,&line,&col);
    return QString("%1,%2,%3")
            .arg(line)
            .arg(col)
            .arg(m_parser->tokenText(token.begin,token.end).replace('\n',"\\n"));
}

QString DebugVisitor::getIndent()
{
    return QString().fill(' ', indent*4);
}

void DebugVisitor::visitArgumentList( ArgumentListAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(arg_list)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    DefaultVisitor::visitArgumentList( node );
    indent--;
    kDebug(9024) << getIndent() << "END(arg_list)(" << getTokenInfo(node->startToken) << ")";
}


void DebugVisitor::visitOrOperator( OrOperatorAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(or_op)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    DefaultVisitor::visitOrOperator( node );
    indent--;
    kDebug(9024) << getIndent() << "END(or_op)(" << getTokenInfo(node->endToken)  << ")";
}

void DebugVisitor::visitScope( ScopeAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(scope)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    visitNode( node->functionArguments );
    visitNode( node->orOperator );
    visitNode( node->scopeBody );
    indent--;
    kDebug(9024) << getIndent() << "END(scope)(" << getTokenInfo(node->endToken)  << ")";

}

void DebugVisitor::visitFunctionArguments( FunctionArgumentsAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(function_args)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    DefaultVisitor::visitFunctionArguments( node );
    indent--;
    kDebug(9024) << getIndent() << "END(function_args)(" << getTokenInfo(node->endToken)  << ")";
}

void DebugVisitor::visitOp( OpAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(op)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    kDebug(9024) << getIndent() << "optoken =" << getTokenInfo(node->optoken);
    DefaultVisitor::visitOp( node );
    indent--;
    kDebug(9024) << getIndent() << "END(op)(" << getTokenInfo(node->endToken)  << "))";
}

void DebugVisitor::visitProject( ProjectAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(project)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    DefaultVisitor::visitProject( node );
    indent--;
    kDebug(9024) << getIndent() << "END(project)(" << getTokenInfo(node->endToken)  << ")";
}

void DebugVisitor::visitScopeBody( ScopeBodyAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(scope_body)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    DefaultVisitor::visitScopeBody( node );
    indent--;
    kDebug(9024) << getIndent() << "END(scope_body)(" << getTokenInfo(node->endToken)  << ")";
}

void DebugVisitor::visitStatement( StatementAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(stmt)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    kDebug(9024) << getIndent() << "isExclam=" << node->isExclam;
    if( !node->isNewline )
    {
        kDebug(9024) << getIndent() << "id=" << getTokenInfo(node->id);
    }
    DefaultVisitor::visitStatement( node );
    indent--;
    kDebug(9024) << getIndent() << "END(stmt)(" << getTokenInfo(node->endToken)  << ")";
}

void DebugVisitor::visitValue( ValueAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(value)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    kDebug(9024) << getIndent() << "value=" << getTokenInfo(node->value);
    DefaultVisitor::visitValue( node );
    indent--;
    kDebug(9024) << getIndent() << "END(value)(" << getTokenInfo(node->endToken)  << ")";
}

void DebugVisitor::visitValueList( ValueListAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(value_list)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    DefaultVisitor::visitValueList( node );
    indent--;
    kDebug(9024) << getIndent() << "END(value_list)(" << getTokenInfo(node->endToken)  << ")";
}

void DebugVisitor::visitVariableAssignment( VariableAssignmentAst *node )
{
    kDebug(9024) << getIndent() << "BEGIN(variable_assignment)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    DefaultVisitor::visitVariableAssignment( node );
    indent--;
    kDebug(9024) << getIndent() << "END(variable_assignment)(" << getTokenInfo(node->endToken)  << ")";
}

void DebugVisitor::visitItem( ItemAst *node )
{

    kDebug(9024) << getIndent() << "BEGIN(item)(" << getTokenInfo(node->startToken)  << ")";
    indent++;
    kDebug(9024) << getIndent() << "id=" << getTokenInfo(node->id);
    DefaultVisitor::visitItem( node );
    indent--;
    kDebug(9024) << getIndent() << "END(item)(" << getTokenInfo(node->endToken)  << ")";
}

}
