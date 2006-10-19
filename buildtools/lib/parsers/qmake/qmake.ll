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

#include <qmake_yacc.h>
#include <stdlib.h>

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

%x list
%x funcargs

delim             [ \t]
ws                {delim}+
quote             "\""
var_value         [^\n\t ]*
quoted_var_value  {quote}{var_value}({var_value}|"\t"|" ")*{quote}
letter            [A-Za-z]
digit             [0-9]
id_simple         ({digit}|{letter}|\!|-|_|\*|\$)({letter}|{digit}|\||\!|-|_|\*|\$|\.|\+|\-)*
id_args           [^\n]*\)
number            {digit}+
comment           #.*
comment_cont      {ws}*#.*\n
cont              \\{ws}*\n

%%

<list,INITIAL>{ws}     {}
<list,INITIAL>{cont}   { BEGIN(list); return CONT; }
{id_simple}            { yylval.value = yytext; return (ID_SIMPLE); }

<list>{comment_cont} {
    yylval.value = yytext;
    BEGIN(list);
    return (LIST_COMMENT);
    }

<funcargs>{id_args} {
    yylval.value = yytext;
    yylval.value = yylval.value.mid(0, yylval.value.length()-1);
    unput(')');
    BEGIN(INITIAL);
    return (ID_ARGS);
    }

<list>{var_value}        { yylval.value = yytext; return VARIABLE_VALUE; }
<list>{quoted_var_value} { yylval.value = yytext; return QUOTED_VARIABLE_VALUE; }

"="                      { BEGIN(list); yylval.value = yytext; return EQ; }
"+="                     { BEGIN(list); yylval.value = yytext; return PLUSEQ; }
"-="                     { BEGIN(list); yylval.value = yytext; return MINUSQE; }
"*="                     { BEGIN(list); yylval.value = yytext; return STAREQ; }
"~="                     { BEGIN(list); yylval.value = yytext; return TILDEEQ; }
"{"                      { return LCURLY; }
"}"                      { return RCURLY; }
"("                      { BEGIN(funcargs); return LBRACE; }
<funcargs,INITIAL>")"    { BEGIN(INITIAL); return RBRACE; }
":"                      { yylval.value = yytext; return COLON; }
<list,INITIAL>"\n"       { BEGIN(INITIAL); return NEWLINE; }

{comment}                { yylval.value = yytext; return (COMMENT); }

%%
