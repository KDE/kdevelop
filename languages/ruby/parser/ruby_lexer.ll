%{
/*****************************************************************************
 * Copyright (c) 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                           *
 * Permission is hereby granted, free of charge, to any person obtaining     *
 * a copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including       *
 * without limitation the rights to use, copy, modify, merge, publish,       *
 * distribute, sublicense, and/or sell copies of the Software, and to        *
 * permit persons to whom the Software is furnished to do so, subject to     *
 * the following conditions:                                                 *
 *                                                                           *
 * The above copyright notice and this permission notice shall be            *
 * included in all copies or substantial portions of the Software.           *
 *                                                                           *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE    *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION    *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION     *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.           *
 *****************************************************************************/
%}

%option c++
%option yyclass="ruby::Lexer"
%option noyywrap

%{

#define DONT_INCLUDE_FLEXLEXER
#include "ruby_lexer.h"

%}

%x expect_hash
%x expect_array_access
%x expect_leading_colon2
%x expect_unary
%x expect_operator



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

DecimalDigit    [0-9]
HexDigit        [0-9a-fA-F]
OctalDigit      [0-7]
BinaryDigit     [01]

OctalNumber     [-]?0{DecimalDigit}+
Number          [-]?{DecimalDigit}+
HexNumber       [-]?0x{HexDigit}+
BinaryNumber    [-]?0b{BinaryDigit}+
Float           [-]?{DecimalDigit}*\.{DecimalDigit}+

ShortUnicodeEscape  [\\][u]{HexDigit}{HexDigit}{HexDigit}{HexDigit}
LongUnicodeEscape   [\\][U]{HexDigit}{HexDigit}{HexDigit}{HexDigit}{HexDigit}{HexDigit}{HexDigit}{HexDigit}
UnicodeEscape   {ShortUnicodeEscape}|{LongUnicodeEscape}
HexEscape       [\\][x]{HexDigit}{HexDigit}?{HexDigit}?{HexDigit}?
SimpleEscape    [\\]([']|["]|[\\]|[0abfnrtv])
Escape          ({SimpleEscape}|{UnicodeEscape}|{HexEscape})

Whitespace      [ \t\v\f]+
Linebreak       [\n\r]+
Regexp          \/[^/]*\/

Identifier      [a-z_][a-zA-Z_0-9]*
Constant        [A-Z_]*
ClassName       [A-Z][a-zA-Z_0-9]*




%%

 /* keywords */

"end"       { return ruby::parser::Token_END; }
"else"      { return ruby::parser::Token_ELSE; }
"elsif"     { return ruby::parser::Token_ELSIF; }
"rescue"    { return ruby::parser::Token_RESCUE; }
"ensure"    { return ruby::parser::Token_ENSURE; }
"when"      { return ruby::parser::Token_WHEN; }
%"if"       { return ruby::parser::Token_IF_MODIFIER; }
%"while"    { return ruby::parser::Token_WHILE_MODIFIER; }
%"unless"   { return ruby::parser::Token_UNLESS_MODIFIER; }
%"until"    { return ruby::parser::Token_UNTIL_MODIFIER; }
%"rescue"   { return ruby::parser::Token_RESCUE_MODIFIER; }
"undef"     { return ruby::parser::Token_UNDEF; }
"alias"     { return ruby::parser::Token_ALIAS; }
"begin"     { return ruby::parser::Token_BEGIN; }
"do"        { return ruby::parser::Token_DO; }
"return"    { return ruby::parser::Token_RETURN; }
"break"     { return ruby::parser::Token_BREAK; }
"next"      { return ruby::parser::Token_NEXT; }
"nil"       { return ruby::parser::Token_NIL; }
"true"      { return ruby::parser::Token_TRUE; }
"false"     { return ruby::parser::Token_FALSE; }
"__FILE__"  { return ruby::parser::Token_FILE; }
"__LINE__"  { return ruby::parser::Token_LINE; }
"self"      { return ruby::parser::Token_SELF; }
"super"     { return ruby::parser::Token_SUPER; }
"retry"     { return ruby::parser::Token_RETRY; }
"yield"     { return ruby::parser::Token_YIELD; }
"defined?"  { return ruby::parser::Token_DEFINED; }
"redo"      { return ruby::parser::Token_REDO; }
"if"        { return ruby::parser::Token_IF; }
"case"      { return ruby::parser::Token_CASE; }
"until"     { return ruby::parser::Token_UNTIL; }
"while"     { return ruby::parser::Token_WHILE; }
"for"       { return ruby::parser::Token_FOR; }
"module"    { return ruby::parser::Token_MODULE; }
"def"       { return ruby::parser::Token_DEF; }
"then"      { return ruby::parser::Token_THEN; }
"class"     { return ruby::parser::Token_CLASS; }
"unless"    { return ruby::parser::Token_UNLESS; }
"or"        { return ruby::parser::Token_OR; }
"and"       { return ruby::parser::Token_AND; }
"in"        { return ruby::parser::Token_IN; }
"END"       { return ruby::parser::Token_END_UPCASE; }
"BEGIN"     { return ruby::parser::Token_BEGIN_UPCASE; }


 /* strings */
[']({Escape}|{Multibyte}|[^\\\n\'])[']        { return ruby::parser::Token_SINGLE_QUOTED_STRING; }
["]({Escape}|{Multibyte}|[^\\\n\"])*["]       { return ruby::parser::Token_DOUBLE_QUOTED_STRING; }
[<][<][-]?([^{Linebreak}]*)({Escape}|{Multibyte}|[^\\\n\'])[<][<]\1 { return ruby::parser::Token_HERE_DOC_BEGIN; }


 /* identifiers, function names, constants */
{Identifier}[:][:]          { return ruby::parser::Token_FUNCTION; }
{Identifier}[.]             { return ruby::parser::Token_FUNCTION; }
{Identifier}[?]             { return ruby::parser::Token_FUNCTION; }
{Identifier}[!]             { return ruby::parser::Token_FUNCTION; }
{Identifier}                { return ruby::parser::Token_IDENTIFIER; }
{Constant}                  { return ruby::parser::Token_CONSTANT; }
{ClassName}                 { return ruby::parser::Token_CONSTANT; }


 /* numbers */
{OctalNumber}               { return ruby::parser::Token_OCTAL; }
{HexNumber}                 { return ruby::parser::Token_HEX; }
{BinaryNumber}              { return ruby::parser::Token_BINARY; }
{Number}                    { return ruby::parser::Token_INTEGER; }
{Float}                     { return ruby::parser::Token_FLOAT; }


 /* operators and punctuation */

"("                         { return ruby::parser::Token_LPAREN; }
")"                         { return ruby::parser::Token_RPAREN; }
<expect_array_access>"["    { return ruby::parser::Token_LBRACK; }
"["                         { return ruby::parser::Token_LBRACK; }
"]"                         { return ruby::parser::Token_RBRACK; }
<expect_hash>"{"            { return ruby::parser::Token_LCURLY_HASH; }
"{"                         { return ruby::parser::Token_LCURLY_BLOCK; }
","                         { return ruby::parser::Token_COMMA; }
<expect_leading_colon2>"::" { return ruby::parser::Token_LEADING_TWO_COLON; }
"::"                        { return ruby::parser::Token_TWO_COLON; }
":"                         { return ruby::parser::Token_COLON; }

"!="                        { return ruby::parser::Token_NOT_EQUAL; }
"!~"                        { return ruby::parser::Token_NOT_MATCH; }
"!"                         { return ruby::parser::Token_NOT; }
"~"                         { return ruby::parser::Token_BNOT; }
"+="                        { return ruby::parser::Token_PLUS_ASSIGN; }
"-="                        { return ruby::parser::Token_MINUS_ASSIGN; }
"**="                       { return ruby::parser::Token_POWER_ASSIGN; }
"*="                        { return ruby::parser::Token_STAR_ASSIGN; }
"&&="                       { return ruby::parser::Token_LOGICAL_AND_ASSIGN; }
"||="                       { return ruby::parser::Token_LOGICAL_OR_ASSIGN; }
"&="                        { return ruby::parser::Token_BAND_ASSIGN; }
"^="                        { return ruby::parser::Token_BXOR_ASSIGN; }
"|="                        { return ruby::parser::Token_BOR_ASSIGN; }
">>="                       { return ruby::parser::Token_RIGHT_SHIFT_ASSIGN; }
<expect_unary>"+"           { return ruby::parser::Token_UNARY_PLUS; }
"+"                         { return ruby::parser::Token_PLUS; }
<expect_unary>"-"           { return ruby::parser::Token_UNARY_MINUS; }
"-"                         { return ruby::parser::Token_MINUS; }
"**"                        { return ruby::parser::Token_POWER; }
"*"                         { return ruby::parser::Token_STAR; }
"<=>"                       { return ruby::parser::Token_COMPARE; }
">="                        { return ruby::parser::Token_GREATER_OR_EQUAL; }
"<="                        { return ruby::parser::Token_LESS_OR_EQUAL; }
">>"                        { return ruby::parser::Token_RIGHT_SHIFT; }
<expect_operator>"<<"       { return ruby::parser::Token_LEFT_SHIFT; }
"<"                         { return ruby::parser::Token_LESS_THAN; }
">"                         { return ruby::parser::Token_GREATER_THAN; }
"^"                         { return ruby::parser::Token_BXOR; }
"||"                        { return ruby::parser::Token_LOGICAL_OR; }
"|"                         { return ruby::parser::Token_BOR; }
"&&"                        { return ruby::parser::Token_LOGICAL_AND; }
<expect_operator>"&"        { return ruby::parser::Token_BAND; }
"&"                         { return ruby::parser::Token_BLOCK_ARG_PREFIX; }
"==="                       { return ruby::parser::Token_CASE_EQUAL; }
"=="                        { return ruby::parser::Token_EQUAL; }
"=~"                        { return ruby::parser::Token_MATCH; }
"=>"                        { return ruby::parser::Token_ASSOC; }
"="                         { return ruby::parser::Token_ASSIGN; }

";"                         { return ruby::parser::Token_SEMI; }

{Linebreak}                 { return ruby::parser::Token_LINE_BREAK; }

<expect_operator>"/="       { return ruby::parser::Token_DIV_ASSIGN; }
<expect_operator>"/"        { return ruby::parser::Token_DIV; }
{Regexp}                    { return ruby::parser::Token_REGEX; }



%%

namespace ruby
{

Lexer::Lexer( parser *parser, char *contents )
{
    restart( parser, contents );
}

void Lexer::restart( parser *parser, char *contents )
{
    cleanup();

    m_parser = parser;
    m_lineTable = parser->token_stream->line_table();
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
        m_lineTable->newline( m_currentOffset );
        break;

    default:
        break;
    }

    return (c == 0) ? 0 : (buf[0] = c, 1);
}

void Lexer::cleanup()
{
}

} // end of namespace ruby

