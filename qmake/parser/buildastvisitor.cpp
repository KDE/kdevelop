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

#include <kdebug.h>

namespace QMake
{

BuildASTVisitor::BuildASTVisitor(parser* parser, ProjectAST* project)
    : m_parser(parser)
{
    aststack.push(project);
}

BuildASTVisitor::~BuildASTVisitor()
{
    aststack.clear();
    m_parser = 0;
}

void BuildASTVisitor::visit_arg_list( arg_list_ast *node )
{
    //Nothing to be done here as we just need to iterate through the items
    default_visitor::visit_arg_list(node);
}

void BuildASTVisitor::visit_function_args( function_args_ast *node )
{
    //Nothing to be done here as we just need to iterate through the items
    default_visitor::visit_function_args(node);
}

void BuildASTVisitor::visit_or_op( or_op_ast *node )
{
    //Nothing to be done here as we just need to iterate through the items
    default_visitor::visit_or_op(node);
}

void BuildASTVisitor::visit_item( item_ast *node )
{
    if( node->func_args )
    {
        FunctionCallAST* call = new FunctionCallAST( aststack.top() );
        ValueAST* val = new ValueAST();
        val->setValue( getTokenString( node->id ) );
        QPair<std::size_t,std::size_t> line_col = getTokenLineAndColumn(node->id);
        val->setLine( line_col.first );
        val->setColumn( line_col.second );
        call->setFunctionName( val );
        OrAST* orast = stackTop<OrAST>();
        orast->addScope( call );
        aststack.push( call );
        default_visitor::visit_item( node );
        aststack.pop();
    }else
    {
        SimpleScopeAST* simple = new SimpleScopeAST( aststack.top() );
        ValueAST* val = new ValueAST();
        val->setValue( getTokenString( node->id ) );
        QPair<std::size_t,std::size_t> line_col = getTokenLineAndColumn(node->id);
        val->setLine( line_col.first );
        val->setColumn( line_col.second );
        simple->setScopeName( val );
        OrAST* orast = stackTop<OrAST>();
        orast->addScope( simple );
        default_visitor::visit_item( node );
    }
}

void BuildASTVisitor::visit_scope( scope_ast *node )
{
    if( node->or_op )
    {
        OrAST* orast = new OrAST( aststack.top() );
        if( node->func_args )
        {
            FunctionCallAST* ast = new FunctionCallAST( orast );
            aststack.push( ast );
            visit_node( node->func_args );
            aststack.pop();
            orast->addScope( ast );
        }else
        {
            SimpleScopeAST* simple = new SimpleScopeAST( orast );
            orast->addScope( simple );
        }
        aststack.push(orast);
        visit_node( node->or_op );
    }else
    {
        if( node->func_args )
        {
            FunctionCallAST* call = new FunctionCallAST( aststack.top() );
            aststack.push( call );
        }else
        {
            SimpleScopeAST* simple = new SimpleScopeAST( aststack.top() );
            aststack.push( simple );
        }
        visit_node( node->func_args );
    }
    if( node->scope_body )
    {
        ScopeBodyAST* scopebody = new ScopeBodyAST(aststack.top());
        ScopeAST* scope = stackTop<ScopeAST>();
        scope->setScopeBody( scopebody );
        aststack.push( scopebody );
        visit_node( node->scope_body );
        aststack.pop();
    }
}

void BuildASTVisitor::visit_op( op_ast *node )
{
    AssignmentAST* assign = stackTop<AssignmentAST>();
    ValueAST* val = new ValueAST();
    val->setValue( getTokenString( node->optoken ) );
    QPair<std::size_t,std::size_t> line_col = getTokenLineAndColumn( node->optoken );
    val->setLine( line_col.first );
    val->setColumn( line_col.second );
    assign->setOp( val );
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
    if( !node->isNewline )
    {
        StatementAST* stmt = stackPop<StatementAST>();
        ValueAST* val = new ValueAST();
        val->setValue( getTokenString( node->id ) );
        QPair<std::size_t,std::size_t> line_col = getTokenLineAndColumn( node->id );
        val->setLine( line_col.first );
        val->setColumn( line_col.second );
        if( node->isExclam )
        {
            val->setValue( "!"+val->value() );
        }
        stmt->setIdentifier( val );
        ScopeBodyAST* scope = stackTop<ScopeBodyAST>();
        scope->addStatement(stmt);
    }
}

void BuildASTVisitor::visit_value( value_ast *node )
{
    AssignmentAST* assign = dynamic_cast<AssignmentAST*>( aststack.top() );
    if( assign )
    {
        ValueAST* value = new ValueAST( assign );
        value->setValue( getTokenString(node->value) );
        assign->addValue( value );
    }else
    {
        FunctionCallAST* call = stackTop<FunctionCallAST>();
        ValueAST* value = new ValueAST( call );
        value->setValue( getTokenString(node->value) );
        QPair<std::size_t,std::size_t> line_col = getTokenLineAndColumn(node->value);
        value->setLine( line_col.first );
        value->setColumn( line_col.second );
        call->addArgument( value );
    }
    default_visitor::visit_value(node);
}

void BuildASTVisitor::visit_value_list( value_list_ast *node )
{
    default_visitor::visit_value_list(node);
}

void BuildASTVisitor::visit_variable_assignment( variable_assignment_ast *node )
{
    AssignmentAST* assign = new AssignmentAST(aststack.top());
    aststack.push(assign);
    default_visitor::visit_variable_assignment(node);
}

template <typename T> T* BuildASTVisitor::stackTop()
{
    if( aststack.isEmpty() )
    {
        kDebug(9024) << kBacktrace() << endl;
        kFatal(9024) << k_funcinfo << "ERROR: AST stack is empty, this should never happen" << endl;
        exit(255);
    }
    T* ast = dynamic_cast<T*>(aststack.top());
    if( !ast )
    {
        kDebug(9024) << kBacktrace() << endl;
        kFatal(9024) << k_funcinfo << "ERROR: AST stack is screwed, doing a hard exit " << aststack.top()->type() << endl;
        exit(255);
    }
    return ast;
}

template <typename T> T* BuildASTVisitor::stackPop()
{
    if( aststack.isEmpty() )
    {
        kDebug(9024) << kBacktrace() << endl;
        kFatal(9024) << k_funcinfo << "ERROR: AST stack is empty, this should never happen" << endl;
        exit(255);
    }
    AST* tmp = aststack.pop();
    T* ast = dynamic_cast<T*>(tmp);
    if( !ast )
    {
        kDebug(9024) << kBacktrace() << endl;
        kFatal(9024) << k_funcinfo << "ERROR: AST stack is screwed, doing a hard exit" << tmp->type() << endl;
        exit(255);
    }
    return ast;
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
