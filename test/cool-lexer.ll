
%{
#include "cool.h"

extern std::size_t _M_token_begin, _M_token_end;
extern char *_G_contents;
extern std::size_t _G_current_offset;

#define YY_INPUT(buf, result, max_size) \
 do \
   { \
     int c = _G_contents[_G_current_offset++]; \
     result = c == 0 ? YY_NULL : (buf[0] = c, 1); \
   } \
 while (0)

#define YY_USER_INIT \
  _M_token_begin = _M_token_end = 0; \
  _G_current_offset = 0;

#define YY_USER_ACTION \
  _M_token_begin = _M_token_end; \
  _M_token_end += yyleng;
%}

%x IN_STRING
%x IN_COMMENT

%%

[ \t\r\n]+  /* skip */ ;
"--".*      /* skip */ ;

"@" return cool::Token_AT;
"case" return cool::Token_CASE;
"esac" return cool::Token_ESAC;
"class" return cool::Token_CLASS;
"Class" return cool::Token_CLASS;
":" return cool::Token_COLON;
"," return cool::Token_COMMA;
"/" return cool::Token_DIVIDE;
"." return cool::Token_DOT;
"else" return cool::Token_ELSE;
"=" return cool::Token_EQUAL;
"false" return cool::Token_FALSE;
"if" return cool::Token_IF;
"in" return cool::Token_IN;
"fi" return cool::Token_FI;
"inherits" return cool::Token_INHERITS;
"isvoid" return cool::Token_ISVOID;
"{" return cool::Token_LBRACE;
"<-" return cool::Token_LEFT_ARROW;
"<" return cool::Token_LESS;
"<=" return cool::Token_LESS_EQUAL;
"let" return cool::Token_LET;
"loop" return cool::Token_LOOP;
"(" return cool::Token_LPAREN;
"-" return cool::Token_MINUS;
"new" return cool::Token_NEW;
"not" return cool::Token_NOT;
"of" return cool::Token_OF;
"+" return cool::Token_PLUS;
"pool" return cool::Token_POOL;
"}" return cool::Token_RBRACE;
"=>" return cool::Token_RIGHT_ARROW;
")" return cool::Token_RPAREN;
";" return cool::Token_SEMICOLON;
"*" return cool::Token_STAR;
"then" return cool::Token_THEN;
"~" return cool::Token_TILDE;
"true" return cool::Token_TRUE;
"while" return cool::Token_WHILE;

[A-Z][a-zA-Z0-9_]* return cool::Token_TYPE;
[a-z_][a-zA-Z0-9_]* return cool::Token_ID;
[0-9]+ return cool::Token_INTEGER;

"(*"                          BEGIN(IN_COMMENT);
<IN_COMMENT>"*)"              { BEGIN(INITIAL); }
<IN_COMMENT>\n                /*advance*/ ;
<IN_COMMENT>.                 /*advance*/ ;

"\""([^"\\]|\\.)*"\""          { return cool::Token_STRING; }

. return yytext[0];

%%

int yywrap() { return 1; }
