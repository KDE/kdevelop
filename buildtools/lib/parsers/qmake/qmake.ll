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

#include <stdlib.h>

#define DONT_INCLUDE_FLEXLEXER

#include "qmake_lex.h"

/**
@file qmake.ll
QMake Lexer

There are 3 types of identifiers recognized by this lexer:
-id_simple: examples of such identifiers are qmake variables and scoped variables
at the left of the operator in assignments (like "SOURCES" in "SOURCES+=foo.cpp goo.cpp");
-id_list: those are "value list identifiers" at the right side in assignments
(like "foo.cpp goo.cpp" in "SOURCES+=foo.cpp goo.cpp");
-id_args: function arguments recognized as one identifier
(example: ""${QMAKE_FILE} is intended only for Windows!""
in "!win32-*:!wince-*:error("${QMAKE_FILE} is intended only for Windows!")" statements).
.

To recognize those identifiers two additional start conditions are used: list and funcargs.

@note "Not" operator (!) is recognized as a part of an identifier. Linefeeds passed to
the parser as NEWLINE tokens to preserve file structure but whitespaces are stripped
so no indentation is preserved by this lexer (and parser).

To debug this lexer, put the line below into the next flex file section.
%option debug
*/
%}

%option noyywrap
%option yylineno
%option c++
%option yyclass="QMake::Lexer"

%x list
%x list_with_comment
%x funcargs

delim             [ \t]
ws                {delim}+
begin_ws          ^{delim}+[^\r\n\t ]
quote             "\""
var_value         [^\r\n\t ]*[^\r\n\t \\]
quoted_var_value  {quote}({var_value}|[\t ])({var_value}|[\t ])*{quote}
letter            [A-Za-z]
digit             [0-9]
id_simple         ({digit}|{letter}|\!|-|_|\*|\$)({letter}|{digit}|\||\!|-|_|\*|\$|\.)*
id_args           [^\r\n]*\)
number            {digit}+
comment           #.*
comment_cont      {ws}*#.*(\n|\r|\r\n)
comment_cont_nn   {ws}*#.*
cont              \\{ws}*(\n|\r|\r\n)

%%
<INITIAL>{ws} {}

<list,list_with_comment>{ws} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::LIST_WS;
}

<list,list_with_comment>{begin_ws} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    unput(char(mylval->value.at(mylval->value.length()-1).latin1()));
    mylval->value = mylval->value.mid(0, mylval->value.length()-1);
    return Parser::token::token::INDENT;
}

<list,list_with_comment,INITIAL>{cont} {
    BEGIN(list);
    return Parser::token::token::CONT;
}
{id_simple} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return (Parser::token::token::ID_SIMPLE);
}

<list_with_comment>{comment_cont_nn} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    BEGIN(list);
    return (Parser::token::token::LIST_COMMENT_WITHOUT_NEWLINE);
}

<list>{comment_cont} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    BEGIN(list);
    return (Parser::token::token::LIST_COMMENT);
    }

<funcargs>{id_args} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    mylval->value = mylval->value.mid(0, mylval->value.length()-1);
    unput(')');
    BEGIN(INITIAL);
    return (Parser::token::token::ID_ARGS);
    }

<list,list_with_comment>{var_value} {
    BEGIN(list_with_comment);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::VARIABLE_VALUE;
}

<list,list_with_comment>{quoted_var_value} {
    BEGIN(list_with_comment);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::QUOTED_VARIABLE_VALUE;
}

"=" {
BEGIN(list);
mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
return Parser::token::token::EQ;
}

"+=" {
    BEGIN(list);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::PLUSEQ;
}

"-=" {
    BEGIN(list);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::MINUSEQ;
}

"*=" {
    BEGIN(list);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::STAREQ;
}

"~=" {
    BEGIN(list);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::TILDEEQ;
}

"{" {
    return Parser::token::token::LCURLY;
}

":"{delim}*"{" {
    return Parser::token::token::LCURLY;
}

"}" {
    return Parser::token::token::RCURLY;
}

"(" {
    BEGIN(funcargs);
    return Parser::token::token::LBRACE;
}

<funcargs,INITIAL>")" {
    BEGIN(INITIAL);
    return Parser::token::token::RBRACE;
}

":" {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::COLON;
}

<list,list_with_comment,INITIAL>"\n" {
    BEGIN(INITIAL);
    return Parser::token::token::NEWLINE;
}

{comment} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return (Parser::token::token::COMMENT);
}

%%
namespace QMake
{
    Lexer::Lexer( std::istream* argin, std::ostream* argout )
        : yyFlexLexer(argin, argout), mylval(0)
    {
    }

    int Lexer::yylex( QMake::Parser::semantic_type* yylval )
    {
        mylval = yylval;
        return yylex();
    }
}

int QMakelex( QMake::Parser::semantic_type* yylval, QMake::Lexer* lexer)
{
    return lexer->yylex( yylval );
}

