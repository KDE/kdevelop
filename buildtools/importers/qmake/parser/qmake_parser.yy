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
        Result() : stmt(0), scopebody(0), scope(0), fnarg(0), funccall(0) {}
        QString value;
        QMake::StatementAST* stmt;
        QMake::ScopeBodyAST* scopebody;
        QMake::ScopeAST* scope;
        QMake::FunctionArgAST* fnarg;
        QMake::FunctionCallAST* funccall;
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
%start project

%token WS VARIABLE DOLLAR COLON COMMA LCURLY RCURLY
%token LPAREN RPAREN EQUAL OR PLUSEQ MINUSEQ TILDEEQ STAREQ
%token NEWLINE CONT COMMENT EXCLAM EMPTYLINE VAR_VALUE
%token QMVARIABLE SHELLVARIABLE FUNCTIONNAME
%token FUNCTIONCALL SCOPENAME QUOTED_VAR_VALUE FNVALUE

%%

project:
        {
            *project = new ProjectAST();
        }
    statements
        {
            foreach( StatementAST* s, $<stmtlist>2)
            {
                (*project)->addStatement( s );
            }
        }
    ;

statements: statements statement
        {
            $<stmtlist>$.append( $<stmt>2 );
        }
    |
        {
            $<stmtlist>$.clear();
        }
    ;

statement: comment
        {
            $<stmt>$ = new CommentAST( $<value>1 );
        }
    | EMPTYLINE
        {
            $<stmt>$ = new NewlineAST( $<value>1 );
        }
    | variable_assignment
        {
            $<stmt>$ = $<stmt>1;
        }
    | scope
        {
            $<stmt>$ = $<stmt>1;
        }
    | ws functioncall NEWLINE
        {
            $<stmt>$ = new ScopeAST( $<funccall>2, $<value>1 );
        }
    | or_op
        {
            $<stmt>$ = $<stmt>1;
        }
    ;

scope: scope_head scope_body
        {
            ScopeAST* node = $<scope>1;
            node->setScopeBody( $<scopebody>2 );
            $<stmt>$ = node;
        }
    ;

or_op: ws functioncall OR ws functioncall scope_body
        {
            $<funccall>2->setWhitespace($<value>1);
            $<funccall>5->setWhitespace($<value>4);
            OrAST* node = new OrAST( $<funccall>2, $<value>3, $<funccall>5, $<scopebody>6 );
            $<stmt>$ = node;
        }
    ;

scope_head: ws scope_name
        {
            ScopeAST* node = new ScopeAST( $<value>2, $<value>1 );
            $<scope>$ = node;
        }
    | ws functioncall
        {
            ScopeAST* node = new ScopeAST( $<funccall>2, $<value>1 );
            $<scope>$ = node;
        }
    ;

scope_body: ws LCURLY COMMENT NEWLINE statements ws RCURLY NEWLINE
        {
            ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2+$<value>3+$<value>4, $<stmtlist>5, $<value>6+$<value>7+$<value>8 );
            $<scopebody>$ = node;
        }
    | ws LCURLY NEWLINE statements ws RCURLY NEWLINE
        {
            ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2+$<value>3, $<stmtlist>4, $<value>5+$<value>6+$<value>7 );
            $<scopebody>$ = node;
        }
    | ws LCURLY NEWLINE statements ws RCURLY
        {
            ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2+$<value>3, $<stmtlist>4, $<value>5+$<value>6 );
            $<scopebody>$ = node;
        }
    | ws LCURLY COMMENT NEWLINE statements ws RCURLY
        {
            ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2+$<value>3+$<value>4, $<stmtlist>5, $<value>6+$<value>7 );
            $<scopebody>$ = node;
        }
    | ws COLON statement
        {
            ScopeBodyAST* node = new ScopeBodyAST( $<value>1+$<value>2, $<stmt>3 );
            $<scopebody>$ = node;
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
            FunctionCallAST* node = new FunctionCallAST( $<value>1, $<value>2, $<arglist>3, $<value>4 );
            $<funccall>$ = node;
        }
    | FUNCTIONNAME LPAREN RPAREN
        {
            $<funccall>$ = new FunctionCallAST( $<value>1, $<value>2, QList<FunctionArgAST*>(), $<value>3 );
        }
    | EXCLAM FUNCTIONNAME LPAREN functionargs RPAREN
        {
            $<funccall>$ = new FunctionCallAST( $<value>1+$<value>2, $<value>3, $<arglist>4, $<value>5 );
        }
    | EXCLAM FUNCTIONNAME LPAREN RPAREN
        {
            $<funccall>$ = new FunctionCallAST( $<value>1+$<value>2, $<value>3, QList<FunctionArgAST*>(), $<value>4 );
        }
    ;

functionargs: functionargs COMMA functionarg
        {
            $<arglist>$.append( $<fnarg>3 );
        }
    | functionarg
        {
            $<arglist>$.clear();
            $<arglist>$.append( $<fnarg>1 );
        }
    ;

functionarg: ws fnvalue
        {
            $<fnarg>$ = new SimpleFunctionArgAST( $<value>2, $<value>1 );
        }
    | ws FUNCTIONCALL ws LPAREN functionargs RPAREN ws
        {
            $<fnarg>$ = new FunctionCallAST( $<value>2, $<value>3+$<value>4, $<arglist>5, $<value>6+$<value>7, $<value>1 );
        }
    ;

fnvalue: fnvalue FNVALUE
        {
            $<value>$ += $<value>2;
        }
    | fnvalue QMVARIABLE
        {
            $<value>$ += $<value>2;
        }
    | fnvalue SHELLVARIABLE
        {
            $<value>$ += $<value>2;
        }
    | fnvalue WS
        {
            $<value>$ += $<value>2;
        }
    | fnvalue TILDEEQ
        {
            $<value>$ += $<value>2;
        }
    | fnvalue PLUSEQ
        {
            $<value>$ += $<value>2;
        }
    | fnvalue MINUSEQ
        {
            $<value>$ += $<value>2;
        }
    | fnvalue STAREQ
        {
            $<value>$ += $<value>2;
        }
    | fnvalue EQUAL
        {
            $<value>$ += $<value>2;
        }
    | fnvalue COLON
        {
            $<value>$ += $<value>2;
        }
    | fnvalue OR
        {
            $<value>$ += $<value>2;
        }
    | fnvalue EXCLAM
        {
            $<value>$ += $<value>2;
        }
    | fnvalue DOLLAR
        {
            $<value>$ += $<value>2;
        }
    | fnvalue LPAREN fnvalue RPAREN
        {
            $<value>$ += $<value>2+$<value>3+$<value>4;
        }
    | DOLLAR
        {
            $<value>$ = $<value>1;
        }
    | LPAREN fnvalue RPAREN
        {
            $<value>$ = $<value>1+$<value>2+$<value>3;
        }
    | FNVALUE
        {
            $<value>$ = $<value>1;
        }
    | QMVARIABLE
        {
            $<value>$ = $<value>1;
        }
    | SHELLVARIABLE
        {
            $<value>$ = $<value>1;
        }
    | TILDEEQ
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
    | EQUAL
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
    | EXCLAM
        {
            $<value>$ = $<value>1;
        }
    ;

variable_assignment: ws VARIABLE op values COMMENT NEWLINE
        {
            $<stmt>$ = new AssignmentAST( $<value>2, $<value>3, $<values>4, $<value>5, $<value>1 );
        }
    | ws VARIABLE op COMMENT NEWLINE
        {
            $<stmt>$ = new AssignmentAST( $<value>2, $<value>3, QStringList(), $<value>4, $<value>1 );
        }
    | ws VARIABLE op values NEWLINE
        {
            $<stmt>$ = new AssignmentAST( $<value>2, $<value>3, $<values>4, "", $<value>1 );
        }
    | ws VARIABLE op NEWLINE
        {
            $<stmt>$ = new AssignmentAST( $<value>2, $<value>3, QStringList(), "", $<value>1 );
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