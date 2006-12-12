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
    QList<FunctionArgAST*> arglist;
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
%token NEWLINE CONT COMMENT EXCLAM EMPTYLINE
%token SEMICOLON VARIABLE FUNCTIONNAME ELSE
%token FUNCTIONCALL

%%

project:
    {
        project = new ProjectAST();
    }
    statements
    ;

statements: statements statement
        {
project->addStatement( static_cast<StatementAST*>( $<node>2 ) );
        }
    |
    ;

statement: comment
        {
$<node>$ = new CommentAST( $<value>1 );
        }
    | EMPTYLINE
{
$<node>$ = new NewlineAST( $<value>1 );
}
    | variable_assignment
{
$<node>$ = $<node>1;
}
    | scope
{
$<node>$ = $<node>1;
}
    | ws functioncall NEWLINE
{
$<node>$ = new ScopeAST( static_cast<FunctionCallAST*>( $<node>2 ), $<value>1 );
}
    | or_op
{
$<node>$ = $<node>1;
}
    ;

scope: scope_head scope_body
{
ScopeAST* node = static_cast<ScopeAST*>( $<node>1 );
node->setScopeBody( static_cast<ScopeBodyAST*>( $<node>2 ) );
    $<node>$ = node;
}
    | ws ELSE scope_body
{
    ScopeAST* node = new ScopeAST("else", $<value>1 );
    node->setScopeBody( static_cast<ScopeBodyAST*>( $<node>2 ) );
    $<node>$ = node;
}
    ;

or_op: scope_head OR scope_head scope_body
{
    OrAST* node = new OrAST( static_cast<FunctionCallAST*>( $<node>1 ), $<value>2,
                             static_cast<FunctionCallAST*>( $<node>3 ),
                             static_cast<ScopeBodyAST*>( $<node>4 ) );
    $<node>$ = node;
}
    ;

scope_head: ws scope_name
{
$<node>$ = new ScopeAST( $<value>2, $<value>1 );

}
    | ws functioncall
{
AST* node = $<node>2;
node->setWhitespace( $<value>1 );
$<node>$ = node;
}
    ;

scope_body: ws LCURLY COMMENT NEWLINE statements ws RCURLY NEWLINE
{
ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2+$<value>3+$<value>4, $<stmtlist>5, $<value>6+$<value>7+$<value>8 );
$<node>$ = node;
}
    | ws LCURLY NEWLINE statements ws RCURLY NEWLINE
{
ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2+$<value>3, $<stmtlist>4, $<value>5+$<value>6+$<value>7 );
$<node>$ = node;
}
    | ws LCURLY NEWLINE statements ws RCURLY
{
ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2+$<value>3, $<stmtlist>4, $<value>5+$<value>6 );
$<node>$ = node;
}
    | ws LCURLY COMMENT NEWLINE statements ws RCURLY
{
ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2+$<value>3+$<value>4, $<stmtlist>5, $<value>6+$<value>7 );
$<node>$ = node;
}
    | ws COLON statement
{
ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2, static_cast<StatementAST*>( $<node>3 ) );
$<node>$ = node;
}
    ;

scope_name: IDENTIFIER
{
$<value>$ = $<value>1;
}
    | EXCLAM IDENTIFIER
{
$<value>$ = $<value>1+$<value>2;
}
    ;

variable_assignment: ws IDENTIFIER op values COMMENT NEWLINE
{
    $<node>$ = new AssignmentAST( $<value>2, $<value>3, $<values>4, $<value>5, $<value>1 );
}
    | ws IDENTIFIER op COMMENT NEWLINE
{
    $<node>$ = new AssignmentAST( $<value>2, $<value>3, QStringList(), $<value>4, $<value>1 );
}
    | ws IDENTIFIER op values NEWLINE
{
    $<node>$ = new AssignmentAST( $<value>2, $<value>3, $<values>4, "", $<value>1 );
}
    | ws IDENTIFIER op NEWLINE
{
    $<node>$ = new AssignmentAST( $<value>2, $<value>3, QStringList(), "", $<value>1 );
}
    ;

values: values WS value
{
$<values>$.append( $<value>2 );
$<values>$.append( $<value>3 );
}
    | values WS braceenclosedval
{
$<values>$.append( $<value>2 );
$<values>$.append( $<value>3 );
}
    | values WS quotedval
{
$<values>$.append( $<value>2 );
$<values>$.append( $<value>3 );
}
    | values cont
{
for( QStringList::const_iterator it = $<values>2.begin(); it != $<values>2.end(); ++it )
    $<values>$.append( *it );
}
    | braceenclosedval
{
$<values>$ = QStringList();
$<values>$.append( $<value>1 );
}
    | quotedval
{
$<values>$ = QStringList();
$<values>$.append( $<value>1 );
}
    | value
{
$<values>$ = QStringList();
$<values>$.append( $<value>1 );
}
    | CONT
{
$<values>$ = QStringList();
$<values>$.append( $<value>1 );
}
    ;

cont: CONT
{
$<values>$ = QStringList();
$<values>$.append( $<value>1 );
}
    | CONT value
{
$<values>$ = QStringList();
$<values>$.append( $<value>1 );
$<values>$.append( $<value>2 );
}
    | CONT braceenclosedval
{
$<values>$ = QStringList();
$<values>$.append( $<value>1 );
$<values>$.append( $<value>2 );
}
    | CONT quotedval
{
$<values>$ = QStringList();
$<values>$.append( $<value>1 );
$<values>$.append( $<value>2 );
}
    ;

braceenclosedval: LPAREN wsvalues RPAREN SEMICOLON
{
$<value>$ = $<value>1+$<value>2+$<value>3+$<value>4;
}
    | LPAREN wsvalues RPAREN
{
$<value>$ = $<value>1+$<value>2+$<value>3;
}
    ;

quotedval: QUOTE wsvalues QUOTE
{
$<value>$ = $<value>1+$<value>2+$<value>3;
}
    ;

value: value valuepart
{
$<value>$ += $<value>2;
}
    | valuepart
{
$<value>$ = $<value>1;
}
    ;

valuepart: IDENTIFIER
{
$<value>$ = $<value>1;
}
    | COLON
{
$<value>$ = $<value>1;
}
    | OR
{
$<value>$ = $<value>1;
}
    | op
{
$<value>$ = $<value>1;
}
    | SPECIALCHAR
{
$<value>$ = $<value>1;
}
    | VARIABLE
{
$<value>$ = $<value>1;
}
    | SEMICOLON
{
$<value>$ = $<value>1;
}
    | FUNCTIONCALL LPAREN functionargs RPAREN
{
    QString value = $<value>1+$<value>2;
    for( QList<FunctionArgAST*>::const_iterator it = $<arglist>3.begin(); it != $<arglist>3.end(); ++it )
    {
        (*it)->writeToString( value );
    }
    for( QList<FunctionArgAST*>::const_iterator it = $<arglist>3.begin(); it != $<arglist>3.end(); ++it )
    {
        delete (*it);
    }
    $<arglist>3.clear();
    $<value>$ = value+$<value>4;
}
    | DOLLAR DOLLAR LCURLY functioncall RCURLY
{
    QString value = $<value>1+$<value>2+$<value>3;
    static_cast<FunctionCallAST*>( $<node>4 )->writeToString( value );
    delete $<node>4;
    $<value>$ = value+$<value>5;
}
    ;

functioncall: FUNCTIONNAME LPAREN functionargs RPAREN
{
$<node>$ = new FunctionCallAST( $<value>1, $<value>2, $<arglist>3, $<value>4 );
}
    | FUNCTIONNAME LPAREN ws RPAREN
{
$<node>$ = new FunctionCallAST( $<value>1, $<value>2, QList<FunctionArgAST*>(), $<value>4 );
}
    | EXCLAM FUNCTIONNAME LPAREN functionargs RPAREN
{
$<node>$ = new FunctionCallAST( $<value>1+$<value>2, $<value>3, $<arglist>4, $<value>5 );
}
    | EXCLAM FUNCTIONNAME LPAREN ws RPAREN
{
$<node>$ = new FunctionCallAST( $<value>1+$<value>2, $<value>3, QList<FunctionArgAST*>(), $<value>5 );
}
    ;

functionargs: functionargs COMMA functionarg
{
$<arglist>$.append( static_cast<FunctionArgAST*>( $<node>3 ) );
}
    | functionarg
{
$<arglist>$.clear();
$<arglist>$.append( static_cast<FunctionArgAST*>( $<node>1 ) );
}
    ;

functionarg: wsvalues ws
{
$<node>$ = new SimpleFunctionArgAST( $<value>1+$<value>2 );
}
    | ws FUNCTIONCALL ws LPAREN functionargs RPAREN ws
{
$<node>$ = new FunctionCallAST( $<value>2, $<value>3+$<value>4, $<arglist>5, $<value>6+$<value>7, $<value>1 );
}
    ;

wsvalues: wsvalues WS value
{
$<value>$ += $<value>1+$<value>2;
}
    | wsvalues WS braceenclosedval
{
$<value>$ += $<value>1+$<value>2;
}
    | wsvalues WS quotedval
{
$<value>$ += $<value>1+$<value>2;
}
    | wsvalues op value
{
$<value>$ += $<value>1+$<value>2;
}
    | wsvalues op braceenclosedval
{
$<value>$ += $<value>1+$<value>2;
}
    | wsvalues op quotedval
{
$<value>$ += $<value>1+$<value>2;
}
    | braceenclosedval
{
$<value>$ = $<value>1;
}
    | quotedval
{
$<value>$ = $<value>1;
}
    | ws value
{
$<value>$ = $<value>1;
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
    | STAREQ
{
$<value>$ = $<value>1;
}
    | TILDEEQ
{
$<value>$ = $<value>1;
}
    ;

ws: WS
{
$<value>$ = $<value>1;
}
    |
{
$<value>$ = "";
}
    ;

comment: COMMENT NEWLINE
{
$<value>$ = $<value>1+$<value>2;
}
    ;

%%

#include "qmake_lexer.cpp"

