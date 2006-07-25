%{
/*****************************************************************************
 * This file is part of KDevelop.                                            *
 * Copyright (c) 2005, 2006 Jakob Petsovits <jpetso@gmx.at>                  *
 *                                                                           *
 * This program is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This grammar is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/


#include "java.h"

#include <iostream>

/* call this before calling yylex(): */
void lexer_restart(java::parser* parser);

extern std::size_t _G_token_begin, _G_token_end;
extern char *_G_contents;



/* the rest of these declarations are internal to the lexer,
 * don't use them outside of this file. */

std::size_t _G_current_offset;
java::parser* _G_parser;


#define YY_INPUT(buf, result, max_size) \
  { \
    int c = _G_contents[_G_current_offset++]; \
    result = c == 0 ? YY_NULL : (buf[0] = c, 1); \
  }

#define YY_USER_INIT \
_G_token_begin = _G_token_end = 0; \
_G_current_offset = 0; \
\
unsigned char *contents = (unsigned char *) _G_contents; \
if (contents[0] == 0xEF && contents[1] == 0xBB && contents[2] == 0xBF) { \
  _G_token_begin = _G_token_end = 3; \
  _G_current_offset = 3; \
} // check for and ignore the UTF-8 byte order mark

#define YY_USER_ACTION \
_G_token_begin = _G_token_end; \
_G_token_end += yyleng;

%}


 /* UTF-8 sequences, generated with the Unicode.hs script from
  * http://lists.gnu.org/archive/html/help-flex/2005-01/msg00043.html */

 /* \u0024, \u0041-\u005a, \u005f, \u0061-\u007a: one byte in UTF-8 */
Letter1         [A-Za-z_$]
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

Letter          {Letter1}|{Letter2}|{Letter3}|{Letter4}|{Letter5}|{Letter6}|{Letter7}|{Letter8}

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
 /* Any multi-byte Unicode character. Single-byte ones are just . in lex. */
Multibyte       {Multibyte1}|{Multibyte2}|{Multibyte3}|{Multibyte4}


 /* non-Unicode stuff */

HexDigit        [0-9a-fA-F]
Digit           {Digit1}|{Digit2}|{Digit3}|{Digit4}|{Digit5}|{Digit6}|{Digit7}|{Digit8}|{Digit9}
OctalDigit      [0-7]
NonZeroDigit    [1-9]

UnicodeEscape   [\\][u]+{HexDigit}{HexDigit}{HexDigit}{HexDigit}
OctalEscape     [\\]{OctalDigit}({Digit}({Digit})?)?
SimpleEscape    [\\]([']|["]|[\\]|[rnbft])
Escape          {SimpleEscape}|{UnicodeEscape}|{OctalEscape}

IntSuffix       [Ll]
DecimalNum      ([0]|{NonZeroDigit}{Digit}*){IntSuffix}?
OctalNum        [0]{OctalDigit}+{IntSuffix}?
HexNum          [0][xX]{HexDigit}+{IntSuffix}?
IntegerLiteral  {DecimalNum}|{OctalNum}|{HexNum}

Sign            [+-]
FloatSuffix     [fF]|[dD]
SignedInt       {Sign}?{Digit}+
DecimalExponent [eE]{SignedInt}?
BinaryExponent  [pP]{SignedInt}?
Float1          {Digit}+[\.]{Digit}*{DecimalExponent}?{FloatSuffix}?
Float2          [\.]{Digit}+{DecimalExponent}?{FloatSuffix}?
Float3          {Digit}+{DecimalExponent}{FloatSuffix}?
Float4          {Digit}+{DecimalExponent}?{FloatSuffix}
HexFloatNum     [0][xX]{HexDigit}*[\.]{HexDigit}+
HexFloat1       {HexNum}[\.]?{BinaryExponent}{FloatSuffix}?
HexFloat2       {HexFloatNum}{BinaryExponent}{FloatSuffix}?
FloatingPoint   {Float1}|{Float2}|{Float3}|{Float4}|{HexFloat1}|{HexFloat2}


%x IN_BLOCKCOMMENT

%%

 /* whitespace, newlines and comments */

[ \f\t]+        /* skip */ ;
"\r\n"|\r|\n    /* { newLine(); } */ ;

"//"[^\r\n]*    /* line comments, skip */ ;

"/*"            BEGIN(IN_BLOCKCOMMENT);
<IN_BLOCKCOMMENT>{
[^*\r\n]*       /* eat anything that's not a '*' */ ;
"*"+[^*/\r\n]*  /* eat up '*'s that are not followed by slashes or newlines */;
"\r\n"|\r|\n    /* { newLine(); } */ ;
"*"+"/"         BEGIN(INITIAL);
<<EOF>> {
    _G_parser->report_problem( java::parser::error,
      "Encountered end of file in an unclosed block comment");
    return java::parser::Token_EOF;
}
}


 /* seperators */

"("             return java::parser::Token_LPAREN;
")"             return java::parser::Token_RPAREN;
"{"             return java::parser::Token_LBRACE;
"}"             return java::parser::Token_RBRACE;
"["             return java::parser::Token_LBRACKET;
"]"             return java::parser::Token_RBRACKET;
","             return java::parser::Token_COMMA;
";"             return java::parser::Token_SEMICOLON;
"."             return java::parser::Token_DOT;
"@"             {
    if (_G_parser->compatibility_mode() >= java::parser::java15_compatibility)
      return java::parser::Token_AT;
    else {
      _G_parser->report_problem( java::parser::error,
        "Annotations are not supported by Java 1.4 or earlier");
      return java::parser::Token_INVALID;
    }
}


 /* operators */

"?"             return java::parser::Token_QUESTION;
":"             return java::parser::Token_COLON;
"!"             return java::parser::Token_BANG;
"~"             return java::parser::Token_TILDE;
"=="            return java::parser::Token_EQUAL;
"<"             return java::parser::Token_LESS_THAN;
"<="            return java::parser::Token_LESS_EQUAL;
">"             return java::parser::Token_GREATER_THAN;
">="            return java::parser::Token_GREATER_EQUAL;
"!="            return java::parser::Token_NOT_EQUAL;
"&&"            return java::parser::Token_LOG_AND;
"||"            return java::parser::Token_LOG_OR;
"++"            return java::parser::Token_INCREMENT;
"--"            return java::parser::Token_DECREMENT;
"="             return java::parser::Token_ASSIGN;
"+"             return java::parser::Token_PLUS;
"+="            return java::parser::Token_PLUS_ASSIGN;
"-"             return java::parser::Token_MINUS;
"-="            return java::parser::Token_MINUS_ASSIGN;
"*"             return java::parser::Token_STAR;
"*="            return java::parser::Token_STAR_ASSIGN;
"/"             return java::parser::Token_SLASH;
"/="            return java::parser::Token_SLASH_ASSIGN;
"&"             return java::parser::Token_BIT_AND;
"&="            return java::parser::Token_BIT_AND_ASSIGN;
"|"             return java::parser::Token_BIT_OR;
"|="            return java::parser::Token_BIT_OR_ASSIGN;
"^"             return java::parser::Token_BIT_XOR;
"^="            return java::parser::Token_BIT_XOR_ASSIGN;
"%"             return java::parser::Token_REMAINDER;
"%="            return java::parser::Token_REMAINDER_ASSIGN;
"<<"            return java::parser::Token_LSHIFT;
"<<="           return java::parser::Token_LSHIFT_ASSIGN;
">>"            return java::parser::Token_SIGNED_RSHIFT;
">>="           return java::parser::Token_SIGNED_RSHIFT_ASSIGN;
">>>"           return java::parser::Token_UNSIGNED_RSHIFT;
">>>="          return java::parser::Token_UNSIGNED_RSHIFT_ASSIGN;
"..."           {
    if (_G_parser->compatibility_mode() >= java::parser::java15_compatibility)
      return java::parser::Token_ELLIPSIS;
    else {
      _G_parser->report_problem( java::parser::error,
        "Variable-length argument lists are not supported by Java 1.4 or earlier");
      return java::parser::Token_INVALID;
    }
}


 /* reserved words */

"abstract"      return java::parser::Token_ABSTRACT;
"assert"        {
    if (_G_parser->compatibility_mode() >= java::parser::java14_compatibility)
      return java::parser::Token_ASSERT;
    else
      return java::parser::Token_IDENTIFIER;
}
"boolean"       return java::parser::Token_BOOLEAN;
"break"         return java::parser::Token_BREAK;
"byte"          return java::parser::Token_BYTE;
"case"          return java::parser::Token_CASE;
"catch"         return java::parser::Token_CATCH;
"char"          return java::parser::Token_CHAR;
"class"         return java::parser::Token_CLASS;
"const"         {
    _G_parser->report_problem( java::parser::error,
      "\"const\": reserved but unused (invalid) keyword");
    return java::parser::Token_CONST;
}
"continue"      return java::parser::Token_CONTINUE;
"default"       return java::parser::Token_DEFAULT;
"do"            return java::parser::Token_DO;
"double"        return java::parser::Token_DOUBLE;
"else"          return java::parser::Token_ELSE;
"enum"          {
    if (_G_parser->compatibility_mode() >= java::parser::java15_compatibility)
      return java::parser::Token_ENUM;
    else
      return java::parser::Token_IDENTIFIER;
}
"extends"       return java::parser::Token_EXTENDS;
"false"         return java::parser::Token_FALSE;
"final"         return java::parser::Token_FINAL;
"finally"       return java::parser::Token_FINALLY;
"float"         return java::parser::Token_FLOAT;
"for"           return java::parser::Token_FOR;
"goto"          {
    _G_parser->report_problem( java::parser::error,
      "\"goto\": reserved but unused (invalid) keyword");
    return java::parser::Token_GOTO;
}
"if"            return java::parser::Token_IF;
"implements"    return java::parser::Token_IMPLEMENTS;
"import"        return java::parser::Token_IMPORT;
"instanceof"    return java::parser::Token_INSTANCEOF;
"int"           return java::parser::Token_INT;
"interface"     return java::parser::Token_INTERFACE;
"long"          return java::parser::Token_LONG;
"native"        return java::parser::Token_NATIVE;
"new"           return java::parser::Token_NEW;
"null"          return java::parser::Token_NULL;
"package"       return java::parser::Token_PACKAGE;
"private"       return java::parser::Token_PRIVATE;
"protected"     return java::parser::Token_PROTECTED;
"public"        return java::parser::Token_PUBLIC;
"return"        return java::parser::Token_RETURN;
"short"         return java::parser::Token_SHORT;
"static"        return java::parser::Token_STATIC;
"strictfp"      return java::parser::Token_STRICTFP;
"super"         return java::parser::Token_SUPER;
"switch"        return java::parser::Token_SWITCH;
"synchronized"  return java::parser::Token_SYNCHRONIZED;
"this"          return java::parser::Token_THIS;
"throw"         return java::parser::Token_THROW;
"throws"        return java::parser::Token_THROWS;
"transient"     return java::parser::Token_TRANSIENT;
"true"          return java::parser::Token_TRUE;
"try"           return java::parser::Token_TRY;
"void"          return java::parser::Token_VOID;
"volatile"      return java::parser::Token_VOLATILE;
"while"         return java::parser::Token_WHILE;


 /* characters and strings */

[']({Escape}|{Multibyte}|[^\\\r\n\'])[']   return java::parser::Token_CHARACTER_LITERAL;
[']({Escape}|{Multibyte}|[\\][^\\\r\n\']|[^\\\r\n\'])*([\\]?[\r\n]|[']) {
    _G_parser->report_problem( java::parser::error,
      std::string("Invalid character literal: ") + yytext );
    return java::parser::Token_CHARACTER_LITERAL;
}

["]({Escape}|{Multibyte}|[^\\\r\n\"])*["]  return java::parser::Token_STRING_LITERAL;
["]({Escape}|{Multibyte}|[\\][^\\\r\n\"]|[^\\\r\n\"])*([\\]?[\r\n]|["]) {
    _G_parser->report_problem( java::parser::error,
      std::string("Invalid string literal: ") + yytext );
    return java::parser::Token_STRING_LITERAL;
}


 /* identifiers and number literals */

{Letter}({Letter}|{Digit})*  return java::parser::Token_IDENTIFIER;

{IntegerLiteral}   return java::parser::Token_INTEGER_LITERAL;
{FloatingPoint}    return java::parser::Token_FLOATING_POINT_LITERAL;


 /* everything else is not a valid lexeme */

.               return java::parser::Token_INVALID;

%%

void lexer_restart(java::parser* _parser) {
  _G_parser = _parser;
  yyrestart(NULL);
  BEGIN(INITIAL); // is not set automatically by yyrestart()
  YY_USER_INIT
}

int yywrap() { return 1; }
