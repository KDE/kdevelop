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
%option debug
%option yylineno

single_ws              [ \t]
multi_ws               {single_ws}+
ws                     {single_ws}*
quote                  "\""
newline                \n
continuation           \\
lbrace                 (
rbrace                 )
lbracket               {
rbracket               }
letter                 [a-zA-Z]
digit                  [0-9]
word                   ({digit}|{letter}|_)({letter}|{digit}|_|\-|\*|\.)*
comma                  ,
commentstart           #
op                     (=|\+=|\-=|\*=|~=|\^=)
dollar                 \$
%%

{commentstart}.*{newline}   {
            yylval.value = QString::fromUtf8( yytext );
            yylval.value = yylval.value.left( yylval.value.length() - 1 );
            unput('\n');
            return COMMENT;
    }
{newline}                   { return NEWLINE; }
{continuation}              { yylval.value = QString::fromUtf8( yytext ); return CONT; }
{op}                        { yylval.value = yytext; return OP; }
{dollar}                    { yylval.value = yytext; return DOLLAR; }
{word}                      { yylval.value = yytext; return WORD; }
{single_ws}+                { yylval.value = yytext; return WS; }
{quote}                     { yylval.value = yytext; return QUOTE; }

%%

