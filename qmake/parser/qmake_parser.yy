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
    StatementAST* stmtnode;
    QStringList valuelist;
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

%token WS
%token LETTER
%token DIGIT
%token COMMA
%token UNDERSCORE
%token DOT
%token EQUAL
%token PLUSEQ
%token MINUSEQ
%token TILDEEQ
%token STAREQ
%token LCURLY
%token RCURLY
%token LBRACE
%token RBRACE
%token DOLLAR
%token QUOTE
%token SLASH
%token CONT
%token COMMENT
%token SIMPLEVAL
%token NEWLINE
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
            projects.top()->addStatement($<stmtnode>2);
            $<stmtnode>2->setDepth( depth );
        }
    |
    ;

statement:  variable_assignment
        {
            $<stmtnode>$ = $<stmtnode>1;
        }
    | NEWLINE
        {
            $<stmtnode>$ = new NewlineAST();
        }
    | COMMENT
        {
            $<stmtnode>$ = new CommentAST( $<value>1 );
        }
    | ws
        {
            $<stmtnode>$ = new WhitespaceAST( $<value>1 );
        }
    ;

variable_assignment: variablename op_ws values
        {
            $<stmtnode>$ = new AssignmentAST( $<value>1, dynamic_cast<OpAST*>($<node>2), $<valuelist>3 );
            $<valuelist>3.clear();
        }
    ;

variablename: variablename variablechar
        {
            $<value>$ += $<value>2;
        }
    | variablebegin
        {
            $<value>$ = $<value>1;
        }
    ;

variablebegin: LETTER
        {
            $<value>$ = $<value>1;
        }
    | DIGIT
        {
            $<value>$ = $<value>1;
        }
    | UNDERSCORE
        {
            $<value>$ = $<value>1;
        }
    ;

variablechar:  LETTER
        {
            $<value>$ = $<value>1;
        }
    | DIGIT
        {
            $<value>$ = $<value>1;
        }
    | UNDERSCORE
        {
            $<value>$ = $<value>1;
        }
    | DOT
        {
            $<value>$ = $<value>1;
        }
    ;

values: values value
        {
            $<valuelist>$.append( $<value>2 );
        }
    | values CONT ws NEWLINE
        {
            $<valuelist>$.append( $<value>2 );
            $<valuelist>$.append( $<value>3 );
            $<valuelist>$.append( $<value>4 );
        }
    | value
        {
            $<valuelist>$.append( $<value>1 );
        }
    ;

value: DOLLAR DOLLAR LCURLY simpleval RCURLY
        {
            $<value>$ = $<value>1+$<value>2+$<value>3+$<value>4+$<value>5;
        }
    | DOLLAR DOLLAR simpleval
        {
            $<value>$ = $<value>1+$<value>2+$<value>3;
        }
    | DOLLAR LBRACE simpleval RBRACE
        {
            $<value>$ = $<value>1+$<value>2+$<value>3+$<value>4;
        }
    | simpleval
        {
            $<value>$ = $<value>1;
        }
    | QUOTE quotedval QUOTE
        {
            $<value>$ = $<value>1+$<value>2+$<value>3;
        }
    | ws
        {
            $<value>$ = $<value>1;
        }
    ;

simpleval: simpleval LETTER
        {
            $<value>$ += $<value>2;
        }
    | simpleval DOT
        {
            $<value>$ += $<value>2;
        }
    | simpleval COMMA
        {
            $<value>$ += $<value>2;
        }
    | simpleval UNDERSCORE
        {
            $<value>$ += $<value>2;
        }
    | simpleval DIGIT
        {
            $<value>$ += $<value>2;
        }
    | simpleval SIMPLEVAL
        {
            $<value>$ += $<value>2;
        }
    | UNDERSCORE
        {
            $<value>$ = $<value>1;
        }
    | COMMA
        {
            $<value>$ = $<value>1;
        }
    | DOT
        {
            $<value>$ = $<value>1;
        }
    | LETTER
        {
            $<value>$ = $<value>1;
        }
    | DIGIT
        {
            $<value>$ = $<value>1;
        }
    | SIMPLEVAL
        {
            $<value>$ = $<value>1;
        }
    ;

quotedval: quotedval simpleval
        {
            $<value>$ += $<value>2;
        }
    | quotedval ws
        {
            $<value>$ += $<value>2;
        }
    |
        {
            $<value>$ = "";
        }
    ;

op_ws: ws op ws
        {
            $<node>$ = new OpAST( $<value>1, $<value>2, $<value>3 );
        }
    | ws op
        {
            $<node>$ = new OpAST( $<value>1, $<value>2, "" );
        }
    | op ws
        {
            $<node>$ = new OpAST( "", $<value>1, $<value>2 );
        }
    | op
        {
            $<node>$ = new OpAST( "", $<value>1, "" );
        }
    ;

ws: ws WS
        {
            $<value>$ += $<value>1;
        }
    | WS
        {
            $<value>$ = $<value>1;
        }
    ;


op: EQUAL
        {
            $<value>$ = $<value>1;
        }
    | MINUSEQ
        {
            $<value>$ = $<value>1;
        }
    | PLUSEQ
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

%%

#include "qmake_lexer.cpp"

