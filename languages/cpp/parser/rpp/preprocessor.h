/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "symbol.h"
#include "smallobject.h"
#include "list.h"

#include <list>
#include <map>
#include <set>
#include <string>

struct buffered_file
{
  std::size_t pos;
  std::size_t file_size;
  int saved_ch;
  char *buffer;
  std::string filename;
  int lineno;

  inline buffered_file(): pos(0), lineno(1) {}
  ~buffered_file();

  bool open(std::string const &fn);
  inline char inp()
  {
#ifdef SAFE_INP
    return pos < file_size ? buffer[pos++] : 0;
#else
    return buffer[pos++];
#endif
  }
};

class preprocessor
{
public:
  typedef std::list<std::string> paths_type;
  typedef paths_type::iterator paths_iterator;
  typedef paths_type::const_iterator paths_const_iterator;

  enum INCLUDE_POLICY {
    LocalInclude,
    GlobaInclude
  };

  enum TOKEN_ENUM {
    PP_CHAR_LITERAL = 1000,
    PP_IDENTIFIER,
    PP_NUMBER,
    PP_PREPROC,
    PP_STRING_LITERAL,
    PP_WHITESPACES,
    PP_WORD,

    PP_ADD,
    PP_AND,
    PP_AND_AND,
    PP_COLON,
    PP_DIVIDE,
    PP_EQ_EQ,
    PP_GT,
    PP_GT_EQ,
    PP_LT,
    PP_LT_EQ,
    PP_NOT,
    PP_NOT_EQ,
    PP_OR,
    PP_OR_OR,
    PP_QUESTION,
    PP_REMAINDER,
    PP_STAR,
    PP_SUB,
    PP_XOR
  };

  enum DIRECTIVE_ENUM {
    PP_UNKNOWN,
    PP_DEFINE,
    PP_ELIF,
    PP_ELSE,
    PP_ENDIF,
    PP_IF,
    PP_IFDEF,
    PP_IFNDEF,
    PP_INCLUDE,
    PP_UNDEF
  };

public:
  paths_type system_include_paths;
  paths_type include_paths;

public:
  preprocessor();
  ~preprocessor();

  void process_file(std::string const &filename);

  std::string find_include_file(std::string const &filename,
		    INCLUDE_POLICY include_policy = GlobaInclude) const;

private:
  inline void inp();	// ... character
  int  next_token();	// ... token
  void skip_line();     // ... line
  void tokenize();	// ... file

  inline int skipping() const
  { return _M_skipping[iflevel]; }

  void handle_directive(DIRECTIVE_ENUM directive);
  void handle_define();
  void handle_elif();
  void handle_else();
  void handle_endif();
  void handle_if();
  void handle_ifdef(DIRECTIVE_ENUM pp = PP_IFDEF);
  void handle_ifndef() { handle_ifdef(PP_IFNDEF); }
  void handle_include();
  void handle_undef();

  void scan_char();
  void scan_identifier();
  void scan_number();
  void scan_whitespaces();
  void scan_newline();
  void scan_preproc();
  void scan_quote();
  void scan_slash();
  void scan_backslash();

  bool macro_defined(char const *data, std::size_t size) const;
  bool test_if_level();

  long eval_primary();
  long eval_multiplicative();
  long eval_additive();
  long eval_relational();
  long eval_equality();
  long eval_and();
  long eval_xor();
  long eval_or();
  long eval_logical_and();
  long eval_logical_or();
  long eval_constant_expression();
  long eval_expression();

  long token_long_value() const;
  void token_string_value(char const **data, std::size_t *size) const;

  static std::size_t file_size(std::string const &filename);
  static DIRECTIVE_ENUM preproc_type(char const *data, std::size_t size);

private:
  struct _String
  {
    char const *data;
    std::size_t size;

    inline _String()
      : data(0), size(0) {}

    inline _String(char const *d, std::size_t s)
      : data(d), size(s) {}

    inline std::string as_string() const
    { return std::string(data, size); }
  };

  struct _String_compare
  {
    inline bool operator()(_String const &a, _String const &b) const
    {
      return std::lexicographical_compare(a.data, a.data + a.size,
					  b.data, b.data + b.size);
    }
  };

  _String read_identifier();
  int index_of_formal(const ListNode<_String> *formals, _String const &ide) const;

private:
  // rpp's state
  int ch;
  int token;
  int _M_skipping[512];
  int _M_true_test[512];
  int iflevel;
  buffered_file *_M_current_file;

  bool produced;
  std::size_t _M_token_start;
  std::size_t _M_token_end;

  // flags
  bool _M_accept_preproc;
  bool _M_check_header_protection;
  bool _M_macro_expr;

  // macro table and macro cache
  struct _Macro
  {
    bool enabled;
    bool fun_like;
    const ListNode<_String> *formals;
    _String value;
  };

  _Macro *resolve_macro(char const *data, std::size_t size);
  void collect_one_argument(ListNode<_String> const **argument);
  void collect_arguments(ListNode<ListNode<_String> const*> const **arguments);

  typedef std::map<name_symbol const*, _Macro> macro_table;
  typedef void (preprocessor::*scan_fun_type)();
  typedef std::map<std::string, name_symbol const*> header_protection_table;

  pool _M_memory_pool;
  name_table _M_symbol_table;
  macro_table _M_macro_table;
  header_protection_table _M_header_protection;

  static scan_fun_type _S_scan_table[];
  static bool _S_scan_table_initialized;
};

inline void preprocessor::inp()
{
  do { ch = _M_current_file->inp(); }
  while (ch < 0);
}

#endif // PREPROCESSOR_H

// kate: space-indent on; indent-width 2; replace-tabs on;
