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

#include "java_parser.h"

#include <iostream>

#ifndef DONT_INCLUDE_FLEXLEXER
#include <FlexLexer.h>
#endif

namespace java
{

class Lexer : public yyFlexLexer
{
public:
  Lexer(java::parser *parser, char *contents) { restart(parser, contents); }
  void restart(java::parser *parser, char *contents);

  int yylex();
  char *contents()          { return _M_contents;    }
  std::size_t token_begin() { return _M_token_begin; }
  std::size_t token_end()   { return _M_token_end;   }

protected:
  // reads a character, and returns 1 as the number of characters read
  // (or 0 when the end of the string is reached)
  virtual int LexerInput(char *buf, int /*max_size*/)
  {
    int c = _M_contents[_M_current_offset++];
    return (c == 0) ? 0 : (buf[0] = c, 1);
  }

  // dismisses any lexer output (which should not happen anyways)
  virtual void LexerOutput(const char * /*buf*/, int /*max_size*/) { return; }
  virtual void LexerError(const char */*msg*/) { return; }

protected:
  java::parser* _M_parser;
  char *_M_contents;
  std::size_t _M_token_begin, _M_token_end;
  std::size_t _M_current_offset;
};

} // end of namespace java
