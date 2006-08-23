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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
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
class Problem;
class ParseSession;

typedef void (Lexer::*scan_fun_ptr)();

/**Token.*/
class Token
{
public:
  ///kind of the token @see TOKEN_KIND enum reference.
  int kind;
  ///position in the c++ source buffer.
  std::size_t position;
  ///size of the token text in the c++ source buffer.
  std::size_t size;
  ///pointer to the text in the c++ source buffer (not valid when buffer is destroyed).
  char const *text;

  QString symbol() const { return QString::fromUtf8(&text[position], size); }

  ///@todo adymo: find out what @p right_brace is
  union
  {
    //const NameSymbol *symbol;
    std::size_t right_brace;
  } extra;
};

/**
 * A class which holds an array of input buffer offsets
 * for use in translating from a line number to an input
 * buffer position.
 *
 * A line number may not necessarily represent the input
 * buffer line numbers; it depends on how the class is used.
 */
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

  /**
   * Returns the \a line and \a column of the given \a offset in this table.
   */
  void positionAt(std::size_t offset, int *line, int *column) const;

  inline std::size_t &operator[](int index)
  { return lines[index]; }

private:
  /// An array of input buffer offsets
  std::size_t *lines;
  /// The size of the allocated array
  std::size_t line_count;
  /// The index to the next index in the lines array
  std::size_t current_line;

  friend class Lexer;
};

/**Stream of tokens found by lexer.
Internally works like an array of @ref Token continuosly allocated.
All tokens are destructed when this stream is deleted.

The stream has a "cursor" which is simply an integer which defines
the offset (index) of the token currently "observed" from the beginning of
the stream.*/
class TokenStream
{
private:
  TokenStream(const TokenStream &);
  void operator = (const TokenStream &);

public:
  /**Creates a token stream with the default size of 1024 tokens.*/
  inline TokenStream(std::size_t size = 1024)
     : tokens(0),
       index(0),
       token_count(0)
  {
    resize(size);
  }

  inline ~TokenStream()
  { ::free(tokens); }

  /**@return the size of the token stream.*/
  inline std::size_t size() const
  { return token_count; }

  /**@return the "cursor" - the offset (index) of the token
  currently "observed" from the beginning of the stream.*/
  inline std::size_t cursor() const
  { return index; }

  /**Sets the cursor to the position @p i.*/
  inline void rewind(int i)
  { index = i; }

  /**Resizes the token stream.*/
  void resize(std::size_t size)
  {
    Q_ASSERT(size > 0);
    tokens = (Token*) ::realloc(tokens, sizeof(Token) * size);
    token_count = size;
  }

  /**Updates the cursor position to point to the next token and returns
  the cursor.*/
  inline std::size_t nextToken()
  { return index++; }

  /**@return the kind of the next (LA) token in the stream.*/
  inline int lookAhead(std::size_t i = 0) const
  { return tokens[index + i].kind; }

  /**@return the kind of the current token in the stream.*/
  inline int kind(std::size_t i) const
  { return tokens[i].kind; }

  /**@return the position of the current token in the c++ source buffer.*/
  inline std::size_t position(std::size_t i) const
  { return tokens[i].position; }

  /**@return the name symbol of the current token.*/
  //inline const NameSymbol *symbol(std::size_t i) const
  //{ return tokens[i].extra.symbol; }

  /**@return the position of the matching right brace in the
  c++ source buffer.
  @todo this doesn't seem to work as the lexer does not provide this
  information at the moment.*/
  inline std::size_t matchingBrace(std::size_t i) const
  { return tokens[i].extra.right_brace; }

  /**@return the token at position @p index.*/
  inline Token &operator[](int index)
  { return tokens[index]; }

  /**@return the token at position @p index.*/
  inline const Token &token(int index) const
  { return tokens[index]; }

private:
  Token *tokens;
  std::size_t index;
  std::size_t token_count;

private:
  friend class Lexer;
};

/**C++ Lexer.*/
class Lexer
{
public:
  /**
   * Constructor.
   *
   * \param token_stream Provides a stream of tokens to the lexer.
   * \param location_table a table which will be filled with non-preprocessed line -> offset values
   * \param line_table a table which will be filled with (non-preproccessed line which contains a preprocessor line) -> offset values
   */
  Lexer(Control *control);

  /**Finds tokens in the @p contents buffer and fills the @ref token_stream.*/
  void tokenize(ParseSession* session);

  ParseSession* session;

private:
  /**Fills the scan table with method pointers.*/
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

  Problem createProblem() const;

private:
  Control *control;
  const char *cursor;
  std::size_t index;

  ///scan table contains pointers to the methods to scan for various token types
  static scan_fun_ptr s_scan_table[];
  static scan_fun_ptr s_scan_keyword_table[];
  static bool s_initialized;
};

#endif // LEXER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
