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
#include <iostream>


%}

%option noyywrap
%option yylineno
%option debug

ws            [ \t\f]
letter        [a-zA-Z]
digit         [0-9]
newline       ("\n"|"\r\n"|"\r")
specialchar   ("@"|"%"|"&"|"^"|"\'"|"<"|"?"|">"|"/"|"+"|"-"|"*"|"~"|".")
identifier    ({letter}|{digit}|"_")(({letter}|{digit}|"_")|".")*

%%

{ws}+           { yylval.value = yytext; return WS; }
"$"             { yylval.value = yytext; return DOLLAR; }
"{"             { yylval.value = yytext; return LCURLY; }
"}"             { yylval.value = yytext; return RCURLY; }
"("             { yylval.value = yytext; return LPAREN; }
")"             { yylval.value = yytext; return RPAREN; }
"["             { yylval.value = yytext; return LBRACKET; }
"]"             { yylval.value = yytext; return RBRACKET; }
"+="            { yylval.value = yytext; return PLUSEQ; }
"~="            { yylval.value = yytext; return TILDEEQ; }
"-="            { yylval.value = yytext; return MINUSEQ; }
"*="            { yylval.value = yytext; return STAREQ; }
"="             { yylval.value = yytext; return EQUAL; }
":"             { yylval.value = yytext; return COLON; }
","             { yylval.value = yytext; return COMMA; }
"!"             { yylval.value = yytext; return EXCLAM; }
{specialchar}   { yylval.value = yytext; return SPECIALCHAR; }
"_"             { yylval.value = yytext; return UNDERSCORE; }
"|"             { yylval.value = yytext; return OR; }
"$$"{identifier}"("    { yylval.value = yytext; unput('('); return FUNCTIONNAME; }
{identifier}    { yylval.value = yytext; return IDENTIFIER; }
"$$"{identifier}    { yylval.value = yytext; return VARIABLE; }
"$${"{identifier}"}"    { yylval.value = yytext; return VARIABLE; }
"$("{identifier}")"    { yylval.value = yytext; return VARIABLE; }
"$$["{identifier}"]"    { yylval.value = yytext; return VARIABLE; }
"\\"{newline}   { yylval.value = yytext; return CONT; }
"#"[^\n]*       { yylval.value = yytext; return COMMENT; }
"\""            { yylval.value = yytext; return QUOTE; }
^{ws}*{newline} { yylval.value = yytext; return EMPTYLINE; }
{newline}       { yylval.value = yytext; return NEWLINE; }
";"             { yylval.value = yytext; return SEMICOLON; }


%%

