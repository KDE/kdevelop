/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef LEXER_H
#define LEXER_H

#include "symbol.h"

#include <QtCore/QString>
#include <cstdlib>

struct NameSymbol;
class Lexer;
class Control;

typedef void (Lexer::*scan_fun_ptr)();

class Token
{
public:
  int kind;
  std::size_t position;
  std::size_t size;
  char const *text;

  union
  {
    const NameSymbol *symbol;
    std::size_t right_brace;
  } extra;
};

class LocationTable
{
private:
  LocationTable(const LocationTable &source);
  void operator = (const LocationTable &source);

public:
  inline LocationTable(std::size_t size = 1024)
    : lines(0),
      line_count(0),
      current_line(0)
  {
    resize(size);
  }

  inline ~LocationTable()
  {
    free(lines);
  }

  inline std::size_t size() const
  { return line_count; }

  void resize(std::size_t size)
  {
    Q_ASSERT(size > 0);
    lines = (std::size_t*) ::realloc(lines, sizeof(std::size_t) * size);
    line_count = size;
  }

  void positionAt(std::size_t offset, int *line, int *column) const;

  inline std::size_t &operator[](int index)
  { return lines[index]; }

private:
  std::size_t *lines;
  std::size_t line_count;
  std::size_t current_line;

  friend class Lexer;
};

class TokenStream
{
private:
  TokenStream(const TokenStream &);
  void operator = (const TokenStream &);

public:
  inline TokenStream(std::size_t size = 1024)
     : tokens(0),
       index(0),
       token_count(0)
  {
    resize(size);
  }

  inline ~TokenStream()
  { ::free(tokens); }

  inline std::size_t size() const
  { return token_count; }

  inline std::size_t cursor() const
  { return index; }

  inline void rewind(int i)
  { index = i; }

  void resize(std::size_t size)
  {
    Q_ASSERT(size > 0);
    tokens = (Token*) ::realloc(tokens, sizeof(Token) * size);
    token_count = size;
  }

  inline std::size_t nextToken()
  { return index++; }

  inline int lookAhead(std::size_t i = 0) const
  { return tokens[index + i].kind; }

  inline int kind(std::size_t i) const
  { return tokens[i].kind; }

  inline std::size_t position(std::size_t i) const
  { return tokens[i].position; }

  inline const NameSymbol *symbol(std::size_t i) const
  { return tokens[i].extra.symbol; }

  inline std::size_t matchingBrace(std::size_t i) const
  { return tokens[i].extra.right_brace; }

  inline Token &operator[](int index)
  { return tokens[index]; }

  inline const Token &token(int index) const
  { return tokens[index]; }

private:
  Token *tokens;
  std::size_t index;
  std::size_t token_count;

private:
  friend class Lexer;
};

class Lexer
{
public:
  Lexer(TokenStream &token_stream,
	LocationTable &location_table,
	LocationTable &line_table,
	Control *control);

  void tokenize(const char *contents, std::size_t size);

  TokenStream &token_stream;
  LocationTable &location_table;
  LocationTable &line_table;

  void positionAt(std::size_t offset, int *line, int *column,
		  QString *filename) const;

private:
  void initialize_scan_table();
  void scan_newline();
  void scan_white_spaces();
  void scan_identifier_or_keyword();
  void scan_identifier_or_literal();
  void scan_int_constant();
  void scan_char_constant();
  void scan_string_constant();
  void scan_invalid_input();
  void scan_preprocessor();

  // keywords
  void scanKeyword0();
  void scanKeyword2();
  void scanKeyword3();
  void scanKeyword4();
  void scanKeyword5();
  void scanKeyword6();
  void scanKeyword7();
  void scanKeyword8();
  void scanKeyword9();
  void scanKeyword10();
  void scanKeyword11();
  void scanKeyword12();
  void scanKeyword13();
  void scanKeyword14();
  void scanKeyword16();

  // operators
  void scan_not();
  void scan_remainder();
  void scan_and();
  void scan_left_paren();
  void scan_right_paren();
  void scan_star();
  void scan_plus();
  void scan_comma();
  void scan_minus();
  void scan_dot();
  void scan_divide();
  void scan_colon();
  void scan_semicolon();
  void scan_less();
  void scan_equal();
  void scan_greater();
  void scan_question();
  void scan_left_bracket();
  void scan_right_bracket();
  void scan_xor();
  void scan_left_brace();
  void scan_or();
  void scan_right_brace();
  void scan_tilde();
  void scan_EOF();

  void extract_line(int offset, int *line, QString *filename) const;

private:
  Control *control;
  const unsigned char *cursor;
  const unsigned char *begin_buffer;
  const unsigned char *end_buffer;
  std::size_t index;

  static scan_fun_ptr s_scan_table[];
  static scan_fun_ptr s_scan_keyword_table[];
  static bool s_initialized;
};

#endif // LEXER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
