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

%token WORD
%token WS
%token NEWLINE
%token COMMENT

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

statement: comment
        {
            $<node>$ = $<node>1;
        }
    | newline
        {
            $<node>$ = $<node>1;
        }
    ;

comment: COMMENT
        {
            $<node>$ = new CommentAST( $<value>1 );
        }
    ;

newline: NEWLINE
        {
            $<node>$ = new NewlineAST();
        }
    ;

%%

#include "qmake_lexer.cpp"

