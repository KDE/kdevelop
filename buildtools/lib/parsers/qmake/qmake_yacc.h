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
     ID_LIST = 259,
     EQ = 260,
     PLUSEQ = 261,
     MINUSQE = 262,
     STAREQ = 263,
     TILDEEQ = 264,
     LBRACE = 265,
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
/* Tokens.  */
#define ID_SIMPLE 258
#define ID_LIST 259
#define EQ 260
#define PLUSEQ 261
#define MINUSQE 262
#define STAREQ 263
#define TILDEEQ 264
#define LBRACE 265
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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

