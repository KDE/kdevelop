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

#include <set>
#include <map>
#include <vector>
#include <string>
#include <iterator>
#include <iostream>
#include <fstream>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../rxx_allocator.h"

template <typename _CharT>
class mini_string
{
  typedef std::char_traits<_CharT> traits_type;
  typedef std::size_t size_type;

  _CharT const *_M_begin;
  std::size_t _M_size;

public:
  inline mini_string ():
    _M_begin (0), _M_size(0) {}

  explicit mini_string (std::string const &__s):
    _M_begin (__s.c_str ()), _M_size (__s.size ()) {}

  inline mini_string (_CharT const *__begin, std::size_t __size):
    _M_begin (__begin), _M_size (__size) {}

  inline _CharT const *begin () const { return _M_begin; }
  inline _CharT const *end () const { return _M_begin + _M_size; }

  inline int size () const { return _M_size; }

  inline int compare (mini_string const &__other) const
  {
    size_type const __size = this->size();
    size_type const __osize = __other.size();
    size_type const __len = std::min (__size,  __osize);

    int __r = traits_type::compare (_M_begin, __other._M_begin, __len);
    if (!__r)
      __r =  __size - __osize;

    return __r;
  }

  inline bool operator < (mini_string const &__other) const
  { return compare (__other) < 0; }
};

typedef mini_string<char> fast_string;

class symbol
{
  static rxx_allocator<char> allocator;
  static std::set<fast_string> string_set;

public:
  static fast_string const *get (char const *__data, std::size_t __size)
  {
    using namespace std;

    set<fast_string>::iterator it = string_set.find (fast_string (__data, __size));
    if (it == string_set.end ())
      {
        char *where = allocator.allocate (__size + 1);
        memcpy(where, __data, __size);
        where[__size] = '\0';
        fast_string str (where, __size);
        it = string_set.insert (str).first;
      }

    return &*it;
  }

  template <typename _InputIterator>
  static fast_string const *get (_InputIterator __first, _InputIterator __last)
  {
    assert ((__last - __first) < 512);
    char buffer[512], *cp = buffer;
    std::copy (__first, __last, cp);
    return get (buffer, __last - __first);
  }

  static fast_string const *get (std::string const &__s)
  { return get (__s.c_str (), __s.size ()); }
};

rxx_allocator<char> symbol::allocator;
std::set<fast_string> symbol::string_set;

struct pp_macro
{
  std::string definition;
  std::vector<fast_string const *> formals;

  union
  {
    int unsigned state;

    struct
    {
      int unsigned hidden: 1;
      int unsigned function_like: 1;
      int unsigned variadics: 1;
    };
  };

  inline pp_macro():
    state (0) {}
};

struct pp_environment: private std::map<fast_string const *, pp_macro>
{
  inline bool bind (fast_string const *__name, pp_macro const &__macro)
  { return insert (std::make_pair (__name, __macro)).second; }

  inline void unbind (fast_string const *__name)
  { erase (__name); }

  inline pp_macro *resolve (fast_string const *__name)
  {
    iterator it = find (__name);
    return it != end () ? &(*it).second : 0;
  }
};

struct pp_skip_white_spaces
{
  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    for (; __first != __last; ++__first)
      {
        if (! std::isspace (*__first))
          break;
      }

    return __first;
  }
};

struct pp_skip_blanks
{
  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    for (; __first != __last; ++__first)
      {
        if (*__first == '\\')
          {
            ++__first;
            if (__first == __last)
              break;
          }
        else if (*__first == '\n' || !std::isspace (*__first))
          break;
      }

    return __first;
  }
};

struct pp_skip_line
{
  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    while (__first != __last && *__first != '\n')
      {
        bool skip = (*__first++ == '\\');

        if (skip && __first != __last)
          ++__first;
      }

    return __first;
  }
};

struct pp_skip_comment_or_divop
{
  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      MAYBE_BEGIN,
      BEGIN,
      MAYBE_END,
      END,
      IN_COMMENT,
      IN_CXX_COMMENT
    } state (MAYBE_BEGIN);

    for (; __first != __last; ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case MAYBE_BEGIN:
              if (*__first != '/')
                return __first;

              state = BEGIN;
              break;

            case BEGIN:
              if (*__first == '*')
                state = IN_COMMENT;
              else if (*__first == '/')
                state = IN_CXX_COMMENT;
              else
                return __first;
              break;

            case IN_COMMENT:
              if (*__first == '*')
                state = MAYBE_END;
              break;

            case IN_CXX_COMMENT:
              if (*__first == '\n')
                state = END;
              break;

            case MAYBE_END:
              if (*__first == '/')
                state = END;
              else if (*__first != '*')
                state = IN_COMMENT;
              break;

            case END:
              return __first;
          }
      }

    return __first;
  }
};

struct pp_skip_identifier
{
  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    for (; __first != __last; ++__first)
      {
        if (! std::isalnum (*__first) && *__first != '_')
          break;
      }

    return __first;
  }
};

struct pp_skip_number
{
  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    for (; __first != __last; ++__first)
      {
        if (! std::isalnum (*__first) && *__first != '.')
          break;
      }

    return __first;
  }
};

struct pp_skip_string_literal
{
  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      BEGIN,
      IN_STRING,
      QUOTE,
      END
    } state (BEGIN);

    for (; __first != __last; ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case BEGIN:
              if (*__first != '\"')
                return __first;
              state = IN_STRING;
              break;

            case IN_STRING:
              assert (*__first != '\n');

              if (*__first == '\"')
                state = END;
              else if (*__first == '\\')
                state = QUOTE;
              break;

            case QUOTE:
              state = IN_STRING;
              break;

            case END:
              return __first;
          }
      }

    return __first;
  }
};

struct pp_skip_char_literal
{
  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    enum {
      BEGIN,
      IN_STRING,
      QUOTE,
      END
    } state (BEGIN);

    for (; state != END && __first != __last; ++__first)
      {
        switch (state)
          {
            default:
              assert (0);
              break;

            case BEGIN:
              if (*__first != '\'')
                return __first;
              state = IN_STRING;
              break;

            case IN_STRING:
              assert (*__first != '\n');

              if (*__first == '\'')
                state = END;
              else if (*__first == '\\')
                state = QUOTE;
              break;

            case QUOTE:
              state = IN_STRING;
              break;
          }
      }

    return __first;
  }
};

struct pp_skip_argument
{
  pp_skip_white_spaces skip_white_spaces;
  pp_skip_identifier skip_number;
  pp_skip_identifier skip_identifier;
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;
  pp_skip_comment_or_divop skip_comment_or_divop;

  template <typename _InputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last)
  {
    int depth = 0;

    while (__first != __last)
      {
        if (!depth && (*__first == ')' || *__first == ','))
          break;
        else if (*__first == '(')
          ++depth, ++__first;
        else if (*__first == ')')
          --depth, ++__first;
        else if (*__first == '\"')
          __first = skip_string_literal (__first, __last);
        else if (*__first == '\'')
          __first = skip_char_literal (__first, __last);
        else if (*__first == '/')
          __first = skip_comment_or_divop (__first, __last);
        else if (std::isalpha (*__first) || *__first == '_')
          __first = skip_identifier (__first, __last);
        else if (std::isdigit (*__first))
          __first = skip_number (__first, __last);
        else
          ++__first;
      }

    return __first;
  }
};

struct pp_frame
{
  pp_macro *expanding_macro;
  std::vector<std::string> *actuals;

  pp_frame (pp_macro *__expanding_macro, std::vector<std::string> *__actuals):
    expanding_macro (__expanding_macro), actuals (__actuals) {}
};

class pp_macro_expander
{
  pp_environment &env;
  pp_frame *frame;

  pp_skip_white_spaces skip_white_spaces;
  pp_skip_number skip_number;
  pp_skip_identifier skip_identifier;
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;
  pp_skip_argument skip_argument;
  pp_skip_comment_or_divop skip_comment_or_divop;
  pp_skip_blanks skip_blanks;

  std::string const *resolve_formal (fast_string const *__name)
  {
    assert (__name != 0);

    if (! frame)
      return 0;

    assert (frame->expanding_macro != 0);

    std::vector<fast_string const *> const formals = frame->expanding_macro->formals;
    for (std::size_t index = 0; index < formals.size(); ++index)
      {
        fast_string const *formal = formals[index];

        if (formal != __name)
          continue;
        else if (frame->actuals && index < frame->actuals->size())
          return &(*frame->actuals)[index];
        else
          assert (0); // internal error?
      }

    return 0;
  }

public:
  pp_macro_expander (pp_environment &__env, pp_frame *__frame = 0):
    env (__env), frame (__frame) {}

  template <typename _InputIterator, typename _OutputIterator>
  _InputIterator operator () (_InputIterator __first, _InputIterator __last, _OutputIterator __result)
  {
    __first = skip_blanks (__first, __last);

    while (__first != __last)
      {
        if (!frame && *__first == '#')
          {
            return __first;
          }
        else if (*__first == '#')
          {
            __first = skip_blanks (++__first, __last);

            _InputIterator end_id = skip_identifier (__first, __last);
            *__result++ = '\"';
            this->operator () (__first, end_id, __result);
            *__result++ = '\"';
            __first = end_id;
          }
        else if (*__first == '/')
          {
            _InputIterator next_pos = skip_comment_or_divop (__first, __last);
            std::copy (__first, next_pos, __result);
            __first = next_pos;
          }
        else if (*__first == '\"')
          {
            _InputIterator next_pos = skip_string_literal (__first, __last);
            std::copy (__first, next_pos, __result);
            __first = next_pos;
          }
        else if (*__first == '\'')
          {
            _InputIterator next_pos = skip_char_literal (__first, __last);
            std::copy (__first, next_pos, __result);
            __first = next_pos;
          }
        else if (*__first != '\n' && std::isspace (*__first))
          {
            for (; __first != __last; ++__first)
              {
                if (*__first == '\n' || !std::isspace (*__first))
                  break;
              }

            *__result = ' ';
          }
        else if (std::isdigit (*__first))
          {
            _InputIterator next_pos = skip_number (__first, __last);
            std::copy (__first, next_pos, __result);
            __first = next_pos;
          }
        else if (std::isalpha (*__first) || *__first == '_')
          {
            _InputIterator name_end = skip_identifier (__first, __last);
            fast_string const *name = symbol::get (__first, name_end);
            __first = name_end; // advance

            // search for the paste token
            _InputIterator next = skip_blanks (__first, __last);
            if (next != __last && *next == '#')
              {
                ++next;
                if (next != __last && *next == '#')
                  __first = skip_blanks(++next, __last);
              }

            if (std::string const *actual = resolve_formal (name))
              {
                std::copy (actual->begin (), actual->end (), __result);
                continue;
              }

            static bool hide_next = false;

            pp_macro *macro = env.resolve (name);
            if (! macro || macro->hidden || hide_next)
              {
                hide_next = (name == symbol::get ("defined", 7));
                std::copy (name->begin (), name->end (), __result);
                continue;
              }

            if (! macro->function_like)
              {
                std::string const &definition = macro->definition;
                std::copy (definition.begin (), definition.end (), __result);
                continue;
              }

            // function like macro
            _InputIterator arg_it = skip_white_spaces (__first, __last);

            if (arg_it == __last || *arg_it  != '(')
              {
                std::copy (name->begin (), name->end (), __result);
                __first = name_end;
                continue;
              }

            std::vector<std::string> actuals;
            actuals.reserve (5);
            ++arg_it; // skip '('

            pp_macro_expander expand_actual (env, frame);

            _InputIterator arg_end = skip_argument (arg_it, __last);
            if (arg_it != arg_end)
              {
                std::string actual (arg_it, arg_end);
                actuals.resize (actuals.size() + 1);
                expand_actual (actual.begin (), actual.end(), std::back_inserter (actuals.back()));
                arg_it = arg_end;
              }

            while (arg_it != __last && *arg_end == ',')
              {
                ++arg_it; // skip ','

                arg_end = skip_argument (arg_it, __last);
                std::string actual (arg_it, arg_end);
                actuals.resize (actuals.size() + 1);
                expand_actual (actual.begin (), actual.end(), std::back_inserter (actuals.back()));
                arg_it = arg_end;
              }

              assert (arg_it != __last && *arg_it == ')');

              ++arg_it; // skip ')'
              __first = arg_it;

#if 0 // ### enable me
              assert ((macro->variadics && macro->formals.size () >= actuals.size ())
                          || macro->formals.size() == actuals.size());
#endif

              pp_frame frame (macro, &actuals);
              pp_macro_expander expand_macro (env, &frame);
              macro->hidden = true;
              expand_macro (macro->definition.begin (), macro->definition.end (), __result);
              macro->hidden = false;
          }
        else
          *__result++ = *__first++;
      }

    return __first;
  }
};

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

  int find_include_file(std::string const &filename) const
  {
    assert (! filename.empty());

    if (filename[0] == '/')
      return  ::open (filename.c_str(), O_RDONLY, S_IRUSR);

    for (std::vector<std::string>::const_reverse_iterator it = include_paths.rbegin ();
        it != include_paths.rend (); ++it)
      {
        std::string path = *it;
        path += '/';
        path += filename;

        int fd = ::open (path.c_str(), O_RDONLY, S_IRUSR);
        if (fd != -1) {
          static std::set<std::string> opened; // ### remove me
          if (opened.find (filename) != opened.end())
            {
              close (fd);
              return -1;
            }

          opened.insert (filename);
          return fd;
        }
      }

    return -1;
  }

public:
  pp (pp_environment &__env):
    env (__env), expand (env)
  {
    iflevel = 0;
    _M_skipping[iflevel] = 0;
    _M_true_test[iflevel] = 0;

    include_paths
      .push_back(".");
 }

  template <typename _OutputIterator>
  void file (std::string const &filename, _OutputIterator __result)
  {
    int fd = ::open (filename.c_str(), O_RDONLY, S_IRUSR);
    if (fd != -1)
      file (fd, __result);
    else
      std::cerr << "** WARNING file ``" << filename << " not found!" << std::endl;
  }

  template <typename _OutputIterator>
  void file (int fd, _OutputIterator __result)
  {
    assert (fd != -1);

    struct stat st;
    fstat(fd, &st);
    std::size_t size = st.st_size;
    char *buffer = (char *) ::mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    close (fd);

    if (!buffer || buffer == (char*) -1)
      return;

    this->operator () (buffer, buffer + size, __result);
    ::munmap(buffer, size);
  }

  template <typename _OutputIterator>
  _InputIterator handle_include(_InputIterator __first, _InputIterator __last,
        _OutputIterator __result)
  {
    assert (*__first == '<' || *__first == '"');
    int quote = (*__first == '"') ? '"' : '>';
    ++__first;

    _InputIterator end_name = __first;
    for (; end_name != __last; ++end_name)
      {
        assert (*end_name != '\n');

        if (*end_name == quote)
          break;
      }

    std::string filename(__first, end_name);
    int fd = find_include_file (filename);
    if (fd != -1)
      file (fd, __result);

    return __first;
  }

  _InputIterator handle_define (_InputIterator __first, _InputIterator __last)
  {
    pp_macro macro;
    macro.definition.reserve (255);

    __first = skip_blanks (__first, __last);
    _InputIterator end_macro_name = skip_identifier (__first, __last);
    std::string macro_name (__first, end_macro_name);
    __first = end_macro_name;

    if (__first != __last && *__first == '(')
      {
        macro.function_like = true;
        macro.formals.reserve (5);

        __first = skip_blanks (++__first, __last); // skip '('
        _InputIterator arg_end = skip_identifier (__first, __last);
        if (__first != arg_end)
          macro.formals.push_back (symbol::get (__first, arg_end));

        __first = skip_blanks (arg_end, __last);

        if (*__first == '.')
          {
            std::cerr << "** WARNING variadics not supported" << std::endl;
            while (*__first == '.')
              ++__first;
          }

        while (__first != __last && *__first == ',')
          {
            __first = skip_blanks (++__first, __last);

            arg_end = skip_identifier (__first, __last);
            if (__first != arg_end)
              macro.formals.push_back (symbol::get (__first, arg_end));

            __first = skip_blanks (arg_end, __last);

            if (*__first == '.')
              {
                std::cerr << "** WARNING variadics not supported" << std::endl;
                while (*__first == '.')
                  ++__first;
              }
          }

        assert (*__first == ')');
        ++__first;
      }

    __first = skip_blanks (__first, __last);

    while (__first != __last && *__first != '\n')
      {
        if (*__first == '\\')
          {
            __first = skip_blanks (++__first, __last);
            macro.definition += ' ';
            continue;
          }

        macro.definition += *__first++;
      }

    env.bind (symbol::get (macro_name), macro);

    return __first;
  }

  _InputIterator skip (_InputIterator __first, _InputIterator __last)
  {
    pp_skip_comment_or_divop skip_comment_or_divop;
    pp_skip_string_literal skip_string_literal;
    pp_skip_char_literal skip_char_literal;

    while (__first != __last && *__first != '#')
      {
        if (*__first == '/')
          __first = skip_comment_or_divop (__first, __last);
        else if (*__first == '"')
          __first = skip_string_literal (__first, __last);
        else if (*__first == '\'')
          __first = skip_char_literal (__first, __last);
        else
          ++__first;
      }

    return __first;
  }

  template <typename _OutputIterator>
  void operator () (_InputIterator __first, _InputIterator __last, _OutputIterator __result)
  {
    while (__first != __last)
      {
        if (skipping ())
          __first = skip (__first, __last);
        else
          __first = expand (__first, __last, __result);

        if (__first != __last)
          {
            assert (*__first == '#');
            ++__first; // skip '#'
            __first = skip_white_spaces (__first, __last);

            _InputIterator end_id = skip_identifier (__first, __last);
            fast_string const *directive (symbol::get (__first, end_id));

            __first = skip_line (end_id, __last);
            (void) handle_directive (directive, end_id, __first, __result);
          }
      }
  }

private:
  enum { MAX_LEVEL = 512 };
  int _M_skipping[MAX_LEVEL];
  int _M_true_test[MAX_LEVEL];
  int iflevel;

  static fast_string const *pp_define;
  static fast_string const *pp_include;
  static fast_string const *pp_elif;
  static fast_string const *pp_else;
  static fast_string const *pp_endif;
  static fast_string const *pp_if;
  static fast_string const *pp_ifdef;
  static fast_string const *pp_ifndef;
  static fast_string const *pp_undef;

  template <typename _OutputIterator>
  _InputIterator handle_directive(fast_string const *d,
          _InputIterator __first, _InputIterator __last, _OutputIterator __result)
  {
    __first = skip_blanks (__first, __last);

    if (d == pp_define && !skipping ())
      __first = handle_define (__first, __last);
    else if (d == pp_include && !skipping ())
      return handle_include (__first, __last, __result);
    else if (d == pp_elif)
      return handle_elif (__first, __last);
    else if (d == pp_else)
      return handle_else (__first, __last);
    else if (d == pp_endif)
      return handle_endif (__first, __last);
    else if (d == pp_if)
      return handle_if (__first, __last);
    else if (d == pp_ifdef)
      return handle_ifdef (false, __first, __last);
    else if (d == pp_ifndef)
      return handle_ifdef (true, __first, __last);
    else if (d == pp_undef && !skipping ())
      return handle_undef(__first, __last);

    return __first;
  }

  bool test_if_level()
  {
    bool result = !_M_skipping[iflevel++];
    _M_skipping[iflevel] = _M_skipping[iflevel - 1];
    _M_true_test[iflevel] = false;
    return result;
  }

  inline int skipping() const
  { return _M_skipping[iflevel]; }

  _InputIterator eval_primary(_InputIterator __first, _InputIterator __last, long *result)
  {
    bool expect_paren = false;
    int token;
    __first = next_token (__first, __last, &token);

    switch (token)
      {
      case TOKEN_NUMBER:
        *result = token_value;
        break;

      case TOKEN_DEFINED:
        __first = next_token (__first, __last, &token);

        if (token == '(')
          {
            expect_paren = true;
            __first = next_token (__first, __last, &token);
          }

        if (token != TOKEN_IDENTIFIER)
          {
            std::cerr << "** WARNING expected ``identifier'' found:" << char(token) << std::endl;
            *result = 0;
            break;
          }

        *result = env.resolve (token_name) != 0;

        next_token (__first, __last, &token); // skip '('

        if (expect_paren)
          {
            _InputIterator next = next_token (__first, __last, &token);
            if (token != ')')
              std::cerr << "** WARNING expected ``)''" << std::endl;
            else
              __first = next;
          }
        break;

      case TOKEN_IDENTIFIER:
        // std::cerr << "** WARING unexpected identifier ``" << *token_name << "''" << std::endl;
        *result = 0;
        break;

      case '!':
        __first = eval_primary (__first, __last, result);
        *result = !*result;
        return __first;

      case '(':
        __first = eval_constant_expression(__first, __last, result);
        peek_token (__first, __last, &token);

        if (token != ')') {
          std::cerr << "** WARNING expected ``)'' = " << token << std::endl;
        }else
          __first = next_token(__first, __last, &token);
        break;

      default:
        *result = 0;
      }

    return __first;
  }

  _InputIterator eval_multiplicative(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_primary(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == '*' || token == '/' || token == '%')
      {
        long value;
        __first = eval_primary(next, __last, &value);

        if (token == '*')
          *result = *result * value;
        else if (token == '/')
          {
            if (value == 0)
              {
                std::cerr << "** WARNING division by zero" << std::endl;
                *result = 0;
              }
            else
              *result = *result / value;
          }
        else
          {
            if (value == 0)
              {
                std::cerr << "** WARNING division by zero" << std::endl;
                *result = 0;
              }
            else
              *result = *result % value;
          }
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_additive(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_multiplicative(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == '+' || token == '-')
      {
        long value;
        __first = eval_multiplicative(next, __last, &value);

        if (token == '+')
          *result = *result + value;
        else
          *result = *result - value;
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_shift(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_additive(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == TOKEN_LT_LT || token == TOKEN_GT_GT)
      {
        long value;
        __first = eval_additive (next, __last, &value);

        if (token == TOKEN_LT_LT)
          *result = *result << value;
        else
          *result = *result >> value;
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_relational(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_shift(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == '<'
        || token == '>'
        || token == TOKEN_LT_EQ
        || token == TOKEN_GT_EQ)
      {
        long value;
        __first = eval_shift(next, __last, &value);

        switch (token)
          {
            default:
              assert (0);
              break;

            case '<':
              *result = *result < value;
              break;

            case '>':
              *result = *result < value;
              break;

            case TOKEN_LT_EQ:
              *result = *result <= value;
              break;

            case TOKEN_GT_EQ:
              *result = *result >= value;
              break;
          }
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_equality(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_relational(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == TOKEN_EQ_EQ || token == TOKEN_NOT_EQ)
      {
        long value;
        __first = eval_relational(next, __last, &value);

        if (token == TOKEN_EQ_EQ)
          *result = *result == value;
        else
          *result = *result != value;
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_and(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_equality(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == '&')
      {
        long value;
        __first = eval_equality(next, __last, &value);
        *result = *result & value;
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_xor(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_and(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == '^')
      {
        long value;
        __first = eval_and(next, __last, &value);
        *result = *result ^ value;
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_or(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_xor(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == '|')
      {
        long value;
        __first = eval_xor(next, __last, &value);
        *result = *result | value;
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_logical_and(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_or(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == TOKEN_AND_AND)
      {
        long value;
        __first = eval_or(next, __last, &value);
        *result = *result && value;
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_logical_or(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_logical_and (__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    while (token == TOKEN_OR_OR)
      {
        long value;
        __first = eval_logical_and(next, __last, &value);
        *result = *result || value;
        next = next_token (__first, __last, &token);
      }

    return __first;
  }

  _InputIterator eval_constant_expression(_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = eval_logical_or(__first, __last, result);

    int token;
    _InputIterator next = next_token (__first, __last, &token);

    if (token == '?')
      {
        long left_value;
        __first = eval_constant_expression(next, __last, &left_value);
        __first = skip_blanks (__first, __last);

        __first = next_token(__first, __last, &token);
        if (token == ':')
          {
            long right_value;
            __first = eval_constant_expression(__first, __last, &right_value);

            *result = *result ? left_value : right_value;
          }
        else
          {
            std::cerr << "** WARNING expected ``:'' = " << int (token) << std::endl;
            *result = left_value;
          }
      }

    return __first;
  }

  _InputIterator eval_expression (_InputIterator __first, _InputIterator __last, long *result)
  {
    __first = skip_blanks (__first, __last);
    std::string o (__first, __last);
    __first = eval_constant_expression (__first, __last, result);
    // std::cerr << "eval expr:" << o << " ==> " << *result << std::endl;
    return __first;
  }

  _InputIterator handle_if (_InputIterator __first, _InputIterator __last)
  {
    if (test_if_level())
      {
        pp_macro_expander expand_condition (env);
        std::string condition;
        expand_condition (__first, __last, std::back_inserter (condition));

        long result = 0;
        eval_expression(condition.c_str (), condition.c_str () + condition.size (), &result);

        _M_true_test[iflevel] = result;
        _M_skipping[iflevel] = !result;
      }

    return __first;
  }

  _InputIterator handle_else (_InputIterator __first, _InputIterator /*__last*/)
  {
    if (iflevel == 0 && !skipping ())
      {
        std::cerr << "** WARNING #else without #if" << std::endl;
      }
    else if (iflevel > 0 && _M_skipping[iflevel - 1])
      {
        _M_skipping[iflevel] = true;
      }
    else
      {
        _M_skipping[iflevel] = _M_true_test[iflevel];
      }

    return __first;
  }

  _InputIterator handle_elif (_InputIterator __first, _InputIterator __last)
  {
    assert(iflevel > 0);

    if (iflevel == 0 && !skipping())
      {
        std::cerr << "** WARNING #else without #if" << std::endl;
      }
    else if (!_M_true_test[iflevel] && !_M_skipping[iflevel - 1])
      {
        long result;
        __first = eval_expression(__first, __last, &result);
        _M_true_test[iflevel] = result;
        _M_skipping[iflevel] = !result;
      }
    else
      {
        _M_skipping[iflevel] = true;
      }

    return __first;
  }

  _InputIterator handle_endif (_InputIterator __first, _InputIterator /*__last*/)
  {
    if (iflevel == 0 && !skipping())
      {
        std::cerr << "** WARNING #endif without #if" << std::endl;
      }
    else
      {
        _M_skipping[iflevel] = 0;
        _M_true_test[iflevel] = 0;

        --iflevel;
      }

    return __first;
  }

  _InputIterator handle_ifdef (bool check_undefined, _InputIterator __first, _InputIterator __last)
  {
    if (test_if_level())
      {
        _InputIterator macro_name_end = skip_identifier (__first, __last);
        std::string macro_name (__first, macro_name_end);
        __first = macro_name_end;

        bool value = env.resolve (symbol::get (macro_name)) != 0;

        if (check_undefined)
          value = !value;

        _M_true_test[iflevel] = value;
        _M_skipping[iflevel] = !value;
      }

    return __first;
  }

  _InputIterator handle_undef(_InputIterator __first, _InputIterator __last)
  {
    __first = skip_white_spaces (__first, __last);
    _InputIterator macro_name_end = skip_identifier (__first, __last);
    assert (macro_name_end != __first);

    std::string macro_name (__first, macro_name_end);
    __first = macro_name_end;

    env.unbind (symbol::get (macro_name));
    return __first;
  }

  union
  {
    long token_value;
    fast_string const *token_name;
  };

  char peek_char (_InputIterator __first, _InputIterator __last)
  {
    if (__first == __last)
      return 0;

    return *++__first;
  }

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

  inline _InputIterator peek_token (_InputIterator __first, _InputIterator __last, int *kind)
  {
    (void) next_token (__first, __last, kind);
    return __first;
  }

  _InputIterator next_token (_InputIterator __first, _InputIterator __last, int *kind)
  {
    __first = skip_blanks (__first, __last);

    if (__first == __last)
      {
        *kind = 0;
        return __first;
      }

    char ch = *__first;
    char ch2 = peek_char (__first, __last);

    switch (ch)
      {
        case '/':
          if (ch2 == '/' || ch2 == '*')
            {
              __first = skip_comment_or_divop (__first, __last);
              return next_token (__first, __last, kind);
            }
          *kind = '/';
          break;

        case '<':
          ++__first;
          if (ch2 == '<')
            {
              ++__first;
              *kind = TOKEN_LT_LT;
            }
          else if (ch2 == '=')
            {
              ++__first;
              *kind = TOKEN_LT_EQ;
            }
          else
            *kind = '<';

          return __first;

        case '>':
          ++__first;
          if (ch2 == '>')
            {
              ++__first;
              *kind = TOKEN_GT_GT;
            }
          else if (ch2 == '=')
            {
              ++__first;
              *kind = TOKEN_GT_EQ;
            }
          else
            *kind = '>';

          return __first;

        case '!':
          ++__first;
          if (ch2 == '=')
            {
              ++__first;
              *kind = TOKEN_NOT_EQ;
            }
          else
            *kind = '!';

          return __first;

        case '=':
          ++__first;
          if (ch2 == '=')
            {
              ++__first;
              *kind = TOKEN_EQ_EQ;
            }
          else
            *kind = '=';

          return __first;

        case '|':
          ++__first;
          if (ch2 == '|')
            {
              ++__first;
              *kind = TOKEN_OR_OR;
            }
          else
            *kind = '|';

          return __first;

        case '&':
          ++__first;
          if (ch2 == '&')
            {
              ++__first;
              *kind = TOKEN_AND_AND;
            }
          else
            *kind = '&';

          return __first;

        default:
          if (std::isalpha (ch) || ch == '_')
            {
              _InputIterator end = skip_identifier (__first, __last);
              token_name = symbol::get (__first, end);
              __first = end;

              if (token_name == symbol::get ("defined", 7))
                *kind = TOKEN_DEFINED;
              else
                *kind = TOKEN_IDENTIFIER;
            }
          else if (std::isdigit (ch))
            {
              _InputIterator end = skip_number (__first, __last);
              token_value = strtol (std::string (__first, __last).c_str (), 0, 0);
              __first = end;

              *kind = TOKEN_NUMBER;
            }
          else
            *kind = *__first++;
      }

    return __first;
  }
};

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_define = symbol::get ("define");

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_include = symbol::get ("include");

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_elif = symbol::get ("elif");

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_else = symbol::get ("else");

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_endif = symbol::get ("endif");

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_if = symbol::get ("if");

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_ifdef = symbol::get ("ifdef");

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_ifndef = symbol::get ("ifndef");

template <typename _OutputIterator>
fast_string const *pp<_OutputIterator>::pp_undef = symbol::get ("undef");

struct null_output_iterator
{
  typedef std::random_access_iterator_tag iterator_category;
  typedef char value_type;
  typedef char &reference;
  typedef char *pointer;
  typedef std::ptrdiff_t difference_type;
  char ch;

  null_output_iterator(): ch(0) {}

  char &operator * () { return ch; }
  null_output_iterator &operator ++ () { return *this; }
  null_output_iterator &operator ++ (int) { return *this; }
};

int main (int /*argc*/, char *argv[])
{
  char const *filename = *++argv;
  if (!filename)
    {
      std::cerr << "usage: pp file.cpp" << std::endl;
      return EXIT_FAILURE;
    }

  pp_environment env;

  pp<> preprocess(env);

  null_output_iterator null_out;
  preprocess.file ("pp-configuration", null_out); // ### put your macros here!
  preprocess.file (filename, std::ostream_iterator<char> (std::cout));

  return EXIT_SUCCESS;
}

