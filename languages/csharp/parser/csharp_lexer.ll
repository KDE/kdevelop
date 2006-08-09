%top{
/*****************************************************************************
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
}

%option c++
%option yyclass="csharp::Lexer"
%option noyywrap


%{

#define DONT_INCLUDE_FLEXLEXER
#include "csharp_lexer.h"


#define YY_USER_ACTION \
_M_token_begin = _M_token_end; \
_M_token_end += yyleng;

// This is meant to be called with the first token in a pre-processor line.
// Pre-processing completely bypasses the normal tokenizing process.
#define PP_PROCESS_TOKEN(t) \
  { \
    csharp_pp::parser pp_parser; \
    csharp_pp::parser::pp_parse_result result = \
      pp_parser.pp_parse_line( csharp_pp::parser::Token_##t, pp_current_scope(), this ); \
    \
    if (result == csharp_pp::parser::result_eof) \
      { \
        BEGIN(INITIAL); \
        return 0; /* end of file */  \
      } \
    else if (result == csharp_pp::parser::result_invalid) \
      { \
        BEGIN(INITIAL); \
        return parser::Token_INVALID; \
      } \
    else if (result == csharp_pp::parser::result_ok) \
      { \
        if (pp_current_scope()->is_active()) \
          BEGIN(INITIAL); \
        else \
          BEGIN(PP_SKIPPED_SECTION_PART); \
      } \
  }

%}


 /* UTF-8 sequences, generated with the Unicode.hs script from
  * http://lists.gnu.org/archive/html/help-flex/2005-01/msg00043.html */

 /* 0x0041-0x005a, 0x005f, 0x0061-0x007a: one byte in UTF-8 */
Letter1         [A-Za-z_]
 /* 0x00c0-0x00d6, 0x00d8-0x00f6, 0x00f8-0x00ff */
Letter2         [\xC3]([\x80-\x96]|[\x98-\xB6]|[\xB8-\xBF])
 /* 0x0100-0x1fff */
Letter3         [\xC4-\xDF][\x80-\xBF]|([\xE0][\xA0-\xBF]|[\xE1][\x80-\xBF])[\x80-\xBF]
 /* 0x3040-0x318f */
Letter4         [\xE3]([\x86][\x80-\x8F]|[\x81-\x85][\x80-\xBF])
 /* 0x3300-0x337f */
Letter5         [\xE3][\x8C-\x8D][\x80-\xBF]
 /* 0x3400-0x3d2d */
Letter6         [\xE3](\xB4[\x80-\xAD]|[\x90-\xB3][\x80-\xBF])
 /* 0x4e00-0x9fff */
Letter7         ([\xE4][\xB8-\xBF]|[\xE5-\xE9][\x80-\xBF])[\x80-\xBF]
 /* 0xf900-0xfaff */
Letter8         [\xEF][\xA4-\xAB][\x80-\xBF]

Letter          {Letter1}|{Letter2}|{Letter3}|{Letter4}|{Letter5}|{Letter6}|{Letter7}|{Letter8}


 /* 0x0080-0xffff */
Multibyte1      ([\xC2-\xDF]|[\xE0][\xA0-\xBF]|[\xE1-\xEF][\x80-\xBF])[\x80-\xBF]
 /* 0x10000-0x10ffff */
Multibyte2      (\xF0[\x90-\xBF]|\xF4[\x80-\x8F]|[\xF1-\xF3][\x80-\xBF])[\x80-\xBF][\x80-\xBF]
 /* Unicode characters above 0x10ffff are not supported by C#. */
 /* Any multi-byte Unicode character. Single-byte ones are just . in lex. */
Multibyte       {Multibyte1}|{Multibyte2}


 /* 0x2028 and 0x2029, currently unused in this lexer
    for performance and convenience reasons. */
UnicodeNewLine  [\xE2][\x80][\xA8-\xA9]


 /* non-Unicode stuff */

Whitespace      [ \t\v\f]+
NewLine         ("\r\n"|\r|\n)
LineComment     "//"[^\r\n]*

DecimalDigit    [0-9]
HexDigit        [0-9a-fA-F]

ShortUnicodeEscape  [\\][u]{HexDigit}{HexDigit}{HexDigit}{HexDigit}
LongUnicodeEscape   [\\][U]{HexDigit}{HexDigit}{HexDigit}{HexDigit}{HexDigit}{HexDigit}{HexDigit}{HexDigit}
UnicodeEscape   {ShortUnicodeEscape}|{LongUnicodeEscape}
HexEscape       [\\][x]{HexDigit}{HexDigit}?{HexDigit}?{HexDigit}?
SimpleEscape    [\\]([']|["]|[\\]|[0abfnrtv])
Escape          ({SimpleEscape}|{UnicodeEscape}|{HexEscape})

IntegerSuffix   (([Ll][Uu]?)|([Uu][Ll]?))
DecimalLiteral  {DecimalDigit}+{IntegerSuffix}?
HexLiteral      [0][xX]{HexDigit}+{IntegerSuffix}?
IntegerLiteral  ({DecimalLiteral}|{HexLiteral})

Sign            [+-]
RealSuffix      ([fF]|[dD]|[mM])
Exponent        [eE]{Sign}?{DecimalDigit}+
Real1           {DecimalDigit}+[\.]{DecimalDigit}+{Exponent}?{RealSuffix}?
Real2           [\.]{DecimalDigit}+{Exponent}?{RealSuffix}?
Real3           {DecimalDigit}+{Exponent}{RealSuffix}?
Real4           {DecimalDigit}+{RealSuffix}
RealLiteral     ({Real1}|{Real2}|{Real3}|{Real4})
InvalidReal     {DecimalDigit}+[\.]{Exponent}?{RealSuffix}?

AvailableIdentifier {Letter}({Letter}|{DecimalDigit})*
VerbatimIdentifier  [@]{Letter}({Letter}|{DecimalDigit})*
Identifier      ({AvailableIdentifier}|{VerbatimIdentifier})

ppPrefix        ^{Whitespace}?[#]{Whitespace}?
ppNewLine       {Whitespace}?{LineComment}?{NewLine}


%x IN_BLOCKCOMMENT
%x PP_EXPECT_NEW_LINE
%x PP_DECLARATION
%x PP_IF_CLAUSE
%x PP_LINE
%x PP_MESSAGE
%x PP_PRAGMA
%x PP_SKIPPED_SECTION_PART

%%

 /* whitespace, newlines and comments */

{Whitespace}    /* skip */ ;
{NewLine}       /* { newLine(); } */ ;

{LineComment}   /* line comments, skip */ ;

"/*"            BEGIN(IN_BLOCKCOMMENT);
<IN_BLOCKCOMMENT>{
[^*\r\n]*       /* eat anything that's not a '*' */ ;
"*"+[^*/\r\n]*  /* eat up '*'s that are not followed by slashes or newlines */;
{NewLine}       /* { newLine(); } */ ;
"*"+"/"         BEGIN(INITIAL);
<<EOF>> {
    _M_parser->report_problem( parser::error,
      "Encountered end of file in an unclosed block comment" );
    cleanup();
    return parser::Token_EOF;
}
}


 /* seperators */

"("             return parser::Token_LPAREN;
")"             return parser::Token_RPAREN;
"{"             return parser::Token_LBRACE;
"}"             return parser::Token_RBRACE;
"["             return parser::Token_LBRACKET;
"]"             return parser::Token_RBRACKET;
","             return parser::Token_COMMA;
";"             return parser::Token_SEMICOLON;
"."             return parser::Token_DOT;


 /* operators */

":"             return parser::Token_COLON;
"::" {
    if( _M_parser->compatibility_mode() >= parser::csharp20_compatibility ) {
      return parser::Token_SCOPE;
    }
    else {
      _M_parser->report_problem( parser::error,
        "Global alias access (with \"::\") is not supported by C# 1.0" );
      return parser::Token_INVALID;
    }
}
"?"             return parser::Token_QUESTION;
"??" {
    if( _M_parser->compatibility_mode() >= parser::csharp20_compatibility ) {
      return parser::Token_QUESTIONQUESTION;
    }
    else {
      _M_parser->report_problem( parser::error,
        "Null coalescing expressions (with \"??\") are not supported by C# 1.0" );
      return parser::Token_INVALID;
    }
}
"!"             return parser::Token_BANG;
"~"             return parser::Token_TILDE;
"=="            return parser::Token_EQUAL;
"<"             return parser::Token_LESS_THAN;
"<="            return parser::Token_LESS_EQUAL;
">"             return parser::Token_GREATER_THAN;
">="            return parser::Token_GREATER_EQUAL;
"!="            return parser::Token_NOT_EQUAL;
"&&"            return parser::Token_LOG_AND;
"||"            return parser::Token_LOG_OR;
"->"            return parser::Token_ARROW_RIGHT; // TODO: new in 2.0 or not?
"++"            return parser::Token_INCREMENT;
"--"            return parser::Token_DECREMENT;
"="             return parser::Token_ASSIGN;
"+"             return parser::Token_PLUS;
"+="            return parser::Token_PLUS_ASSIGN;
"-"             return parser::Token_MINUS;
"-="            return parser::Token_MINUS_ASSIGN;
"*"             return parser::Token_STAR;
"*="            return parser::Token_STAR_ASSIGN;
"/"             return parser::Token_SLASH;
"/="            return parser::Token_SLASH_ASSIGN;
"&"             return parser::Token_BIT_AND;
"&="            return parser::Token_BIT_AND_ASSIGN;
"|"             return parser::Token_BIT_OR;
"|="            return parser::Token_BIT_OR_ASSIGN;
"^"             return parser::Token_BIT_XOR;
"^="            return parser::Token_BIT_XOR_ASSIGN;
"%"             return parser::Token_REMAINDER;
"%="            return parser::Token_REMAINDER_ASSIGN;
"<<"            return parser::Token_LSHIFT;
"<<="           return parser::Token_LSHIFT_ASSIGN;
">>"            return parser::Token_RSHIFT;
">>="           return parser::Token_RSHIFT_ASSIGN;


 /* reserved words */

"abstract"      return parser::Token_ABSTRACT;
"as"            return parser::Token_AS;
"base"          return parser::Token_BASE;
"bool"          return parser::Token_BOOL;
"break"         return parser::Token_BREAK;
"byte"          return parser::Token_BYTE;
"case"          return parser::Token_CASE;
"catch"         return parser::Token_CATCH;
"char"          return parser::Token_CHAR;
"checked"       return parser::Token_CHECKED;
"class"         return parser::Token_CLASS;
"const"         return parser::Token_CONST;
"continue"      return parser::Token_CONTINUE;
"decimal"       return parser::Token_DECIMAL;
"default"       return parser::Token_DEFAULT;
"delegate"      return parser::Token_DELEGATE;
"do"            return parser::Token_DO;
"double"        return parser::Token_DOUBLE;
"else"          return parser::Token_ELSE;
"enum"          return parser::Token_ENUM;
"event"         return parser::Token_EVENT;
"explicit"      return parser::Token_EXPLICIT;
"extern"        return parser::Token_EXTERN;
"false"         return parser::Token_FALSE;
"finally"       return parser::Token_FINALLY;
"fixed"         return parser::Token_FIXED;
"float"         return parser::Token_FLOAT;
"for"           return parser::Token_FOR;
"foreach"       return parser::Token_FOREACH;
"goto"          return parser::Token_GOTO;
"if"            return parser::Token_IF;
"implicit"      return parser::Token_IMPLICIT;
"in"            return parser::Token_IN;
"int"           return parser::Token_INT;
"interface"     return parser::Token_INTERFACE;
"internal"      return parser::Token_INTERNAL;
"is"            return parser::Token_IS;
"lock"          return parser::Token_LOCK;
"long"          return parser::Token_LONG;
"namespace"     return parser::Token_NAMESPACE;
"new"           return parser::Token_NEW;
"null"          return parser::Token_NULL;
"object"        return parser::Token_OBJECT;
"operator"      return parser::Token_OPERATOR;
"out"           return parser::Token_OUT;
"override"      return parser::Token_OVERRIDE;
"params"        return parser::Token_PARAMS;
"private"       return parser::Token_PRIVATE;
"protected"     return parser::Token_PROTECTED;
"public"        return parser::Token_PUBLIC;
"readonly"      return parser::Token_READONLY;
"ref"           return parser::Token_REF;
"return"        return parser::Token_RETURN;
"sbyte"         return parser::Token_SBYTE;
"sealed"        return parser::Token_SEALED;
"short"         return parser::Token_SHORT;
"sizeof"        return parser::Token_SIZEOF;
"stackalloc"    return parser::Token_STACKALLOC;
"static"        return parser::Token_STATIC;
"string"        return parser::Token_STRING;
"struct"        return parser::Token_STRUCT;
"switch"        return parser::Token_SWITCH;
"this"          return parser::Token_THIS;
"throw"         return parser::Token_THROW;
"true"          return parser::Token_TRUE;
"try"           return parser::Token_TRY;
"typeof"        return parser::Token_TYPEOF;
"uint"          return parser::Token_UINT;
"ulong"         return parser::Token_ULONG;
"unchecked"     return parser::Token_UNCHECKED;
"unsafe"        return parser::Token_UNSAFE;
"ushort"        return parser::Token_USHORT;
"using"         return parser::Token_USING;
"virtual"       return parser::Token_VIRTUAL;
"void"          return parser::Token_VOID;
"volatile"      return parser::Token_VOLATILE;
"while"         return parser::Token_WHILE;


 /* Non-keyword identifiers. They only have special meaning in
  * specific contexts and are treated as identifiers otherwise.
  * Many of those have been introduced by C# 2.0. */

"add"           return parser::Token_ADD;
"alias" {
    if( _M_parser->compatibility_mode() >= parser::csharp20_compatibility )
      return parser::Token_ALIAS;
    else
      return parser::Token_IDENTIFIER;
}
"get"           return parser::Token_GET;
"global" {
    if( _M_parser->compatibility_mode() >= parser::csharp20_compatibility )
      return parser::Token_GLOBAL;
    else
      return parser::Token_IDENTIFIER;
}
"partial" {
    if( _M_parser->compatibility_mode() >= parser::csharp20_compatibility )
      return parser::Token_PARTIAL;
    else
      return parser::Token_IDENTIFIER;
}
"remove"        return parser::Token_REMOVE;
"set"           return parser::Token_SET;
"value"         return parser::Token_VALUE;
"where" {
    if( _M_parser->compatibility_mode() >= parser::csharp20_compatibility )
      return parser::Token_WHERE;
    else
      return parser::Token_IDENTIFIER;
}
"yield" {
    if( _M_parser->compatibility_mode() >= parser::csharp20_compatibility )
      return parser::Token_YIELD;
    else
      return parser::Token_IDENTIFIER;
}

 /* A non-keyword identifier that is not marked as such by the specification,
  * for global attributes: */
"assembly"      return parser::Token_ASSEMBLY;


 /* characters and strings */

[']({Escape}|{Multibyte}|[^\\\r\n\'])[']   return parser::Token_CHARACTER_LITERAL;
[']({Escape}|{Multibyte}|[\\][^\\\r\n\']|[^\\\r\n\'])*(([\\]?[\r\n])|[']) {
    _M_parser->report_problem( parser::error,
      std::string("Invalid character literal: ") + yytext );
    return parser::Token_CHARACTER_LITERAL;
}

["]({Escape}|{Multibyte}|[^\\\r\n\"])*["]  return parser::Token_STRING_LITERAL;
["]({Escape}|{Multibyte}|[\\][^\\\r\n\"]|[^\\\r\n\"])*(([\\]?[\r\n])|["]) {
    _M_parser->report_problem( parser::error,
      std::string("Invalid string literal: ") + yytext );
    return parser::Token_STRING_LITERAL;
}
 /* verbatim strings: */
[@]["](["]["]|[^"])*["]                 return parser::Token_STRING_LITERAL;



 /* the pre-processor */

<INITIAL,PP_SKIPPED_SECTION_PART>{
{ppPrefix}"define"  BEGIN(PP_DECLARATION);     PP_PROCESS_TOKEN(PP_DEFINE);
{ppPrefix}"undef"   BEGIN(PP_DECLARATION);     PP_PROCESS_TOKEN(PP_UNDEF);
{ppPrefix}"if"      BEGIN(PP_IF_CLAUSE);       PP_PROCESS_TOKEN(PP_IF);
{ppPrefix}"elif"    BEGIN(PP_IF_CLAUSE);       PP_PROCESS_TOKEN(PP_ELIF);
{ppPrefix}"else"    BEGIN(PP_EXPECT_NEW_LINE); PP_PROCESS_TOKEN(PP_ELSE);
{ppPrefix}"endif"   BEGIN(PP_EXPECT_NEW_LINE); PP_PROCESS_TOKEN(PP_ENDIF);
{ppPrefix}"line"    BEGIN(PP_LINE);            PP_PROCESS_TOKEN(PP_LINE);
{ppPrefix}"error"{Whitespace}?      BEGIN(PP_MESSAGE); PP_PROCESS_TOKEN(PP_ERROR);
{ppPrefix}"warning"{Whitespace}?    BEGIN(PP_MESSAGE); PP_PROCESS_TOKEN(PP_WARNING);
{ppPrefix}"region"{Whitespace}?     BEGIN(PP_MESSAGE); PP_PROCESS_TOKEN(PP_REGION);
{ppPrefix}"endregion"{Whitespace}?  BEGIN(PP_MESSAGE); PP_PROCESS_TOKEN(PP_ENDREGION);
{ppPrefix}"pragma"{Whitespace}? {
    if( _M_parser->compatibility_mode() >= parser::csharp20_compatibility ) {
      BEGIN(PP_PRAGMA); PP_PROCESS_TOKEN(PP_PRAGMA);
    }
    else {
      BEGIN(INITIAL);
      _M_parser->report_problem( parser::error,
        "#pragma directives are not supported by C# 1.0" );
      return parser::Token_INVALID;
    }
}
{ppPrefix}{Identifier} {
    _M_parser->report_problem( parser::error,
      std::string("Invalid pre-processor directive: ``") + yytext + "''" );
    return parser::Token_INVALID;
}
}

<PP_EXPECT_NEW_LINE,PP_DECLARATION,PP_IF_CLAUSE,PP_LINE,PP_MESSAGE,PP_PRAGMA>{
<<EOF>> {
    _M_parser->report_problem( parser::warning,
      "No newline at the end of the file" );
    return csharp_pp::parser::Token_EOF;
}
}

<PP_EXPECT_NEW_LINE,PP_DECLARATION,PP_IF_CLAUSE,PP_LINE>{
{Whitespace}        /* skip */ ;
{LineComment}       /* before an unexpected EOF, skip */ ;
}


<PP_EXPECT_NEW_LINE>{
{ppNewLine}         return csharp_pp::parser::Token_PP_NEW_LINE;
.                   return csharp_pp::parser::Token_PP_INVALID;
}

<PP_DECLARATION>{
"true"|"false" {
    _M_parser->report_problem( parser::error,
      "You may not define ``true'' or ``false'' with #define or #undef" );
    return csharp_pp::parser::Token_PP_CONDITIONAL_SYMBOL;  // we could do Token_INVALID here,
    // but this way the error is shown and the parser continues, I prefer this.
}
{Identifier}        return csharp_pp::parser::Token_PP_CONDITIONAL_SYMBOL; // ...including keywords
{ppNewLine}         return csharp_pp::parser::Token_PP_NEW_LINE;
.                   return csharp_pp::parser::Token_PP_INVALID;
}

<PP_IF_CLAUSE>{
"=="                return csharp_pp::parser::Token_PP_EQUAL;
"!="                return csharp_pp::parser::Token_PP_NOT_EQUAL;
"&&"                return csharp_pp::parser::Token_PP_LOG_AND;
"||"                return csharp_pp::parser::Token_PP_LOG_OR;
"!"                 return csharp_pp::parser::Token_PP_BANG;
"true"              return csharp_pp::parser::Token_PP_TRUE;
"false"             return csharp_pp::parser::Token_PP_FALSE;
"("                 return csharp_pp::parser::Token_PP_LPAREN;
")"                 return csharp_pp::parser::Token_PP_RPAREN;
{Identifier}        return csharp_pp::parser::Token_PP_CONDITIONAL_SYMBOL;
{ppNewLine}         return csharp_pp::parser::Token_PP_NEW_LINE;
.                   return csharp_pp::parser::Token_PP_INVALID;
}

<PP_SKIPPED_SECTION_PART>{
 /* splitting the line at "#" keeps the token shorter than real directives, */
 /* so that those are recognized and not taken as skipped text as well.     */
[^#\r\n]*           /* skip */ ;
"#"                 /* skip */ ;
{NewLine}           /* skip */ ;
}

<PP_LINE>{
{ppNewLine}         return csharp_pp::parser::Token_PP_NEW_LINE;
{DecimalDigit}+     return csharp_pp::parser::Token_PP_LINE_NUMBER;
["][^\"\r\n]+["]    return csharp_pp::parser::Token_PP_FILE_NAME;
"default"           return csharp_pp::parser::Token_PP_DEFAULT;
{Identifier}        return csharp_pp::parser::Token_PP_IDENTIFIER_OR_KEYWORD;
.                   return csharp_pp::parser::Token_PP_INVALID;
}

<PP_MESSAGE>{
[^\r\n]+            return csharp_pp::parser::Token_PP_MESSAGE;
{NewLine}           return csharp_pp::parser::Token_PP_NEW_LINE;
}

<PP_PRAGMA>{
[^\r\n]+            return csharp_pp::parser::Token_PP_PRAGMA_TEXT;
{NewLine}           return csharp_pp::parser::Token_PP_NEW_LINE;
}


 /* identifiers and number literals */

{Identifier}        return parser::Token_IDENTIFIER;

{IntegerLiteral}    return parser::Token_INTEGER_LITERAL;
{RealLiteral}       return parser::Token_REAL_LITERAL;


 /* everything else is not a valid lexeme */

.                   return parser::Token_INVALID;


 /* some additional checking for unclosed #ifs and #regions at the EOF */

<INITIAL,PP_SKIPPED_SECTION_PART>{
<<EOF>> {
  cleanup();
  return parser::Token_EOF;
}
}

%%

namespace csharp
{

void Lexer::restart(parser *parser, char *contents)
{
  _M_parser = parser;
  _M_contents = contents;
  _M_pp_root_scope = 0;
  _M_token_begin = _M_token_end = 0;
  _M_current_offset = 0;

  // check for and ignore the UTF-8 byte order mark
  unsigned char *ucontents = (unsigned char *) _M_contents;
  if (ucontents[0] == 0xEF && ucontents[1] == 0xBB && ucontents[2] == 0xBF)
    {
      _M_token_begin = _M_token_end = 3;
      _M_current_offset = 3;
    }

  yyrestart(NULL);
  BEGIN(INITIAL); // is not set automatically by yyrestart()
}

csharp_pp::scope* Lexer::pp_current_scope()
{
  if (_M_pp_root_scope == 0)
    {
      _M_pp_root_scope = new csharp_pp::scope(_M_parser);
    }
  return _M_pp_root_scope->current_scope();
}

void Lexer::cleanup()
{
  // check for open scopes, and pop them / report errors as needed
  if (_M_pp_root_scope != 0)
    {
      csharp_pp::scope* current_scope = pp_current_scope();
      csharp_pp::scope::scope_type scope_type = current_scope->type();

      while (scope_type != csharp_pp::scope::type_root)
        {
          if (scope_type == csharp_pp::scope::type_if)
            {
              _M_parser->report_problem( parser::error,
                "Encountered end of file in an unclosed #if/#elif/#else section" );
            }
          else if (scope_type == csharp_pp::scope::type_region)
            {
              _M_parser->report_problem( parser::error,
                "Encountered end of file in an unclosed #region section" );
            }

          if ( !current_scope->pop_scope(scope_type, &current_scope) )
            break;

          scope_type = current_scope->type();
        }

      delete _M_pp_root_scope;
      _M_pp_root_scope = 0;
    }
}

} // end of namespace csharp

