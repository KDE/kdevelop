/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     WS = 258,
     VARIABLENAME = 259,
     FUNCTIONNAME = 260,
     VARVAL = 261,
     DOLLAR = 262,
     COLON = 263,
     COMMA = 264,
     LCURLY = 265,
     RCURLY = 266,
     LBRACE = 267,
     RBRACE = 268,
     QUOTE = 269,
     EQUAL = 270,
     PLUSEQ = 271,
     MINUSEQ = 272,
     TILDEEQ = 273,
     STAREQ = 274,
     NEWLINE = 275,
     CONT = 276,
     COMMENT = 277,
     FUNCARGVAL = 278
   };
#endif
/* Tokens.  */
#define WS 258
#define VARIABLENAME 259
#define FUNCTIONNAME 260
#define VARVAL 261
#define DOLLAR 262
#define COLON 263
#define COMMA 264
#define LCURLY 265
#define RCURLY 266
#define LBRACE 267
#define RBRACE 268
#define QUOTE 269
#define EQUAL 270
#define PLUSEQ 271
#define MINUSEQ 272
#define TILDEEQ 273
#define STAREQ 274
#define NEWLINE 275
#define CONT 276
#define COMMENT 277
#define FUNCARGVAL 278




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

