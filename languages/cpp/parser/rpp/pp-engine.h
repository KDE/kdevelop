/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

//krazy:excludeall=inline

#include <QtCore/QHash>

#include <QtCore/QStack>
#include "cpprppexport.h"

#include <language/duchain/problem.h>

#include "pp-macro.h"
#include "pp-macro-expander.h"
#include "pp-scanner.h"

typedef QVector<unsigned int> PreprocessedContents;

namespace rpp {

class Preprocessor;
class Environment;

struct Value
{
  Value() : kind(Kind_Long), l(0) {}

  enum Kind {
    Kind_Long,
    Kind_ULong
  };

  Kind kind;

  union {
    long long l;
    unsigned long long ul;
  };

  inline bool is_ulong () const { return kind == Kind_ULong; }

  inline void set_ulong (unsigned long v)
  {
    ul = v;
    kind = Kind_ULong;
  }

  inline void set_long (long v)
  {
    l = v;
    kind = Kind_Long;
  }

  inline bool is_zero () const { return l == 0; }

#define PP_DEFINE_BIN_OP2(op) \
  inline Value operator op(const Value &other) \
  { \
    Value ret; \
    if (is_ulong () || other.is_ulong ()) \
      ret.set_ulong (ul op other.ul); \
    else \
      ret.set_long (l op other.l); \
    return ret; \
  }

#define PP_DEFINE_BIN_OP(op, op2) \
  inline Value &operator op2(const Value &other) \
  { \
    if (is_ulong () || other.is_ulong ()) \
      set_ulong (ul op other.ul); \
    else \
      set_long (l op other.l); \
    return *this; \
  }

  PP_DEFINE_BIN_OP(+, +=)
  PP_DEFINE_BIN_OP(-, -=)
  PP_DEFINE_BIN_OP(*, *=)
  PP_DEFINE_BIN_OP(/, /=)
  PP_DEFINE_BIN_OP(%, %=)
  PP_DEFINE_BIN_OP(<<, <<=)
  PP_DEFINE_BIN_OP(>>, >>=)
  PP_DEFINE_BIN_OP(&, &=)
  PP_DEFINE_BIN_OP(|, |=)
  PP_DEFINE_BIN_OP(^, ^=)
  PP_DEFINE_BIN_OP2(<)
  PP_DEFINE_BIN_OP2(>)
  PP_DEFINE_BIN_OP2(!=)
  PP_DEFINE_BIN_OP2(==)
  PP_DEFINE_BIN_OP2(<=)
  PP_DEFINE_BIN_OP2(>=)
  PP_DEFINE_BIN_OP2(&&)
  PP_DEFINE_BIN_OP2(||)

#undef PP_DEFINE_BIN_OP
};


class KDEVCPPRPP_EXPORT pp
{
  Environment* m_environment;
  pp_macro_expander expand;
  pp_skip_identifier skip_identifier;
  pp_skip_comment_or_divop skip_comment_or_divop;
  pp_skip_blanks skip_blanks;
  pp_skip_number skip_number;
  QStack<KDevelop::IndexedString> m_files;
  Preprocessor* m_preprocessor;
  QList<KDevelop::ProblemPointer> m_problems;

  enum { MAX_LEVEL = 512 };
  int _M_skipping[MAX_LEVEL];
  int _M_true_test[MAX_LEVEL];
  int iflevel;
  int nextToken;
  bool haveNextToken;
  bool hideNext;
  bool hadGuardCandidate, checkGuardEnd;
  KDevelop::IndexedString guardCandidate;

  union {
    long long token_value;
    unsigned long long token_uvalue;
  };
  KDevelop::IndexedString token_text;

  enum TOKEN_TYPE
  {
    TOKENS_START = 1000,
    TOKEN_NUMBER,
    TOKEN_UNUMBER,
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
    TOKENS_END
  };

public:
  pp(Preprocessor* preprocessor);
  ~pp();

  Preprocessor* preprocessor();

  enum StringType { File, Data };

  ///@todo Remove
  inline QString currentFileNameString () const { return m_files.top().str(); }
  
  inline KDevelop::IndexedString currentFileName () const { return m_files.top(); }

  Value eval_expression (Stream& input);

  /** Read file @p fileName and preprocess
   *  Currently the file is expected to be utf8-encoded. */
  PreprocessedContents processFile(const QString& fileName);

  /** Preprocess @p fileName with content @p data. Do not actually open file @p fileName
   *  Currently the file is expected to be utf8-encoded. */
  PreprocessedContents processFile(const QString& fileName, const QByteArray& data);

  void operator () (Stream& input, Stream& output);

  void checkMarkNeeded(Stream& input, Stream& output);

  bool hideNextMacro() const;
  void setHideNextMacro(bool hideNext);

  Environment* environment() const;
  // once set, belongs to the engine
  void setEnvironment(Environment* env);

  const QList<KDevelop::ProblemPointer>& problems() const;
  void problemEncountered(const KDevelop::ProblemPointer& problem);

  //Returns a hash-value computed from all until currently open branching-conditions and their decisions(like #ifdef's)
  uint branchingHash() const;
  
private:
  void processFileInternal(const QString& fileName, const QByteArray& fileContent, PreprocessedContents& result);

  int skipping() const;
  bool test_if_level();

  void createProblem(Stream& input, const QString& description);
  
  void skip(Stream& input, Stream& output, bool outputText = true);

  Value eval_primary(Stream& input);

  Value eval_multiplicative(Stream& input);

  Value eval_additive(Stream& input);

  Value eval_shift(Stream& input);

  Value eval_relational(Stream& input);

  Value eval_equality(Stream& input);

  Value eval_and(Stream& input);

  Value eval_xor(Stream& input);

  Value eval_or(Stream& input);

  Value eval_logical_and(Stream& input);

  Value eval_logical_or(Stream& input);

  Value eval_constant_expression(Stream& input);

  void handle_directive(uint directive, Stream& input, Stream& output);

  void handle_include(bool skip_current_path, Stream& input, Stream& output);

  void handle_define(Stream& input);

  void handle_if(Stream& input);

  void handle_else(int sourceLine);

  void handle_elif(Stream& input);

  void handle_endif(Stream& input, Stream& output);

  void handle_ifdef(bool check_undefined, Stream& input);

  void handle_undef(Stream& input);

  int next_token (Stream& input);
  int next_token_accept (Stream& input);
  void accept_token();
};

}

#endif // PP_ENGINE_H


