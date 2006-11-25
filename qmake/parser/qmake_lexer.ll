%{
/***************************************************************************
 *   Copyright (C) 2006 by Andreas Pakulat apaku@gmx.de                    *
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

#include "qmake_parser.hpp"

%}

%option noyywrap
%option debug
%option yylineno

single_ws              [ \t]
multi_ws               {single_ws}+
quote                  "\""
newline                \n
continuation           \\
lbrace                 (
rbrace                 )
lbracket               {
rbracket               }
letter                 [a-zA-Z]
digit                  [0-9]
word                   ({digit}|{letter}|_)({letter}|{digit}|_|-|\*|\.)*
comma                  ,
commentstart           #

%%

{commentstart}.*{newline}             { return COMMENT; }
^{newline}$                           { return NEWLINE; }

%%

