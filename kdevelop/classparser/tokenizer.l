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
%{
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <ctype.h>
  #include "tokenizer.h"
  #include "FlexLexer.h"

  class MyLexer : yyFlexLexer
  {
    public:
    int yyinput() { return yyFlexLexer::yyinput(); }
  };

  int cont=0;           /* Continue parsing defines. */
  int line_num=1;       /* Counter for the current line being parsed         */
  int comment_start=0;  /* Line where a comment starts (for nested comments) */

  /* Function prototypes */
  void eatPreparser(MyLexer *lex);
  void eatComment(MyLexer *lex);
%}

WS           [ \t\r]+
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
%%
    /*==============> Strip whitespace <==============*/
{WS}         {}

    /*==============> Special keyword <==============*/
"class"         {return CPCLASS;}
"public"        {return PUBLIC;}
"protected"     {return PROTECTED;}
"private"       {return PRIVATE;}
"const"         {return CONST;}
"static"        {return STATIC;}
"virtual"       {return CPVIRTUAL;}
"signals"       {return QTSIGNAL;}
"slots"         {return QTSLOT;}
"enum"          {return CPENUM;}
"typedef"       {return CPTYPEDEF;}
"friend"        {return CPFRIEND;}
"struct"        {return CPSTRUCT;}
"union"         {return CPUNION;}
"operator"      {return CPOPERATOR;}
"template"      {return CPTEMPLATE;}
"throw"         {return CPTHROW;}

    /*==============> Ignored stuff <==============*/
"Q_EXPORT"       {}
"Q_EXPORTH"      {}
"__BEGIN_DECLS"  {}
"__END_DECLS"    {}
"inline"         {}
"volatile"       {}

    /*==============> String <==============*/
"L"?["]{SCHAR}*["]     {return STRING;}
"L"?[']{CCHAR}+[']    {return STRING;}

    /*==============> Normal operators <==============*/
":"          {return ':';}
"::"         {return CLCL;}
"~"          {return '~';}
"="          {return '=';}
"<"          {return '<';}
">"          {return '>';}
"&"          {return '&';}
"+"          {return '+';}
"-"          {return '-';}
"*"          {return '*';}
"/"          {return '/';}
"%"          {return '%';}
"|"          {return '|';}
"^"          {return '^';}

    /*==============> End of Phrase <==============*/
";"          {return ';';}

    /*==============> Comma <==============*/
","          {return ',';}

    /*==============> Paranthesis <==============*/
"("          {return '(';}
")"          {return ')';}

    /*==============> Brackets <==============*/
"{"          {return '{';}
"}"          {return '}';}
"["          {return '[';}
"]"          {return ']';}

    /*==============> Identifiers <==============*/
{ID}         {return ID;}

    /*==============> Numbers <==============*/
{NUM}        {return NUM;}

    /*==============> Inrease the line counter at CR <==============*/
"\n"         {line_num++;}
 
    /*==============> Skip preprocessor directives <==============*/   
"#"[acdeimpsuw ]     eatPreparser((MyLexer *)this);
 
    /*==============> Strip // comments <==============*/
"//"(.)*[\n<<EOF>>] {line_num++;}

    /*==============> Strip comments <==============*/
"/*"                 eatComment((MyLexer *)this);

    /*==============> All other characters are ignored <==============*/
.            {}

%%

void eatPreparser(MyLexer *lex)
{
  char c='\0'; /* The current character from the input stream */
  bool foundSlash=false;
  bool cont=false;
  bool done=false;

  while ( !done )
  {
    c=lex->yyinput();

    switch( c )
    {
      case '\\':
        // Take care of escaped '\'
        cont = !cont;
        break;
      case '\n':
        done = !cont;
        cont = false;
        break;
      case '/':
        foundSlash = true;
        break;
      case '*':
        if( foundSlash )
          eatComment(lex);

        foundSlash = false;
        break;
      case EOF:
        done = 1;
      default:
        foundSlash = false;
        break;
    }
  }
}

void eatComment(MyLexer *lex)
{
  bool done = false;  /* Tells when to exit the loop */
  bool asteriscFound = false; /* Flags that we have found an asterisc. */
  char c='\0'; /* The current character from the input stream */
 
  while ( !done )
  {
    c=lex->yyinput();

    switch( c )
    {
      case '*':
        asteriscFound = 1;
        break;
      case '/':
        done = asteriscFound;
        asteriscFound = 0;
        break;
      case EOF:
        done = 1;
        break;
      default:
        asteriscFound = 0;
        break;
    }
  }
}
