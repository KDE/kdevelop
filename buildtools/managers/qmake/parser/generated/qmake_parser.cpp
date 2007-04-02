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
  Parser::Parser (QMake::Lexer* lexer_yyarg, QMake::ProjectAST* project_yyarg)
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

    {
            Q_FOREACH( StatementAST* s, (yysemantic_stack_[(1) - (1)].stmtlist))
            {
                project->addStatement( s );
            }
        ;}
    break;

  case 3:

    {
            (yyval.stmtlist).append( (yysemantic_stack_[(2) - (2)].stmt) );
        ;}
    break;

  case 4:

    {
            (yyval.stmtlist).clear();
        ;}
    break;

  case 5:

    {
            CommentAST* node = new CommentAST();
            node->setComment( (yysemantic_stack_[(1) - (1)].value) );
            (yyval.stmt) = node;
        ;}
    break;

  case 6:

    {
            NewlineAST* node = new NewlineAST();
            node->setWhitespace( (yysemantic_stack_[(1) - (1)].value) );
            (yyval.stmt) = node;
        ;}
    break;

  case 7:

    {
            (yyval.stmt) = (yysemantic_stack_[(1) - (1)].stmt);
        ;}
    break;

  case 8:

    {
            (yyval.stmt) = (yysemantic_stack_[(1) - (1)].stmt);
        ;}
    break;

  case 9:

    {
            ScopeAST* node = (yysemantic_stack_[(3) - (1)].scope);
            (yysemantic_stack_[(3) - (3)].scopebody)->setWhitespace( (yysemantic_stack_[(3) - (2)].value) );
            node->setScopeBody( (yysemantic_stack_[(3) - (3)].scopebody) );
            (yyval.stmt) = node;
        ;}
    break;

  case 10:

    {
            ScopeAST* node = (yysemantic_stack_[(2) - (1)].scope);
            node->setLineEnding( (yysemantic_stack_[(2) - (2)].value) );
            (yyval.stmt) = node;
        ;}
    break;

  case 11:

    {
            SimpleScopeAST* node = new SimpleScopeAST();
            node->setScopeName( (yysemantic_stack_[(2) - (2)].value) );
            node->setWhitespace( (yysemantic_stack_[(2) - (1)].value) );
            (yyval.scope) = node;
        ;}
    break;

  case 12:

    {
            ScopeAST* node = (yysemantic_stack_[(2) - (2)].funccall);
            node->setWhitespace( (yysemantic_stack_[(2) - (1)].value) );
            (yyval.scope) = node;
        ;}
    break;

  case 13:

    {
        OrAST* node = new OrAST();
        node->setLeftCall( (yysemantic_stack_[(4) - (2)].funccall) );
        node->setRightCall( (yysemantic_stack_[(4) - (4)].funccall) );
        node->setOrOp( (yysemantic_stack_[(4) - (3)].value) );
        node->setWhitespace( (yysemantic_stack_[(4) - (1)].value) );
        (yyval.scope) = node;
    ;}
    break;

  case 14:

    {
            ScopeBodyAST* node = new ScopeBodyAST();
			node->setBegin( (yysemantic_stack_[(7) - (1)].value)+(yysemantic_stack_[(7) - (2)].value)+(yysemantic_stack_[(7) - (3)].value) );
			node->setStatements( (yysemantic_stack_[(7) - (4)].stmtlist) );
			node->setEnd( (yysemantic_stack_[(7) - (5)].value)+(yysemantic_stack_[(7) - (6)].value)+(yysemantic_stack_[(7) - (7)].value) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 15:

    {
            ScopeBodyAST* node = new ScopeBodyAST();
			node->setBegin( (yysemantic_stack_[(6) - (1)].value)+(yysemantic_stack_[(6) - (2)].value) );
			node->setStatements( (yysemantic_stack_[(6) - (3)].stmtlist) );
			node->setEnd( (yysemantic_stack_[(6) - (4)].value)+(yysemantic_stack_[(6) - (5)].value)+(yysemantic_stack_[(6) - (6)].value) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 16:

    {
            ScopeBodyAST* node = new ScopeBodyAST();
			node->setBegin( (yysemantic_stack_[(5) - (1)].value)+(yysemantic_stack_[(5) - (2)].value) );
			node->setStatements( (yysemantic_stack_[(5) - (3)].stmtlist) );
			node->setEnd( (yysemantic_stack_[(5) - (4)].value)+(yysemantic_stack_[(5) - (5)].value) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 17:

    {
            ScopeBodyAST* node = new ScopeBodyAST();
			node->setBegin( (yysemantic_stack_[(6) - (1)].value)+(yysemantic_stack_[(6) - (2)].value)+(yysemantic_stack_[(6) - (3)].value) );
			node->setStatements( (yysemantic_stack_[(6) - (4)].stmtlist) );
			node->setEnd( (yysemantic_stack_[(6) - (5)].value)+(yysemantic_stack_[(6) - (6)].value) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 18:

    {
            ScopeBodyAST* node = new ScopeBodyAST();
            node->setBegin( (yysemantic_stack_[(2) - (1)].value) );
            node->addStatement( (yysemantic_stack_[(2) - (2)].stmt) );
            (yyval.scopebody) = node;
        ;}
    break;

  case 19:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 20:

    {
            (yyval.value) = (yysemantic_stack_[(2) - (1)].value)+(yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 21:

    {
            FunctionCallAST* node = new FunctionCallAST();
            node->setFunctionName( (yysemantic_stack_[(4) - (1)].value) );
            node->setBegin( (yysemantic_stack_[(4) - (2)].value) );
            node->setArguments( (yysemantic_stack_[(4) - (3)].values) );
            node->setEnd( (yysemantic_stack_[(4) - (4)].value) );
            (yyval.funccall) = node;
        ;}
    break;

  case 22:

    {
            FunctionCallAST* node = new FunctionCallAST();
            node->setFunctionName( (yysemantic_stack_[(3) - (1)].value) );
            node->setBegin( (yysemantic_stack_[(3) - (2)].value) );
            node->setEnd( (yysemantic_stack_[(3) - (3)].value) );
            (yyval.funccall) = node;
        ;}
    break;

  case 23:

    {
            FunctionCallAST* node = new FunctionCallAST();
            node->setFunctionName( (yysemantic_stack_[(5) - (1)].value)+(yysemantic_stack_[(5) - (2)].value) );
            node->setBegin( (yysemantic_stack_[(5) - (3)].value) );
            node->setArguments( (yysemantic_stack_[(5) - (4)].values) );
            node->setEnd( (yysemantic_stack_[(5) - (5)].value) );
            (yyval.funccall) = node;
        ;}
    break;

  case 24:

    {
            FunctionCallAST* node = new FunctionCallAST();
            node->setFunctionName( (yysemantic_stack_[(4) - (1)].value)+(yysemantic_stack_[(4) - (2)].value) );
            node->setBegin( (yysemantic_stack_[(4) - (3)].value) );
            node->setEnd( (yysemantic_stack_[(4) - (4)].value) );
            (yyval.funccall) = node;
        ;}
    break;

  case 25:

    {
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 26:

    {
            (yyval.values).clear();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 27:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 28:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 29:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 30:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 31:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 32:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 33:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 34:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 35:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 36:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 37:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 38:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 39:

    {
            (yyval.value) += (yysemantic_stack_[(2) - (2)].value);
        ;}
    break;

  case 40:

    {
            (yyval.value) += (yysemantic_stack_[(4) - (2)].value)+(yysemantic_stack_[(4) - (3)].value)+(yysemantic_stack_[(4) - (4)].value);
        ;}
    break;

  case 41:

    {
            (yyval.value) += (yysemantic_stack_[(6) - (2)].value)+(yysemantic_stack_[(6) - (3)].value)+(yysemantic_stack_[(6) - (4)].value)+(yysemantic_stack_[(6) - (5)].values).join(",")+(yysemantic_stack_[(6) - (6)].value);
        ;}
    break;

  case 42:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 43:

    {
            (yyval.value) = (yysemantic_stack_[(3) - (1)].value)+(yysemantic_stack_[(3) - (2)].value)+(yysemantic_stack_[(3) - (3)].value);
        ;}
    break;

  case 44:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 45:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 46:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 47:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 48:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 49:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 50:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 51:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 52:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 53:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 54:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 55:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 56:

    {
            (yyval.value) = (yysemantic_stack_[(5) - (1)].value)+(yysemantic_stack_[(5) - (2)].value)+(yysemantic_stack_[(5) - (3)].value)+(yysemantic_stack_[(5) - (4)].values).join(",")+(yysemantic_stack_[(5) - (5)].value);
        ;}
    break;

  case 57:

    {
        	AssignmentAST* node = new AssignmentAST();
        	node->setWhitespace( (yysemantic_stack_[(5) - (1)].value) );
        	node->setVariable( (yysemantic_stack_[(5) - (2)].value) );
        	node->setOp( (yysemantic_stack_[(5) - (3)].value) );
        	node->setValues( (yysemantic_stack_[(5) - (4)].values) );
        	node->setLineEnding( (yysemantic_stack_[(5) - (5)].value) );
            (yyval.stmt) = node;
        ;}
    break;

  case 58:

    {
            AssignmentAST* node = new AssignmentAST();
        	node->setWhitespace( (yysemantic_stack_[(4) - (1)].value) );
        	node->setVariable( (yysemantic_stack_[(4) - (2)].value) );
        	node->setOp( (yysemantic_stack_[(4) - (3)].value) );
        	node->setLineEnding( (yysemantic_stack_[(4) - (4)].value) );
            (yyval.stmt) = node;
        ;}
    break;

  case 59:

    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 60:

    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 61:

    {
            (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 62:

    {
            (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 63:

    {

            (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 64:

    {

            (yyval.values).append( (yysemantic_stack_[(4) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(4) - (4)].value) );
        ;}
    break;

  case 65:

    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 66:

    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 67:

    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 68:

    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(1) - (1)].value) );
        ;}
    break;

  case 69:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 70:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 71:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 72:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 73:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 74:

    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 75:

    {
            (yyval.value) = "";
        ;}
    break;

  case 76:

    {
            (yyval.value) = (yysemantic_stack_[(2) - (1)].value)+(yysemantic_stack_[(2) - (2)].value);
        ;}
    break;


    /* Line 675 of lalr1.cc.  */

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
  const signed char Parser::yypact_ninf_ = -47;
  const short int
  Parser::yypact_[] =
  {
       -47,     6,     5,   -47,   -47,    -3,   -47,   -47,   -47,    37,
     -47,    22,   -47,   -47,   -47,    38,   200,    39,    29,   -47,
     -47,    32,    27,   124,   -47,   -47,   -47,   -47,   -47,   -47,
     183,    42,   -47,     7,    15,   -47,   -47,    35,   -47,   -47,
     -47,   -47,   -47,   176,    61,   -47,   -47,   -47,   133,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,    55,
     -47,    72,   157,    44,   -47,    27,   -47,   -19,   -47,    55,
      55,   -47,    96,    85,    53,   133,   -47,   -47,   -47,   -47,
     133,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
      55,   -47,    33,    27,   -47,   -47,   -16,   -14,   -47,   -47,
     133,   157,   109,    70,    69,   171,   -47,   -47,   -47,   -47,
      97,   -47,   133,   -47,    75,   -47,   120,   -47,   -47
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
         4,     0,    75,     1,    74,     0,     6,     3,     8,    75,
       7,     0,     5,    76,    10,     0,     0,     0,     0,    19,
      11,    12,    75,     0,     9,    69,    70,    71,    73,    72,
       0,     0,    20,     0,     0,    18,     4,     0,    58,    67,
      66,    68,    65,     0,     0,    54,    42,    52,     0,    22,
      51,    53,    48,    49,    47,    50,    55,    45,    46,    75,
      44,     0,    26,     0,    13,    75,     4,     0,    57,    75,
      75,    24,     0,     0,     0,     0,    21,    30,    39,    36,
       0,    35,    37,    32,    33,    31,    34,    38,    28,    29,
      75,    27,     0,    75,    59,    60,     0,     0,    23,    43,
       0,    25,     0,     0,    16,     0,    61,    63,    62,    64,
       0,    40,     0,    15,    17,    56,     0,    14,    41
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -47,   -47,   -33,    83,   -47,   -47,   -47,   -47,    60,   -43,
     -46,   -47,   -47,   -47,    -9,   -47
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,     1,     2,     7,     8,     9,    24,    20,    21,    61,
      62,    10,    43,    30,    11,    12
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char Parser::yytable_ninf_ = -3;
  const signed char
  Parser::yytable_[] =
  {
        15,    72,    73,    65,    94,    -2,     3,   106,     4,   108,
      45,    95,    46,    47,   107,    13,   109,    48,    49,    50,
      51,    52,    53,    54,    55,     5,    16,     6,    56,   101,
       4,    57,    58,    93,   102,    59,    63,    16,    60,    33,
       4,    18,   104,    17,    22,    34,    23,     5,    18,     6,
      74,    19,    44,    66,    17,    14,    92,   110,     4,    18,
      96,    97,    19,   100,    45,    31,    46,    47,    32,   116,
      31,    48,    71,    50,    51,    52,    53,    54,    55,    75,
     112,   103,    56,    76,   105,    57,    58,   113,    77,    59,
      78,    79,    60,   117,    64,    80,    99,    81,    82,    83,
      84,    85,    86,    75,    75,    35,    87,    98,   115,    88,
      89,     0,    77,    90,    78,    79,    91,     0,     0,    80,
     111,    81,    82,    83,    84,    85,    86,    75,     0,     0,
      87,   118,     0,    88,    89,     0,    45,    90,    46,    47,
      91,     0,    36,    48,    37,    50,    51,    52,    53,    54,
      55,     0,     0,     0,    56,     0,     0,    57,    58,     0,
      77,    59,    78,    79,    60,     0,     0,    80,     0,    81,
      82,    83,    84,    85,    86,    16,     0,     0,    87,    67,
     114,    88,    89,     0,     0,    90,     0,     0,    91,     0,
       0,     0,    17,     0,    68,    69,     0,    18,     0,     0,
      19,    38,    39,    70,     0,     0,    40,     0,     0,     0,
      41,     0,    25,    42,    26,    27,    28,    29
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
         9,    44,    48,    36,    23,     0,     0,    23,     3,    23,
       3,    30,     5,     6,    30,    18,    30,    10,    11,    12,
      13,    14,    15,    16,    17,    20,     4,    22,    21,    75,
       3,    24,    25,    66,    80,    28,    21,     4,    31,    10,
       3,    26,     9,    21,     6,    13,     8,    20,    26,    22,
      59,    29,    10,    18,    21,    18,    65,   100,     3,    26,
      69,    70,    29,    10,     3,    26,     5,     6,    29,   112,
      26,    10,    11,    12,    13,    14,    15,    16,    17,     7,
      10,    90,    21,    11,    93,    24,    25,    18,     3,    28,
       5,     6,    31,    18,    34,    10,    11,    12,    13,    14,
      15,    16,    17,     7,     7,    22,    21,    11,    11,    24,
      25,    -1,     3,    28,     5,     6,    31,    -1,    -1,    10,
      11,    12,    13,    14,    15,    16,    17,     7,    -1,    -1,
      21,    11,    -1,    24,    25,    -1,     3,    28,     5,     6,
      31,    -1,    18,    10,    20,    12,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    21,    -1,    -1,    24,    25,    -1,
       3,    28,     5,     6,    31,    -1,    -1,    10,    -1,    12,
      13,    14,    15,    16,    17,     4,    -1,    -1,    21,     3,
       9,    24,    25,    -1,    -1,    28,    -1,    -1,    31,    -1,
      -1,    -1,    21,    -1,    18,    19,    -1,    26,    -1,    -1,
      29,    18,    19,    27,    -1,    -1,    23,    -1,    -1,    -1,
      27,    -1,    12,    30,    14,    15,    16,    17
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    33,    34,     0,     3,    20,    22,    35,    36,    37,
      43,    46,    47,    18,    18,    46,     4,    21,    26,    29,
      39,    40,     6,     8,    38,    12,    14,    15,    16,    17,
      45,    26,    29,    10,    13,    35,    18,    20,    18,    19,
      23,    27,    30,    44,    10,     3,     5,     6,    10,    11,
      12,    13,    14,    15,    16,    17,    21,    24,    25,    28,
      31,    41,    42,    21,    40,    34,    18,     3,    18,    19,
      27,    11,    41,    42,    46,     7,    11,     3,     5,     6,
      10,    12,    13,    14,    15,    16,    17,    21,    24,    25,
      28,    31,    46,    34,    23,    30,    46,    46,    11,    11,
      10,    42,    42,    46,     9,    46,    23,    30,    23,    30,
      41,    11,    10,    18,     9,    11,    41,    18,    11
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
         0,    32,    33,    34,    34,    35,    35,    35,    35,    36,
      36,    37,    37,    37,    38,    38,    38,    38,    38,    39,
      39,    40,    40,    40,    40,    41,    41,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    43,    43,    44,
      44,    44,    44,    44,    44,    44,    44,    44,    44,    45,
      45,    45,    45,    45,    46,    46,    47
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     1,     2,     0,     1,     1,     1,     1,     3,
       2,     2,     2,     4,     7,     6,     5,     6,     2,     1,
       2,     4,     3,     5,     4,     3,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       4,     6,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     5,     5,     4,     3,
       3,     4,     4,     4,     4,     1,     1,     1,     1,     1,
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
  "CONT_COMMENT", "FUNCTIONCALL", "SCOPENAME", "QUOTED_VAR_VALUE",
  "FNVALUE", "$accept", "project", "statements", "statement", "scope",
  "scope_head", "scope_body", "scope_name", "functioncall", "functionargs",
  "functionarg", "variable_assignment", "values", "op", "ws", "comment", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
        33,     0,    -1,    34,    -1,    34,    35,    -1,    -1,    47,
      -1,    22,    -1,    43,    -1,    36,    -1,    37,    46,    38,
      -1,    37,    18,    -1,    46,    39,    -1,    46,    40,    -1,
      46,    40,    13,    40,    -1,     8,    20,    18,    34,    46,
       9,    18,    -1,     8,    18,    34,    46,     9,    18,    -1,
       8,    18,    34,    46,     9,    -1,     8,    20,    18,    34,
      46,     9,    -1,     6,    35,    -1,    29,    -1,    21,    29,
      -1,    26,    10,    41,    11,    -1,    26,    10,    11,    -1,
      21,    26,    10,    41,    11,    -1,    21,    26,    10,    11,
      -1,    41,     7,    42,    -1,    42,    -1,    42,    31,    -1,
      42,    24,    -1,    42,    25,    -1,    42,     3,    -1,    42,
      16,    -1,    42,    14,    -1,    42,    15,    -1,    42,    17,
      -1,    42,    12,    -1,    42,     6,    -1,    42,    13,    -1,
      42,    21,    -1,    42,     5,    -1,    42,    10,    42,    11,
      -1,    42,    28,    46,    10,    41,    11,    -1,     5,    -1,
      10,    42,    11,    -1,    31,    -1,    24,    -1,    25,    -1,
      16,    -1,    14,    -1,    15,    -1,    17,    -1,    12,    -1,
       6,    -1,    13,    -1,     3,    -1,    21,    -1,    28,    46,
      10,    41,    11,    -1,    46,     4,    45,    44,    18,    -1,
      46,     4,    45,    18,    -1,    44,     3,    23,    -1,    44,
       3,    30,    -1,    44,    19,    46,    23,    -1,    44,    27,
      46,    23,    -1,    44,    19,    46,    30,    -1,    44,    27,
      46,    30,    -1,    30,    -1,    23,    -1,    19,    -1,    27,
      -1,    12,    -1,    14,    -1,    15,    -1,    17,    -1,    16,
      -1,     3,    -1,    -1,    20,    18,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  Parser::yyprhs_[] =
  {
         0,     0,     3,     5,     8,     9,    11,    13,    15,    17,
      21,    24,    27,    30,    35,    43,    50,    56,    63,    66,
      68,    71,    76,    80,    86,    91,    95,    97,   100,   103,
     106,   109,   112,   115,   118,   121,   124,   127,   130,   133,
     136,   141,   148,   150,   154,   156,   158,   160,   162,   164,
     166,   168,   170,   172,   174,   176,   178,   184,   190,   195,
     199,   203,   208,   213,   218,   223,   225,   227,   229,   231,
     233,   235,   237,   239,   241,   243,   244
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,    72,    72,    81,    86,    91,    97,   103,   107,   113,
     120,   128,   135,   141,   152,   160,   168,   176,   184,   193,
     197,   203,   212,   220,   229,   239,   243,   250,   254,   258,
     262,   266,   270,   274,   278,   282,   286,   290,   294,   298,
     302,   306,   310,   314,   318,   322,   326,   330,   334,   338,
     342,   346,   350,   354,   358,   362,   366,   372,   382,   393,
     398,   403,   409,   415,   422,   429,   434,   439,   444,   451,
     455,   459,   463,   467,   473,   478,   483
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
  const int Parser::yylast_ = 217;
  const int Parser::yynnts_ = 16;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 3;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 32;

  const unsigned int Parser::yyuser_token_number_max_ = 286;
  const Parser::token_number_type Parser::yyundef_token_ = 2;

} // namespace QMake




namespace QMake
{
    void Parser::error(const location_type& /*l*/, const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}
