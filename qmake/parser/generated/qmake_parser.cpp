/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++

   Copyright (C) 2002, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.

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

// Take the name prefix into account.
#define yylex   QMakelex

#include "qmake_parser.hpp"

/* User implementation prologue.  */


/* Line 317 of lalr1.cc.  */
#line 45 "/home/andreas/KDE-work/4.0/kdevelop/build/buildtools/importers/qmake/parser/qmake_parser.cpp"

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG							\
  for (bool yydebugcond_ = yydebug_; yydebugcond_; yydebugcond_ = false)	\
    (*yycdebug_)

/* Enable debugging if requested.  */
#if YYDEBUG

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab

namespace QMake
{
#if YYERROR_VERBOSE

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

#endif

  /// Build a parser object.
  Parser::Parser (QMake::Lexer* lexer_yyarg, QMake::ProjectAST** project_yyarg)
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
      lexer (lexer_yyarg),
      project (project_yyarg)
  {
  }

  Parser::~Parser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  Parser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    switch (yytype)
      {
         default:
	  break;
      }
  }


  void
  Parser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif /* ! YYDEBUG */

  void
  Parser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
  
	default:
	  break;
      }
  }

  void
  Parser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }


  int
  Parser::parse ()
  {
    /// Look-ahead and look-ahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    /* State.  */
    int yyn;
    int yylen = 0;
    int yystate = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the look-ahead.
    semantic_type yylval;
    /// Location of the look-ahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location yyerror_range[2];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;
    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without look-ahead.  */
    yyn = yypact_[yystate];
    if (yyn == yypact_ninf_)
      goto yydefault;

    /* Read a look-ahead token.  */
    if (yychar == yyempty_)
      {
	YYCDEBUG << "Reading a token: ";
	yychar = yylex (&yylval, lexer);
      }


    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yyn == 0 || yyn == yytable_ninf_)
	goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Accept?  */
    if (yyn == yyfinal_)
      goto yyacceptlab;

    /* Shift the look-ahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted unless it is eof.  */
    if (yychar != yyeof_)
      yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
	  case 2:
#line 74 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            *project = new ProjectAST();
        ;}
    break;

  case 3:
#line 78 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            foreach( StatementAST* s, (yysemantic_stack_[(2) - (2)].stmtlist))
            {
                (*project)->addStatement( s );
            }
        ;}
    break;

  case 4:
#line 87 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmtlist).append( (yysemantic_stack_[(2) - (2)].stmt) );
        ;}
    break;

  case 5:
#line 91 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmtlist).clear();
        ;}
    break;

  case 6:
#line 97 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = new CommentAST( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 7:
#line 101 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = new NewlineAST( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 8:
#line 105 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = (yysemantic_stack_[(1) - (1)].stmt);
        ;}
    break;

  case 9:
#line 109 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = (yysemantic_stack_[(1) - (1)].stmt);
        ;}
    break;

  case 10:
#line 113 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = new ScopeAST( (yysemantic_stack_[(3) - (2)].funccall), (yysemantic_stack_[(3) - (1)].value) );
        ;}
    break;

  case 11:
#line 117 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = (yysemantic_stack_[(1) - (1)].stmt);
        ;}
    break;

  case 12:
#line 123 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeAST* node = (yysemantic_stack_[(2) - (1)].scope);
            node->setScopeBody( (yysemantic_stack_[(2) - (2)].scopebody) );
            (yyval.stmt) = node;
        ;}
    break;

  case 13:
#line 131 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yysemantic_stack_[(6) - (2)].funccall)->setWhitespace((yysemantic_stack_[(6) - (1)].value));
            (yysemantic_stack_[(6) - (5)].funccall)->setWhitespace((yysemantic_stack_[(6) - (4)].value));
            OrAST* node = new OrAST( (yysemantic_stack_[(6) - (2)].funccall), (yysemantic_stack_[(6) - (3)].value), (yysemantic_stack_[(6) - (5)].funccall), (yysemantic_stack_[(6) - (6)].scopebody) );
            (yyval.stmt) = node;
        ;}
    break;

  case 14:
#line 140 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeAST* node = new ScopeAST( (yysemantic_stack_[(2) - (2)].value), (yysemantic_stack_[(2) - (1)].value) );
            (yyval.scope) = node;
        ;}
    break;

  case 15:
#line 145 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeAST* node = new ScopeAST( (yysemantic_stack_[(2) - (2)].funccall), (yysemantic_stack_[(2) - (1)].value) );
            (yyval.scope) = node;
        ;}
    break;

  case 16:
#line 152 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(8) - (1)].value)+(yysemantic_stack_[(8) - (2)].value)+(yysemantic_stack_[(8) - (3)].value)+(yysemantic_stack_[(8) - (4)].value), (yysemantic_stack_[(8) - (5)].stmtlist), (yysemantic_stack_[(8) - (6)].value)+(yysemantic_stack_[(8) - (7)].value)+(yysemantic_stack_[(8) - (8)].value) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 17:
#line 157 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(7) - (1)].value)+(yysemantic_stack_[(7) - (2)].value)+(yysemantic_stack_[(7) - (3)].value), (yysemantic_stack_[(7) - (4)].stmtlist), (yysemantic_stack_[(7) - (5)].value)+(yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 18:
#line 162 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(6) - (1)].value)+(yysemantic_stack_[(6) - (2)].value)+(yysemantic_stack_[(6) - (3)].value), (yysemantic_stack_[(6) - (4)].stmtlist), (yysemantic_stack_[(6) - (5)].value)+(yysemantic_stack_[(6) - (6)].value) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 19:
#line 167 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(7) - (1)].value)+(yysemantic_stack_[(7) - (2)].value)+(yysemantic_stack_[(7) - (3)].value)+(yysemantic_stack_[(7) - (4)].value), (yysemantic_stack_[(7) - (5)].stmtlist), (yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 20:
#line 172 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(3) - (1)].value)+(yysemantic_stack_[(3) - (2)].value), (yysemantic_stack_[(3) - (3)].stmt) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 21:
#line 179 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 22:
#line 183 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(2) - (1)].value)+(yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 23:
#line 189 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            FunctionCallAST* node = new FunctionCallAST( (yysemantic_stack_[(4) - (1)].value), (yysemantic_stack_[(4) - (2)].value), (yysemantic_stack_[(4) - (3)].arglist), (yysemantic_stack_[(4) - (4)].value) );
            (yyval.funccall) = node;
        ;}
    break;

  case 24:
#line 194 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.funccall) = new FunctionCallAST( (yysemantic_stack_[(3) - (1)].value), (yysemantic_stack_[(3) - (2)].value), QList<FunctionArgAST*>(), (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 25:
#line 198 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.funccall) = new FunctionCallAST( (yysemantic_stack_[(5) - (1)].value)+(yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), (yysemantic_stack_[(5) - (4)].arglist), (yysemantic_stack_[(5) - (5)].value) );
        ;}
    break;

  case 26:
#line 202 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.funccall) = new FunctionCallAST( (yysemantic_stack_[(4) - (1)].value)+(yysemantic_stack_[(4) - (2)].value), (yysemantic_stack_[(4) - (3)].value), QList<FunctionArgAST*>(), (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 27:
#line 208 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.arglist).append( (yysemantic_stack_[(3) - (3)].fnarg) );
        ;}
    break;

  case 28:
#line 212 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.arglist).clear();
            (yyval.arglist).append( (yysemantic_stack_[(1) - (1)].fnarg) );
        ;}
    break;

  case 29:
#line 219 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.fnarg) = new SimpleFunctionArgAST( (yysemantic_stack_[(2) - (2)].value), (yysemantic_stack_[(2) - (1)].value) );
        ;}
    break;

  case 30:
#line 223 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.fnarg) = new FunctionCallAST( (yysemantic_stack_[(7) - (2)].value), (yysemantic_stack_[(7) - (3)].value)+(yysemantic_stack_[(7) - (4)].value), (yysemantic_stack_[(7) - (5)].arglist), (yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value), (yysemantic_stack_[(7) - (1)].value) );
        ;}
    break;

  case 31:
#line 229 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 32:
#line 233 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 33:
#line 237 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 34:
#line 241 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 35:
#line 245 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 36:
#line 249 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 37:
#line 253 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 38:
#line 257 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 39:
#line 261 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 40:
#line 265 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 41:
#line 269 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 42:
#line 273 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 43:
#line 277 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 44:
#line 281 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(4) - (2)].value)+(yysemantic_stack_[(4) - (3)].value)+(yysemantic_stack_[(4) - (4)].value);
        ;}
    break;

  case 45:
#line 285 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 46:
#line 289 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(3) - (1)].value)+(yysemantic_stack_[(3) - (2)].value)+(yysemantic_stack_[(3) - (3)].value);
        ;}
    break;

  case 47:
#line 293 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 48:
#line 297 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 49:
#line 301 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 50:
#line 305 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 51:
#line 309 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 52:
#line 313 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 53:
#line 317 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 54:
#line 321 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 55:
#line 325 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 56:
#line 329 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 57:
#line 333 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 58:
#line 339 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = new AssignmentAST( (yysemantic_stack_[(6) - (2)].value), (yysemantic_stack_[(6) - (3)].value), (yysemantic_stack_[(6) - (4)].values), (yysemantic_stack_[(6) - (5)].value), (yysemantic_stack_[(6) - (1)].value) );
        ;}
    break;

  case 59:
#line 343 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = new AssignmentAST( (yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), QStringList(), (yysemantic_stack_[(5) - (4)].value), (yysemantic_stack_[(5) - (1)].value) );
        ;}
    break;

  case 60:
#line 347 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = new AssignmentAST( (yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), (yysemantic_stack_[(5) - (4)].values), "", (yysemantic_stack_[(5) - (1)].value) );
        ;}
    break;

  case 61:
#line 351 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmt) = new AssignmentAST( (yysemantic_stack_[(4) - (2)].value), (yysemantic_stack_[(4) - (3)].value), QStringList(), "", (yysemantic_stack_[(4) - (1)].value) );
        ;}
    break;

  case 62:
#line 357 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 63:
#line 362 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 64:
#line 367 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 65:
#line 373 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {

            (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 66:
#line 380 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 67:
#line 385 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 68:
#line 390 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 69:
#line 397 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 70:
#line 401 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 71:
#line 405 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 72:
#line 409 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 73:
#line 413 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 74:
#line 419 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 75:
#line 423 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = "";
        ;}
    break;

  case 76:
#line 429 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(2) - (1)].value)+(yysemantic_stack_[(2) - (2)].value);
        ;}
    break;


    /* Line 675 of lalr1.cc.  */
#line 944 "/home/andreas/KDE-work/4.0/kdevelop/build/buildtools/importers/qmake/parser/qmake_parser.cpp"
	default: break;
      }
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	error (yylloc, yysyntax_error_ (yystate));
      }

    yyerror_range[0] = yylloc;
    if (yyerrstatus_ == 3)
      {
	/* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

	if (yychar <= yyeof_)
	  {
	  /* Return failure if at end of input.  */
	  if (yychar == yyeof_)
	    YYABORT;
	  }
	else
	  {
	    yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
	    yychar = yyempty_;
	  }
      }

    /* Else will try to reuse look-ahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[0] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (yyn != yypact_ninf_)
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	YYABORT;

	yyerror_range[0] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    if (yyn == yyfinal_)
      goto yyacceptlab;

    yyerror_range[1] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the look-ahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		   &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyeof_ && yychar != yyempty_)
      yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval, &yylloc);

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (yystate_stack_.height () != 1)
      {
	yydestruct_ ("Cleanup: popping",
		   yystos_[yystate_stack_[0]],
		   &yysemantic_stack_[0],
		   &yylocation_stack_[0]);
	yypop_ ();
      }

    return yyresult;
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (int yystate)
  {
    std::string res;
    YYUSE (yystate);
#if YYERROR_VERBOSE
    int yyn = yypact_[yystate];
    if (yypact_ninf_ < yyn && yyn <= yylast_)
      {
	/* Start YYX at -YYN if negative to avoid negative indexes in
	   YYCHECK.  */
	int yyxbegin = yyn < 0 ? -yyn : 0;

	/* Stay within bounds of both yycheck and yytname.  */
	int yychecklim = yylast_ - yyn + 1;
	int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
	int count = 0;
	for (int x = yyxbegin; x < yyxend; ++x)
	  if (yycheck_[x + yyn] == x && x != yyterror_)
	    ++count;

	// FIXME: This method of building the message is not compatible
	// with internationalization.  It should work like yacc.c does it.
	// That is, first build a string that looks like this:
	// "syntax error, unexpected %s or %s or %s"
	// Then, invoke YY_ on this string.
	// Finally, use the string as a format to output
	// yytname_[tok], etc.
	// Until this gets fixed, this message appears in English only.
	res = "syntax error, unexpected ";
	res += yytnamerr_ (yytname_[tok]);
	if (count < 5)
	  {
	    count = 0;
	    for (int x = yyxbegin; x < yyxend; ++x)
	      if (yycheck_[x + yyn] == x && x != yyterror_)
		{
		  res += (!count++) ? ", expecting " : " or ";
		  res += yytnamerr_ (yytname_[x]);
		}
	  }
      }
    else
#endif
      res = YY_("syntax error");
    return res;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char Parser::yypact_ninf_ = -64;
  const short int
  Parser::yypact_[] =
  {
       -64,    42,   -64,   -64,    40,   -64,    31,   -64,   -64,   -64,
     -64,    49,   -64,    33,   -64,   -64,   -64,     4,   163,    76,
      60,   -64,   -64,    -5,    44,    91,   -64,   -64,   -64,   -64,
     -64,   -14,    71,   -64,     8,    49,   -64,   -64,   -64,    64,
     -64,   -64,    83,   -64,   -64,    26,    69,   -64,    56,   -64,
     123,    74,    44,   -64,   -64,    45,   -64,    49,    88,   -64,
      62,    49,   -64,   -64,   -64,   146,   -64,   -64,   -64,   -64,
     -64,   -64,   -64,   -64,   -64,    49,   -64,   102,    65,    49,
      29,    44,   -64,   -64,    70,   -64,   -64,   -64,    11,   100,
     -64,   -64,   -64,   146,   -64,   -64,   -64,   -64,   -64,   -64,
     -64,   -64,   -64,   -64,   -64,   103,    30,   -64,   -64,   -64,
      49,    73,   -64,   104,    85,   -64,   -64,    49,   -64
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
         2,     0,     5,     1,    75,    74,     0,     7,     4,     9,
      11,    75,     8,     0,     6,    76,    12,     0,     0,     0,
       0,    21,    14,    15,    75,     0,    69,    70,    71,    73,
      72,     0,     0,    22,    75,    75,    10,    20,     5,     0,
      61,    68,     0,    67,    66,     0,    75,    24,     0,    28,
       0,     0,    75,     5,    59,     0,    60,    75,     0,    26,
       0,    75,    23,    45,    55,     0,    54,    56,    51,    52,
      50,    53,    57,    48,    49,    75,    47,    29,     0,    75,
       0,    75,    62,    63,     0,    58,    25,    27,     0,     0,
      34,    43,    40,     0,    39,    41,    36,    37,    35,    38,
      42,    32,    33,    31,    13,    18,     0,    64,    65,    46,
      75,     0,    17,    19,     0,    44,    16,    75,    30
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -64,   -64,   -64,   -35,    96,   -64,   -64,   -64,    46,   -64,
      79,   -45,    63,   -63,   -64,   -64,   -64,    -4,   -64
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,     1,     2,     4,     8,     9,    10,    11,    16,    22,
      23,    48,    49,    77,    12,    45,    31,    50,    14
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char Parser::yytable_ninf_ = -4;
  const signed char
  Parser::yytable_[] =
  {
        13,    60,    88,    52,    40,    41,    42,    17,    35,    43,
      24,     5,    25,    36,    90,    44,    91,    92,    81,    47,
      13,    93,   109,    94,    95,    96,    97,    98,    99,    55,
     111,    51,   100,    18,    18,   101,   102,    18,   105,   113,
      -3,   103,     3,     5,    56,    57,    58,     5,    80,    15,
      19,    19,     5,    84,    19,    20,    20,    21,    21,    20,
       6,    21,     7,    61,     6,   114,     7,    62,    82,    61,
      34,    89,     5,    86,    83,    17,    90,   106,    91,    92,
      59,    46,    53,    93,   115,    94,    95,    96,    97,    98,
      99,    32,    61,   107,   100,    78,   117,   101,   102,   108,
      20,    54,    32,   103,    33,    90,    85,    91,    92,    38,
     110,    39,    93,   118,    94,    95,    96,    97,    98,    99,
      37,   112,   116,   100,    87,   104,   101,   102,    63,    64,
      79,     0,   103,    65,     0,    66,    67,    68,    69,    70,
      71,     0,     0,     0,    72,     0,     0,    73,    74,     0,
      75,    63,    64,    76,     0,     0,    65,     0,    66,    67,
      68,    69,    70,    71,     0,     0,     0,    72,     0,     0,
      73,    74,     0,     0,     0,    26,    76,    27,    28,    29,
      30
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
         4,    46,    65,    38,    18,    19,    20,    11,    13,    23,
       6,     3,     8,    18,     3,    29,     5,     6,    53,    11,
      24,    10,    11,    12,    13,    14,    15,    16,    17,     3,
      93,    35,    21,     4,     4,    24,    25,     4,     9,     9,
       0,    30,     0,     3,    18,    19,    20,     3,    52,    18,
      21,    21,     3,    57,    21,    26,    26,    28,    28,    26,
      20,    28,    22,     7,    20,   110,    22,    11,    23,     7,
      10,    75,     3,    11,    29,    79,     3,    81,     5,     6,
      11,    10,    18,    10,    11,    12,    13,    14,    15,    16,
      17,    26,     7,    23,    21,    21,    11,    24,    25,    29,
      26,    18,    26,    30,    28,     3,    18,     5,     6,    18,
      10,    20,    10,   117,    12,    13,    14,    15,    16,    17,
      24,    18,    18,    21,    61,    79,    24,    25,     5,     6,
      51,    -1,    30,    10,    -1,    12,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    21,    -1,    -1,    24,    25,    -1,
      27,     5,     6,    30,    -1,    -1,    10,    -1,    12,    13,
      14,    15,    16,    17,    -1,    -1,    -1,    21,    -1,    -1,
      24,    25,    -1,    -1,    -1,    12,    30,    14,    15,    16,
      17
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    32,    33,     0,    34,     3,    20,    22,    35,    36,
      37,    38,    45,    48,    49,    18,    39,    48,     4,    21,
      26,    28,    40,    41,     6,     8,    12,    14,    15,    16,
      17,    47,    26,    28,    10,    13,    18,    35,    18,    20,
      18,    19,    20,    23,    29,    46,    10,    11,    42,    43,
      48,    48,    34,    18,    18,     3,    18,    19,    20,    11,
      42,     7,    11,     5,     6,    10,    12,    13,    14,    15,
      16,    17,    21,    24,    25,    27,    30,    44,    21,    41,
      48,    34,    23,    29,    48,    18,    11,    43,    44,    48,
       3,     5,     6,    10,    12,    13,    14,    15,    16,    17,
      21,    24,    25,    30,    39,     9,    48,    23,    29,    11,
      10,    44,    18,     9,    42,    11,    18,    11,    48
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    31,    33,    32,    34,    34,    35,    35,    35,    35,
      35,    35,    36,    37,    38,    38,    39,    39,    39,    39,
      39,    40,    40,    41,    41,    41,    41,    42,    42,    43,
      43,    44,    44,    44,    44,    44,    44,    44,    44,    44,
      44,    44,    44,    44,    44,    44,    44,    44,    44,    44,
      44,    44,    44,    44,    44,    44,    44,    44,    45,    45,
      45,    45,    46,    46,    46,    46,    46,    46,    46,    47,
      47,    47,    47,    47,    48,    48,    49
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       3,     1,     2,     6,     2,     2,     8,     7,     6,     7,
       3,     1,     2,     4,     3,     5,     4,     3,     1,     2,
       7,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     4,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     6,     5,
       5,     4,     3,     3,     4,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     2
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const Parser::yytname_[] =
  {
    "$end", "error", "$undefined", "WS", "VARIABLE", "DOLLAR", "COLON",
  "COMMA", "LCURLY", "RCURLY", "LPAREN", "RPAREN", "EQUAL", "OR", "PLUSEQ",
  "MINUSEQ", "TILDEEQ", "STAREQ", "NEWLINE", "CONT", "COMMENT", "EXCLAM",
  "EMPTYLINE", "VAR_VALUE", "QMVARIABLE", "SHELLVARIABLE", "FUNCTIONNAME",
  "FUNCTIONCALL", "SCOPENAME", "QUOTED_VAR_VALUE", "FNVALUE", "$accept",
  "project", "@1", "statements", "statement", "scope", "or_op",
  "scope_head", "scope_body", "scope_name", "functioncall", "functionargs",
  "functionarg", "fnvalue", "variable_assignment", "values", "op", "ws",
  "comment", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
        32,     0,    -1,    -1,    33,    34,    -1,    34,    35,    -1,
      -1,    49,    -1,    22,    -1,    45,    -1,    36,    -1,    48,
      41,    18,    -1,    37,    -1,    38,    39,    -1,    48,    41,
      13,    48,    41,    39,    -1,    48,    40,    -1,    48,    41,
      -1,    48,     8,    20,    18,    34,    48,     9,    18,    -1,
      48,     8,    18,    34,    48,     9,    18,    -1,    48,     8,
      18,    34,    48,     9,    -1,    48,     8,    20,    18,    34,
      48,     9,    -1,    48,     6,    35,    -1,    28,    -1,    21,
      28,    -1,    26,    10,    42,    11,    -1,    26,    10,    11,
      -1,    21,    26,    10,    42,    11,    -1,    21,    26,    10,
      11,    -1,    42,     7,    43,    -1,    43,    -1,    48,    44,
      -1,    48,    27,    48,    10,    42,    11,    48,    -1,    44,
      30,    -1,    44,    24,    -1,    44,    25,    -1,    44,     3,
      -1,    44,    16,    -1,    44,    14,    -1,    44,    15,    -1,
      44,    17,    -1,    44,    12,    -1,    44,     6,    -1,    44,
      13,    -1,    44,    21,    -1,    44,     5,    -1,    44,    10,
      44,    11,    -1,     5,    -1,    10,    44,    11,    -1,    30,
      -1,    24,    -1,    25,    -1,    16,    -1,    14,    -1,    15,
      -1,    17,    -1,    12,    -1,     6,    -1,    13,    -1,    21,
      -1,    48,     4,    47,    46,    20,    18,    -1,    48,     4,
      47,    20,    18,    -1,    48,     4,    47,    46,    18,    -1,
      48,     4,    47,    18,    -1,    46,     3,    23,    -1,    46,
       3,    29,    -1,    46,    19,    48,    23,    -1,    46,    19,
      48,    29,    -1,    29,    -1,    23,    -1,    19,    -1,    12,
      -1,    14,    -1,    15,    -1,    17,    -1,    16,    -1,     3,
      -1,    -1,    20,    18,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  Parser::yyprhs_[] =
  {
         0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    23,    25,    28,    35,    38,    41,    50,    58,    65,
      73,    77,    79,    82,    87,    91,    97,   102,   106,   108,
     111,   119,   122,   125,   128,   131,   134,   137,   140,   143,
     146,   149,   152,   155,   158,   163,   165,   169,   171,   173,
     175,   177,   179,   181,   183,   185,   187,   189,   191,   198,
     204,   210,   215,   219,   223,   228,   233,   235,   237,   239,
     241,   243,   245,   247,   249,   251,   252
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,    74,    74,    74,    86,    91,    96,   100,   104,   108,
     112,   116,   122,   130,   139,   144,   151,   156,   161,   166,
     171,   178,   182,   188,   193,   197,   201,   207,   211,   218,
     222,   228,   232,   236,   240,   244,   248,   252,   256,   260,
     264,   268,   272,   276,   280,   284,   288,   292,   296,   300,
     304,   308,   312,   316,   320,   324,   328,   332,   338,   342,
     346,   350,   356,   361,   366,   372,   379,   384,   389,   396,
     400,   404,   408,   412,   418,   423,   428
  };

  // Print the state stack on the debug stream.
  void
  Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "), ";
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  Parser::token_number_type
  Parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int Parser::yyeof_ = 0;
  const int Parser::yylast_ = 180;
  const int Parser::yynnts_ = 19;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 3;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 31;

  const unsigned int Parser::yyuser_token_number_max_ = 285;
  const Parser::token_number_type Parser::yyundef_token_ = 2;

} // namespace QMake

#line 434 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"


namespace QMake
{
    void Parser::error(const location_type& /*l*/, const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}
