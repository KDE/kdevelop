%{
%{CPP_TEMPLATE}

#include "%{LANGUAGE}.h"

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

void lexer_restart(void) {
  yyrestart(NULL);
  YY_USER_INIT
}

%}
 /* This file includes some commonly-used definitions. If the language
  * that is to be lexed doesn't need those, you might just want to delete
  * the unneeded ones. */


 /* UTF-8 letters and digits, generated with the Unicode.hs script from
  * http://lists.gnu.org/archive/html/help-flex/2005-01/msg00043.html */
 /* This is the character set used in Java, but should also apply to
  * other languages using Unicode. */

 /* \u0024, \u0041-\u005a, \u005f, \u0061-\u007a: one byte in UTF-8 */
Letter1         [$A-Z_a-z]
 /* \u00c0-\u00d6, \u00d8-\u00f6, \u00f8-\u00ff */
Letter2         [\xC3]([\x80-\x96]|[\x98-\xB6]|[\xB8-\xBF])
 /* \u0100-\u1fff */
Letter3         [\xC4-\xDF][\x80-\xBF]|([\xE0][\xA0-\xBF]|[\xE1][\x80-\xBF])[\x80-\xBF]
 /* \u3040-\u318f */
Letter4         [\xE3]([\x86][\x80-\x8F]|[\x81-\x85][\x80-\xBF])
 /* \u3300-\u337f */
Letter5         [\xE3][\x8C-\x8D][\x80-\xBF]
 /* \u3400-\u3d2d */
Letter6         [\xE3](\xB4[\x80-\xAD]|[\x90-\xB3][\x80-\xBF])
 /* \u4e00-\u9fff */
Letter7         ([\xE4][\xB8-\xBF]|[\xE5-\xE9][\x80-\xBF])[\x80-\xBF]
 /* \uf900-\ufaff */
Letter8         [\xEF][\xA4-\xAB][\x80-\xBF]

 /* \u0030-\u0039: ISO-LATIN-1 digits */
Digit1          [0-9]
 /* \u0660-\u0669, \u06f0-\u06f9: Arabic-Indic and extended Ar.-Indic digits */
Digit2          [\xD9][\xA0-\xA9]|[\xDB][\xB0-\xB9]
 /* \u0966-\u096f, \u09e6-\u09ef: Devanagari digits */
Digit3          [\xE0]([\xA5]|[\xA7])[\xA6-\xAF]
 /* \u0a66-\u0a6f, \u0ae6-\u0aef */
Digit4          [\xE0]([\xA9]|[\xAB])[\xA6-\xAF]
 /* \u0b66-\u0b6f, \u0be7-\u0bef */
Digit5          [\xE0]([\xAD][\xA6-\xAF]|[\xAF][\xA7-\xAF])
 /* \u0c66-\u0c6f, \u0ce6-\u0cef, \u0d66-\u0d6f */
Digit6          [\xE0]([\xB1]|[\xB3]|[\xB5])[\xA6-\xAF]
 /* \u0e50-\u0e59, \u0ed0-\u0ed9 */
Digit7          [\xE0]([\xB9]|[\xBB])[\x90-\x99]
 /* \u1040-\u1049 */
Digit8          [\xE1][\x81][\x80-\x89]
 /* \uff10-\uff19: Fullwidth digits */
Digit9          [\xEF][\xBC][\x90-\x99]

 /* \u0080-\uffff */
Multibyte1      ([\xC2-\xDF]|[\xE0][\xA0-\xBF]|[\xE1-\xEF][\x80-\xBF])[\x80-\xBF]
 /* \u10000-\u1fffff */
Multibyte2      ([\xF0][\x90-\xBF]|[\xF1-\xF7][\x80-\xBF])[\x80-\xBF][\x80-\xBF]
 /* \u200000-\u3ffffff */
Multibyte3      ([\xF8][\x88-\xBF]|[\xF9-\xFB][\x80-\xBF])[\x80-\xBF][\x80-\xBF][\x80-\xBF]
 /* \u4000000-\u7fffffff */
Multibyte4      ([\xFC][\x84-\xBF]|[\xFD][\x80-\xBF])[\x80-\xBF][\x80-\xBF][\x80-\xBF]


 /* Any Unicode letter. */
Letter          {Letter1}|{Letter2}|{Letter3}|{Letter4}|{Letter5}|{Letter6}|{Letter7}|{Letter8}
 /* Any Unicode digit. */
Digit           {Digit1}|{Digit2}|{Digit3}|{Digit4}|{Digit5}|{Digit6}|{Digit7}|{Digit8}|{Digit9}
 /* Any multi-byte Unicode character. Single-byte ones are just . in lex. */
Multibyte       {Multibyte1}|{Multibyte2}|{Multibyte3}|{Multibyte4}


 /* Non-Unicode definitions. */

NewLine         "\r\n"|[\r]|[\n]
NonNewLine      [^\r\n]
WhiteSpace      [ \f\t\r\n]

HexDigit        [0-9a-fA-F]
OctalDigit      [0-7]
NonZeroDigit    [1-9]

DecimalNum      ([0]|{NonZeroDigit}{Digit}*)
OctalNum        [0]{OctalDigit}+
HexNum          [0]([x]|[X]){HexDigit}+
IntegerLiteral  ({DecimalNum}|{OctalNum}|{HexNum})

Escape          [\\]([r]|[n]|[b]|[f]|[t]|[\\]|[']|["])


/* Define any lexer states other than INITIAL here. */
%x IN_BLOCKCOMMENT

%%%%

 /* Here are the actual regular expressions that will be lexed. Again,
  * if they're not right for your language, just adapt or delete them. */

 /* Seperators */

"("             return %{LANGUAGE}::Token_LPAREN;
")"             return %{LANGUAGE}::Token_RPAREN;
"{"             return %{LANGUAGE}::Token_LBRACE;
"}"             return %{LANGUAGE}::Token_RBRACE;
"["             return %{LANGUAGE}::Token_LBRACKET;
"]"             return %{LANGUAGE}::Token_RBRACKET;
","             return %{LANGUAGE}::Token_COMMA;
";"             return %{LANGUAGE}::Token_SEMICOLON;
"."             return %{LANGUAGE}::Token_DOT;



 /* Operators */

"?"             return %{LANGUAGE}::Token_QUESTION;
":"             return %{LANGUAGE}::Token_COLON;
"!"             return %{LANGUAGE}::Token_BANG;
"=="            return %{LANGUAGE}::Token_EQUAL;
"<"             return %{LANGUAGE}::Token_LESS_THAN;
"<="            return %{LANGUAGE}::Token_LESS_EQUAL;
">"             return %{LANGUAGE}::Token_GREATER_THAN;
">="            return %{LANGUAGE}::Token_GREATER_EQUAL;
"!="            return %{LANGUAGE}::Token_NOT_EQUAL;
"&&"            return %{LANGUAGE}::Token_LOG_AND;
"||"            return %{LANGUAGE}::Token_LOG_OR;
"="             return %{LANGUAGE}::Token_ASSIGN;
"+"             return %{LANGUAGE}::Token_PLUS;
"-"             return %{LANGUAGE}::Token_MINUS;
"*"             return %{LANGUAGE}::Token_STAR;
"/"             return %{LANGUAGE}::Token_SLASH;
"%"             return %{LANGUAGE}::Token_REMAINDER;
"..."           return %{LANGUAGE}::Token_ELLIPSIS;



 /* Comments and whitespace: stuff to be skipped */

{WhiteSpace}        /* skip */;
{NewLine}           /* to count newlines, do something like: { newLine(); } */;

"//"{NonNewLine}*   /* line comments, skip */;

"/*"            BEGIN(IN_BLOCKCOMMENT);
<IN_BLOCKCOMMENT>{
[^*\r\n]*       /* eat anything that's not a '*' or a newline */;
"*"+[^*/\r\n]*  /* eat up '*'s that are not followed by slashes or newlines */;
{NewLine}       /* ...which enables newlines to be counted: { newLine(); } */;
"*/"            BEGIN(INITIAL);
<<EOF>> {
    // error output "Encountered end of file in an unclosed block comment...");
    BEGIN(INITIAL); // is not set automatically by yyrestart()
    return %{LANGUAGE}::Token_EOF;
}
}



 /* Reserved words */

"abstract"      return %{LANGUAGE}::Token_ABSTRACT;
"boolean"       return %{LANGUAGE}::Token_BOOLEAN;
"break"         return %{LANGUAGE}::Token_BREAK;
 /* ... */
"true"          return %{LANGUAGE}::Token_TRUE;
"void"          return %{LANGUAGE}::Token_VOID;
"while"         return %{LANGUAGE}::Token_WHILE;
 /* ...or whatever you need. */



 /* Literals */

 /* Character and string literals, Unicode version.
  * Including error reports on invalid ones.
  * Admittedly, it's a little confusing, but it works perfectly. */

 /* This recognizes one character inside single quotes. */
[\']({Escape}|{Multibyte}|[^\'\r\n])[\']  return %{LANGUAGE}::Token_CHARACTER_LITERAL;
[\']({Escape}|{Multibyte}|[\\][^\'\r\n\\]|[^\'\r\n\\])*(([\\]?([\r]|[\n]))|[\']) {
    // error output "Invalid character literal: " + yytext;
    return %{LANGUAGE}::Token_CHARACTER_LITERAL;
}

 /* This recognizes any amount of characters inside double quotes. */
[\"]({Escape}|{Multibyte}|[^\"\r\n\\])*[\"]  return %{LANGUAGE}::Token_STRING_LITERAL;
[\"]({Escape}|{Multibyte}|[\\][^\"\r\n\\]|[^\"\r\n\\])*(([\\]?([\r]|[\n]))|[\"]) {
    // error output "Invalid string literal: " + yytext;
    return %{LANGUAGE}::Token_STRING_LITERAL;
}


 /* Character and string literals, non-Unicode version.
  * You can take these instead of the above if you need to. */
 /*
[\']({Escape}|[^\'\r\n])[\']  return %{LANGUAGE}::Token_CHARACTER_LITERAL;
[\']({Escape}|[\\][^\'\r\n\\]|[^\'\r\n\\])*(([\\]?([\r]|[\n]))|[\']) {
    // error output "Invalid character literal: " + yytext;
    return %{LANGUAGE}::Token_CHARACTER_LITERAL;
}

[\"]({Escape}|[^\"\r\n\\])*[\"]  return %{LANGUAGE}::Token_STRING_LITERAL;
[\"]({Escape}|[\\][^\"\r\n\\]|[^\"\r\n\\])*(([\\]?([\r]|[\n]))|[\"]) {
    // error output "Invalid string literal: " + yytext;
    return %{LANGUAGE}::Token_STRING_LITERAL;
}
 */

{IntegerLiteral}             return %{LANGUAGE}::Token_INTEGER_LITERAL;
 /* Floats are too complex and quite individual to the language.
    Wouldn't have been a good idea to include them in this template.
{FloatingPoint}              return %{LANGUAGE}::Token_FLOATING_POINT_LITERAL;
 */



 /* Identifiers */

{Letter}({Letter}|{Digit})*  return %{LANGUAGE}::Token_IDENTIFIER;



 /* Everything that is not handled up to now is not part of the language. */
.               return %{LANGUAGE}::Token_INVALID;

%%%%

int yywrap() { return 1; }

