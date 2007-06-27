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
        Result() : stmt(0), scopebody(0), scope(0), funccall(0) {}
        QString value;
        QMake::StatementAST* stmt;
        QMake::ScopeBodyAST* scopebody;
        QMake::ScopeAST* scope;
        QMake::FunctionCallAST* funccall;
        QStringList values;
        QList<QMake::StatementAST*> stmtlist;
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
%parse-param { QMake::ProjectAST* project }
%lex-param   { QMake::Lexer* lexer }
%start project

%token WS VARIABLE DOLLAR COLON COMMA LCURLY RCURLY
%token LPAREN RPAREN EQUAL OR PLUSEQ MINUSEQ TILDEEQ STAREQ
%token NEWLINE CONT COMMENT EXCLAM EMPTYLINE VAR_VALUE
%token QMVARIABLE SHELLVARIABLE FUNCTIONNAME CONT_COMMENT
%token FUNCTIONCALL SCOPENAME QUOTED_VAR_VALUE FNVALUE

%%

project:
    statements
        {
            Q_FOREACH( StatementAST* s, $<stmtlist>1)
            {
                project->addStatement( s );
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
            CommentAST* node = new CommentAST();
            node->setComment( $<value>1 );
            $<stmt>$ = node;
        }
    | EMPTYLINE
        {
            NewlineAST* node = new NewlineAST();
            node->setWhitespace( $<value>1 );
            $<stmt>$ = node;
        }
    | variable_assignment
        {
            $<stmt>$ = $<stmt>1;
        }
    | scope
        {
            $<stmt>$ = $<stmt>1;
        }
    ;

scope: scope_head ws scope_body
        {
            ScopeAST* node = $<scope>1;
            $<scopebody>3->setWhitespace( $<value>2 );
            node->setScopeBody( $<scopebody>3 );
            $<stmt>$ = node;
        }
    | scope_head NEWLINE
        {
            ScopeAST* node = $<scope>1;
            node->setLineEnding( $<value>2 );
            $<stmt>$ = node;
        }
    ;

scope_head: ws scope_name
        {
            SimpleScopeAST* node = new SimpleScopeAST();
            node->setScopeName( $<value>2 );
            node->setWhitespace( $<value>1 );
            $<scope>$ = node;
        }
    | ws functioncall
        {
            ScopeAST* node = $<funccall>2;
            node->setWhitespace( $<value>1 );
            $<scope>$ = node;
        }
    | ws functioncall OR functioncall
    {
        OrAST* node = new OrAST();
        node->setLeftCall( $<funccall>2 );
        node->setRightCall( $<funccall>4 );
        node->setOrOp( $<value>3 );
        node->setWhitespace( $<value>1 );
        $<scope>$ = node;
    }
    ;

scope_body: LCURLY COMMENT NEWLINE statements ws RCURLY NEWLINE
        {
            ScopeBodyAST* node = new ScopeBodyAST();
			node->setBegin( $<value>1+$<value>2+$<value>3 );
			node->setStatements( $<stmtlist>4 );
			node->setEnd( $<value>5+$<value>6+$<value>7 );
            $<scopebody>$ = node;
        }
    | LCURLY NEWLINE statements ws RCURLY NEWLINE
        {
            ScopeBodyAST* node = new ScopeBodyAST();
			node->setBegin( $<value>1+$<value>2 );
			node->setStatements( $<stmtlist>3 );
			node->setEnd( $<value>4+$<value>5+$<value>6 );
            $<scopebody>$ = node;
        }
    | LCURLY NEWLINE statements ws RCURLY
        {
            ScopeBodyAST* node = new ScopeBodyAST();
			node->setBegin( $<value>1+$<value>2 );
			node->setStatements( $<stmtlist>3 );
			node->setEnd( $<value>4+$<value>5 );
            $<scopebody>$ = node;
        }
    | LCURLY COMMENT NEWLINE statements ws RCURLY
        {
            ScopeBodyAST* node = new ScopeBodyAST();
			node->setBegin( $<value>1+$<value>2+$<value>3 );
			node->setStatements( $<stmtlist>4 );
			node->setEnd( $<value>5+$<value>6 );
            $<scopebody>$ = node;
        }
    | COLON statement
        {
            ScopeBodyAST* node = new ScopeBodyAST();
            node->setBegin( $<value>1 );
            node->addStatement( $<stmt>2 );
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

functioncall: FUNCTIONNAME ws LPAREN functionargs RPAREN
        {
            FunctionCallAST* node = new FunctionCallAST();
            node->setFunctionName( $<value>1 );
            node->setBegin( $<value>2+$<value>3 );
            node->setArguments( $<values>4 );
            node->setEnd( $<value>5 );
            $<funccall>$ = node;
        }
    | FUNCTIONNAME ws LPAREN RPAREN
        {
            FunctionCallAST* node = new FunctionCallAST();
            node->setFunctionName( $<value>1 );
            node->setBegin( $<value>2+$<value>3 );
            node->setEnd( $<value>4 );
            $<funccall>$ = node;
        }
    | EXCLAM FUNCTIONNAME ws LPAREN functionargs RPAREN
        {
            FunctionCallAST* node = new FunctionCallAST();
            node->setFunctionName( $<value>1+$<value>2 );
            node->setBegin( $<value>3+$<value>4 );
            node->setArguments( $<values>5 );
            node->setEnd( $<value>6 );
            $<funccall>$ = node;
        }
    | EXCLAM FUNCTIONNAME ws LPAREN RPAREN
        {
            FunctionCallAST* node = new FunctionCallAST();
            node->setFunctionName( $<value>1+$<value>2 );
            node->setBegin( $<value>3+$<value>4 );
            node->setEnd( $<value>5 );
            $<funccall>$ = node;
        }
    ;

functionargs: functionargs COMMA functionarg
        {
            $<values>$.append( $<value>3 );
        }
    | functionarg
        {
            $<values>$.clear();
            $<values>$.append( $<value>1 );
        }
    ;

functionarg: functionarg FNVALUE
        {
            $<value>$ += $<value>2;
        }
    | functionarg QMVARIABLE
        {
            $<value>$ += $<value>2;
        }
    | functionarg SHELLVARIABLE
        {
            $<value>$ += $<value>2;
        }
    | functionarg WS
        {
            $<value>$ += $<value>2;
        }
    | functionarg TILDEEQ
        {
            $<value>$ += $<value>2;
        }
    | functionarg PLUSEQ
        {
            $<value>$ += $<value>2;
        }
    | functionarg MINUSEQ
        {
            $<value>$ += $<value>2;
        }
    | functionarg STAREQ
        {
            $<value>$ += $<value>2;
        }
    | functionarg EQUAL
        {
            $<value>$ += $<value>2;
        }
    | functionarg COLON
        {
            $<value>$ += $<value>2;
        }
    | functionarg OR
        {
            $<value>$ += $<value>2;
        }
    | functionarg EXCLAM
        {
            $<value>$ += $<value>2;
        }
    | functionarg DOLLAR
        {
            $<value>$ += $<value>2;
        }
    | functionarg LPAREN functionarg RPAREN
        {
            $<value>$ += $<value>2+$<value>3+$<value>4;
        }
    | functionarg FUNCTIONCALL ws LPAREN functionargs RPAREN
        {
            $<value>$ += $<value>2+$<value>3+$<value>4+$<values>5.join(",")+$<value>6;
        }
    | DOLLAR
        {
            $<value>$ = $<value>1;
        }
    | LPAREN functionarg RPAREN
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
    | WS
        {
            $<value>$ = $<value>1;
        }
    | EXCLAM
        {
            $<value>$ = $<value>1;
        }
    | FUNCTIONCALL ws LPAREN functionargs RPAREN
        {
            $<value>$ = $<value>1+$<value>2+$<value>3+$<values>4.join(",")+$<value>5;
        }
    ;

variable_assignment: ws VARIABLE op values NEWLINE
        {
        	AssignmentAST* node = new AssignmentAST();
        	node->setWhitespace( $<value>1 );
        	node->setVariable( $<value>2 );
        	node->setOp( $<value>3 );
        	node->setValues( $<values>4 );
        	node->setLineEnding( $<value>5 );
            $<stmt>$ = node;
        }
    | ws VARIABLE op NEWLINE
        {
            AssignmentAST* node = new AssignmentAST();
        	node->setWhitespace( $<value>1 );
        	node->setVariable( $<value>2 );
        	node->setOp( $<value>3 );
        	node->setLineEnding( $<value>4 );
            $<stmt>$ = node;
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
    | values CONT_COMMENT ws VAR_VALUE
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
    | values CONT_COMMENT ws QUOTED_VAR_VALUE
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
    | CONT_COMMENT
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