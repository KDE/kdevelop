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

#include "csharp_parser.h"
#include "csharp_pp_parser.h"

#include <iostream>

#ifndef DONT_INCLUDE_FLEXLEXER
#include "FlexLexer.h"
#endif

// The YY_USER_ACTION macro is called whenever a token is found by Flex
#define YY_USER_ACTION \
m_tokenBegin = m_tokenEnd; \
m_tokenEnd += yyleng;


namespace csharp
{

class Lexer : public yyFlexLexer
{
public:
    Lexer( csharp::parser *parser, char *contents );
    void restart( csharp::parser *parser, char *contents );

    virtual ~Lexer() { cleanup(); }

    int yylex();
    char *contents()         { return m_contents;   }
    std::size_t tokenBegin() { return m_tokenBegin; }
    std::size_t tokenEnd()   { return m_tokenEnd;   }

private:
    // Takes care of handling and destroying unclosed scopes.
    void cleanup();

    // retrieves the upper-most pre-processor scope
    csharp_pp::scope* ppCurrentScope();

protected:
    // custom input, replacing the Flex default input stdin
    virtual int LexerInput( char *buf, int max_size );

    // dismisses any lexer output (which should not happen anyways)
    virtual void LexerOutput( const char * /*buf*/, int /*max_size*/ ) { return; }
    virtual void LexerError( const char */*msg*/ ) { return; }

private:
    csharp::parser* m_parser;
    char *m_contents;
    std::size_t m_tokenBegin, m_tokenEnd;
    std::size_t m_currentOffset;
    csharp_pp::scope* m_ppRootScope;
    kdev_pg_location_table *m_locationTable;
};

} // end of namespace csharp

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
