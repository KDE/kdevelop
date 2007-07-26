%{
/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *   Copyright (C) 2006 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
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

namespace QMake
{
    class Lexer;

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
    QString indent;
};

#define YYSTYPE Result
typedef Result YYSTYPE;
}

extern int QMakelex( QMake::Result* yylval, QMake::Lexer* lexer );

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
//QValueStack<ProjectAST*> projects;

/**
The current depth of AST node is stored here.
AST depth is important to know because automatic indentation can
be easily implemented (the parser itself looses all information
about indentation).
*/
// int depth = 0;

/*
To debug this parser, put the line below into the next bison file section.
Don't forget to uncomment "yydebug = 1" line in qmakedriver.cpp.
%debug
*/
%}

%skeleton "lalr1.cc"
%define "parser_class_name" "Parser"
%name-prefix="QMake"
%parse-param { QMake::Lexer* lexer }
%parse-param { QValueStack<ProjectAST*>& projects }
%parse-param { int depth }
%lex-param   { QMake::Lexer* lexer }
%start project

%token ID_SIMPLE
%token EQ
%token PLUSEQ
%token MINUSEQ
%token STAREQ
%token TILDEEQ
%token LBRACE
%token RBRACE
%token COLON
%token NEWLINE
%token COMMENT
%token CONT
%token COMMENT_CONT
%token RCURLY
%token LCURLY
%token ID_ARGS
%token QUOTED_VARIABLE_VALUE
%token VARIABLE_VALUE
%token LIST_WS
%token ENDOFFILE
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

variable_assignment : ID_SIMPLE operator multiline_values listws NEWLINE
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values = $<values>3 ;
            node->values.append( $<value>4 );
            node->values.append( $<value>5 );
            node->indent = $<indent>3;
            $<node>$ = node;
        }
    | ID_SIMPLE operator multiline_values listws ENDOFFILE
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values = $<values>3 ;
            node->values.append( $<value>4 );
            node->indent = $<indent>3;
            $<node>$ = node;
        }
    | ID_SIMPLE operator multiline_values listws CONT
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values = $<values>3 ;
            node->values.append( $<value>4 );
            node->values.append( $<value>5 );
            node->indent = $<indent>3;
            $<node>$ = node;
        }
    | ID_SIMPLE operator multiline_values listws COMMENT_CONT
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values = $<values>3 ;
            node->values.append( $<value>4 );
            node->values.append( $<value>5 );
            node->indent = $<indent>3;
            $<node>$ = node;
        }
    | ID_SIMPLE operator listws NEWLINE
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values.append( $<value>3 );
            node->values.append( $<value>4 );
            $<node>$ = node;
        }
    | ID_SIMPLE operator listws ENDOFFILE
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values.append( $<value>3 );
            $<node>$ = node;
        }
    | ID_SIMPLE operator listws COMMENT
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values.append( $<value>3 );
            node->values.append( $<value>4 );
            $<node>$ = node;
        }
    | ID_SIMPLE operator multiline_values listws COMMENT
        {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = $<value>1;
            node->op = $<value>2;
            node->values = $<values>3;
            node->values.append( $<value>4 );
            node->values.append( $<value>5 );
            node->indent = $<indent>3;
            $<node>$ = node;
        }
    ;

possible_value : variable_value | COMMENT_CONT | CONT

multiline_values : multiline_values LIST_WS possible_value
        {
            $<values>$.append( $<value>2 );
            $<values>$.append( $<value>3 );
        }
    | multiline_values variable_value
        {
            $<values>$.append( $<value>2 );
        }
    | multiline_values listws CONT listws possible_value
        {
            $<values>$.append( $<value>2 );
            $<values>$.append( $<value>3 );
            $<values>$.append( $<value>4 );
            $<values>$.append( $<value>5 );
            if( $<indent>4 != "" && $<indent>$ == "" )
                $<indent>$ = $<indent>4;
        }
    | multiline_values listws COMMENT_CONT listws possible_value
        {
            $<values>$.append( $<value>2 );
            $<values>$.append( $<value>3 );
            $<values>$.append( $<value>4 );
            $<values>$.append( $<value>5 );
            if( $<indent>4 != "" && $<indent>$ == "" )
                $<indent>$ = $<indent>4;
        }
    | listws possible_value
        {
            $<values>$ = QStringList();
            $<values>$.append( $<value>1 );
            $<values>$.append( $<value>2 );
        }
    ;


variable_value : VARIABLE_VALUE     { $<value>$ = $<value>1; }
    | QUOTED_VARIABLE_VALUE  { $<value>$ = $<value>1; }
    ;


listws: LIST_WS
        {
            $<value>$ = $<value>1;
            $<indent>$ = $<value>1;
        }
    |
        {
            $<value>$ = QString();
            $<indent>$ = QString();
        }
    ;
operator : EQ
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

comment : COMMENT
        {
            CommentAST *node = new CommentAST();
            node->comment = $<value>1;
            $<node>$ = node;
        } 
    ;

emptyline : NEWLINE
    ;

%%


namespace QMake
{
    void Parser::error(const location_type& /*l*/, const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}
