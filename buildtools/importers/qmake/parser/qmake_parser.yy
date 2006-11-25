%{
/***************************************************************************
 *   Copyright (C) 2006 by Andreas Pakulat                                 *
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
*/

#include <stdio.h>

void yyerror(const char* str)
{
    printf("%s\n", str);
}

int yylex();

%}

%token WORD
%token WS
%token NEWLINE
%token COMMENT

%%

project: comment
    | emptyline
    |
    ;

comment: COMMENT
    ;

emptyline: NEWLINE
    ;

%%

#include "qmake_lexer.cpp"

