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


  int cont=0;           /* Continue parsing defines. */
  int line_num=1;       /* Counter for the current line being parsed         */
  int comment_start=0;  /* Line where a comment starts (for nested comments) */

  /* Function prototypes */
  char *return_id(char *yytext_id);
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
%x comment
%x define
%%
    /*==============> Strip whitespace and tabs <==============*/
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

    /*==============> Ignored stuff <==============*/
"Q_EXPORT"       {}
"Q_EXPORTH"      {}
"__BEGIN_DECLS"  {}
"__END_DECLS"    {}
"inline"         {}

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
{NUM}        {return STRING;}

    /*==============> Inrease the line counter at CR <==============*/
"\n"         {line_num++;}
 
    /*==============> Skip defines <==============*/   
"#"[acdeimpsuw ]       BEGIN(define);
<define>[^\\\n]*       
<define><<EOF>>        yyterminate();
<define>"\\"           {cont=1;}
<define>"\n"           {if(cont==1) 
                          cont=0;
                        else 
                          BEGIN(INITIAL);}
 
    /*==============> Strip // comments <==============*/
"//"(.)*[\n<<EOF>>] {line_num++;}

    /*==============> Strip comments <==============*/
"/*"                   BEGIN(comment);
<comment>[^*\n]*        /* eat anything that's not a '*' */ 
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */ 
<comment>\n             ++line_num;
<comment>"*"+"/"        BEGIN(INITIAL);

    /*==============> All other characters are ignored <==============*/
.            {}

%%
