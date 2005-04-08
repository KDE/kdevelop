%{
/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   Copyright (c) 2005 by Matt Rogers                                     *
 *   mattr@kde.org                                                         *
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
Autotools Parser

Simple LALR parser which builds the syntax tree (see @ref Autotools::AST).

@todo Recognize comments after statements like:
noinst_HEADERS = foo.h #regognize me

@fixme Parser fails on files that do not end with a newline
@fixme 1 shift/reduce conflict in "line_body" rule
*/

#include <qvaluestack.h>
#include "autotoolsast.h"

#define YYSTYPE_IS_DECLARED

using namespace AutoTools;

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
    
    For example we have in Makefile.am file:
    @code
    foo_SOURCES = foo1.cpp \
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
    printf("bison error: %s\n", str);
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
%token ID_LIST
%token LBRACE
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
%token DOT
%token RCURLY
%token LCURLY
%token ID_ARGS
%token LIST_COMMENT
%token ID_LIST_SINGLE
%token IF_KEYWORD
%token ELSE_KEYWORD
%token ENDIF_KEYWORD
%token KEYWORD
%token RULE

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

	if ( $<node>2->nodeType() == AST::ProjectAST && 
	     static_cast<ProjectAST*>( $<node>2 )->scopedID == "if" )
	{
		$<node>2->setDepth(depth);
		depth++;
	}
	else if ( $<node>2->nodeType() == AST::ProjectAST && 
	          static_cast<ProjectAST*>( $<node>2 )->scopedID == "else" )
	{
		--depth;
		$<node>2->setDepth(depth);
		++depth;
	}
	else if ( $<node>2->nodeType() == AST::ProjectAST && 
	          static_cast<ProjectAST*>( $<node>2 )->scopedID == "endif" )
	{
		--depth;
		$<node>2->setDepth(depth);
	}
	else
		$<node>2->setDepth(depth);
}
|
;

statement : variable_assignment
{
	$<node>$ = $<node>1;
}
| automake_if
{
	$<node>$ = $<node>1;
}
| else_statement
{
	$<node>$ = $<node>1;
}
| endif_statement
{
	$<node>$ = $<node>1;
}
| target
{
	$<node>$ = $<node>1;
}
| am_rule
{
	$<node>$ = $<node>1;
}
| include_directive 
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

automake_if: IF_KEYWORD scoped_identifier
{
	ConditionAST* projectAST = new ConditionAST();
	projectAST->type = "if";
	projectAST->conditionName = $<value>2;
	$<node>$ = projectAST;
};

endif_statement: ENDIF_KEYWORD
{
	ConditionAST* past = new ConditionAST();
	past->type= "endif";
	$<node>$ = past;
}
| ENDIF_KEYWORD scoped_identifier
{
	ConditionAST* past = new ConditionAST();
	past->type= "endif";
	past->conditionName = $<value>2;
	$<node>$ = past;
}
;

else_statement: ELSE_KEYWORD 
{
	ConditionAST* past = new ConditionAST();
	past->type = "else";
	$<node>$ = past;
}
| ELSE_KEYWORD scoped_identifier
{
	ConditionAST* past = new ConditionAST();
	past->type = "else";
	past->conditionName = $<value>2;
	$<node>$ = past;
}
;

variable_assignment : scoped_identifier operator multiline_values
{
	AssignmentAST *node = new AssignmentAST();
	node->scopedID = $<value>1;
	node->op = $<value>2;
	node->values = $<values>3;
	$<node>$ = node;
}
;

scoped_identifier : ID_SIMPLE scoped_identifier  
{ $<value>$ = $<value>1 + $<value>2; }
| ID_SIMPLE
;

multiline_values : multiline_values line_body
{
	$<values>$.append($<value>2);
}
|   { $<values>$.clear(); }
    ;

line_body : ID_LIST CONT         { $<value>$ = $<value>1 + " \\\n"; }
    | ID_LIST_SINGLE NEWLINE     { $<value>$ = $<value>1 + "\n"; }
    | CONT                       { $<value>$ = "\\\n"; }
    | LIST_COMMENT
    ;

target: scoped_identifier COLON multiline_values
{
	AutomakeTargetAST *node = new AutomakeTargetAST();
	node->target = $<value>1;
	node->deps = $<values>3;
	$<node>$ = node;
}
;

am_rule: RULE
{
	ProjectAST* node = new ProjectAST(ProjectAST::Rule);
	node->scopedID = $<value>1;
	$<node>$ = node;
}
;

include_directive: KEYWORD scoped_identifier
{
	AssignmentAST *node = new AssignmentAST();
	node->scopedID = $<value>1;
	node->values = QStringList($<value>2);
	$<node>$ = node;
}
;

operator : EQ | PLUSEQ
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

#include "autotools_lex.cpp"
