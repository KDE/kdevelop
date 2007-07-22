/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <pakulat@rostock.zgdv.de>                     *
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

#include "buildastvisitor.h"

#include "qmakeast.h"
#include "qmake_parser.h"
#include "qmake_ast.h"

#include <QtCore/QPair>

namespace QMake
{

BuildASTVisitor::BuildASTVisitor(parser* parser, ProjectAST* project)
    : m_parser(parser)
{
    aststack.push(project);
}

void BuildASTVisitor::visit_arg_list( arg_list_ast *node )
{
    default_visitor::visit_arg_list(node);
}

void BuildASTVisitor::visit_function_args( function_args_ast *node )
{
    default_visitor::visit_function_args(node);
}

void BuildASTVisitor::visit_or_op( or_op_ast *node )
{
    default_visitor::visit_or_op(node);
}

void BuildASTVisitor::visit_item( item_ast *node )
{
    default_visitor::visit_item(node);
}

void BuildASTVisitor::visit_scope( scope_ast *node )
{
    default_visitor::visit_scope(node);
}

void BuildASTVisitor::visit_op( op_ast *node )
{
    default_visitor::visit_op(node);
}

void BuildASTVisitor::visit_project( project_ast *node )
{
    default_visitor::visit_project(node);
}

void BuildASTVisitor::visit_scope_body( scope_body_ast *node )
{
    default_visitor::visit_scope_body(node);
}

void BuildASTVisitor::visit_stmt( stmt_ast *node )
{
    default_visitor::visit_stmt(node);
}

void BuildASTVisitor::visit_value( value_ast *node )
{
    default_visitor::visit_value(node);
}

void BuildASTVisitor::visit_value_list( value_list_ast *node )
{
    default_visitor::visit_value_list(node);
}

void BuildASTVisitor::visit_variable_assignment( variable_assignment_ast *node )
{
    default_visitor::visit_variable_assignment(node);
}

QString BuildASTVisitor::getTokenString(std::size_t idx)
{
    QMake::parser::token_type token = m_parser->token_stream->token(idx);
    return m_parser->tokenText(token.begin,token.end).replace("\n","\\n");
}

QPair<std::size_t,std::size_t> BuildASTVisitor::getTokenLineAndColumn( std::size_t idx )
{
    QPair<std::size_t,std::size_t> info;
    std::size_t line,col;
    QMake::parser::token_type token = m_parser->token_stream->token(idx);
    m_parser->token_stream->start_position(idx,&line,&col);
    info.first = line;
    info.second = col;
    return info;
}

}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
