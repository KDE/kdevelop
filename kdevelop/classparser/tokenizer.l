/********************************************************************
* Name    : Lexical analysis                                        *
* ------------------------------------------------------------------*
* File    : tokenizer.l                                             *
* Author  : Jonas Nordin                                            *
* Date    : Mon Mar 15 15:13:45 CET 1999                            *
*                                                                   *
* ------------------------------------------------------------------*
* Purpose : Defining a tokinzer/lexical analyser with errorhandler. *
*           Removes all comments and handles includes.              *
*                                                                   *
* ------------------------------------------------------------------*
* Modifications:                                                    *
*                                                                   *
* ------------------------------------------------------------------*
*********************************************************************/
%Start COMMENT PREPARSER PPCOMMENT

%{
  #include <stdlib.h>
  #include <stdio.h>
  #include <ctype.h>
  #include <qstring.h>
  #include "tokenizer.h"
  #include "FlexLexer.h"

  class MyLexer : public yyFlexLexer
  {
    public:
    char *gettext() {return yytext;};
  };

  /* Line where a comment starts. */
  int comment_start = -1;

  /* Line where a comment ends. */
  int comment_end = -1;

  /* Last parsed comment. */
  QString comment;

  /* Function prototypes */
  bool beginComment(MyLexer *lex);
  void inComment(MyLexer *lex);
  bool endComment(MyLexer *lex);
%}

WS           [[:blank:]\r]+
LETTER       [A-Za-z_~]
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
<INITIAL,PREPARSER>{WS}         {}

    /*==============> Special keyword <==============*/
<INITIAL>"class"         {return CPCLASS;}
<INITIAL>"public"        {return PUBLIC;}
<INITIAL>"protected"     {return PROTECTED;}
<INITIAL>"private"       {return PRIVATE;}
<INITIAL>"const"         {return CONST;}
<INITIAL>"static"        {return STATIC;}
<INITIAL>"virtual"       {return CPVIRTUAL;}
<INITIAL>"signals"       {return QTSIGNAL;}
<INITIAL>"slots"         {return QTSLOT;}
<INITIAL>"enum"          {return CPENUM;}
<INITIAL>"typedef"       {return CPTYPEDEF;}
<INITIAL>"friend"        {return CPFRIEND;}
<INITIAL>"struct"        {return CPSTRUCT;}
<INITIAL>"union"         {return CPUNION;}
<INITIAL>"operator"      {return CPOPERATOR;}
<INITIAL>"template"      {return CPTEMPLATE;}
<INITIAL>"throw"         {return CPTHROW;}

    /*==============> Ignored stuff <==============*/
<INITIAL>"Q_EXPORT"       {}
<INITIAL>"Q_EXPORTH"      {}
<INITIAL>"__BEGIN_DECLS"  {}
<INITIAL>"__END_DECLS"    {}
<INITIAL>"inline"         {}
<INITIAL>"volatile"       {}
<INITIAL>"afx_msg"        {}

    /*==============> String <==============*/
<INITIAL>"L"?["]{SCHAR}*["]     {return STRING;}
<INITIAL>"L"?[']{CCHAR}+[']    {return STRING;}

    /*==============> Normal operators <==============*/
<INITIAL>":"          {return ':';}
<INITIAL>"::"         {return CLCL;}
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

    /*==============> Skip preprocessor directives <==============*/
<INITIAL>"#"[acdeimpsuw ]	{BEGIN (PREPARSER);}
<PREPARSER>[^\\]"\n"	{ BEGIN (0);}
<PREPARSER>"/*"		{beginComment((MyLexer *) this);
                                  BEGIN (PPCOMMENT);}
<PPCOMMENT>"*/"        { endComment((MyLexer *) this);
                       BEGIN (PREPARSER); }

    /*==============> Strip // comments <==============*/
<INITIAL,PREPARSER>"//"(.)*[\n<<EOF>>] {}

    /*==============> Strip comments <==============*/
<INITIAL>"/*"        {beginComment((MyLexer *) this);
                                  BEGIN (COMMENT);}

<COMMENT>"*/"        { endComment((MyLexer *) this);
                       BEGIN (0); }

<COMMENT,PPCOMMENT>.  {inComment((MyLexer *) this);}

    /*==============> All other characters are ignored <==============*/
<INITIAL,PREPARSER>.  {}

%%

bool beginComment(MyLexer *lex)
{
 comment_start = lex->lineno();
 comment="/*";
 return true;
}

void inComment(MyLexer *lex)
{
  char *text=lex->gettext();
  comment+=text;
};

bool endComment(MyLexer *lex)
{
 comment_end = lex->lineno();
 comment+="*/";
 return true;
}

