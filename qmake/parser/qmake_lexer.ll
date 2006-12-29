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

#include <QtCore/QString>

#define DONT_INCLUDE_FLEXLEXER

#include "qmake_lexer.h"


%}

%option noyywrap
%option yylineno
%option c++
%option yyclass="QMake::Lexer"
%option debug

%x assignment
%x fnarg
%x op

ws            [ \t\f]
letter        [a-zA-Z]
digit         [0-9]
newline       ("\n"|"\r\n"|"\r")
identifier    ({letter}|{digit}|"_")(({letter}|{digit}|"_")|".")*
op            ("="|"+="|"-="|"~="|"*=")
non_ws_cont   [^ \t\f\r\n\\]+
non_cont      [^\n\r\\]+
fnvalue       ([^ \t\f\n\r,$()]|"$("[^ \t\f\n\r,$()]+")")+
%%

<fnarg,assignment,INITIAL>"$"   { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::token::DOLLAR; }
"{"                             { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::LCURLY; }
"}"                             { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::RCURLY; }
<fnarg,INITIAL>"("              { BEGIN(fnarg); bracecount++; mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::LPAREN; }
<fnarg,INITIAL>")"              {
                                    bracecount--;
                                    if( bracecount == 0 )
                                        BEGIN(INITIAL);
                                    mylval->value = QString::fromLocal8Bit(YYText(), YYLeng());
                                    return Parser::token::RPAREN;
                                }
<op>{ws}*"+="{ws}*              { BEGIN(assignment);mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::PLUSEQ; }
<op>{ws}*"~="{ws}*              { BEGIN(assignment);mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::TILDEEQ; }
<op>{ws}*"-="{ws}*              { BEGIN(assignment);mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::MINUSEQ; }
<op>{ws}*"*="{ws}*              { BEGIN(assignment);mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::STAREQ; }
<op>{ws}*"="{ws}*               { BEGIN(assignment);mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::EQUAL; }
":"                             { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::COLON; }
<fnarg,INITIAL>","              { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::COMMA; }
"!"                             { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::EXCLAM; }
"|"                             { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::OR; }
{identifier}/{ws}*"{"           { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::SCOPENAME; }
{identifier}/{ws}*":"           { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::SCOPENAME; }
{identifier}/{ws}*"("           { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::FUNCTIONNAME; }
<fnarg>"$$"{identifier}/{ws}*"("    {
                                        mylval->value = QString::fromLocal8Bit(YYText(), YYLeng());
                                        return Parser::token::FUNCTIONCALL;
                                    }
{identifier}/{ws}*{op}          { BEGIN(op); mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::VARIABLE; }
"$$"{identifier}                { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::QMVARIABLE; }
"$${"{identifier}"}"            { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::QMVARIABLE; }
"$("{identifier}")"             { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::SHELLVARIABLE; }
"$$["{identifier}"]"            { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::QMVARIABLE; }
<assignment>{ws}*"\\"{newline}  { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::CONT; }
<assignment>{non_ws_cont}       { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::VAR_VALUE; }
<assignment>"\""{non_cont}"\""  { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::QUOTED_VAR_VALUE; }
^{ws}*{newline}                 { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::EMPTYLINE; }
<assignment,INITIAL>{ws}*{newline}  {
                                        BEGIN(INITIAL);
                                        mylval->value = QString::fromLocal8Bit(YYText(), YYLeng());
                                        return Parser::token::NEWLINE;
                                    }

<assignment,fnarg,INITIAL>{ws}+     {
                                        mylval->value = QString::fromLocal8Bit(YYText(), YYLeng());
                                        return Parser::token::WS;
                                    }
{ws}*"#"[^\n]*                  { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::COMMENT; }
<fnarg>{fnvalue}                { mylval->value = QString::fromLocal8Bit(YYText(), YYLeng()); return Parser::token::FNVALUE; }

%%

namespace QMake
{
    Lexer::Lexer( std::istream* argin, std::ostream* argout )
        : yyFlexLexer(argin, argout), bracecount(0)
    {
    }

    int Lexer::yylex( QMake::Result* yylval )
    {
        mylval = yylval;
        return yylex();
    }
}

int QMakelex( QMake::Result* yylval, QMake::Lexer* lexer)
{
    return lexer->yylex( yylval );
}
