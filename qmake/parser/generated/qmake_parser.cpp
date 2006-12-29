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
#line 70 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            *project = new ProjectAST();
        ;}
    break;

  case 3:
#line 74 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            foreach( StatementAST* s, (yysemantic_stack_[(2) - (2)].stmtlist))
            {
                (*project)->addStatement( s );
            }
        ;}
    break;

  case 4:
#line 83 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmtlist).append( static_cast<StatementAST*>( (yysemantic_stack_[(2) - (2)].node) ) );
        ;}
    break;

  case 5:
#line 87 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmtlist).clear();
        ;}
    break;

  case 6:
#line 93 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new CommentAST( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 7:
#line 97 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new NewlineAST( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 8:
#line 101 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 9:
#line 105 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 10:
#line 109 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new ScopeAST( static_cast<FunctionCallAST*>( (yysemantic_stack_[(3) - (2)].node) ), (yysemantic_stack_[(3) - (1)].value) );
        ;}
    break;

  case 11:
#line 113 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 12:
#line 119 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeAST* node = static_cast<ScopeAST*>( (yysemantic_stack_[(2) - (1)].node) );
            node->setScopeBody( static_cast<ScopeBodyAST*>( (yysemantic_stack_[(2) - (2)].node) ) );
            (yyval.node) = node;
        ;}
    break;

  case 13:
#line 127 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            OrAST* node = new OrAST( static_cast<FunctionCallAST*>( (yysemantic_stack_[(4) - (1)].node) ), (yysemantic_stack_[(4) - (2)].value),
                                     static_cast<FunctionCallAST*>( (yysemantic_stack_[(4) - (3)].node) ),
                                     static_cast<ScopeBodyAST*>( (yysemantic_stack_[(4) - (4)].node) ) );
            (yyval.node) = node;
        ;}
    break;

  case 14:
#line 136 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeAST* node = new ScopeAST( (yysemantic_stack_[(2) - (2)].value), (yysemantic_stack_[(2) - (1)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 15:
#line 141 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeAST* node = new ScopeAST( static_cast<FunctionCallAST*>((yysemantic_stack_[(2) - (2)].node)), (yysemantic_stack_[(2) - (1)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 16:
#line 148 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(8) - (1)].value)+(yysemantic_stack_[(8) - (2)].value)+(yysemantic_stack_[(8) - (3)].value)+(yysemantic_stack_[(8) - (4)].value), (yysemantic_stack_[(8) - (5)].stmtlist), (yysemantic_stack_[(8) - (6)].value)+(yysemantic_stack_[(8) - (7)].value)+(yysemantic_stack_[(8) - (8)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 17:
#line 153 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(7) - (1)].value)+(yysemantic_stack_[(7) - (2)].value)+(yysemantic_stack_[(7) - (3)].value), (yysemantic_stack_[(7) - (4)].stmtlist), (yysemantic_stack_[(7) - (5)].value)+(yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 18:
#line 158 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(6) - (1)].value)+(yysemantic_stack_[(6) - (2)].value)+(yysemantic_stack_[(6) - (3)].value), (yysemantic_stack_[(6) - (4)].stmtlist), (yysemantic_stack_[(6) - (5)].value)+(yysemantic_stack_[(6) - (6)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 19:
#line 163 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(7) - (1)].value)+(yysemantic_stack_[(7) - (2)].value)+(yysemantic_stack_[(7) - (3)].value)+(yysemantic_stack_[(7) - (4)].value), (yysemantic_stack_[(7) - (5)].stmtlist), (yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 20:
#line 168 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(3) - (1)].value)+(yysemantic_stack_[(3) - (2)].value), static_cast<StatementAST*>( (yysemantic_stack_[(3) - (3)].node) ) );
            (yyval.node) = node;
        ;}
    break;

  case 21:
#line 175 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 22:
#line 179 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(2) - (1)].value)+(yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 23:
#line 185 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            FunctionCallAST* node = new FunctionCallAST( (yysemantic_stack_[(4) - (1)].value), (yysemantic_stack_[(4) - (2)].value), (yysemantic_stack_[(4) - (3)].arglist), QString((yysemantic_stack_[(4) - (4)].value)) );
            (yyval.node) = node;
        ;}
    break;

  case 24:
#line 190 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(4) - (1)].value), (yysemantic_stack_[(4) - (2)].value), QList<FunctionArgAST*>(), QString((yysemantic_stack_[(4) - (4)].value)) );
        ;}
    break;

  case 25:
#line 194 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(5) - (1)].value)+(yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), (yysemantic_stack_[(5) - (4)].arglist), (yysemantic_stack_[(5) - (5)].value) );
        ;}
    break;

  case 26:
#line 198 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(5) - (1)].value)+(yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), QList<FunctionArgAST*>(), (yysemantic_stack_[(5) - (5)].value) );
        ;}
    break;

  case 27:
#line 204 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.arglist).append( static_cast<FunctionArgAST*>( (yysemantic_stack_[(3) - (3)].node) ) );
        ;}
    break;

  case 28:
#line 208 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.arglist).clear();
            (yyval.arglist).append( static_cast<FunctionArgAST*>( (yysemantic_stack_[(1) - (1)].node) ) );
        ;}
    break;

  case 29:
#line 215 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new SimpleFunctionArgAST( (yysemantic_stack_[(2) - (2)].value), (yysemantic_stack_[(2) - (1)].value) );
        ;}
    break;

  case 30:
#line 219 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(7) - (2)].value), (yysemantic_stack_[(7) - (3)].value)+(yysemantic_stack_[(7) - (4)].value), (yysemantic_stack_[(7) - (5)].arglist), (yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value), (yysemantic_stack_[(7) - (1)].value) );
        ;}
    break;

  case 31:
#line 225 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 32:
#line 229 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 33:
#line 233 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 34:
#line 237 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 35:
#line 241 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 36:
#line 245 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 37:
#line 249 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 38:
#line 253 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 39:
#line 257 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 40:
#line 261 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 41:
#line 265 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 42:
#line 269 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 43:
#line 273 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 44:
#line 277 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) += (yysemantic_stack_[(4) - (2)].value)+(yysemantic_stack_[(4) - (3)].value)+(yysemantic_stack_[(4) - (4)].value);
        ;}
    break;

  case 45:
#line 281 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 46:
#line 285 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(3) - (1)].value)+(yysemantic_stack_[(3) - (2)].value)+(yysemantic_stack_[(3) - (3)].value);
        ;}
    break;

  case 47:
#line 289 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 48:
#line 293 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 49:
#line 297 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 50:
#line 301 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 51:
#line 305 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 52:
#line 309 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 53:
#line 313 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 54:
#line 317 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 55:
#line 321 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 56:
#line 325 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 57:
#line 329 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 58:
#line 335 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new AssignmentAST( (yysemantic_stack_[(6) - (2)].value), (yysemantic_stack_[(6) - (3)].value), (yysemantic_stack_[(6) - (4)].values), (yysemantic_stack_[(6) - (5)].value), (yysemantic_stack_[(6) - (1)].value) );
        ;}
    break;

  case 59:
#line 339 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new AssignmentAST( (yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), QStringList(), (yysemantic_stack_[(5) - (4)].value), (yysemantic_stack_[(5) - (1)].value) );
        ;}
    break;

  case 60:
#line 343 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new AssignmentAST( (yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), (yysemantic_stack_[(5) - (4)].values), "", (yysemantic_stack_[(5) - (1)].value) );
        ;}
    break;

  case 61:
#line 347 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new AssignmentAST( (yysemantic_stack_[(4) - (2)].value), (yysemantic_stack_[(4) - (3)].value), QStringList(), "", (yysemantic_stack_[(4) - (1)].value) );
        ;}
    break;

  case 62:
#line 353 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 63:
#line 358 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 64:
#line 363 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 65:
#line 369 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {

            (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 66:
#line 376 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 67:
#line 381 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 68:
#line 386 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 69:
#line 393 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 70:
#line 397 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 71:
#line 401 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 72:
#line 405 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 73:
#line 409 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 74:
#line 415 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 75:
#line 419 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.value) = "";
        ;}
    break;

  case 76:
#line 425 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
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
  const signed char Parser::yypact_ninf_ = -66;
  const short int
  Parser::yypact_[] =
  {
       -66,    11,   -66,   -66,    22,   -66,    19,   -66,   -66,   -66,
     -66,    27,   -66,    72,   -66,   -66,    61,   -66,     4,   185,
      10,    65,   -66,   -66,    69,    61,    68,     1,   124,   -66,
     -66,   -66,   -66,   -66,   186,    75,   -66,    61,   -66,   -66,
     -66,   -66,   -66,    77,   -66,   -66,    81,   -66,   -66,   176,
      61,    -4,   -66,    99,     1,   -66,   -66,    12,   -66,    61,
      84,    55,   122,    61,   -66,   -66,   -66,   168,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,    61,   -66,
       3,    25,     1,   -66,   -66,    96,   -66,   -66,   -66,   -66,
     145,    44,    93,   -66,   -66,   -66,   168,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,    88,   182,   -66,
     -66,   -66,    61,    67,   -66,    89,    79,   -66,   -66,    61,
     -66
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
         2,     0,     5,     1,    75,    74,     0,     7,     4,     9,
      11,    75,     8,     0,     6,    76,    75,    12,     0,     0,
       0,     0,    21,    14,    15,    75,     0,    75,     0,    69,
      70,    71,    73,    72,     0,     0,    22,    75,    10,    13,
      15,    20,     5,     0,    61,    68,     0,    67,    66,     0,
      75,     0,    28,     0,    75,     5,    59,     0,    60,    75,
       0,     0,     0,    75,    23,    45,    55,     0,    24,    54,
      56,    51,    52,    50,    53,    57,    48,    49,    75,    47,
      29,     0,    75,    62,    63,     0,    58,    25,    26,    27,
       0,     0,     0,    34,    43,    40,     0,    39,    41,    36,
      37,    35,    38,    42,    32,    33,    31,    18,     0,    64,
      65,    46,    75,     0,    17,    19,     0,    44,    16,    75,
      30
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -66,   -66,   -66,   -10,    90,   -66,   -66,   102,    97,   -66,
      95,   -49,    78,   -65,   -66,   -66,   -66,   -11,   -66
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,     1,     2,     4,     8,     9,    10,    11,    17,    23,
      24,    51,    52,    80,    12,    49,    34,    13,    14
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char Parser::yytable_ninf_ = -4;
  const signed char
  Parser::yytable_[] =
  {
        18,    61,    91,    63,     5,    26,    93,    64,    94,    95,
      27,     3,    28,    96,    18,    97,    98,    99,   100,   101,
     102,     6,    -3,     7,   103,     5,    53,   104,   105,    19,
       5,   113,    54,   106,   107,    83,    35,    15,    36,    62,
      16,    84,     6,    81,     7,    82,    20,    93,    85,    94,
      95,    21,    90,    22,    96,   111,    97,    98,    99,   100,
     101,   102,    63,   116,     5,   103,    87,    92,   104,   105,
      93,   108,    94,    95,   106,    37,    19,    96,   117,    97,
      98,    99,   100,   101,   102,    50,    63,    38,   103,    20,
     119,   104,   105,    20,    21,    55,    22,   106,    21,    56,
      22,    90,    86,   112,    65,    66,   114,   118,   120,    67,
      68,    69,    70,    71,    72,    73,    74,    41,    25,   109,
      75,    40,    39,    76,    77,   110,    78,    65,    66,    79,
       0,     0,    67,    88,    69,    70,    71,    72,    73,    74,
       0,    89,    42,    75,    43,     0,    76,    77,     0,    78,
      65,    66,    79,     0,     0,    67,     0,    69,    70,    71,
      72,    73,    74,     0,     0,     0,    75,     0,     0,    76,
      77,     0,    78,    65,    66,    79,     0,     0,    67,    57,
      69,    70,    71,    72,    73,    74,    19,     0,     0,    75,
       0,   115,    76,    77,    58,    59,    60,    29,    79,    30,
      31,    32,    33,    20,    44,    45,    46,     0,    21,    47,
      22,     0,     0,     0,     0,    48
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
        11,    50,    67,     7,     3,    16,     3,    11,     5,     6,
       6,     0,     8,    10,    25,    12,    13,    14,    15,    16,
      17,    20,     0,    22,    21,     3,    37,    24,    25,     4,
       3,    96,    42,    30,     9,    23,    26,    18,    28,    50,
      13,    29,    20,    54,    22,    55,    21,     3,    59,     5,
       6,    26,    63,    28,    10,    11,    12,    13,    14,    15,
      16,    17,     7,   112,     3,    21,    11,    78,    24,    25,
       3,    82,     5,     6,    30,    10,     4,    10,    11,    12,
      13,    14,    15,    16,    17,    10,     7,    18,    21,    21,
      11,    24,    25,    21,    26,    18,    28,    30,    26,    18,
      28,   112,    18,    10,     5,     6,    18,    18,   119,    10,
      11,    12,    13,    14,    15,    16,    17,    27,    16,    23,
      21,    26,    25,    24,    25,    29,    27,     5,     6,    30,
      -1,    -1,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    63,    18,    21,    20,    -1,    24,    25,    -1,    27,
       5,     6,    30,    -1,    -1,    10,    -1,    12,    13,    14,
      15,    16,    17,    -1,    -1,    -1,    21,    -1,    -1,    24,
      25,    -1,    27,     5,     6,    30,    -1,    -1,    10,     3,
      12,    13,    14,    15,    16,    17,     4,    -1,    -1,    21,
      -1,     9,    24,    25,    18,    19,    20,    12,    30,    14,
      15,    16,    17,    21,    18,    19,    20,    -1,    26,    23,
      28,    -1,    -1,    -1,    -1,    29
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    32,    33,     0,    34,     3,    20,    22,    35,    36,
      37,    38,    45,    48,    49,    18,    13,    39,    48,     4,
      21,    26,    28,    40,    41,    38,    48,     6,     8,    12,
      14,    15,    16,    17,    47,    26,    28,    10,    18,    39,
      41,    35,    18,    20,    18,    19,    20,    23,    29,    46,
      10,    42,    43,    48,    34,    18,    18,     3,    18,    19,
      20,    42,    48,     7,    11,     5,     6,    10,    11,    12,
      13,    14,    15,    16,    17,    21,    24,    25,    27,    30,
      44,    48,    34,    23,    29,    48,    18,    11,    11,    43,
      48,    44,    48,     3,     5,     6,    10,    12,    13,    14,
      15,    16,    17,    21,    24,    25,    30,     9,    48,    23,
      29,    11,    10,    44,    18,     9,    42,    11,    18,    11,
      48
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
       3,     1,     2,     4,     2,     2,     8,     7,     6,     7,
       3,     1,     2,     4,     4,     5,     5,     3,     1,     2,
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
      41,    18,    -1,    37,    -1,    38,    39,    -1,    38,    13,
      38,    39,    -1,    48,    40,    -1,    48,    41,    -1,    48,
       8,    20,    18,    34,    48,     9,    18,    -1,    48,     8,
      18,    34,    48,     9,    18,    -1,    48,     8,    18,    34,
      48,     9,    -1,    48,     8,    20,    18,    34,    48,     9,
      -1,    48,     6,    35,    -1,    28,    -1,    21,    28,    -1,
      26,    10,    42,    11,    -1,    26,    10,    48,    11,    -1,
      21,    26,    10,    42,    11,    -1,    21,    26,    10,    48,
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
      19,    23,    25,    28,    33,    36,    39,    48,    56,    63,
      71,    75,    77,    80,    85,    90,    96,   102,   106,   108,
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
         0,    70,    70,    70,    82,    87,    92,    96,   100,   104,
     108,   112,   118,   126,   135,   140,   147,   152,   157,   162,
     167,   174,   178,   184,   189,   193,   197,   203,   207,   214,
     218,   224,   228,   232,   236,   240,   244,   248,   252,   256,
     260,   264,   268,   272,   276,   280,   284,   288,   292,   296,
     300,   304,   308,   312,   316,   320,   324,   328,   334,   338,
     342,   346,   352,   357,   362,   368,   375,   380,   385,   392,
     396,   400,   404,   408,   414,   419,   424
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
  const int Parser::yylast_ = 215;
  const int Parser::yynnts_ = 19;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 3;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 31;

  const unsigned int Parser::yyuser_token_number_max_ = 285;
  const Parser::token_number_type Parser::yyundef_token_ = 2;

} // namespace QMake

#line 430 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"


namespace QMake
{
    void Parser::error(const location_type& /*l*/, const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}
