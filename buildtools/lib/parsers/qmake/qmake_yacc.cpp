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
#line 144 "qmake.yy"
    {
        ProjectAST *projectAST = new ProjectAST();
        projects.push(projectAST);
    ;}
    break;

  case 4:
#line 152 "qmake.yy"
    {
            projects.top()->addChildAST((yysemantic_stack_[(2) - (2)].node));
            (yysemantic_stack_[(2) - (2)].node)->setDepth(depth);
        ;}
    break;

  case 6:
#line 160 "qmake.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 7:
#line 164 "qmake.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 8:
#line 168 "qmake.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 9:
#line 172 "qmake.yy"
    {
            (yyval.node) = (yysemantic_stack_[(1) - (1)].node);
        ;}
    break;

  case 10:
#line 176 "qmake.yy"
    {
            (yyval.node) = new NewLineAST();
        ;}
    break;

  case 11:
#line 182 "qmake.yy"
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
#line 193 "qmake.yy"
    {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = (yysemantic_stack_[(5) - (1)].value);
            node->op = (yysemantic_stack_[(5) - (2)].value);
            node->values = (yysemantic_stack_[(5) - (3)].values) ;
            node->values.append( (yysemantic_stack_[(5) - (4)].value) );
            node->indent = (yysemantic_stack_[(5) - (3)].indent);
            (yyval.node) = node;
        ;}
    break;

  case 13:
#line 203 "qmake.yy"
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

  case 14:
#line 214 "qmake.yy"
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

  case 15:
#line 225 "qmake.yy"
    {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = (yysemantic_stack_[(4) - (1)].value);
            node->op = (yysemantic_stack_[(4) - (2)].value);
            node->values.append( (yysemantic_stack_[(4) - (3)].value) );
            node->values.append( (yysemantic_stack_[(4) - (4)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 16:
#line 234 "qmake.yy"
    {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = (yysemantic_stack_[(4) - (1)].value);
            node->op = (yysemantic_stack_[(4) - (2)].value);
            node->values.append( (yysemantic_stack_[(4) - (3)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 17:
#line 242 "qmake.yy"
    {
            AssignmentAST *node = new AssignmentAST();
            node->scopedID = (yysemantic_stack_[(4) - (1)].value);
            node->op = (yysemantic_stack_[(4) - (2)].value);
            node->values.append( (yysemantic_stack_[(4) - (3)].value) );
            node->values.append( (yysemantic_stack_[(4) - (4)].value) );
            (yyval.node) = node;
        ;}
    break;

  case 18:
#line 251 "qmake.yy"
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

  case 22:
#line 266 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(3) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(3) - (3)].value) );
        ;}
    break;

  case 23:
#line 271 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(2) - (2)].value) );
        ;}
    break;

  case 24:
#line 275 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(5) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (4)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (5)].value) );
            if( (yysemantic_stack_[(5) - (4)].indent) != "" && (yyval.indent) == "" )
                (yyval.indent) = (yysemantic_stack_[(5) - (4)].indent);
        ;}
    break;

  case 25:
#line 284 "qmake.yy"
    {
            (yyval.values).append( (yysemantic_stack_[(5) - (2)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (3)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (4)].value) );
            (yyval.values).append( (yysemantic_stack_[(5) - (5)].value) );
            if( (yysemantic_stack_[(5) - (4)].indent) != "" && (yyval.indent) == "" )
                (yyval.indent) = (yysemantic_stack_[(5) - (4)].indent);
        ;}
    break;

  case 26:
#line 293 "qmake.yy"
    {
            (yyval.values) = QStringList();
            (yyval.values).append( (yysemantic_stack_[(2) - (1)].value) );
            (yyval.values).append( (yysemantic_stack_[(2) - (2)].value) );
        ;}
    break;

  case 27:
#line 301 "qmake.yy"
    { (yyval.value) = (yysemantic_stack_[(1) - (1)].value); ;}
    break;

  case 28:
#line 302 "qmake.yy"
    { (yyval.value) = (yysemantic_stack_[(1) - (1)].value); ;}
    break;

  case 29:
#line 307 "qmake.yy"
    {
            (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
            (yyval.indent) = (yysemantic_stack_[(1) - (1)].value);
        ;}
    break;

  case 30:
#line 312 "qmake.yy"
    {
            (yyval.value) = QString();
            (yyval.indent) = QString();
        ;}
    break;

  case 31:
#line 318 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 32:
#line 322 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 33:
#line 326 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 34:
#line 330 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 35:
#line 334 "qmake.yy"
    {
        (yyval.value) = (yysemantic_stack_[(1) - (1)].value);
    ;}
    break;

  case 36:
#line 340 "qmake.yy"
    {
            ProjectAST *projectAST = new ProjectAST(ProjectAST::Scope);
            projects.push(projectAST);
            projects.top()->scopedID = (yysemantic_stack_[(1) - (1)].value);
            depth++;
        ;}
    break;

  case 37:
#line 347 "qmake.yy"
    {
            (yyval.node) = projects.pop();
            depth--;
        ;}
    break;

  case 38:
#line 354 "qmake.yy"
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

  case 39:
#line 372 "qmake.yy"
    {
            (yyval.node) = projects.pop();
            depth--;
        ;}
    break;

  case 40:
#line 378 "qmake.yy"
    { (yyval.value) = (yysemantic_stack_[(1) - (1)].value); ;}
    break;

  case 41:
#line 379 "qmake.yy"
    { (yyval.value) = ""; ;}
    break;

  case 43:
#line 384 "qmake.yy"
    {
            projects.top()->addChildAST((yysemantic_stack_[(2) - (2)].node));
            (yysemantic_stack_[(2) - (2)].node)->setDepth(depth);
        ;}
    break;

  case 45:
#line 392 "qmake.yy"
    {
            ProjectAST *projectAST = new ProjectAST(ProjectAST::FunctionScope);
            projects.push(projectAST);
            projects.top()->scopedID = "else";
            projects.top()->args = "";
            depth++;
        ;}
    break;

  case 46:
#line 400 "qmake.yy"
    {
            (yyval.node) = projects.pop();
            depth--;
        ;}
    break;

  case 47:
#line 405 "qmake.yy"
    {
            (yyval.node) = new ProjectAST();
        ;}
    break;

  case 48:
#line 411 "qmake.yy"
    {
            CommentAST *node = new CommentAST();
            node->comment = (yysemantic_stack_[(1) - (1)].value);
            (yyval.node) = node;
        ;}
    break;


    /* Line 675 of lalr1.cc.  */
#line 756 "qmake_yacc.cpp"
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
  const signed char Parser::yypact_ninf_ = -44;
  const signed char
  Parser::yypact_[] =
  {
       -44,    19,   -44,   -44,     8,    45,   -44,   -44,   -44,   -44,
     -44,   -44,   -44,   -44,   -44,   -44,   -44,   -44,   -44,    12,
      10,    31,   -44,    24,   -44,   -15,    -5,     8,   -44,   -44,
     -44,   -44,   -44,    26,   -44,    14,   -44,   -44,   -44,   -44,
     -44,   -44,   -44,   -44,     0,    31,   -44,   -44,   -44,    18,
      18,   -44,   -44,    20,    26,    26,    27,   -44,   -44,   -44,
     -44,    31,    39,   -44
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
         2,     0,     5,     1,     3,    36,    49,    48,     4,     6,
       7,     8,     9,    10,    31,    32,    33,    34,    35,    41,
      30,    44,    40,     0,    29,    30,     0,     0,     5,    37,
      38,    28,    27,    29,    23,     0,    15,    17,    21,    20,
      16,    26,    19,    43,     0,    44,    22,    11,    18,    13,
      14,    12,    42,    47,     0,     0,     0,    39,    24,    25,
      45,    44,     0,    46
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -44,   -44,   -44,     7,    29,   -44,   -32,   -44,    22,   -25,
     -44,   -44,   -44,   -44,   -44,   -44,   -43,   -44,   -44,   -44,
     -44
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,     1,     2,     4,     8,     9,    41,    25,    42,    26,
      20,    10,    21,    11,    45,    23,    29,    57,    61,    12,
      13
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char Parser::yytable_ninf_ = -31;
  const signed char
  Parser::yytable_[] =
  {
        35,    46,    53,     5,    31,    32,    33,    36,    37,    38,
      39,     5,     6,     7,    31,    32,    52,    40,    62,     3,
       6,     7,    58,    59,    54,    55,    47,    48,    49,    50,
      22,    24,   -30,   -30,    30,    44,    51,   -30,   -30,    24,
      38,    39,    27,    56,    60,    31,    32,    34,    28,    14,
      15,    16,    17,    18,    19,    63,    43
  };

  /* YYCHECK.  */
  const unsigned char
  Parser::yycheck_[] =
  {
        25,    33,    45,     3,    19,    20,    21,    12,    13,    14,
      15,     3,    12,    13,    19,    20,    16,    22,    61,     0,
      12,    13,    54,    55,    49,    50,    12,    13,    14,    15,
      18,    21,    14,    15,    10,    28,    22,    19,    20,    21,
      14,    15,    11,    23,    17,    19,    20,    25,    17,     4,
       5,     6,     7,     8,     9,    16,    27
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    25,    26,     0,    27,     3,    12,    13,    28,    29,
      35,    37,    43,    44,     4,     5,     6,     7,     8,     9,
      34,    36,    18,    39,    21,    31,    33,    11,    17,    40,
      10,    19,    20,    21,    32,    33,    12,    13,    14,    15,
      22,    30,    32,    28,    27,    38,    30,    12,    13,    14,
      15,    22,    16,    40,    33,    33,    23,    41,    30,    30,
      17,    42,    40,    16
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    24,    26,    25,    27,    27,    28,    28,    28,    28,
      28,    29,    29,    29,    29,    29,    29,    29,    29,    30,
      30,    30,    31,    31,    31,    31,    31,    32,    32,    33,
      33,    34,    34,    34,    34,    34,    36,    35,    38,    37,
      39,    39,    40,    40,    40,    42,    41,    41,    43,    44
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     5,     5,     5,     5,     4,     4,     4,     5,     1,
       1,     1,     3,     2,     5,     5,     2,     1,     1,     1,
       0,     1,     1,     1,     1,     1,     0,     3,     0,     7,
       1,     0,     3,     2,     0,     0,     5,     0,     1,     1
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
  "QUOTED_VARIABLE_VALUE", "VARIABLE_VALUE", "LIST_WS", "ENDOFFILE",
  "\"else\"", "$accept", "project", "@1", "statements", "statement",
  "variable_assignment", "possible_value", "multiline_values",
  "variable_value", "listws", "operator", "scope", "@2", "function_call",
  "@3", "function_args", "scope_body", "else_statement", "@4", "comment",
  "emptyline", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
        25,     0,    -1,    -1,    26,    27,    -1,    27,    28,    -1,
      -1,    29,    -1,    35,    -1,    37,    -1,    43,    -1,    44,
      -1,     3,    34,    31,    33,    12,    -1,     3,    34,    31,
      33,    22,    -1,     3,    34,    31,    33,    14,    -1,     3,
      34,    31,    33,    15,    -1,     3,    34,    33,    12,    -1,
       3,    34,    33,    22,    -1,     3,    34,    33,    13,    -1,
       3,    34,    31,    33,    13,    -1,    32,    -1,    15,    -1,
      14,    -1,    31,    21,    30,    -1,    31,    32,    -1,    31,
      33,    14,    33,    30,    -1,    31,    33,    15,    33,    30,
      -1,    33,    30,    -1,    20,    -1,    19,    -1,    21,    -1,
      -1,     4,    -1,     5,    -1,     6,    -1,     7,    -1,     8,
      -1,    -1,     3,    36,    40,    -1,    -1,     3,     9,    39,
      10,    38,    40,    41,    -1,    18,    -1,    -1,    17,    27,
      16,    -1,    11,    28,    -1,    -1,    -1,    23,    17,    42,
      40,    16,    -1,    -1,    13,    -1,    12,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  Parser::yyprhs_[] =
  {
         0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    27,    33,    39,    45,    50,    55,    60,    66,
      68,    70,    72,    76,    79,    85,    91,    94,    96,    98,
     100,   101,   103,   105,   107,   109,   111,   112,   116,   117,
     125,   127,   128,   132,   135,   136,   137,   143,   144,   146
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,   144,   144,   144,   151,   156,   159,   163,   167,   171,
     175,   181,   192,   202,   213,   224,   233,   241,   250,   263,
     263,   263,   265,   270,   274,   283,   292,   301,   302,   306,
     312,   317,   321,   325,   329,   333,   340,   339,   354,   353,
     378,   379,   382,   383,   388,   392,   391,   405,   410,   418
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
      15,    16,    17,    18,    19,    20,    21,    22,    23
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int Parser::yyeof_ = 0;
  const int Parser::yylast_ = 56;
  const int Parser::yynnts_ = 21;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 3;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 24;

  const unsigned int Parser::yyuser_token_number_max_ = 278;
  const Parser::token_number_type Parser::yyundef_token_ = 2;

} // namespace QMake

#line 421 "qmake.yy"



namespace QMake
{
    void Parser::error(const location_type& /*l*/, const std::string& m)
    {
        std::cerr << m << std::endl;
    }
}

