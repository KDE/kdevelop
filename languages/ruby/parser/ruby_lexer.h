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

#include "ruby_parser.h"

#include <iostream>

#ifndef DONT_INCLUDE_FLEXLEXER
#include "FlexLexer.h"
#endif

// The YY_USER_ACTION macro is called whenever a token is found by Flex
#define YY_USER_ACTION \
m_tokenBegin = m_tokenEnd; \
m_tokenEnd += yyleng;

namespace ruby
{

class Lexer : public yyFlexLexer
{
public:
    Lexer( ruby::parser *parser, char *contents );
    void restart( ruby::parser *parser, char *contents );

    int yylex();
    char *contents()         { return m_contents;   }
    std::size_t tokenBegin() { return m_tokenBegin; }
    std::size_t tokenEnd()   { return m_tokenEnd;   }

    bool setExpectArrayArguments(bool b);

protected:
    // custom input, replacing the Flex default input stdin
    virtual int LexerInput( char *buf, int max_size );

    // dismisses any lexer output (which should not happen anyways)
    virtual void LexerOutput( const char * /*buf*/, int /*max_size*/ ) { return; }
    virtual void LexerError( const char */*msg*/ ) { return; }

private:
    ruby::parser* m_parser;
    char *m_contents;
    std::size_t m_tokenBegin, m_tokenEnd;
    std::size_t m_currentOffset;
    kdev_pg_location_table *m_locationTable;
};

} // end of namespace csharp

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
