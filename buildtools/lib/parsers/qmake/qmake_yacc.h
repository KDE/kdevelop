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
     ID_SIMPLE = 258,
     EQ = 259,
     PLUSEQ = 260,
     MINUSQE = 261,
     STAREQ = 262,
     TILDEEQ = 263,
     LBRACE = 264,
     RBRACE = 265,
     COLON = 266,
     NUMSIGN = 267,
     NEWLINE = 268,
     NUMBER = 269,
     COMMENT = 270,
     CONT = 271,
     RCURLY = 272,
     LCURLY = 273,
     ID_ARGS = 274,
     LIST_COMMENT = 275,
     LIST_COMMENT_WITHOUT_NEWLINE = 276,
     QUOTED_VARIABLE_VALUE = 277,
     VARIABLE_VALUE = 278,
     INDENT = 279,
     LIST_WS = 280,
     OR_OPERATOR = 281
   };
#endif
/* Tokens.  */
#define ID_SIMPLE 258
#define EQ 259
#define PLUSEQ 260
#define MINUSQE 261
#define STAREQ 262
#define TILDEEQ 263
#define LBRACE 264
#define RBRACE 265
#define COLON 266
#define NUMSIGN 267
#define NEWLINE 268
#define NUMBER 269
#define COMMENT 270
#define CONT 271
#define RCURLY 272
#define LCURLY 273
#define ID_ARGS 274
#define LIST_COMMENT 275
#define LIST_COMMENT_WITHOUT_NEWLINE 276
#define QUOTED_VARIABLE_VALUE 277
#define VARIABLE_VALUE 278
#define INDENT 279
#define LIST_WS 280
#define OR_OPERATOR 281




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

