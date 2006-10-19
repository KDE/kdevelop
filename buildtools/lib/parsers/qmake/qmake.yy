%{
/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/**
@file qmake.yy
QMake Parser

Simple LALR parser which builds the syntax tree (see @ref QMake::AST).

@todo Recognize comments after statements like:
SOURCES = foo #regognize me

@fixme Parser fails on files that do not end with a newline
@fixme 1 shift/reduce conflict in "line_body" rule
*/

#include <qvaluestack.h>
#include "qmakeast.h"
#include <qregexp.h>

#define YYSTYPE_IS_DECLARED

using namespace QMake;

/**
The yylval type.
*/
struct Result {
    Result(): node(0) {}

    /**Type of semantic value for simple grammar rules.*/
    QString value;
    /**Type of semantic value for grammar rules which are parts of AST.*/
    AST *node;
    /**Type of semantic value for "multiline_values" grammar rule.
    Each line of multiline value is stored as a string in the list.

    For example we have in .pro file:
    @code
    SOURCE = foo1.cpp \
        foo2.cpp \
        foo3.cpp foo4.cpp
    @endcode
    The string list will be populated with three strings:
    <pre>
    foo1.cpp
    foo2.cpp
    foo3.cpp foo4.cpp
    </pre>
    */
    QStringList values;
};

typedef Result YYSTYPE;

void yyerror(const char *str) {
    printf("%s\n", str);
}

int yylex();

/**
The stack to store ProjectAST pointers when a new child
ProjectAST is created and filled with statements.

Parser creates root ProjectAST for a .pro file, pushes it onto the stack and starts
adding statements. Each statement is added as a child StatementAST to the ProjectAST
currently on the top in the stack.

When a scope or function scope statement is parsed, the child ProjectAST is created
and pushed onto the stack. Therefore all statements which belong to the scope
or function scope are added as childs to their direct parent (scope or function scope).
*/
QValueStack<ProjectAST*> projects;

/**
The current depth of AST node is stored here.
AST depth is important to know because automatic indentation can
be easily implemented (the parser itself looses all information
about indentation).
*/
int depth = 0;

/*
To debug this parser, put the line below into the next bison file section.
Don't forget to uncomment "yydebug = 1" line in qmakedriver.cpp.
%debug
*/
%}

%token ID_SIMPLE
%token EQ
%token PLUSEQ
%token MINUSQE
%token STAREQ
%token TILDEEQ
%token LBRACE
%token RBRACE
%token COLON
%token NUMSIGN
%token NEWLINE
%token NUMBER
%token COMMENT
%token CONT
%token RCURLY
%token LCURLY
%token ID_ARGS
%token LIST_COMMENT
%token QUOTED_VARIABLE_VALUE
%token VARIABLE_VALUE
%%

project :
    {
        ProjectAST *projectAST = new ProjectAST();
        projects.push(projectAST);
    }
    statements
    ;

statements : statements statement
        {
            projects.top()->addChildAST($<node>2);
            $<node>2->setDepth(depth);
        }
    |
    ;

statement : variable_assignment
        {
            $<node>$ = $<node>1;
        }
    | scope
        {
            $<node>$ = $<node>1;
        }
    | function_call
        {
            $<node>$ = $<node>1;
        }
    | comment
        {
            $<node>$ = $<node>1;
        }
    | emptyline
        {
            $<node>$ = new NewLineAST();
        }
    ;

variable_assignment : ID_SIMPLE operator multiline_values
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values = $<values>3;
            $<node>$ = node;
        }
    ;

multiline_values : multiline_values line_body
        {
            $<values>$ += $<values>2
        }
    |   { $<values>$.clear(); }
    ;

line_body : line_body variable_value { $<values>$.append( $<value>2 ); }
    | variable_value { $<values>$.append( $<value>1 ); }
    | CONT                   { $<values>$.append("\\\n"); }
    | NEWLINE                { $<values>$.append("\n"); }
    | LIST_COMMENT
    | RBRACE
    ;

variable_value : VARIABLE_VALUE     { $<value>$ = $<value>1; }
    | QUOTED_VARIABLE_VALUE  { $<value>$ = $<value>1; }
    ;

operator : EQ | PLUSEQ | MINUSQE | STAREQ | TILDEEQ
    ;

scope : ID_SIMPLE
        {
            ProjectAST *projectAST = new ProjectAST(ProjectAST::Scope);
            projects.push(projectAST);
            projects.top()->scopedID = $<value>1;
            depth++;
        }
    scope_body
        {
            $<node>$ = projects.pop();
            depth--;
        }
    ;

function_call : ID_SIMPLE LBRACE function_args RBRACE
        {
            ProjectAST *projectAST = new ProjectAST(ProjectAST::FunctionScope);
            projects.push(projectAST);
            projects.top()->scopedID = $<value>1;
            projects.top()->args = $<value>3;
            depth++;

            //qWarning("%s", $<value>1.ascii());
            if ($<value>1.contains("include"))
            {
                IncludeAST *includeAST = new IncludeAST();
                includeAST->projectName = $<value>3;
                projects.top()->addChildAST(includeAST);
                includeAST->setDepth(depth);
            }
        }
    scope_body
    else_statement
        {
            $<node>$ = projects.pop();
            depth--;
        }
    ;

function_args : ID_ARGS    { $<value>$ = $<value>1; }
    |    { $<value>$ = ""; }
    ;

scope_body : LCURLY statements RCURLY
    | COLON statement 
        { 
            projects.top()->addChildAST($<node>2);
            $<node>2->setDepth(depth);
        }
    |
    ;

else_statement : "else" LCURLY
        {
            ProjectAST *projectAST = new ProjectAST(ProjectAST::FunctionScope);
            projects.push(projectAST);
            projects.top()->scopedID = "else";
            projects.top()->args = "";
            depth++;
        }
    scope_body RCURLY
        {
            $<node>$ = projects.pop();
            depth--;
        }
    |
        {
            $<node>$ = new ProjectAST();
        }
    ;

comment : COMMENT NEWLINE
        {
            CommentAST *node = new CommentAST();
            node->comment = $<value>1 + "\n";
            $<node>$ = node;
        }
    ;

emptyline : NEWLINE
    ;

%%

#include "qmake_lex.cpp"
