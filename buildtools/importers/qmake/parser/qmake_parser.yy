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

#include <QtCore/QStack>
#include <QtCore/QString>
#include <kdebug.h>
#include "qmakeast.h"

/**
 The yylval type
*/

namespace QMake
{
    class Lexer;

    struct Result {
        Result() : node(0) {}
        QString value;
        QMake::AST* node;
        QStringList values;
        QList<QMake::StatementAST*> stmtlist;
        QList<QMake::FunctionArgAST*> arglist;
    };

    #define YYSTYPE Result
    typedef Result YYSTYPE;
}

extern int QMakelex( QMake::Result* yylval, QMake::Lexer* lexer);

%}
%debug

%skeleton "lalr1.cc"
%define "parser_class_name" "Parser"
%name-prefix="QMake"
%parse-param { QMake::Lexer* lexer }
%parse-param { QMake::ProjectAST** project }
%lex-param   { QMake::Lexer* lexer }

%token WS VARIABLE DOLLAR COLON COMMA LCURLY RCURLY
%token LPAREN RPAREN EQUAL OR PLUSEQ MINUSEQ TILDEEQ STAREQ
%token NEWLINE CONT COMMENT EXCLAM EMPTYLINE VAR_VALUE
%token QMVARIABLE SHELLVARIABLE FUNCTIONNAME ELSE
%token FUNCTIONCALL SCOPENAME QUOTED_VAR_VALUE FNVALUE

%%

project:
    {
        *project = new ProjectAST();
    }
    statements
    {
        foreach( StatementAST* s, $<stmtlist>1)
        {
            (*project)->addStatement( s );
        }
    }
    ;

statements: statements statement
        {
            $<stmtlist>$.append( static_cast<StatementAST*>( $<node>2 ) );
        }
    |
        {
            $<stmtlist>$.clear();
        }
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

scope_name: SCOPENAME
{
    $<value>$ = $<value>1;
}
    | EXCLAM SCOPENAME
{
    $<value>$ = $<value>1+$<value>2;
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

functionarg: ws fnvalue
{
    $<node>$ = new SimpleFunctionArgAST( $<value>1 );
}
    | ws FUNCTIONCALL ws LPAREN functionargs RPAREN ws
{
    $<node>$ = new FunctionCallAST( $<value>2, $<value>3+$<value>4, $<arglist>5, $<value>6+$<value>7, $<value>1 );
}
    ;

fnvalue: fnvalue FNVALUE
    | fnvalue QMVARIABLE
    | fnvalue SHELLVARIABLE
    | fnvalue WS
    | fnvalue TILDEEQ
    | fnvalue PLUSEQ
    | fnvalue MINUSEQ
    | fnvalue STAREQ
    | fnvalue EQUAL
    | fnvalue COLON
    | fnvalue OR
    | fnvalue EXCLAM
    | fnvalue DOLLAR
    | fnvalue LPAREN fnvalue RPAREN
    | DOLLAR
    | LPAREN fnvalue RPAREN
    | FNVALUE
    | QMVARIABLE
    | SHELLVARIABLE
    | TILDEEQ
    | PLUSEQ
    | MINUSEQ
    | STAREQ
    | EQUAL
    | COLON
    | OR
    | EXCLAM
    ;

variable_assignment: ws VARIABLE op values COMMENT NEWLINE
{
    $<node>$ = new AssignmentAST( $<value>2, $<value>3, $<values>4, $<value>5, $<value>1 );
}
    | ws VARIABLE op COMMENT NEWLINE
{
    $<node>$ = new AssignmentAST( $<value>2, $<value>3, QStringList(), $<value>4, $<value>1 );
}
    | ws VARIABLE op values NEWLINE
{
    $<node>$ = new AssignmentAST( $<value>2, $<value>3, $<values>4, "", $<value>1 );
}
    | ws VARIABLE op NEWLINE
{
    $<node>$ = new AssignmentAST( $<value>2, $<value>3, QStringList(), "", $<value>1 );
}
    ;

values: values WS VAR_VALUE
{
    $<values>$.append( $<value>2 );
    $<values>$.append( $<value>3 );
}
    | values WS QUOTED_VAR_VALUE
{
    $<values>$.append( $<value>2 );
    $<values>$.append( $<value>3 );
}
    | values CONT ws VAR_VALUE
{
    $<values>$.append( $<value>2 );
    $<values>$.append( $<value>3 );
    $<values>$.append( $<value>4 );
}
    | values CONT ws QUOTED_VAR_VALUE
{

    $<values>$.append( $<value>2 );
    $<values>$.append( $<value>3 );
    $<values>$.append( $<value>4 );
}
    | QUOTED_VAR_VALUE
{
    $<values>$ = QStringList();
    $<values>$.append( $<value>1 );
}
    | VAR_VALUE
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

namespace QMake
{
void Parser::error(const location_type& /*l*/, const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}