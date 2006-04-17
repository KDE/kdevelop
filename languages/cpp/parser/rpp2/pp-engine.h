/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef PP_ENGINE_H
#define PP_ENGINE_H

#include <QHash>
#include <QString>
#include <QStack>

#include "pp-macro.h"
#include "pp-macro-expander.h"
#include "pp-scanner.h"

class Preprocessor;

class pp
{
  QHash<QString, pp_macro*>& m_environment;
  pp_macro_expander expand;
  pp_skip_identifier skip_identifier;
  pp_skip_comment_or_divop skip_comment_or_divop;
  pp_skip_blanks skip_blanks;
  pp_skip_number skip_number;
  QStack<QString> m_files;
  QStack<int> m_includeLineNumbers;
  Preprocessor* m_preprocessor;

  enum { MAX_LEVEL = 512 };
  int _M_skipping[MAX_LEVEL];
  int _M_true_test[MAX_LEVEL];
  int iflevel;
  int lines;

  long token_value;
  QString token_text;

  enum TOKEN_TYPE
  {
    TOKEN_NUMBER = 1000,
    TOKEN_IDENTIFIER,
    TOKEN_DEFINED,
    TOKEN_LT_LT,
    TOKEN_LT_EQ,
    TOKEN_GT_GT,
    TOKEN_GT_EQ,
    TOKEN_EQ_EQ,
    TOKEN_NOT_EQ,
    TOKEN_OR_OR,
    TOKEN_AND_AND,
  };

  enum PP_DIRECTIVE_TYPE
  {
    PP_UNKNOWN_DIRECTIVE,
    PP_DEFINE,
    PP_INCLUDE,
    PP_ELIF,
    PP_ELSE,
    PP_ENDIF,
    PP_IF,
    PP_IFDEF,
    PP_IFNDEF,
    PP_UNDEF
  };

public:
  pp(Preprocessor* preprocessor, QHash<QString, pp_macro*>& environment);

  long eval_expression (Stream& input);

  QString processFile(const QString& filename);
  QString processFile(QIODevice* input);

  void operator () (Stream& input, Stream& output);

  QString createLineMark(const QString& filename, int line);

private:
  inline int skipping() const;
  bool test_if_level();

  PP_DIRECTIVE_TYPE find_directive (const QString& directive) const;

  QString find_header_protection(Stream& input);

  void skip(Stream& input, Stream& output);

  long eval_primary(Stream& input);

  long eval_multiplicative(Stream& input);

  long eval_additive(Stream& input);

  long eval_shift(Stream& input);

  long eval_relational(Stream& input);

  long eval_equality(Stream& input);

  long eval_and(Stream& input);

  long eval_xor(Stream& input);

  long eval_or(Stream& input);

  long eval_logical_and(Stream& input);

  long eval_logical_or(Stream& input);

  long eval_constant_expression(Stream& input);

  void handle_directive(const QString& directive, Stream& input, Stream& output);

  void handle_include(Stream& input, Stream& output);

  void handle_define(Stream& input);

  void handle_if(Stream& input);

  void handle_else();

  void handle_elif(Stream& input);

  void handle_endif();

  void handle_ifdef(bool check_undefined, Stream& input);

  void handle_undef(Stream& input);

  int next_token (Stream& input);
};

#endif // PP_ENGINE_H

// kate: indent-width 2;
