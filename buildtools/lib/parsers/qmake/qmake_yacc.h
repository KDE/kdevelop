/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

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
     ID_LIST = 259,
     LBRACE = 260,
     EQ = 261,
     PLUSEQ = 262,
     MINUSQE = 263,
     STAREQ = 264,
     TILDEEQ = 265,
     RBRACE = 266,
     COLON = 267,
     NUMSIGN = 268,
     NEWLINE = 269,
     NUMBER = 270,
     COMMENT = 271,
     CONT = 272,
     DOT = 273,
     RCURLY = 274,
     LCURLY = 275,
     ID_ARGS = 276,
     LIST_COMMENT = 277,
     ID_LIST_SINGLE = 278
   };
#endif
#define ID_SIMPLE 258
#define ID_LIST 259
#define LBRACE 260
#define EQ 261
#define PLUSEQ 262
#define MINUSQE 263
#define STAREQ 264
#define TILDEEQ 265
#define RBRACE 266
#define COLON 267
#define NUMSIGN 268
#define NEWLINE 269
#define NUMBER 270
#define COMMENT 271
#define CONT 272
#define DOT 273
#define RCURLY 274
#define LCURLY 275
#define ID_ARGS 276
#define LIST_COMMENT 277
#define ID_LIST_SINGLE 278




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



