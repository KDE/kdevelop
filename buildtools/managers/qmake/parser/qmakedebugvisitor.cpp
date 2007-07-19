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
#include "kdev-pg-token-stream.h"

namespace QMake
{

DebugVisitor::DebugVisitor(QMake::parser* parser)
    : mParser(parser), indent(0)
{
}

QString DebugVisitor::getTokenInfo(std::size_t idx)
{
    std::size_t line,col;
    QMake::parser::token_type token = mParser->token_stream->token(idx);
    mParser->token_stream->start_position(idx,&line,&col);
    return QString("%1,%2,%3")
            .arg(line)
            .arg(col)
            .arg(mParser->tokenText(token.begin,token.end).replace("\n","\\n"));
}

QString DebugVisitor::getIndent()
{
    return QString().fill(' ', indent*4);
}

void DebugVisitor::visit_arg_list( arg_list_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(arg_list)( " << getTokenInfo(node->start_token) << " )" << endl;
    indent++;
    default_visitor::visit_arg_list( node );
    indent--;
    kDebug(9024) << getIndent() << "END(arg_list)( " << getTokenInfo(node->start_token) << " )" << endl;
}

void DebugVisitor::visit_argument( argument_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(argument)( " << getTokenInfo(node->start_token) << " )" << endl;
    indent++;
    default_visitor::visit_argument( node );
    indent--;
    kDebug(9024) << getIndent() << "END(argument)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_funcref( funcref_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(funcref)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    kDebug(9024) << getIndent() << "id=" << getTokenInfo(node->id) << endl;
    default_visitor::visit_funcref( node );
    indent--;
    kDebug(9024) << getIndent() << "END(funcref)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_function_args( function_args_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(function_args)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    default_visitor::visit_function_args( node );
    indent--;
    kDebug(9024) << getIndent() << "END(function_args)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_function_scope( function_scope_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(function_scope)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    default_visitor::visit_function_scope( node );
    indent--;
    kDebug(9024) << getIndent() << "END(function_scope)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_id_or_value( id_or_value_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(id_or_value)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    kDebug(9024) << getIndent() << "val=" << getTokenInfo(node->val) << endl;
    default_visitor::visit_id_or_value( node );
    indent--;
    kDebug(9024) << getIndent() << "END(id_or_value)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_op( op_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(op)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    kDebug(9024) << getIndent() << "optoken = " << getTokenInfo(node->optoken) << endl;
    default_visitor::visit_op( node );
    indent--;
    kDebug(9024) << getIndent() << "END(op)(" << getTokenInfo(node->end_token)  << "))" << endl;
}

void DebugVisitor::visit_project( project_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(project)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    default_visitor::visit_project( node );
    indent--;
    kDebug(9024) << getIndent() << "END(project)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_quote_value( quote_value_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(quote_value)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    if( node->token )
    {
        kDebug(9024) << getIndent() << "token=" << getTokenInfo(node->token) << endl;
    }
    default_visitor::visit_quote_value( node );
    indent--;
    kDebug(9024) << getIndent() << "END(quote_value)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_quoted_value( quoted_value_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(quoted_value)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    default_visitor::visit_quoted_value( node );
    indent--;
    kDebug(9024) << getIndent() << "END(quoted_value)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_ref( ref_ast *node )
{
    kDebug(9024) << getIndent() << getIndent() << "BEGIN(ref)( " << getTokenInfo(node->start_token)  << ")" << endl;
    indent++;
    if( node->idref )
    {
        kDebug(9024) << getIndent() << "idref=" << getTokenInfo(node->idref) << endl;
    }
    default_visitor::visit_ref( node );
    indent--;
    kDebug(9024) << getIndent() << "END(ref)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_scope_body( scope_body_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(scope_body)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    default_visitor::visit_scope_body( node );
    indent--;
    kDebug(9024) << getIndent() << "END(scope_body)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_stmt( stmt_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(stmt)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    if( !node->isNewline )
    {
        kDebug(9024) << getIndent() << "id=" << getTokenInfo(node->id) << endl;
    }
    default_visitor::visit_stmt( node );
    indent--;
    kDebug(9024) << getIndent() << "END(stmt)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_value( value_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(value)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    default_visitor::visit_value( node );
    indent--;
    kDebug(9024) << getIndent() << "END(value)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_value_list( value_list_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(value_list)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    default_visitor::visit_value_list( node );
    indent--;
    kDebug(9024) << getIndent() << "END(value_list)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_variable_assignment( variable_assignment_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(variable_assignment)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    default_visitor::visit_variable_assignment( node );
    indent--;
    kDebug(9024) << getIndent() << "END(variable_assignment)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

void DebugVisitor::visit_varref( varref_ast *node )
{
    kDebug(9024) << getIndent() << "BEGIN(varref)( " << getTokenInfo(node->start_token)  << " )" << endl;
    indent++;
    kDebug(9024) << getIndent() << "id=" << getTokenInfo(node->id) << endl;
    default_visitor::visit_varref( node );
    indent--;
    kDebug(9024) << getIndent() << "END(varref)( " << getTokenInfo(node->end_token)  << " )" << endl;
}

}
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
