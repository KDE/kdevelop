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

%x vallist
%x funcargs

delim             [ \t]
ws                {delim}+
newline           (\n|\r|\r\n)
quote             "\""
var_value         [^#\r\n\t ]*[^\r\n\t \\]
quoted_var_value  {quote}({var_value}|[\t ])({var_value}|[\t ])*{quote}
letter            [A-Za-z]
digit             [0-9]
id_simple         ({digit}|{letter}|\!|-|_|\*|\$)({letter}|{digit}|\||\!|-|_|\*|\$|\.|\+)*
id_args           [^\r\n]*\)
number            {digit}+
comment           #[^\r\n]*{newline}
comment_cont      (\\{ws}*#[^\r\n]*{newline}|#[^\r\n]*\\{newline})
cont              \\{ws}*{newline}

%%
<vallist><<EOF>> {
    BEGIN(INITIAL);
    return Parser::token::token::ENDOFFILE;
}
<INITIAL>{ws} {}

<vallist>{ws} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::LIST_WS;
}

<vallist,INITIAL>{cont} {
    BEGIN(vallist);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::CONT;
}

<vallist,INITIAL>{comment_cont} {
    BEGIN(vallist);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::COMMENT_CONT;
}

{id_simple} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return (Parser::token::token::ID_SIMPLE);
}

<funcargs>{id_args} {
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    mylval->value = mylval->value.mid(0, mylval->value.length()-1);
    unput(')');
    BEGIN(INITIAL);
    return (Parser::token::token::ID_ARGS);
    }

<vallist>{var_value} {
    BEGIN(vallist);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::VARIABLE_VALUE;
}

<vallist>{quoted_var_value} {
    BEGIN(vallist);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::QUOTED_VARIABLE_VALUE;
}

"=" {
    BEGIN(vallist);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::EQ;
}

"+=" {
    BEGIN(vallist);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::PLUSEQ;
}

"-=" {
    BEGIN(vallist);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::MINUSEQ;
}

"*=" {
    BEGIN(vallist);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return Parser::token::token::STAREQ;
}

"~=" {
    BEGIN(vallist);
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


<vallist>{ws}{newline} {
    BEGIN(INITIAL);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    setLineEndingFromString( mylval->value );
    return Parser::token::token::NEWLINE;
}

<vallist,INITIAL>{newline} {
    BEGIN(INITIAL);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    setLineEndingFromString( mylval->value );
    return Parser::token::token::NEWLINE;
}

<vallist,INITIAL>{comment} {
    BEGIN(INITIAL);
    mylval->value = QString::fromLocal8Bit( YYText(), YYLeng() );
    return (Parser::token::token::COMMENT);
}

%%
namespace QMake
{
    Lexer::Lexer( std::istream* argin, std::ostream* argout )
        : yyFlexLexer(argin, argout), mylval(0), m_lineEnding(None)
    {
    }

    int Lexer::yylex( QMake::Parser::semantic_type* yylval )
    {
        mylval = yylval;
        return yylex();
    }

    void Lexer::setLineEndingFromString( const QString& str )
    {
        if( str.endsWith("\r\n") && m_lineEnding == None )
            m_lineEnding = Windows;
        else if ( str.endsWith("\r") && m_lineEnding == None )
            m_lineEnding = MacOS;
        else if ( m_lineEnding == None )
            m_lineEnding = Unix;
    }

    Lexer::LineEnding Lexer::lineending()
    {
        return m_lineEnding;
    }
}

int QMakelex( QMake::Parser::semantic_type* yylval, QMake::Lexer* lexer)
{
    return lexer->yylex( yylval );
}

