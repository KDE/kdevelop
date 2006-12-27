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
#line 69 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
        *project = new ProjectAST();
    ;}
    break;

  case 3:
#line 73 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
        foreach( StatementAST* s, (yysemantic_stack_[(2) - (1)].stmtlist))
        {
            (*project)->addStatement( s );
        }
    ;}
    break;

  case 4:
#line 82 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmtlist).append( static_cast<StatementAST*>( (yysemantic_stack_[(2) - (2)].node) ) );
        ;}
    break;

  case 5:
#line 86 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.stmtlist).clear();
        ;}
    break;

  case 6:
#line 92 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
            (yyval.node) = new CommentAST( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 7:
#line 96 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new NewlineAST( (yysemantic_stack_[(1) - (1)].value) );
;}
    break;

  case 8:
#line 100 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
;}
    break;

  case 9:
#line 104 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
;}
    break;

  case 10:
#line 108 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new ScopeAST( static_cast<FunctionCallAST*>( (yysemantic_stack_[(3) - (2)].node) ), (yysemantic_stack_[(3) - (1)].value) );
;}
    break;

  case 11:
#line 112 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
;}
    break;

  case 12:
#line 118 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    ScopeAST* node = static_cast<ScopeAST*>( (yysemantic_stack_[(2) - (1)].node) );
    node->setScopeBody( static_cast<ScopeBodyAST*>( (yysemantic_stack_[(2) - (2)].node) ) );
    (yyval.node) = node;
;}
    break;

  case 13:
#line 124 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    ScopeAST* node = new ScopeAST("else", (yysemantic_stack_[(3) - (1)].value) );
    node->setScopeBody( static_cast<ScopeBodyAST*>( (yysemantic_stack_[(3) - (2)].node) ) );
    (yyval.node) = node;
;}
    break;

  case 14:
#line 132 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    OrAST* node = new OrAST( static_cast<FunctionCallAST*>( (yysemantic_stack_[(4) - (1)].node) ), (yysemantic_stack_[(4) - (2)].value),
                             static_cast<FunctionCallAST*>( (yysemantic_stack_[(4) - (3)].node) ),
                             static_cast<ScopeBodyAST*>( (yysemantic_stack_[(4) - (4)].node) ) );
    (yyval.node) = node;
;}
    break;

  case 15:
#line 141 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new ScopeAST( (yysemantic_stack_[(2) - (2)].value), (yysemantic_stack_[(2) - (1)].value) );
;}
    break;

  case 16:
#line 145 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    AST* node = (yysemantic_stack_[(2) - (2)].node);
    node->setWhitespace( (yysemantic_stack_[(2) - (1)].value) );
    (yyval.node) = node;
;}
    break;

  case 17:
#line 153 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(8) - (1)].value)+(yysemantic_stack_[(8) - (2)].value)+(yysemantic_stack_[(8) - (3)].value)+(yysemantic_stack_[(8) - (4)].value), (yysemantic_stack_[(8) - (5)].stmtlist), (yysemantic_stack_[(8) - (6)].value)+(yysemantic_stack_[(8) - (7)].value)+(yysemantic_stack_[(8) - (8)].value) );
    (yyval.node) = node;
;}
    break;

  case 18:
#line 158 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(7) - (1)].value)+(yysemantic_stack_[(7) - (2)].value)+(yysemantic_stack_[(7) - (3)].value), (yysemantic_stack_[(7) - (4)].stmtlist), (yysemantic_stack_[(7) - (5)].value)+(yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value) );
    (yyval.node) = node;
;}
    break;

  case 19:
#line 163 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(6) - (1)].value)+(yysemantic_stack_[(6) - (2)].value)+(yysemantic_stack_[(6) - (3)].value), (yysemantic_stack_[(6) - (4)].stmtlist), (yysemantic_stack_[(6) - (5)].value)+(yysemantic_stack_[(6) - (6)].value) );
    (yyval.node) = node;
;}
    break;

  case 20:
#line 168 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(7) - (1)].value)+(yysemantic_stack_[(7) - (2)].value)+(yysemantic_stack_[(7) - (3)].value)+(yysemantic_stack_[(7) - (4)].value), (yysemantic_stack_[(7) - (5)].stmtlist), (yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value) );
    (yyval.node) = node;
;}
    break;

  case 21:
#line 173 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    ScopeBodyAST* node = new ScopeBodyAST( (yysemantic_stack_[(3) - (1)].value)+(yysemantic_stack_[(3) - (2)].value), static_cast<StatementAST*>( (yysemantic_stack_[(3) - (3)].node) ) );
    (yyval.node) = node;
;}
    break;

  case 22:
#line 180 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
;}
    break;

  case 23:
#line 184 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(2) - (1)].value)+(yysemantic_stack_[(2) - (2)].value);
;}
    break;

  case 24:
#line 190 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(4) - (1)].value), (yysemantic_stack_[(4) - (2)].value), (yysemantic_stack_[(4) - (3)].arglist), (yysemantic_stack_[(4) - (4)].value) );
;}
    break;

  case 25:
#line 194 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(4) - (1)].value), (yysemantic_stack_[(4) - (2)].value), QList<FunctionArgAST*>(), (yysemantic_stack_[(4) - (4)].value) );
;}
    break;

  case 26:
#line 198 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(5) - (1)].value)+(yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), (yysemantic_stack_[(5) - (4)].arglist), (yysemantic_stack_[(5) - (5)].value) );
;}
    break;

  case 27:
#line 202 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(5) - (1)].value)+(yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), QList<FunctionArgAST*>(), (yysemantic_stack_[(5) - (5)].value) );
;}
    break;

  case 28:
#line 208 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.arglist).append( static_cast<FunctionArgAST*>( (yysemantic_stack_[(3) - (3)].node) ) );
;}
    break;

  case 29:
#line 212 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.arglist).clear();
    (yyval.arglist).append( static_cast<FunctionArgAST*>( (yysemantic_stack_[(1) - (1)].node) ) );
;}
    break;

  case 30:
#line 219 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new SimpleFunctionArgAST( (yysemantic_stack_[(2) - (1)].value) );
;}
    break;

  case 31:
#line 223 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new FunctionCallAST( (yysemantic_stack_[(7) - (2)].value), (yysemantic_stack_[(7) - (3)].value)+(yysemantic_stack_[(7) - (4)].value), (yysemantic_stack_[(7) - (5)].arglist), (yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value), (yysemantic_stack_[(7) - (1)].value) );
;}
    break;

  case 59:
#line 258 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new AssignmentAST( (yysemantic_stack_[(6) - (2)].value), (yysemantic_stack_[(6) - (3)].value), (yysemantic_stack_[(6) - (4)].values), (yysemantic_stack_[(6) - (5)].value), (yysemantic_stack_[(6) - (1)].value) );
;}
    break;

  case 60:
#line 262 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new AssignmentAST( (yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), QStringList(), (yysemantic_stack_[(5) - (4)].value), (yysemantic_stack_[(5) - (1)].value) );
;}
    break;

  case 61:
#line 266 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new AssignmentAST( (yysemantic_stack_[(5) - (2)].value), (yysemantic_stack_[(5) - (3)].value), (yysemantic_stack_[(5) - (4)].values), "", (yysemantic_stack_[(5) - (1)].value) );
;}
    break;

  case 62:
#line 270 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.node) = new AssignmentAST( (yysemantic_stack_[(4) - (2)].value), (yysemantic_stack_[(4) - (3)].value), QStringList(), "", (yysemantic_stack_[(4) - (1)].value) );
;}
    break;

  case 63:
#line 276 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
    (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
;}
    break;

  case 64:
#line 281 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
    (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
;}
    break;

  case 65:
#line 286 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
    (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
    (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
;}
    break;

  case 66:
#line 292 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {

    (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
    (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
    (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
;}
    break;

  case 67:
#line 299 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.values) = QStringList();
    (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
;}
    break;

  case 68:
#line 304 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.values) = QStringList();
    (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
;}
    break;

  case 69:
#line 309 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.values) = QStringList();
    (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
;}
    break;

  case 70:
#line 316 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
;}
    break;

  case 71:
#line 320 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
;}
    break;

  case 72:
#line 324 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
;}
    break;

  case 73:
#line 328 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
;}
    break;

  case 74:
#line 332 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
;}
    break;

  case 75:
#line 338 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
;}
    break;

  case 76:
#line 342 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = "";
;}
    break;

  case 77:
#line 348 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"
    {
    (yyval.value) = (yysemantic_stack_[(2) - (1)].value)+(yysemantic_stack_[(2) - (2)].value);
;}
    break;


    /* Line 675 of lalr1.cc.  */
#line 763 "/home/andreas/KDE-work/4.0/kdevelop/build/buildtools/importers/qmake/parser/qmake_parser.cpp"
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
  const signed char Parser::yypact_ninf_ = -68;
  const short int
  Parser::yypact_[] =
  {
       -68,     5,   -68,   -68,    46,   -68,    22,   -68,   -68,   -68,
     -68,    87,   -68,    33,   -68,   -68,    48,   -68,    -2,   211,
      38,    43,    48,   -68,   -68,    55,    48,   -18,    10,    74,
     -68,   -68,   -68,   -68,   -68,    51,    69,   -68,    48,   -68,
     -68,   -68,   -68,   -68,   -68,    83,   -68,   -68,    89,   -68,
     -68,    23,    48,    88,   -68,   121,    10,   -68,   -68,    42,
     -68,    48,    90,   111,   145,    48,   -68,   -68,   -68,   193,
     -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,
      48,   -68,    99,    18,    10,   -68,   -68,    68,   -68,   -68,
     -68,   -68,   169,     4,    96,   -68,   -68,   -68,   193,   -68,
     -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   -68,   101,
      29,   -68,   -68,   -68,    48,    72,   -68,   110,   114,   -68,
     -68,    48,   -68
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
         2,     0,     5,     1,    76,    75,     0,     7,     4,     9,
      11,    76,     8,     0,     6,    77,    76,    12,     0,     0,
       0,     0,    76,    22,    15,    16,    76,     0,    76,     0,
      70,    71,    72,    74,    73,     0,     0,    23,    76,    13,
      10,    14,    16,    21,     5,     0,    62,    69,     0,    68,
      67,     0,    76,     0,    29,     0,    76,     5,    60,     0,
      61,    76,     0,     0,     0,    76,    24,    46,    56,     0,
      25,    55,    57,    52,    53,    51,    54,    58,    49,    50,
      76,    48,    30,     0,    76,    63,    64,     0,    59,    26,
      27,    28,     0,     0,     0,    35,    44,    41,     0,    40,
      42,    37,    38,    36,    39,    43,    33,    34,    32,    19,
       0,    65,    66,    47,    76,     0,    18,    20,     0,    45,
      17,    76,    31
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -68,   -68,   -68,   -21,   112,   -68,   -68,   113,   117,   -68,
     120,   -51,    76,   -67,   -68,   -68,   -68,    -4,   -68
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,     1,     2,     4,     8,     9,    10,    11,    17,    24,
      25,    53,    54,    82,    12,    51,    35,    18,    14
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char Parser::yytable_ninf_ = -4;
  const signed char
  Parser::yytable_[] =
  {
        13,    63,    93,    20,    28,     3,    29,    95,    21,    96,
      97,    23,    27,     5,    98,   113,    99,   100,   101,   102,
     103,   104,    19,    56,    13,   105,    59,   109,   106,   107,
       6,   115,     7,    19,    55,   108,    84,    19,   117,    20,
      15,    60,    61,    62,    21,    22,    -3,    23,    64,     5,
      20,     5,    83,    38,    20,    21,    22,    87,    23,    21,
      22,    92,    23,   118,    36,    85,     6,    37,     7,    46,
      47,    48,    86,    40,    49,    95,    94,    96,    97,    52,
     110,    50,    98,   119,    99,   100,   101,   102,   103,   104,
       5,   111,    44,   105,    45,    65,   106,   107,   112,    66,
      16,    57,    95,   108,    96,    97,   114,    58,    88,    98,
      92,    99,   100,   101,   102,   103,   104,   122,    65,   116,
     105,    65,    89,   106,   107,   121,    67,    68,   120,    26,
     108,    69,    70,    71,    72,    73,    74,    75,    76,    39,
      43,    91,    77,    41,     0,    78,    79,    42,     0,    80,
      67,    68,    81,     0,     0,    69,    90,    71,    72,    73,
      74,    75,    76,     0,     0,     0,    77,     0,     0,    78,
      79,     0,     0,    80,    67,    68,    81,     0,     0,    69,
       0,    71,    72,    73,    74,    75,    76,     0,     0,     0,
      77,     0,     0,    78,    79,     0,     0,    80,    67,    68,
      81,     0,     0,    69,     0,    71,    72,    73,    74,    75,
      76,     0,     0,     0,    77,     0,     0,    78,    79,     0,
       0,     0,     0,    30,    81,    31,    32,    33,    34
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
         4,    52,    69,    21,     6,     0,     8,     3,    26,     5,
       6,    29,    16,     3,    10,    11,    12,    13,    14,    15,
      16,    17,     4,    44,    28,    21,     3,     9,    24,    25,
      20,    98,    22,     4,    38,    31,    57,     4,     9,    21,
      18,    18,    19,    20,    26,    27,     0,    29,    52,     3,
      21,     3,    56,    10,    21,    26,    27,    61,    29,    26,
      27,    65,    29,   114,    26,    23,    20,    29,    22,    18,
      19,    20,    30,    18,    23,     3,    80,     5,     6,    10,
      84,    30,    10,    11,    12,    13,    14,    15,    16,    17,
       3,    23,    18,    21,    20,     7,    24,    25,    30,    11,
      13,    18,     3,    31,     5,     6,    10,    18,    18,    10,
     114,    12,    13,    14,    15,    16,    17,   121,     7,    18,
      21,     7,    11,    24,    25,    11,     5,     6,    18,    16,
      31,    10,    11,    12,    13,    14,    15,    16,    17,    22,
      28,    65,    21,    26,    -1,    24,    25,    27,    -1,    28,
       5,     6,    31,    -1,    -1,    10,    11,    12,    13,    14,
      15,    16,    17,    -1,    -1,    -1,    21,    -1,    -1,    24,
      25,    -1,    -1,    28,     5,     6,    31,    -1,    -1,    10,
      -1,    12,    13,    14,    15,    16,    17,    -1,    -1,    -1,
      21,    -1,    -1,    24,    25,    -1,    -1,    28,     5,     6,
      31,    -1,    -1,    10,    -1,    12,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    21,    -1,    -1,    24,    25,    -1,
      -1,    -1,    -1,    12,    31,    14,    15,    16,    17
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    33,    34,     0,    35,     3,    20,    22,    36,    37,
      38,    39,    46,    49,    50,    18,    13,    40,    49,     4,
      21,    26,    27,    29,    41,    42,    39,    49,     6,     8,
      12,    14,    15,    16,    17,    48,    26,    29,    10,    40,
      18,    40,    42,    36,    18,    20,    18,    19,    20,    23,
      30,    47,    10,    43,    44,    49,    35,    18,    18,     3,
      18,    19,    20,    43,    49,     7,    11,     5,     6,    10,
      11,    12,    13,    14,    15,    16,    17,    21,    24,    25,
      28,    31,    45,    49,    35,    23,    30,    49,    18,    11,
      11,    44,    49,    45,    49,     3,     5,     6,    10,    12,
      13,    14,    15,    16,    17,    21,    24,    25,    31,     9,
      49,    23,    30,    11,    10,    45,    18,     9,    43,    11,
      18,    11,    49
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
     285,   286
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    32,    34,    33,    35,    35,    36,    36,    36,    36,
      36,    36,    37,    37,    38,    39,    39,    40,    40,    40,
      40,    40,    41,    41,    42,    42,    42,    42,    43,    43,
      44,    44,    45,    45,    45,    45,    45,    45,    45,    45,
      45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
      45,    45,    45,    45,    45,    45,    45,    45,    45,    46,
      46,    46,    46,    47,    47,    47,    47,    47,    47,    47,
      48,    48,    48,    48,    48,    49,    49,    50
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       3,     1,     2,     3,     4,     2,     2,     8,     7,     6,
       7,     3,     1,     2,     4,     4,     5,     5,     3,     1,
       2,     7,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     4,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
       5,     5,     4,     3,     3,     4,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     2
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
  "ELSE", "FUNCTIONCALL", "SCOPENAME", "QUOTED_VAR_VALUE", "FNVALUE",
  "$accept", "project", "@1", "statements", "statement", "scope", "or_op",
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
        33,     0,    -1,    -1,    34,    35,    -1,    35,    36,    -1,
      -1,    50,    -1,    22,    -1,    46,    -1,    37,    -1,    49,
      42,    18,    -1,    38,    -1,    39,    40,    -1,    49,    27,
      40,    -1,    39,    13,    39,    40,    -1,    49,    41,    -1,
      49,    42,    -1,    49,     8,    20,    18,    35,    49,     9,
      18,    -1,    49,     8,    18,    35,    49,     9,    18,    -1,
      49,     8,    18,    35,    49,     9,    -1,    49,     8,    20,
      18,    35,    49,     9,    -1,    49,     6,    36,    -1,    29,
      -1,    21,    29,    -1,    26,    10,    43,    11,    -1,    26,
      10,    49,    11,    -1,    21,    26,    10,    43,    11,    -1,
      21,    26,    10,    49,    11,    -1,    43,     7,    44,    -1,
      44,    -1,    49,    45,    -1,    49,    28,    49,    10,    43,
      11,    49,    -1,    45,    31,    -1,    45,    24,    -1,    45,
      25,    -1,    45,     3,    -1,    45,    16,    -1,    45,    14,
      -1,    45,    15,    -1,    45,    17,    -1,    45,    12,    -1,
      45,     6,    -1,    45,    13,    -1,    45,    21,    -1,    45,
       5,    -1,    45,    10,    45,    11,    -1,     5,    -1,    10,
      45,    11,    -1,    31,    -1,    24,    -1,    25,    -1,    16,
      -1,    14,    -1,    15,    -1,    17,    -1,    12,    -1,     6,
      -1,    13,    -1,    21,    -1,    49,     4,    48,    47,    20,
      18,    -1,    49,     4,    48,    20,    18,    -1,    49,     4,
      48,    47,    18,    -1,    49,     4,    48,    18,    -1,    47,
       3,    23,    -1,    47,     3,    30,    -1,    47,    19,    49,
      23,    -1,    47,    19,    49,    30,    -1,    30,    -1,    23,
      -1,    19,    -1,    12,    -1,    14,    -1,    15,    -1,    17,
      -1,    16,    -1,     3,    -1,    -1,    20,    18,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned short int
  Parser::yyprhs_[] =
  {
         0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    23,    25,    28,    32,    37,    40,    43,    52,    60,
      67,    75,    79,    81,    84,    89,    94,   100,   106,   110,
     112,   115,   123,   126,   129,   132,   135,   138,   141,   144,
     147,   150,   153,   156,   159,   162,   167,   169,   173,   175,
     177,   179,   181,   183,   185,   187,   189,   191,   193,   195,
     202,   208,   214,   219,   223,   227,   232,   237,   239,   241,
     243,   245,   247,   249,   251,   253,   255,   256
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,    69,    69,    69,    81,    86,    91,    95,    99,   103,
     107,   111,   117,   123,   131,   140,   144,   152,   157,   162,
     167,   172,   179,   183,   189,   193,   197,   201,   207,   211,
     218,   222,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   257,
     261,   265,   269,   275,   280,   285,   291,   298,   303,   308,
     315,   319,   323,   327,   331,   337,   342,   347
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
      25,    26,    27,    28,    29,    30,    31
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int Parser::yyeof_ = 0;
  const int Parser::yylast_ = 228;
  const int Parser::yynnts_ = 19;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 3;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 32;

  const unsigned int Parser::yyuser_token_number_max_ = 286;
  const Parser::token_number_type Parser::yyundef_token_ = 2;

} // namespace QMake

#line 353 "/home/andreas/KDE-work/4.0/kdevelop/buildtools/importers/qmake/parser/qmake_parser.yy"


namespace QMake
{
void Parser::error(const location_type& /*l*/, const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}
