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

int openbrace;

%}

%option noyywrap
%option yylineno
%option debug

%x varvalue
%x funcargs
ws            [ \t]*
letter        [a-zA-Z]
digit         [0-9]
varval     [^\n\"\\{(})=$# \t|:]
funcargval [^\n\"\\{(})=$# \t|:,]

%%

<varvalue,funcargs,INITIAL>{ws}        {
                yylval.value = yytext; return WS;
            }

({letter}|{digit}|"_")({letter}|{digit}|"_"|".")*{ws}("+"|"="|"-"|"*"|"~") {
                yylval.value = yytext;
                yylval.value = yylval.value.left( yylval.value.length()-1 );
                yyless( yylval.value.length() );
                return VARIABLENAME;
            }

("!"|{letter}|{digit}|"_")({letter}|{digit}|"_"|".")*{ws}(":"|"{") {
                yylval.value = yytext;
                yylval.value = yylval.value.left( yylval.value.length()-1 );
                yyless( yylval.value.length() );
                return SCOPENAME;
            }

("!"|{letter}|{digit}|"_")({letter}|{digit}|"_"|".")*{ws}"(" {
                yylval.value = yytext;
                yylval.value = yylval.value.left( yylval.value.length()-1 );
                unput('(');
                openbrace = 0;
                BEGIN(funcargs);
                return FUNCTIONNAME;
            }

<funcargs>("!"|{letter}|{digit}|"_")({letter}|{digit}|"_"|".")*{ws}"(" {
                yylval.value = yytext;
                yylval.value = yylval.value.left( yylval.value.length()-1 );
                unput('(');
                BEGIN(funcargs);
                return FUNCTIONNAME;
            }

"+="        { BEGIN(varvalue); yylval.value = yytext; return PLUSEQ; }
"-="        { BEGIN(varvalue); yylval.value = yytext; return MINUSEQ; }
"="         { BEGIN(varvalue); yylval.value = yytext; return EQUAL; }
"*="        { BEGIN(varvalue); yylval.value = yytext; return STAREQ; }
"~="        { BEGIN(varvalue); yylval.value = yytext; return TILDEEQ; }

<varvalue,INITIAL>"\n"  { BEGIN(INITIAL); yylval.value = yytext; return NEWLINE; }

<varvalue>"\\"{ws}"\n"  {yylval.value = yytext; return CONT;}
<varvalue>{varval}+     { yylval.value = yytext; return VARVAL; }
<varvalue,funcargs>"$"  { yylval.value = yytext; return DOLLAR; }

<varvalue>"(" { yylval.value = yytext; return LBRACE; }
<varvalue>")" { yylval.value = yytext; return RBRACE; }
<varvalue>"}" { yylval.value = yytext; return RCURLY; }
<varvalue>"{" { yylval.value = yytext; return LCURLY; }

<varvalue,funcargs>"\""     { yylval.value = yytext; return QUOTE; }
<varvalue,INITIAL>"#"[^\n]* { yylval.value = yytext; return COMMENT;}

<varvalue,funcargs>{ws}","{ws}  { yylval.value = yytext; return COMMA; }
<funcargs>{funcargval}+         { yylval.value = yytext; return FUNCARGVAL; }

<funcargs>{ws}"("       {
                            openbrace++;
                            yylval.value = yytext;
                            return LBRACE;
                        }
<funcargs>{ws}")"       {
                            openbrace--;
                            if( openbrace <= 0 )
                                BEGIN(INITIAL);
                            yylval.value = yytext;
                            return RBRACE;
                        }

<varvalue>":"       { yylval.value = yytext; return COLON; }
<INITIAL>{ws}":"    { BEGIN(INITIAL); yylval.value = yytext; return COLON; }

<funcargs,INITIAL>{ws}"}" { yylval.value = yytext; return RCURLY; }
<funcargs,INITIAL>{ws}"{" { yylval.value = yytext; return LCURLY; }

"|"         { yylval.value = yytext; return OR; }

%%

