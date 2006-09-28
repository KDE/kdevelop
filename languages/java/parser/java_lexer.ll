%{
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
%}

%option c++
%option yyclass="java::Lexer"
%option noyywrap


%{

#define DONT_INCLUDE_FLEXLEXER
#include "java_lexer.h"

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
[\n]            /* skip */ ;

"//"[^\n]*      /* line comments, skip */ ;

"/*"            BEGIN(IN_BLOCKCOMMENT);
<IN_BLOCKCOMMENT>{
[^*\n]*         /* eat anything that's not a '*' */ ;
"*"+[^*/\n]*    /* eat up '*'s that are not followed by slashes or newlines */;
[\n]            /* skip */ ;
"*"+"/"         BEGIN(INITIAL);
<<EOF>> {
    m_parser->report_problem( parser::error,
        "Encountered end of file in an unclosed block comment" );
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
"@"             {
    if (m_parser->compatibility_mode() >= parser::java15_compatibility)
        return parser::Token_AT;
    else {
        m_parser->report_problem( parser::error,
            "Annotations are not supported by Java 1.4 or earlier" );
        return parser::Token_INVALID;
    }
}


 /* operators */

"?"             return parser::Token_QUESTION;
":"             return parser::Token_COLON;
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
">>"            return parser::Token_SIGNED_RSHIFT;
">>="           return parser::Token_SIGNED_RSHIFT_ASSIGN;
">>>"           return parser::Token_UNSIGNED_RSHIFT;
">>>="          return parser::Token_UNSIGNED_RSHIFT_ASSIGN;
"..."           {
    if ( m_parser->compatibility_mode() >= parser::java15_compatibility )
        return parser::Token_ELLIPSIS;
    else {
        m_parser->report_problem( parser::error,
            "Variable-length argument lists are "
            "not supported by Java 1.4 or earlier" );
        return parser::Token_INVALID;
    }
}


 /* reserved words */

"abstract"      return parser::Token_ABSTRACT;
"assert"        {
    if ( m_parser->compatibility_mode() >= parser::java14_compatibility )
        return parser::Token_ASSERT;
    else
        return parser::Token_IDENTIFIER;
}
"boolean"       return parser::Token_BOOLEAN;
"break"         return parser::Token_BREAK;
"byte"          return parser::Token_BYTE;
"case"          return parser::Token_CASE;
"catch"         return parser::Token_CATCH;
"char"          return parser::Token_CHAR;
"class"         return parser::Token_CLASS;
"const"         {
    m_parser->report_problem( parser::error,
        "\"const\": reserved but unused (invalid) keyword" );
    return parser::Token_CONST;
}
"continue"      return parser::Token_CONTINUE;
"default"       return parser::Token_DEFAULT;
"do"            return parser::Token_DO;
"double"        return parser::Token_DOUBLE;
"else"          return parser::Token_ELSE;
"enum"          {
    if ( m_parser->compatibility_mode() >= parser::java15_compatibility )
        return parser::Token_ENUM;
    else
        return parser::Token_IDENTIFIER;
}
"extends"       return parser::Token_EXTENDS;
"false"         return parser::Token_FALSE;
"final"         return parser::Token_FINAL;
"finally"       return parser::Token_FINALLY;
"float"         return parser::Token_FLOAT;
"for"           return parser::Token_FOR;
"goto"          {
    m_parser->report_problem( parser::error,
        "\"goto\": reserved but unused (invalid) keyword" );
    return parser::Token_GOTO;
}
"if"            return parser::Token_IF;
"implements"    return parser::Token_IMPLEMENTS;
"import"        return parser::Token_IMPORT;
"instanceof"    return parser::Token_INSTANCEOF;
"int"           return parser::Token_INT;
"interface"     return parser::Token_INTERFACE;
"long"          return parser::Token_LONG;
"native"        return parser::Token_NATIVE;
"new"           return parser::Token_NEW;
"null"          return parser::Token_NULL;
"package"       return parser::Token_PACKAGE;
"private"       return parser::Token_PRIVATE;
"protected"     return parser::Token_PROTECTED;
"public"        return parser::Token_PUBLIC;
"return"        return parser::Token_RETURN;
"short"         return parser::Token_SHORT;
"static"        return parser::Token_STATIC;
"strictfp"      return parser::Token_STRICTFP;
"super"         return parser::Token_SUPER;
"switch"        return parser::Token_SWITCH;
"synchronized"  return parser::Token_SYNCHRONIZED;
"this"          return parser::Token_THIS;
"throw"         return parser::Token_THROW;
"throws"        return parser::Token_THROWS;
"transient"     return parser::Token_TRANSIENT;
"true"          return parser::Token_TRUE;
"try"           return parser::Token_TRY;
"void"          return parser::Token_VOID;
"volatile"      return parser::Token_VOLATILE;
"while"         return parser::Token_WHILE;


 /* characters and strings */

[']({Escape}|{Multibyte}|[^\\\n\'])[']   return parser::Token_CHARACTER_LITERAL;
[']({Escape}|{Multibyte}|[\\][^\\\n\']|[^\\\n\'])*([\\]?[\n]|[']) {
    m_parser->report_problem( parser::error,
        std::string("Invalid character literal: ") + yytext );
    return parser::Token_CHARACTER_LITERAL;
}

["]({Escape}|{Multibyte}|[^\\\n\"])*["]  return parser::Token_STRING_LITERAL;
["]({Escape}|{Multibyte}|[\\][^\\\n\"]|[^\\\n\"])*([\\]?[\n]|["]) {
    m_parser->report_problem( parser::error,
        std::string("Invalid string literal: ") + yytext );
    return parser::Token_STRING_LITERAL;
}


 /* identifiers and number literals */

{Letter}({Letter}|{Digit})*  return parser::Token_IDENTIFIER;

{IntegerLiteral}   return parser::Token_INTEGER_LITERAL;
{FloatingPoint}    return parser::Token_FLOATING_POINT_LITERAL;


 /* everything else is not a valid lexeme */

.                  return parser::Token_INVALID;

%%

namespace java
{

Lexer::Lexer( parser *parser, char *contents )
{
    restart( parser, contents );
}

void Lexer::restart( parser *parser, char *contents )
{
    m_parser = parser;
    m_locationTable = parser->token_stream->location_table();
    m_contents = contents;
    m_tokenBegin = m_tokenEnd = 0;
    m_currentOffset = 0;

    // check for and ignore the UTF-8 byte order mark
    unsigned char *ucontents = (unsigned char *) m_contents;
    if ( ucontents[0] == 0xEF && ucontents[1] == 0xBB && ucontents[2] == 0xBF )
    {
        m_tokenBegin = m_tokenEnd = 3;
        m_currentOffset = 3;
    }

    yyrestart(NULL);
    BEGIN(INITIAL); // is not set automatically by yyrestart()
}

// reads a character, and returns 1 as the number of characters read
// (or 0 when the end of the string is reached)
int Lexer::LexerInput( char *buf, int /*max_size*/ )
{
    int c = m_contents[ m_currentOffset++ ];

    switch(c)
    {
    case '\r':
        c = '\n'; // only have one single line break character: '\n'
        if ( m_contents[m_currentOffset + 1] == '\n' )
        {
            m_currentOffset++;
            m_tokenEnd++;
        }

        // fall through
    case '\n':
        m_locationTable->newline( m_currentOffset );
        break;

    default:
        break;
    }

    return (c == 0) ? 0 : (buf[0] = c, 1);
}

} // end of namespace java

