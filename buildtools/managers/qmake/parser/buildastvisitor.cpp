/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
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
#include "qmakeparser.h"
#include "ast.h"

#include <QtCore/QPair>

#include <kdebug.h>

namespace QMake
{

BuildASTVisitor::BuildASTVisitor(Parser* parser, ProjectAST* project)
    : m_parser(parser)
{
    aststack.push(project);
}

BuildASTVisitor::~BuildASTVisitor()
{
    aststack.clear();
    m_parser = 0;
}

void BuildASTVisitor::visitArgumentList( ArgumentListAst *node )
{
    //Nothing to be done here as we just need to iterate through the items
    DefaultVisitor::visitArgumentList(node);
}

void BuildASTVisitor::visitFunctionArguments( FunctionArgumentsAst *node )
{
    //Nothing to be done here as we just need to iterate through the items
    DefaultVisitor::visitFunctionArguments(node);
}

void BuildASTVisitor::visitOrOperator( OrOperatorAst *node )
{
    //Nothing to be done here as we just need to iterate through the items
    DefaultVisitor::visitOrOperator(node);
}

void BuildASTVisitor::visitItem( ItemAst *node )
{
    if( node->functionArguments )
    {
        FunctionCallAST* call = new FunctionCallAST( aststack.top() );
        ValueAST* val = new ValueAST();
        val->setValue( getTokenString( node->id ) );
        QPair<qint64,qint64> line_col = getTokenLineAndColumn(node->id);
        val->setLine( line_col.first );
        val->setColumn( line_col.second );
        call->setFunctionName( val );
        OrAST* orast = stackTop<OrAST>();
        orast->addScope( call );
        aststack.push( call );
        DefaultVisitor::visitItem( node );
        aststack.pop();
    }else
    {
        SimpleScopeAST* simple = new SimpleScopeAST( aststack.top() );
        ValueAST* val = new ValueAST();
        val->setValue( getTokenString( node->id ) );
        QPair<qint64,qint64> line_col = getTokenLineAndColumn(node->id);
        val->setLine( line_col.first );
        val->setColumn( line_col.second );
        simple->setScopeName( val );
        OrAST* orast = stackTop<OrAST>();
        orast->addScope( simple );
        DefaultVisitor::visitItem( node );
    }
}

void BuildASTVisitor::visitScope( ScopeAst *node )
{
    if( node->orOperator )
    {
        OrAST* orast = new OrAST( aststack.top() );
        if( node->functionArguments )
        {
            FunctionCallAST* ast = new FunctionCallAST( orast );
            aststack.push( ast );
            visitNode( node->functionArguments );
            aststack.pop();
            orast->addScope( ast );
        }else
        {
            SimpleScopeAST* simple = new SimpleScopeAST( orast );
            orast->addScope( simple );
        }
        aststack.push(orast);
        visitNode( node->orOperator );
    }else
    {
        if( node->functionArguments )
        {
            FunctionCallAST* call = new FunctionCallAST( aststack.top() );
            aststack.push( call );
            visitNode( node->functionArguments );
        }else
        {
            SimpleScopeAST* simple = new SimpleScopeAST( aststack.top() );
            aststack.push( simple );
        }
    }
    if( node->scopeBody )
    {
        ScopeBodyAST* scopebody = new ScopeBodyAST(aststack.top());
        ScopeAST* scope = stackTop<ScopeAST>();
        scope->setScopeBody( scopebody );
        aststack.push( scopebody );
        visitNode( node->scopeBody );
        aststack.pop();
    }
}

void BuildASTVisitor::visitOp( OpAst *node )
{
    AssignmentAST* assign = stackTop<AssignmentAST>();
    ValueAST* val = new ValueAST();
    val->setValue( getTokenString( node->optoken ) );
    QPair<qint64,qint64> line_col = getTokenLineAndColumn( node->optoken );
    val->setLine( line_col.first );
    val->setColumn( line_col.second );
    assign->setOp( val );
    DefaultVisitor::visitOp(node);
}

void BuildASTVisitor::visitProject( ProjectAst *node )
{
    DefaultVisitor::visitProject(node);
}

void BuildASTVisitor::visitScopeBody( ScopeBodyAst *node )
{
    DefaultVisitor::visitScopeBody(node);
}

void BuildASTVisitor::visitStatement( StatementAst *node )
{
    DefaultVisitor::visitStatement(node);
    if( !node->isNewline )
    {
        StatementAST* stmt = stackPop<StatementAST>();
        ValueAST* val = new ValueAST();
        val->setValue( getTokenString( node->id ) );
        QPair<qint64,qint64> line_col = getTokenLineAndColumn( node->id );
        val->setLine( line_col.first );
        val->setColumn( line_col.second );
        if( node->isExclam )
        {
            val->setValue( '!'+val->value() );
        }
        stmt->setIdentifier( val );
        ScopeBodyAST* scope = stackTop<ScopeBodyAST>();
        scope->addStatement(stmt);
    }
}

void BuildASTVisitor::visitValue( ValueAst *node )
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
        QPair<qint64,qint64> line_col = getTokenLineAndColumn(node->value);
        value->setLine( line_col.first );
        value->setColumn( line_col.second );
        call->addArgument( value );
    }
    DefaultVisitor::visitValue(node);
}

void BuildASTVisitor::visitValueList( ValueListAst *node )
{
    DefaultVisitor::visitValueList(node);
}

void BuildASTVisitor::visitVariableAssignment( VariableAssignmentAst *node )
{
    AssignmentAST* assign = new AssignmentAST(aststack.top());
    aststack.push(assign);
    DefaultVisitor::visitVariableAssignment(node);
}

template <typename T> T* BuildASTVisitor::stackTop()
{
    if( aststack.isEmpty() )
    {
        kDebug(9024) << kBacktrace();
        kFatal(9024) << "ERROR: AST stack is empty, this should never happen";
        exit(255);
    }
    T* ast = dynamic_cast<T*>(aststack.top());
    if( !ast )
    {
        kDebug(9024) << kBacktrace();
        kFatal(9024) << "ERROR: AST stack is screwed, doing a hard exit" << aststack.top()->type();
        exit(255);
    }
    return ast;
}

template <typename T> T* BuildASTVisitor::stackPop()
{
    if( aststack.isEmpty() )
    {
        kDebug(9024) << kBacktrace();
        kFatal(9024) << "ERROR: AST stack is empty, this should never happen";
        exit(255);
    }
    AST* tmp = aststack.pop();
    T* ast = dynamic_cast<T*>(tmp);
    if( !ast )
    {
        kDebug(9024) << kBacktrace();
        kFatal(9024) << "ERROR: AST stack is screwed, doing a hard exit" << tmp->type();
        exit(255);
    }
    return ast;
}

QString BuildASTVisitor::getTokenString(qint64 idx)
{
    QMake::Parser::Token token = m_parser->tokenStream->token(idx);
    return m_parser->tokenText(token.begin,token.end).replace("\n","\\n");
}

QPair<qint64,qint64> BuildASTVisitor::getTokenLineAndColumn( qint64 idx )
{
    QPair<qint64,qint64> info;
    qint64 line,col;
    QMake::Parser::Token token = m_parser->tokenStream->token(idx);
    m_parser->tokenStream->startPosition(idx,&line,&col);
    info.first = line;
    info.second = col;
    return info;
}

}

