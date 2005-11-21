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

template <typename _InputIterator = char const*>
class pp
{
  pp_environment &env;
  pp_macro_expander expand;
  pp_skip_white_spaces skip_white_spaces;
  pp_skip_identifier skip_identifier;
  pp_skip_line skip_line;
  pp_skip_comment_or_divop skip_comment_or_divop;
  pp_skip_blanks skip_blanks;
  pp_skip_number skip_number;
  std::vector<std::string> include_paths;

  enum { MAX_LEVEL = 512 };
  int _M_skipping[MAX_LEVEL];
  int _M_true_test[MAX_LEVEL];
  int iflevel;

  union
  {
    long token_value;
    pp_fast_string const *token_name;
  };

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

  static bool _S_initialized;
  static pp_fast_string const *pp_define;
  static pp_fast_string const *pp_include;
  static pp_fast_string const *pp_elif;
  static pp_fast_string const *pp_else;
  static pp_fast_string const *pp_endif;
  static pp_fast_string const *pp_if;
  static pp_fast_string const *pp_ifdef;
  static pp_fast_string const *pp_ifndef;
  static pp_fast_string const *pp_undef;

public:
  pp (pp_environment &__env);

  inline std::back_insert_iterator<std::vector<std::string> > include_paths_inserter ();

  inline void push_include_path (std::string const &__path);

  inline std::vector<std::string>::iterator include_paths_begin ();
  inline std::vector<std::string>::iterator include_paths_end ();

  inline std::vector<std::string>::const_iterator include_paths_begin () const;
  inline std::vector<std::string>::const_iterator include_paths_end () const;

  inline _InputIterator eval_expression (_InputIterator __first, _InputIterator __last, long *result);

  template <typename _OutputIterator>
  void file (std::string const &filename, _OutputIterator __result);

  template <typename _OutputIterator>
  void file (int fd, _OutputIterator __result);

  template <typename _OutputIterator>
  void operator () (_InputIterator __first, _InputIterator __last, _OutputIterator __result);

private:
  int find_include_file(std::string const &filename) const;

  inline int skipping() const;
  bool test_if_level();

  _InputIterator skip (_InputIterator __first, _InputIterator __last);

  _InputIterator eval_primary(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_multiplicative(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_additive(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_shift(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_relational(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_equality(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_and(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_xor(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_or(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_logical_and(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_logical_or(_InputIterator __first, _InputIterator __last, long *result);
  _InputIterator eval_constant_expression(_InputIterator __first, _InputIterator __last, long *result);

  template <typename _OutputIterator>
  _InputIterator handle_directive(pp_fast_string const *d,
          _InputIterator __first, _InputIterator __last, _OutputIterator __result);

  template <typename _OutputIterator>
  _InputIterator handle_include(_InputIterator __first, _InputIterator __last,
        _OutputIterator __result);

  _InputIterator handle_define (_InputIterator __first, _InputIterator __last);
  _InputIterator handle_if (_InputIterator __first, _InputIterator __last);
  _InputIterator handle_else (_InputIterator __first, _InputIterator __last);
  _InputIterator handle_elif (_InputIterator __first, _InputIterator __last);
  _InputIterator handle_endif (_InputIterator __first, _InputIterator __last);
  _InputIterator handle_ifdef (bool check_undefined, _InputIterator __first, _InputIterator __last);
  _InputIterator handle_undef(_InputIterator __first, _InputIterator __last);

  inline char peek_char (_InputIterator __first, _InputIterator __last);
  _InputIterator next_token (_InputIterator __first, _InputIterator __last, int *kind);
};

#endif // PP_ENGINE_H
