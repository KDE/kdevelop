/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

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

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

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
     QUOTED_VARIABLE_VALUE = 276,
     VARIABLE_VALUE = 277,
     INDENT = 278,
     LIST_WS = 279
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
#define QUOTED_VARIABLE_VALUE 276
#define VARIABLE_VALUE 277
#define INDENT 278
#define LIST_WS 279




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



