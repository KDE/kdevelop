/***************************************************************************
                          tokenizer.l  -  Derived from the C++ tokenizer
                          					in the cppsupport KDevelop part
                          					by Jonas Nordin
                             -------------------
    begin                : Wed Dec 12 2000
    copyright            : (C) 2000 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
%Start COMMENT

%{
  #include <stdlib.h>
  #include <qstring.h>
  #include <iostream.h>
  #include "tokenizer.h"
  #include "FlexLexer.h"

  /** This class is a stupid trick to make it possible 
   * to access the getText method. */
  class MyLexer : public yyFlexLexer
  {
    public:
    char *getText() {return yytext;};
  };

  /* Line where a comment starts. */
  int comment_start = -1;

  /* Line where a comment ends. */
  int comment_end = -1;

  /* Last parsed comment. */
  QString comment;

  /* Function prototypes */
  static bool beginComment(MyLexer *lex);
  static void inComment(MyLexer *lex);
  static bool endComment(MyLexer *lex);
%}

WS           [[:blank:]\r]+
LETTER       [A-Za-z_~$\xc0-\xd6\xd8-\xf6\xf8-\xff]
DIGIT        [0-9]
NUM          {DIGIT}+
ID           {LETTER}+({LETTER}|{DIGIT})*

SIMPLE_ESCAPE [abfnrtv'"?\\]
OCTAL_ESCAPE  [0-7]{1,3}
HEX_ESCAPE "x"[0-9a-fA-F]+

ESCAPE_SEQUENCE [\\]({SIMPLE_ESCAPE}|{OCTAL_ESCAPE}|{HEX_ESCAPE})
CCHAR [^'\\\n]|{ESCAPE_SEQUENCE}
SCHAR [^"\\\n]|{ESCAPE_SEQUENCE}

%option yylineno
%option C++ yymore

%%
    /*==============> Strip whitespace <==============*/
<INITIAL>{WS}         {}

    /*==============> Special keyword <==============*/
<INITIAL>"package"       {return JAVAPACKAGE;}
<INITIAL>"class"         {return JAVACLASS;}
<INITIAL>"interface"     {return JAVAINTERFACE;}
<INITIAL>"extends"       {return JAVAEXTENDS;}
<INITIAL>"implements"    {return JAVAIMPLEMENTS;}
<INITIAL>"import"    	 {return JAVAIMPORTS;}
<INITIAL>"public"        {return JAVAPUBLIC;}
<INITIAL>"protected"     {return JAVAPROTECTED;}
<INITIAL>"private"       {return JAVAPRIVATE;}
<INITIAL>"static"        {return JAVASTATIC;}
<INITIAL>"native"        {return JAVANATIVE;}
<INITIAL>"abstract"      {return JAVAABSTRACT;}
<INITIAL>"final"         {return JAVAFINAL;}
<INITIAL>"synchronized"  {return JAVASYNCHRONIZED;}
<INITIAL>"transient"     {return JAVATRANSIENT;}
<INITIAL>"volatile"      {return JAVAVOLATILE;}
<INITIAL>"strictfp"      {return JAVASTRICTFP;}
<INITIAL>"throws"        {return JAVATHROWS;}
<INITIAL>"throw"         {return JAVATHROW;}

    /*==============> String <==============*/
<INITIAL>"L"?["]{SCHAR}*["]     {return STRING;}
<INITIAL>"L"?[']{CCHAR}+[']    {return STRING;}

    /*==============> Normal operators <==============*/
<INITIAL>":"          {return ':';}
<INITIAL>"~"          {return '~';}
<INITIAL>"="          {return '=';}
<INITIAL>"<"          {return '<';}
<INITIAL>">"          {return '>';}
<INITIAL>"&"          {return '&';}
<INITIAL>"+"          {return '+';}
<INITIAL>"-"          {return '-';}
<INITIAL>"*"          {return '*';}
<INITIAL>"/"          {return '/';}
<INITIAL>"%"          {return '%';}
<INITIAL>"|"          {return '|';}
<INITIAL>"^"          {return '^';}
<INITIAL>"!"          {return '!';}

    /*==============> End of Phrase <==============*/
<INITIAL>";"          {return ';';}

    /*==============> Comma <==============*/
<INITIAL>","          {return ',';}

    /*==============> Paranthesis <==============*/
<INITIAL>"("          {return '(';}
<INITIAL>")"          {return ')';}

    /*==============> Brackets <==============*/
<INITIAL>"{"          {return '{';}
<INITIAL>"}"          {return '}';}
<INITIAL>"["          {return '[';}
<INITIAL>"]"          {return ']';}

    /*==============> Identifiers <==============*/
<INITIAL>{ID}         {return ID;}

    /*==============> Numbers <==============*/
<INITIAL>{NUM}        {return NUM;}


    /*==============> Strip // comments <==============*/
<INITIAL>"//"(.)*[\n<<EOF>>] {}

    /*==============> Strip comments <==============*/
<INITIAL>"/*"        {beginComment((MyLexer *) this);
                                  BEGIN (COMMENT);}

<COMMENT>"*/"        { endComment((MyLexer *) this);
                       BEGIN (0); }

<COMMENT>[\n\r]  	{inComment((MyLexer *) this);}
<COMMENT>.  		{inComment((MyLexer *) this);}

    /*==============> All other characters are ignored <==============*/
<INITIAL>[\n]  		{}
<INITIAL>.  		{}

%%

static bool beginComment(MyLexer *lex)
{
 comment_start = lex->lineno();
 comment="/*";
 return true;
}

static void inComment(MyLexer *lex)
{
  char *text=lex->getText();
  comment+=text;
};

static bool endComment(MyLexer *lex)
{
 comment_end = lex->lineno();
 comment+="*/";
 return true;
}

