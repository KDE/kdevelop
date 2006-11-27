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
    QList<AST*> nodelist;
};

typedef Result YYSTYPE;

void yyerror(const char* str)
{
    printf("%s\n", str);
}

int yylex();

QStack<ProjectAST*> projects;

int depth = 0;

%}
%debug
%token WORD
%token WS
%token NEWLINE
%token COMMENT
%token CONT
%token QUOTE
%token LBRACE
%token RBRACE
%token LBRACKET
%token RBRACKET
%token COMMA
%token OP
%token DOLLAR
%token COMMENTLINE

%%

project:
    {
        ProjectAST* projectAST = new ProjectAST();
        projects.push( projectAST );
    }
    statements
    ;

statements: statements statement
        {
            projects.top()->addChild($<node>2);
            $<node>2->setDepth( depth );
        }
    |
    ;

statement:  variable_assignment
        {
            $<node>$ = $<node>1;
        }
    | COMMENT
        {
            $<node>$ = new CommentAST( $<value>1 );
        }
    | NEWLINE
        {
            $<node>$ = new NewlineAST();
        }
    | WS
        {
            $<node>$ = new WhitespaceAST( $<value>1 );
        }
    ;

variable_assignment: WORD WS OP WS values NEWLINE
        {
            AssignmentAST* a = new AssignmentAST( $<value>1 );
            a->addChild( new WhitespaceAST( $<value>2 ) );
            a->addChild( new OpAST( $<value>3 ) );
            a->addChild( new WhitespaceAST( $<value>4 ) );
            a->addValues( $<nodelist>5 );
            a->addChild( new NewlineAST() );
            $<node>$ = a;
        }
    | WORD OP WS values NEWLINE
        {
            AssignmentAST* a = new AssignmentAST( $<value>1 );
            a->addChild( new OpAST( $<value>2 ) );
            a->addChild( new WhitespaceAST( $<value>3 ) );
            a->addValues( $<nodelist>4 );
            a->addChild( new NewlineAST() );
            $<node>$ = a;
        }
    | WORD WS OP values NEWLINE
        {
            AssignmentAST* a = new AssignmentAST( $<value>1 );
            a->addChild( new WhitespaceAST( $<value>2 ) );
            a->addChild( new OpAST( $<value>3 ) );
            a->addValues( $<nodelist>4 );
            a->addChild( new NewlineAST() );
            $<node>$ = a;
        }
    | WORD OP values NEWLINE
        {
            AssignmentAST* a = new AssignmentAST( $<value>1 );
            a->addChild( new OpAST( $<value>2 ) );
            a->addValues( $<nodelist>3 );
            a->addChild( new NewlineAST() );
            $<node>$ = a;
        }
    ;

values: values value
        {
            $<nodelist>$.append( $<node>2 );
        }
    |
        values value COMMENT
        {
            $<nodelist>$.append( $<node>2 );
            $<nodelist>$.append( new CommentAST( $<value>3 ) );
        }
    |
        {
            $<nodelist>$.clear();
        }
    ;

value: WORD
        {
            $<node>$ = new LiteralValueAST( $<value>1 );
        }
    | DOLLAR DOLLAR WORD
        {
            $<node>$ = new QMakeVariableAST( $<value>3, false );
        }
    | DOLLAR DOLLAR RBRACKET WORD LBRACKET
        {
            $<node>$ = new QMakeVariableAST( $<value>4, true );
        }
    | DOLLAR LBRACE WORD RBRACE
        {
            $<node>$ = new EnvironmentVariableAST( $<value>3 );
        }
    | WS
        {
            $<node>$ = new WhitespaceAST( $<value>1 );
        }
    ;

%%

#include "qmake_lexer.cpp"

