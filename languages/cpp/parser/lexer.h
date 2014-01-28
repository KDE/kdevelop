/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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
#include <cppparserexport.h>
#include <QtCore/QString>
#include <cstdlib>
#include <language/duchain/indexedstring.h>

#include <language/duchain/problem.h>

struct NameSymbol;
class Lexer;
class Control;
class ParseSession;

typedef void (Lexer::*scan_fun_ptr)();

/**Token.*/
class KDEVCPPPARSER_EXPORT Token
{
public:
  ///position in the preprocessed buffer
  uint position;
  ///size of the token in the preprocessed buffer. Do not confuse this with symbolLength.
  uint size;
  ///kind of the token @see TOKEN_KIND enum reference.
  quint16 kind;

  inline bool operator==(const Token& o) const
  {
    return o.position == position && o.size == size && o.kind == kind;
  }
};

Q_DECLARE_TYPEINFO(Token, Q_PRIMITIVE_TYPE);

/**Stream of tokens found by lexer.
Internally works like an array of @ref Token continuosly allocated.
All tokens are destructed when this stream is deleted.

The stream has a "cursor" which is simply an integer which defines
the offset (index) of the token currently "observed" from the beginning of
the stream.

TODO: reuse some pool / container class for the token array
NOTE: token_count is actually the *size* of the token pool
      the last actually used token is lastToken
*/
class KDEVCPPPARSER_EXPORT TokenStream : public QVector<Token>
{
private:
  TokenStream(const TokenStream &);
  void operator = (const TokenStream &);

public:
  /**Creates a token stream with the default reserved size of 1024 tokens.*/
  inline TokenStream(ParseSession* _session, uint size = 1024)
    : session(_session)
    , index(0)
  {
    reserve(size);
  }

  /**@return the token at position @p index.*/
  inline const Token &token(int index) const
  { return at(index); }

  /**@return the "cursor" - the offset (index) of the token
  currently "observed" from the beginning of the stream.*/
  inline uint cursor() const
  { return index; }

  /**Sets the cursor to the position @p i.*/
  inline void rewind(int i)
  { index = i; }

  /**Updates the cursor position to point to the next token and returns
  the cursor.*/
  inline uint nextToken()
  { return index++; }

  /**@return the kind of the next (LA) token in the stream.*/
  inline quint16 lookAhead(uint i = 0) const
  { return at(index + i).kind; }

  /**@return the kind of the current token in the stream.*/
  inline quint16 kind(uint i) const
  { return at(i).kind; }

  /**@return the position of the current token in the c++ source buffer.*/
  inline uint position(uint i) const
  { return at(i).position; }

  /**
   * @return The symbol associated to the token.
   *
   * @note This only works if this is a simple symbol, i.e.
   * only consisting of one identifier (not comments).
   * does not work for operators like "->" or numbers like "50"
   */
  KDevelop::IndexedString symbol(const Token& t) const;
  inline KDevelop::IndexedString symbol(uint i) const
  { return symbol(token(i)); }

  /**
   * @return The index of the symbol associated to the token.
   *
   * @note The notes from @c symbol() apply as well.
   */
  uint symbolIndex(const Token& t) const;
  inline uint symbolIndex(uint i) const
  { return symbolIndex(token(i)); }

  /**
   * @return The string representation of the token.
   *
   * @note This always works but is expensive
   */
  QString symbolString(const Token& t) const;
  inline QString symbolString(uint i) const
  { return symbolString(token(i)); }

  /**
   * @return The bytearray representation of the token.
   *
   * @note This always works but is expensive
   */
  QByteArray symbolByteArray(const Token& t) const;
  inline QByteArray symbolByteArray(uint i) const
  { return symbolByteArray(token(i)); }

  /**
   * @return The length of this tokens text representation
   */
  uint symbolLength(const Token& t) const;
  inline uint symbolLength(uint i) const
  { return symbolLength(token(i)); }

  /**
   * Split the right shift token at @p index into two distinct right angle brackets.
   * 
   * Required to support 14.2/3 of the spec, see also:
   * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1757.html
   */
  void splitRightShift(uint index);

private:
  ParseSession* session;
  uint index;
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
  void skipComment();
  /**Fills the scan table with method pointers.*/
  void initialize_scan_table();
  void scan_newline();
  void scan_white_spaces();
  void scan_identifier_or_keyword();
  void scan_identifier_or_literal();
  void scan_int_constant();
  void scan_char_constant();
  void scan_string_constant();
  void scan_raw_string_constant();
  void scan_raw_string_or_identifier();
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

  KDevelop::ProblemPointer createProblem() const;

private:
  Control *control;
  
  struct SpecialCursor {
    bool operator==(uint index) const {
      return *current == index;
    }
    bool operator==(char character) const {
      return *current == (character | 0xffff0000);
    }
    bool isChar() const {
      return ((*current) & 0xffff0000) == 0xffff0000;
    }
    inline char operator*() const {
      if(isChar())
        return (char)*current;
      else
        return 'a'; //Return a valid character, because the identifiers created by the preprocessor are alpha-numerical
    }
    void operator++() {
      ++current;
    }
    void operator+=(int offset) {
      current += offset;
    }
    bool operator !=(const SpecialCursor& rhs) const {
      return current != rhs.current;
    }
    bool operator !=(const uint* rhs) const {
      return current != rhs;
    }
    void operator--() {
      --current;
    }
    bool operator<(const uint* end) const {
      return current < end;
    }
    
    int operator -(const SpecialCursor& rhs) const {
      return (((char*)current) - ((char*)rhs.current)) / sizeof(uint);
    }
    
    uint offsetIn(const uint* base) const {
      return ((char*)current - (char*)base) / sizeof(uint);
    }
    
    SpecialCursor operator +(int offset) {
      SpecialCursor ret(*this);
      ret.current += offset;
      return ret;
    }

    // useful for debugging
    QString toString() const;

    uint* current;
  };
  
  SpecialCursor cursor;
  const uint* endCursor;
  uint index;

  bool m_leaveSize; //Marks the current token that its size should not be automatically set
  bool m_canMergeComment; //Whether we may append new comments to the last encountered one
  bool m_firstInLine;   //Whether the next token is the first one in a line
  
  ///scan table contains pointers to the methods to scan for various token types
  static scan_fun_ptr s_scan_table[];
  static scan_fun_ptr s_scan_keyword_table[];
  static bool s_initialized;
};

#endif // LEXER_H

