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
#include <kdebug.h>

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
%debug

%token WS IDENTIFIER SPECIALCHAR DOLLAR COLON COMMA LCURLY RCURLY
%token LPAREN RPAREN QUOTE EQUAL OR PLUSEQ MINUSEQ TILDEEQ STAREQ
%token NEWLINE CONT COMMENT EXCLAM UNDERSCORE DOT EMPTYLINE
%token SEMICOLON LBRACKET RBRACKET VARIABLE FUNCTIONNAME

%right PLUSEQ
%right MINUSEQ
%right EQUAL
%right STAREQ
%right TILDEEQ

%%

project:
    {
        project = new ProjectAST();
    }
    statements
    ;

statements: statements statement
    |
    ;

statement: comment
    | EMPTYLINE
    | variable_assignment
    | function_scope
    ;

function_scope: ws functioncall ws LCURLY newline statements ws RCURLY NEWLINE
    | ws functioncall COLON statement
    | ws functioncall NEWLINE
    ;

newline: NEWLINE
    |
    ;

variable_assignment: ws IDENTIFIER ws op values COMMENT NEWLINE
    | ws IDENTIFIER ws op values NEWLINE
    ;

values: values WS value
    | values WS braceenclosedval
    | values WS quotedval
    | values cont
    | ws braceenclosedval
    | ws quotedval
    | ws value
    | CONT
    ;

cont: CONT
    | CONT value
    | CONT braceenclosedval
    | CONT quotedval
    ;

braceenclosedval: LPAREN wsvalues RPAREN SEMICOLON
    | LPAREN wsvalues RPAREN
    ;

quotedval: QUOTE wsvalues QUOTE
    ;

value: value IDENTIFIER
    | value SPECIALCHAR
    | value VARIABLE
    | value DOLLAR DOLLAR LCURLY functioncall RCURLY
    | IDENTIFIER
    | SPECIALCHAR
    | VARIABLE
    | DOLLAR DOLLAR LCURLY functioncall RCURLY
    ;

functioncall: IDENTIFIER LPAREN functionargs RPAREN
    | EXCLAM IDENTIFIER ws LPAREN functionargs RPAREN
    ;

functionargs: functionargs COMMA functionarg
    | functionarg
    |
    ;

functionarg: ws fnvalue ws
    | ws FUNCTIONNAME ws LPAREN functionargs RPAREN ws
    ;

fnvalue: fnvalue value
    | value
    ;


wsvalues: wsvalues WS value
    | ws value
    ;

op: EQUAL
    | PLUSEQ
    | MINUSEQ
    | STAREQ
    | TILDEEQ
    ;

ws: WS
    |
    ;

comment: COMMENT NEWLINE
    ;

%%

#include "qmake_lexer.cpp"

