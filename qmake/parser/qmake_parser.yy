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
    kDebug() << str << endl;
}

int yylex();

ProjectAST* project;

%}
%debug

%token WS IDENTIFIER SPECIALCHAR DOLLAR COLON COMMA LCURLY RCURLY
%token LBRACE RBRACE QUOTE EQUAL OR PLUSEQ MINUSEQ TILDEEQ STAREQ
%token NEWLINE CONT COMMENT EXCLAM UNDERSCORE DOT EMPTYLINE
%token SEMICOLON LBRACKET RBRACKET

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

function_scope: WS functioncall WS LCURLY WS newline statements WS RCURLY WS NEWLINE
    | WS functioncall WS COLON WS statement
    | WS functioncall WS NEWLINE
    ;

newline: NEWLINE
    |
    ;

variable_assignment: WS IDENTIFIER WS op values WS COMMENT NEWLINE
    | WS IDENTIFIER WS op values COMMENT NEWLINE
    | WS IDENTIFIER WS op values NEWLINE
    ;

values: values WS value
    | values WS braceenclosedval
    | values WS quotedval
    | values WS cont
    | values cont
    | WS braceenclosedval
    | WS quotedval
    | WS value
    | WS CONT
    ;

cont: CONT
    | CONT value
    | CONT braceenclosedval
    | CONT quotedval
    ;

braceenclosedval: LBRACE wsvalues RBRACE SEMICOLON
    | LBRACE wsvalues RBRACE
    ;

quotedval: QUOTE wsvalues QUOTE
    ;

value: value IDENTIFIER
    | value SPECIALCHAR
    | value DOLLAR DOLLAR LCURLY IDENTIFIER RCURLY
    | value DOLLAR DOLLAR LBRACKET IDENTIFIER RBRACKET
    | value DOLLAR DOLLAR LCURLY functioncall RCURLY
    | value DOLLAR DOLLAR IDENTIFIER
    | value DOLLAR LBRACE IDENTIFIER RBRACE
    | IDENTIFIER
    | SPECIALCHAR
    | DOLLAR DOLLAR LCURLY IDENTIFIER RCURLY
    | DOLLAR DOLLAR LBRACKET IDENTIFIER RBRACKET
    | DOLLAR DOLLAR LCURLY functioncall RCURLY
    | DOLLAR DOLLAR IDENTIFIER
    | DOLLAR LBRACE IDENTIFIER RBRACE
    ;

functioncall: IDENTIFIER WS LBRACE functionargs RBRACE
    | EXCLAM IDENTIFIER WS LBRACE functionargs RBRACE
    ;

functionargs: functionargs COMMA functionarg
    | functionarg
    |
    ;

functionarg: WS fnvalue WS
    | WS DOLLAR DOLLAR functioncall WS
    ;

fnvalue: fnvalue value
    | value
    ;


wsvalues: wsvalues WS value
    | WS value
    ;

op: EQUAL
    | PLUSEQ
    | MINUSEQ
    | STAREQ
    | TILDEEQ
    ;

comment: WS COMMENT NEWLINE
    ;

%%

#include "qmake_lexer.cpp"

