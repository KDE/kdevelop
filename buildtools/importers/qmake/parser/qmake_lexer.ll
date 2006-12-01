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

#include "qmake_parser.hpp"
#include <stdlib.h>
#include <QtCore/QString>

%}

%option noyywrap
%option yylineno

ws            [ \t]
letter        [a-zA-Z]
digit         [0-9]
simpleval     [^\n\\{(})=$,#{letter}{digit}{ws}]

%%

{ws}                { yylval.value = yytext; return WS; }
{letter}            { yylval.value = yytext; return LETTER; }
{digit}             { yylval.value = yytext; return DIGIT; }
"_"                 { yylval.value = yytext; return UNDERSCORE; }
"."                 { yylval.value = yytext; return DOT; }
","                 { yylval.value = yytext; return COMMA; }
"+="                { yylval.value = yytext; return PLUSEQ; }
"-="                { yylval.value = yytext; return MINUSEQ; }
"="                 { yylval.value = yytext; return EQUAL; }
"*="                { yylval.value = yytext; return STAREQ; }
"~="                { yylval.value = yytext; return TILDEEQ; }
"{"                 { yylval.value = yytext; return LCURLY; }
"}"                 { yylval.value = yytext; return RCURLY; }
"("                 { yylval.value = yytext; return LBRACE; }
")"                 { yylval.value = yytext; return RBRACE; }
"$"                 { yylval.value = yytext; return DOLLAR; }
"\""                { yylval.value = yytext; return QUOTE; }
"\n"                { yylval.value = yytext; return NEWLINE; }
"\\"                { yylval.value = yytext; return CONT; }
{simpleval}         { yylval.value = yytext; return SIMPLEVAL; }
"#"[^\n]*           { yylval.value = yytext; return COMMENT; }
%%

