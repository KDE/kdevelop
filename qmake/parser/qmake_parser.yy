%{
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

/**
@file qmake.yy
QMake Parser

Simple LALR parser which builds the syntax tree (see @ref QMake::AST).
*/

#include <stdio.h>
#include "qmakeast.h"
#include <QtCore/QStack>
#include <QtCore/QString>

#define YYSTYPE_IS_DECLARED

using namespace QMake;

/**
 The yylval type
*/

struct Result {
    Result() : node(0) {}
    QString value;
    AST* node;
    QStringList values;
    QList<StatementAST*> stmtlist;
    QList<FunctionArgAST*> fnargs;
};

typedef Result YYSTYPE;

void yyerror(const char* str)
{
    printf("%s\n", str);
}

int yylex();

ProjectAST* project;

%}

%token WS
%token VARIABLENAME
%token FUNCTIONNAME
%token SCOPENAME
%token VARVAL
%token DOLLAR
%token COLON
%token COMMA
%token LCURLY
%token RCURLY
%token LBRACE
%token RBRACE
%token QUOTE
%token EQUAL
%token OR
%token PLUSEQ
%token MINUSEQ
%token TILDEEQ
%token STAREQ
%token NEWLINE
%token CONT
%token COMMENT
%token FUNCARGVAL
%%

project:
    {
        project = new ProjectAST();
    }
    statements
    ;

statements: statements statement
        {
            project->addStatement( static_cast<StatementAST*>($<node>2) );
        }
    |
    ;

statement: variable_assignment
        {
            $<node>$ = $<node>1;
        }
    | function
        {
            $<node>$ = $<node>1;
        }
    | scope
        {
            $<node>$ = $<node>1;
        }
    | or
        {
            $<node>$ = $<node>1;
        }
    | NEWLINE
        {
            $<node>$ = new NewlineAST();
        }
    | WS NEWLINE
        {
            $<node>$ = new NewlineAST( $<value>1 );
        }
    | COMMENT
        {
            $<node>$ = new CommentAST( $<value>1 );
        }
    | WS COMMENT
        {
            $<node>$ = new CommentAST( $<value>2, $<value>1 );
        }
    ;

or: functioncall or_op functioncall LCURLY substatements RCURLY
        {
            $<node>$ = new OrAST( static_cast<FunctionCallAST*>($<node>1), $<value>2, static_cast<FunctionCallAST*>($<node>3),
                                  $<value>4, $<stmtlist>5, $<value>6);
        }
    | functioncall or_op functioncall COLON statement
        {
            $<node>$ = new OrAST( static_cast<FunctionCallAST*>($<node>1), $<value>2, static_cast<FunctionCallAST*>($<node>3),
                                  $<value>4, static_cast<StatementAST*>($<node>5) );
        }
    ;

or_op: WS OR WS
        {
            $<value>$ = $<value>1+$<value>2+$<value>3;
        }
    | WS OR
        {
            $<value>$ = $<value>1+$<value>2;
        }
    | OR WS
        {
            $<value>$ = $<value>1+$<value>2;
        }
    | OR
        {
            $<value>$ = $<value>1;
        }
    ;


scope: SCOPENAME COLON statement
        {
            $<node>$ = new ScopeAST( $<value>1, $<value>2, static_cast<StatementAST*>($<node>3) );
        }
    | SCOPENAME LCURLY substatements RCURLY
        {
            $<node>$ = new ScopeAST( $<value>1, $<value>2, $<stmtlist>3, $<value>4 )
        }
    ;

function: functioncall LCURLY substatements RCURLY
        {
            static_cast<FunctionCallAST*>($<node>1)->setAsFunctionArg( false );
            $<node>$ = new FunctionAST( static_cast<FunctionCallAST*>($<node>1), $<value>2, $<stmtlist>3, $<value>4 )
        }
    | functioncall COLON statement
        {
            static_cast<FunctionCallAST*>($<node>1)->setAsFunctionArg( false );
            $<node>$ = new FunctionAST( static_cast<FunctionCallAST*>($<node>1), $<value>2, static_cast<StatementAST*>($<node>3) );
        }
    | functioncall
        {
            static_cast<FunctionCallAST*>($<node>1)->setAsFunctionArg( false );
            $<node>$ = new FunctionAST( static_cast<FunctionCallAST*>($<node>1) )
        }
;

functioncall: FUNCTIONNAME LBRACE funcargs RBRACE
        {
            $<node>$ = new FunctionCallAST( $<value>1, $<value>2, $<fnargs>3, $<value>4 );
        }
    ;

funcargs: funcargs COMMA funcarg
        {
            $<fnargs>$.append( static_cast<FunctionArgAST*>($<node>3));
        }
    | funcargs COMMA funcarg WS
        {
            $<fnargs>$.append( static_cast<FunctionArgAST*>($<node>3));
        }
    | funcarg
        {
            $<fnargs>$.append( static_cast<FunctionArgAST*>($<node>1));
        }
    | WS funcarg WS
        {
            $<fnargs>$.append( static_cast<FunctionArgAST*>($<node>2));
        }
    | funcarg WS
        {
            $<fnargs>$.append( static_cast<FunctionArgAST*>($<node>1));
        }
    | WS funcarg
        {
            $<fnargs>$.append( static_cast<FunctionArgAST*>($<node>2));
        }
    |
        {
            $<fnargs>$.clear();
        }
    ;

funcarg: DOLLAR DOLLAR functioncall
        {
            static_cast<FunctionCallAST*>($<node>3)->setAsFunctionArg( true );
            $<node>$ = $<node>3;
        }
    | FUNCARGVAL
        {
            $<node>$ = new SimpleFunctionArgAST($<value>1);
        }
    | QUOTE funcwsvalue QUOTE
        {
            $<node>$ = new SimpleFunctionArgAST($<value>1+$<value>2+$<value>3);
        }
    | DOLLAR DOLLAR LCURLY FUNCARGVAL RCURLY
        {
            $<node>$ = new SimpleFunctionArgAST($<value>1+$<value>2+$<value>3+$<value>4+$<value>5);
        }
    | DOLLAR DOLLAR FUNCARGVAL
        {
            $<node>$ = new SimpleFunctionArgAST($<value>1+$<value>2+$<value>3);
        }
    | DOLLAR LBRACE FUNCARGVAL RBRACE
        {
            $<node>$ = new SimpleFunctionArgAST($<value>1+$<value>2+$<value>3+$<value>4);
        }
    ;

funcwsvalue: funcwsvalue FUNCARGVAL
        {
            $<value>$ += $<value>1;
        }
    | funcwsvalue WS
        {
            $<value>$ += $<value>1;
        }
    |
        {
            $<value>$ = "";
        }
    ;


substatements: substatements statement
        {
            $<stmtlist>$.append( static_cast<StatementAST*>($<node>2));
        }
    |
        {
            $<stmtlist>$.clear();
        }
;

variable_assignment: WS VARIABLENAME op values
        {
            $<node>$ = new AssignmentAST( $<value>2, $<value>3, $<values>4, $<value>1 );
        }
    | VARIABLENAME op values
        {
            $<node>$ = new AssignmentAST( $<value>1, $<value>2, $<values>3 );
        }
    ;

op: EQUAL
        {
            $<value>$ = $<value>1;
        }
    | PLUSEQ
        {
            $<value>$ = $<value>1;
        }
    | MINUSEQ
        {
            $<value>$ = $<value>1;
        }
    | TILDEEQ
        {
            $<value>$ = $<value>1;
        }
    | STAREQ
        {
            $<value>$ = $<value>1;
        }
    ;

values: values VARVAL
        {
            $<values>$.append($<value>2);
        }
    | values QUOTE wsvalue QUOTE
        {
            $<values>$.append($<value>2+$<value>3+$<value>4);
        }
    | values DOLLAR DOLLAR LCURLY VARVAL RCURLY
        {
            $<values>$.append($<value>2+$<value>3+$<value>4+$<value>5+$<value>6);
        }
    | values DOLLAR DOLLAR VARVAL
        {
            $<values>$.append($<value>2+$<value>3+$<value>4);
        }
    | values DOLLAR LBRACE VARVAL RBRACE
        {
            $<values>$.append($<value>2+$<value>3+$<value>4+$<value>5);
        }
    | values WS
        {
            $<values>$.append($<value>2);
        }
    | values COMMA
        {
            $<values>$.append($<value>2);
        }
    | values CONT
        {
            $<values>$.append($<value>2);
        }
    |
        {
            $<values>$.clear();
        }
    ;

wsvalue: wsvalue VARVAL
        {
            $<value>$ += $<value>2;
        }
    | wsvalue WS
        {
            $<value>$ += $<value>2;
        }
    |
        {
            $<value>$ = "";
        }
    ;

%%

#include "qmake_lexer.cpp"

