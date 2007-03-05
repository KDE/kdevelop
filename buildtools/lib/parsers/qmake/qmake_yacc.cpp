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

#include "qmake_yacc.hpp"

/* User implementation prologue.  */


/* Line 317 of lalr1.cc.  */
#line 45 "qmake_yacc.cpp"

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
  Parser::Parser (QMake::Lexer* lexer_yyarg, QValueStack<ProjectAST*>& projects_yyarg, int depth_yyarg)
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
      lexer (lexer_yyarg),
      projects (projects_yyarg),
      depth (depth_yyarg)
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
#line 143 "qmake.yy"
    {
        ProjectAST *projectAST = new ProjectAST();
        projects.push(projectAST);
    ;}
    break;

  case 4:
#line 151 "qmake.yy"
    {
            projects.top()->addChildAST((yysemantic_stack_[(2) - (2)].node));
            (yysemantic_stack_[(2) - (2)].node)->setDepth(depth);
        ;}
    break;

  case 6:
#line 159 "qmake.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 7:
#line 163 "qmake.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 8:
#line 167 "qmake.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 9:
#line 171 "qmake.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 10:
#line 175 "qmake.yy"
    {
            (yyval.node) = new NewLineAST();
        ;}
    break;

  case 11:
#line 181 "qmake.yy"
    {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = (yysemantic_stack_[(5) - (1)].value);
            node->op = (yysemantic_stack_[(5) - (2)].value);
            node->values = (yysemantic_stack_[(5) - (3)].values) ;
            node->values.append( (yysemantic_stack_[(5) - (4)].value) );
            node->values.append( (yysemantic_stack_[(5) - (5)].value) );
            node->indent = (yysemantic_stack_[(5) - (3)].indent);
            (yyval.node) = node;
        ;}
    break;

  case 12:
#line 192 "qmake.yy"
    {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = (yysemantic_stack_[(4) - (1)].value);
            node->op = (yysemantic_stack_[(4) - (2)].value);
            node->values.append( (yysemantic_stack_[(4) - (3)].value) );
            node->values.append( (yysemantic_stack_[(4) - (4)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 13:
#line 201 "qmake.yy"
    {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = (yysemantic_stack_[(4) - (1)].value);
            node->op = (yysemantic_stack_[(4) - (2)].value);
            node->values.append( (yysemantic_stack_[(4) - (3)].value) );
            node->values.append( (yysemantic_stack_[(4) - (4)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 14:
#line 210 "qmake.yy"
    {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = (yysemantic_stack_[(5) - (1)].value);
            node->op = (yysemantic_stack_[(5) - (2)].value);
            node->values = (yysemantic_stack_[(5) - (3)].values);
            node->values.append( (yysemantic_stack_[(5) - (4)].value) );
            node->values.append( (yysemantic_stack_[(5) - (5)].value) );
            node->indent = (yysemantic_stack_[(5) - (3)].indent);
            (yyval.node) = node;
        ;}
    break;

  case 15:
#line 223 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 16:
#line 228 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 17:
#line 233 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(5) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (4)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (5)].value) );
            if( (yysemantic_stack_[(5) - (4)].indent) != "" && (yyval.indent) == "" )
                (yyval.indent) = (yysemantic_stack_[(5) - (4)].indent);
        ;}
    break;

  case 18:
#line 242 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 19:
#line 247 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(5) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (4)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (5)].value) );
            if( (yysemantic_stack_[(5) - (4)].indent) != "" && (yyval.indent) == "" )
                (yyval.indent) = (yysemantic_stack_[(5) - (4)].indent);
        ;}
    break;

  case 20:
#line 256 "qmake.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(2) - (1)].value) );
            (yyval.values).append( (yysemantic_stack_[(2) - (2)].value) );
        ;}
    break;

  case 21:
#line 262 "qmake.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(2) - (1)].value) );
            (yyval.values).append( (yysemantic_stack_[(2) - (2)].value) );
        ;}
    break;

  case 22:
#line 268 "qmake.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(2) - (1)].value) );
            (yyval.values).append( (yysemantic_stack_[(2) - (2)].value) );
        ;}
    break;

  case 23:
#line 276 "qmake.yy"
    { (yyval.value) = (yysemantic_stack_[(1) - (1)].value); ;}
    break;

  case 24:
#line 277 "qmake.yy"
    { (yyval.value) = (yysemantic_stack_[(1) - (1)].value); ;}
    break;

  case 25:
#line 282 "qmake.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
            (yyval.indent) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 26:
#line 287 "qmake.yy"
    {
            (yyval.value) = QString();
            (yyval.indent) = QString();
        ;}
    break;

  case 27:
#line 293 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 28:
#line 297 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 29:
#line 301 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 30:
#line 305 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 31:
#line 309 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 32:
#line 315 "qmake.yy"
    {
            ProjectAST *projectAST = new ProjectAST(ProjectAST::Scope);
            projects.push(projectAST);
            projects.top()->scopedID = (yysemantic_stack_[(1) - (1)].value);
            depth++;
        ;}
    break;

  case 33:
#line 322 "qmake.yy"
    {
            (yyval.node) = projects.pop();
            depth--;
        ;}
    break;

  case 34:
#line 329 "qmake.yy"
    {
            ProjectAST *projectAST = new ProjectAST(ProjectAST::FunctionScope);
            projects.push(projectAST);
            projects.top()->scopedID = (yysemantic_stack_[(4) - (1)].value);
            projects.top()->args = (yysemantic_stack_[(4) - (3)].value);
            depth++;

            //qWarning("%s", $<value>1.ascii());
            if ((yysemantic_stack_[(4) - (1)].value).contains("include"))
            {
                IncludeAST *includeAST = new IncludeAST();
                includeAST->projectName = (yysemantic_stack_[(4) - (3)].value);
                projects.top()->addChildAST(includeAST);
                includeAST->setDepth(depth);
            }
        ;}
    break;

  case 35:
#line 347 "qmake.yy"
    {
            (yyval.node) = projects.pop();
            depth--;
        ;}
    break;

  case 36:
#line 353 "qmake.yy"
    { (yyval.value) = (yysemantic_stack_[(1) - (1)].value); ;}
    break;

  case 37:
#line 354 "qmake.yy"
    { (yyval.value) = ""; ;}
    break;

  case 39:
#line 359 "qmake.yy"
    {
            projects.top()->addChildAST((yysemantic_stack_[(2) - (2)].node));
            (yysemantic_stack_[(2) - (2)].node)->setDepth(depth);
        ;}
    break;

  case 41:
#line 367 "qmake.yy"
    {
            ProjectAST *projectAST = new ProjectAST(ProjectAST::FunctionScope);
            projects.push(projectAST);
            projects.top()->scopedID = "else";
            projects.top()->args = "";
            depth++;
        ;}
    break;

  case 42:
#line 375 "qmake.yy"
    {
            (yyval.node) = projects.pop();
            depth--;
        ;}
    break;

  case 43:
#line 380 "qmake.yy"
    {
            (yyval.node) = new ProjectAST();
        ;}
    break;

  case 44:
#line 386 "qmake.yy"
    {
            CommentAST *node = new CommentAST();
            node->comment = (yysemantic_stack_[(1) - (1)].value);
            (yyval.node) = node;
        ;}
    break;


    /* Line 675 of lalr1.cc.  */
#line 731 "qmake_yacc.cpp"
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
  const signed char Parser::yypact_ninf_ = -42;
  const signed char
  Parser::yypact_[] =
  {
       -42,    10,   -42,   -42,     5,    30,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,     1,
       8,    -8,   -42,    20,   -42,    23,    13,     5,   -42,   -42,
     -42,    -6,    28,   -42,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,    -1,    -8,   -42,   -42,   -42,   -14,   -14,   -42,    -2,
      -6,    -6,    14,   -42,   -42,   -42,   -42,    -8,    29,   -42
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
         2,     0,     5,     1,     3,    32,    45,    44,     4,     6,
       7,     8,     9,    10,    27,    28,    29,    30,    31,    37,
      26,    40,    36,     0,    25,    26,     0,     0,     5,    33,
      34,    25,     0,    12,    13,    21,    22,    24,    23,    20,
      39,     0,    40,    15,    11,    14,    16,    18,    38,    43,
       0,     0,     0,    35,    17,    19,    41,    40,     0,    42
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -42,   -42,   -42,    18,    21,   -42,   -42,   -27,   -25,   -42,
     -42,   -42,   -42,   -42,   -42,   -41,   -42,   -42,   -42,   -42
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,     1,     2,     4,     8,     9,    25,    39,    26,    20,
      10,    21,    11,    42,    23,    29,    53,    57,    12,    13
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char Parser::yytable_ninf_ = -27;
  const signed char
  Parser::yytable_[] =
  {
        32,    49,     5,    27,    43,   -26,   -26,    24,     5,    28,
       3,     6,     7,    37,    38,    48,    58,     6,     7,    22,
      52,    50,    51,    54,    55,    33,    34,    35,    36,    24,
      30,    56,    37,    38,    14,    15,    16,    17,    18,    19,
      44,    45,    46,    47,    31,    59,    41,     0,    40
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
        25,    42,     3,    11,    31,    19,    20,    21,     3,    17,
       0,    12,    13,    19,    20,    16,    57,    12,    13,    18,
      22,    46,    47,    50,    51,    12,    13,    14,    15,    21,
      10,    17,    19,    20,     4,     5,     6,     7,     8,     9,
      12,    13,    14,    15,    21,    16,    28,    -1,    27
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    24,    25,     0,    26,     3,    12,    13,    27,    28,
      33,    35,    41,    42,     4,     5,     6,     7,     8,     9,
      32,    34,    18,    37,    21,    29,    31,    11,    17,    38,
      10,    21,    31,    12,    13,    14,    15,    19,    20,    30,
      27,    26,    36,    30,    12,    13,    14,    15,    16,    38,
      31,    31,    22,    39,    30,    30,    17,    40,    38,    16
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    23,    25,    24,    26,    26,    27,    27,    27,    27,
      27,    28,    28,    28,    28,    29,    29,    29,    29,    29,
      29,    29,    29,    30,    30,    31,    31,    32,    32,    32,
      32,    32,    34,    33,    36,    35,    37,    37,    38,    38,
      38,    40,    39,    39,    41,    42
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     5,     4,     4,     5,     3,     3,     5,     3,     5,
       2,     2,     2,     1,     1,     1,     0,     1,     1,     1,
       1,     1,     0,     3,     0,     7,     1,     0,     3,     2,
       0,     0,     5,     0,     1,     1
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const Parser::yytname_[] =
  {
    "$end", "error", "$undefined", "ID_SIMPLE", "EQ", "PLUSEQ", "MINUSEQ",
  "STAREQ", "TILDEEQ", "LBRACE", "RBRACE", "COLON", "NEWLINE", "COMMENT",
  "CONT", "COMMENT_CONT", "RCURLY", "LCURLY", "ID_ARGS",
  "QUOTED_VARIABLE_VALUE", "VARIABLE_VALUE", "LIST_WS", "\"else\"",
  "$accept", "project", "@1", "statements", "statement",
  "variable_assignment", "multiline_values", "variable_value", "listws",
  "operator", "scope", "@2", "function_call", "@3", "function_args",
  "scope_body", "else_statement", "@4", "comment", "emptyline", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
        24,     0,    -1,    -1,    25,    26,    -1,    26,    27,    -1,
      -1,    28,    -1,    33,    -1,    35,    -1,    41,    -1,    42,
      -1,     3,    32,    29,    31,    12,    -1,     3,    32,    31,
      12,    -1,     3,    32,    31,    13,    -1,     3,    32,    29,
      31,    13,    -1,    29,    21,    30,    -1,    29,    31,    14,
      -1,    29,    31,    14,    31,    30,    -1,    29,    31,    15,
      -1,    29,    31,    15,    31,    30,    -1,    31,    30,    -1,
      31,    14,    -1,    31,    15,    -1,    20,    -1,    19,    -1,
      21,    -1,    -1,     4,    -1,     5,    -1,     6,    -1,     7,
      -1,     8,    -1,    -1,     3,    34,    38,    -1,    -1,     3,
       9,    37,    10,    36,    38,    39,    -1,    18,    -1,    -1,
      17,    26,    16,    -1,    11,    27,    -1,    -1,    -1,    22,
      17,    40,    38,    16,    -1,    -1,    13,    -1,    12,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  Parser::yyprhs_[] =
  {
         0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    27,    32,    37,    43,    47,    51,    57,    61,
      67,    70,    73,    76,    78,    80,    82,    83,    85,    87,
      89,    91,    93,    94,    98,    99,   107,   109,   110,   114,
     117,   118,   119,   125,   126,   128
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,   143,   143,   143,   150,   155,   158,   162,   166,   170,
     174,   180,   191,   200,   209,   222,   227,   232,   241,   246,
     255,   261,   267,   276,   277,   281,   287,   292,   296,   300,
     304,   308,   315,   314,   329,   328,   353,   354,   357,   358,
     363,   367,   366,   380,   385,   393
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
      15,    16,    17,    18,    19,    20,    21,    22
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int Parser::yyeof_ = 0;
  const int Parser::yylast_ = 48;
  const int Parser::yynnts_ = 20;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 3;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 23;

  const unsigned int Parser::yyuser_token_number_max_ = 277;
  const Parser::token_number_type Parser::yyundef_token_ = 2;

} // namespace QMake

#line 396 "qmake.yy"



namespace QMake
{
    void Parser::error(const location_type& /*l*/, const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}

