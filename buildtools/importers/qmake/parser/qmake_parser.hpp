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
     LETTER = 259,
     DIGIT = 260,
     COMMA = 261,
     UNDERSCORE = 262,
     DOT = 263,
     EQUAL = 264,
     PLUSEQ = 265,
     MINUSEQ = 266,
     TILDEEQ = 267,
     STAREQ = 268,
     LCURLY = 269,
     RCURLY = 270,
     LBRACE = 271,
     RBRACE = 272,
     DOLLAR = 273,
     QUOTE = 274,
     SLASH = 275,
     CONT = 276,
     COMMENT = 277,
     SIMPLEVAL = 278,
     NEWLINE = 279
   };
#endif
/* Tokens.  */
#define WS 258
#define LETTER 259
#define DIGIT 260
#define COMMA 261
#define UNDERSCORE 262
#define DOT 263
#define EQUAL 264
#define PLUSEQ 265
#define MINUSEQ 266
#define TILDEEQ 267
#define STAREQ 268
#define LCURLY 269
#define RCURLY 270
#define LBRACE 271
#define RBRACE 272
#define DOLLAR 273
#define QUOTE 274
#define SLASH 275
#define CONT 276
#define COMMENT 277
#define SIMPLEVAL 278
#define NEWLINE 279




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

