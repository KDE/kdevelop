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

#include <QtCore/QDebug>

#include "qmake_parser.h"
#include "kdev-pg-token-stream.h"

namespace QMake
{

DebugVisitor::DebugVisitor(QMake::parser* parser)
    : mParser(parser)
{
}

QString DebugVisitor::getTokenInfo(std::size_t idx, std::size_t* line, std::size_t* col)
{
    QMake::parser::token_type token = mParser->token_stream->token(idx);
    mParser->token_stream->start_position(idx,line,col);
    return mParser->tokenText(token.begin,token.end);
}

void DebugVisitor::visit_arg_list( arg_list_ast *node )
{
    qDebug() <<  "BEGIN(arg_list)";
    default_visitor::visit_arg_list( node );
    qDebug() <<  "END(arg_list)";
}

void DebugVisitor::visit_argument( argument_ast *node )
{
    qDebug() <<  "BEGIN(argument)";
    default_visitor::visit_argument( node );
    qDebug() <<  "END(argument)";
}

void DebugVisitor::visit_funcref( funcref_ast *node )
{
    qDebug() <<  "BEGIN(funcref)";
    default_visitor::visit_funcref( node );
    qDebug() <<  "END(funcref)";
}

void DebugVisitor::visit_function_args( function_args_ast *node )
{
    qDebug() <<  "BEGIN(function_args)";
    default_visitor::visit_function_args( node );
    qDebug() <<  "END(function_args)";
}

void DebugVisitor::visit_function_scope( function_scope_ast *node )
{
    qDebug() <<  "BEGIN(function_scope)";
    default_visitor::visit_function_scope( node );
    qDebug() <<  "END(function_scope)";
}

void DebugVisitor::visit_id_or_value( id_or_value_ast *node )
{
    qDebug() <<  "BEGIN(id_or_value)";
    default_visitor::visit_id_or_value( node );
    qDebug() <<  "END(id_or_value)";
}

void DebugVisitor::visit_op( op_ast *node )
{
    std::size_t line,col;
    QString txt = getTokenInfo(node->optoken,&line,&col);
    qDebug() <<  "BEGIN(op, optoken(" << line << col << txt << "))";
    default_visitor::visit_op( node );
    qDebug() <<  "END(op, optoken(" << line << col << txt << "))";
}

void DebugVisitor::visit_project( project_ast *node )
{
    qDebug() <<  "BEGIN(project)";
    default_visitor::visit_project( node );
    qDebug() <<  "END(project)";
}

void DebugVisitor::visit_quote_value( quote_value_ast *node )
{
    qDebug() <<  "BEGIN(quote_value)";
    default_visitor::visit_quote_value( node );
    qDebug() <<  "END(quote_value)";
}

void DebugVisitor::visit_quoted_value( quoted_value_ast *node )
{
    qDebug() <<  "BEGIN(quoted_value)";
    default_visitor::visit_quoted_value( node );
    qDebug() <<  "END(quoted_value)";
}

void DebugVisitor::visit_ref( ref_ast *node )
{
    qDebug() <<  "BEGIN(ref)";
    default_visitor::visit_ref( node );
    qDebug() <<  "END(ref)";
}

void DebugVisitor::visit_scope_body( scope_body_ast *node )
{
    qDebug() <<  "BEGIN(scope_body)";
    default_visitor::visit_scope_body( node );
    qDebug() <<  "END(scope_body)";
}

void DebugVisitor::visit_stmt( stmt_ast *node )
{
    std::size_t line;
    std::size_t col;
    QString txt = getTokenInfo(node->id, &line, &col);
    qDebug() <<  "BEGIN(stmt,id(" << line << col << txt << "))";
    default_visitor::visit_stmt( node );
    qDebug() <<  "END(stmt,id(" << line << col << txt << "))";
}

void DebugVisitor::visit_value( value_ast *node )
{
    qDebug() <<  "BEGIN(value)";
    default_visitor::visit_value( node );
    qDebug() <<  "END(value)";
}

void DebugVisitor::visit_value_list( value_list_ast *node )
{
    qDebug() <<  "BEGIN(value_list)";
    default_visitor::visit_value_list( node );
    qDebug() <<  "END(value_list)";
}

void DebugVisitor::visit_variable_assignment( variable_assignment_ast *node )
{
    qDebug() <<  "BEGIN(variable_assignment)";
    default_visitor::visit_variable_assignment( node );
    qDebug() <<  "END(variable_assignment)";
}

void DebugVisitor::visit_varref( varref_ast *node )
{
    qDebug() <<  "BEGIN(varref)";
    default_visitor::visit_varref( node );
    qDebug() <<  "END(varref)";
}

}
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
