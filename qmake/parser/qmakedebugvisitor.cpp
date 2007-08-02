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

#include "qmake_parser.h"
#include "qmake_ast.h"
#include "kdev-pg-token-stream.h"

namespace QMake
{

DebugVisitor::DebugVisitor(QMake::parser* parser)
    : m_parser(parser), indent(0)
{
}

QString DebugVisitor::getTokenInfo(std::size_t idx)
{
    std::size_t line,col;
    QMake::parser::token_type token = m_parser->token_stream->token(idx);
    m_parser->token_stream->start_position(idx,&line,&col);
    return QString("%1,%2,%3")
            .arg(line)
            .arg(col)
            .arg(m_parser->tokenText(token.begin,token.end).replace("\n","\\n"));
}

QString DebugVisitor::getIndent()
{
    return QString().fill(' ', indent*4);
}

void DebugVisitor::visit_arg_list( arg_list_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(arg_list)(" << getTokenInfo(node->start_token) << ")";
    indent++;
    default_visitor::visit_arg_list( node );
    indent--;
    kDebug(9024) << getIndent() << "END(arg_list)(" << getTokenInfo(node->start_token) << ")";
}


void DebugVisitor::visit_or_op( or_op_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(or_op)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    default_visitor::visit_or_op( node );
    indent--;
    kDebug(9024) << getIndent() << "END(or_op)(" << getTokenInfo(node->end_token)  << ")";
}

void DebugVisitor::visit_scope( scope_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(scope)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    visit_node( node->func_args );
    visit_node( node->or_op );
    visit_node( node->scope_body );
    indent--;
    kDebug(9024) << getIndent() << "END(scope)(" << getTokenInfo(node->end_token)  << ")";

}

void DebugVisitor::visit_function_args( function_args_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(function_args)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    default_visitor::visit_function_args( node );
    indent--;
    kDebug(9024) << getIndent() << "END(function_args)(" << getTokenInfo(node->end_token)  << ")";
}

void DebugVisitor::visit_op( op_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(op)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    kDebug(9024) << getIndent() << "optoken =" << getTokenInfo(node->optoken);
    default_visitor::visit_op( node );
    indent--;
    kDebug(9024) << getIndent() << "END(op)(" << getTokenInfo(node->end_token)  << "))";
}

void DebugVisitor::visit_project( project_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(project)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    default_visitor::visit_project( node );
    indent--;
    kDebug(9024) << getIndent() << "END(project)(" << getTokenInfo(node->end_token)  << ")";
}

void DebugVisitor::visit_scope_body( scope_body_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(scope_body)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    default_visitor::visit_scope_body( node );
    indent--;
    kDebug(9024) << getIndent() << "END(scope_body)(" << getTokenInfo(node->end_token)  << ")";
}

void DebugVisitor::visit_stmt( stmt_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(stmt)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    kDebug(9024) << getIndent() << "isExclam=" << node->isExclam;
    if( !node->isNewline )
    {
        kDebug(9024) << getIndent() << "id=" << getTokenInfo(node->id);
    }
    default_visitor::visit_stmt( node );
    indent--;
    kDebug(9024) << getIndent() << "END(stmt)(" << getTokenInfo(node->end_token)  << ")";
}

void DebugVisitor::visit_value( value_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(value)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    kDebug(9024) << getIndent() << "value=" << getTokenInfo(node->value);
    default_visitor::visit_value( node );
    indent--;
    kDebug(9024) << getIndent() << "END(value)(" << getTokenInfo(node->end_token)  << ")";
}

void DebugVisitor::visit_value_list( value_list_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(value_list)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    default_visitor::visit_value_list( node );
    indent--;
    kDebug(9024) << getIndent() << "END(value_list)(" << getTokenInfo(node->end_token)  << ")";
}

void DebugVisitor::visit_variable_assignment( variable_assignment_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(variable_assignment)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    default_visitor::visit_variable_assignment( node );
    indent--;
    kDebug(9024) << getIndent() << "END(variable_assignment)(" << getTokenInfo(node->end_token)  << ")";
}

void DebugVisitor::visit_item( item_ast *node )
{

    kDebug(9024) << getIndent() << "BEGIN(item)(" << getTokenInfo(node->start_token)  << ")";
    indent++;
    kDebug(9024) << getIndent() << "id=" << getTokenInfo(node->id);
    default_visitor::visit_item( node );
    indent--;
    kDebug(9024) << getIndent() << "END(item)(" << getTokenInfo(node->end_token)  << ")";
}

}
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
