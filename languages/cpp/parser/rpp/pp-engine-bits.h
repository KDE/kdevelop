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

#ifndef PP_ENGINE_BITS_H
#define PP_ENGINE_BITS_H

template <typename _InputIterator>
  template <typename _OutputIterator>
  void pp<_InputIterator>::file (std::string const &filename, _OutputIterator __result)
  {
    int fd = ::open (filename.c_str(), O_RDONLY, S_IRUSR);
    if (fd != -1)
      file (fd, __result);
    else
      std::cerr << "** WARNING file ``" << filename << " not found!" << std::endl;
  }

template <typename _InputIterator>
  template <typename _OutputIterator>
  void pp<_InputIterator>::file (int fd, _OutputIterator __result)
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

template <typename _InputIterator>
  template <typename _OutputIterator>
  _InputIterator pp<_InputIterator>::handle_include(_InputIterator __first, _InputIterator __last,
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
      {
#ifdef QT_MOC
        std::string moc_msg;
        moc_msg += "#moc_include_begin ";
        moc_msg += "\"";
        moc_msg += filename;
        moc_msg += "\"\n";
        std::copy (moc_msg.begin (), moc_msg.end (), __result);
#endif

        file (fd, __result);

#ifdef QT_MOC
        moc_msg = "#moc_include_end 1\n";
        std::copy (moc_msg.begin (), moc_msg.end (), __result);
#endif
      }

    return __first;
  }

template <typename _InputIterator>
  template <typename _OutputIterator>
  void pp<_InputIterator>::operator () (_InputIterator __first, _InputIterator __last, _OutputIterator __result)
  {
    while (true)
      {
        __first = skip_white_spaces (__first, __last);
        if (__first == __last)
          break;
        else if (*__first == '#')
          {
            assert (*__first == '#');
            __first = skip_blanks (++__first, __last);

            _InputIterator end_id = skip_identifier (__first, __last);
            pp_fast_string const *directive (pp_symbol::get (__first, end_id));

            end_id = skip_blanks (end_id, __last);
            __first = skip (end_id, __last);

            (void) handle_directive (directive, end_id, __first, __result);
          }
        else if (*__first == '\n')
          {
            // ### compress the line
            *__result++ = *__first++;
          }
        else if (skipping ())
          __first = skip (__first, __last);
        else
          __first = expand (__first, __last, __result);
      }
  }

template <typename _InputIterator>
  template <typename _OutputIterator>
  _InputIterator pp<_InputIterator>::handle_directive(pp_fast_string const *d,
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

template <typename _InputIterator>
int pp<_InputIterator>::find_include_file(std::string const &filename) const
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
#if 0
        static std::set<std::string> opened; // ### remove me
        if (opened.find (filename) != opened.end())
          {
            close (fd);
            return -1;
          }

        opened.insert (filename);
#endif
        return fd;
      }
    }

  return -1;
}

template <typename _InputIterator>
pp<_InputIterator>::pp (pp_environment &__env):
  env (__env), expand (env)
{
  iflevel = 0;
  _M_skipping[iflevel] = 0;
  _M_true_test[iflevel] = 0;

  if (_S_initialized)
    return;

  pp_define = pp_symbol::get ("define", 6);
  pp_include = pp_symbol::get ("include", 7);
  pp_elif = pp_symbol::get ("elif", 4);
  pp_else = pp_symbol::get ("else", 4);
  pp_endif = pp_symbol::get ("endif", 5);
  pp_if = pp_symbol::get ("if", 2);
  pp_ifdef = pp_symbol::get ("ifdef", 5);
  pp_ifndef = pp_symbol::get ("ifndef", 6);
  pp_undef = pp_symbol::get ("undef", 5);
  _S_initialized = true;
}

template <typename _InputIterator>
inline std::back_insert_iterator<std::vector<std::string> > pp<_InputIterator>::include_paths_inserter ()
{ return std::back_inserter (include_paths); }

template <typename _InputIterator>
inline std::vector<std::string>::iterator pp<_InputIterator>::include_paths_begin ()
{ return include_paths.begin (); }

template <typename _InputIterator>
inline std::vector<std::string>::iterator pp<_InputIterator>::include_paths_end ()
{ return include_paths.end (); }

template <typename _InputIterator>
inline std::vector<std::string>::const_iterator pp<_InputIterator>::include_paths_begin () const
{ return include_paths.begin (); }

template <typename _InputIterator>
inline std::vector<std::string>::const_iterator pp<_InputIterator>::include_paths_end () const
{ return include_paths.end (); }

template <typename _InputIterator>
inline void pp<_InputIterator>::push_include_path (std::string const &__path)
{ include_paths.push_back (__path); }

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::handle_define (_InputIterator __first, _InputIterator __last)
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
        macro.formals.push_back (pp_symbol::get (__first, arg_end));

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
            macro.formals.push_back (pp_symbol::get (__first, arg_end));

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
          if (__first != __last && *__first == '\n')
            __first = skip_blanks (++__first, __last);

          macro.definition += ' ';
          continue;
        }

      macro.definition += *__first++;
    }

  env.bind (pp_symbol::get (macro_name), macro);

  return __first;
}

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::skip (_InputIterator __first, _InputIterator __last)
{
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;

  while (__first != __last && *__first != '\n')
    {
      if (*__first == '/')
        __first = skip_comment_or_divop (__first, __last);
      else if (*__first == '"')
        __first = skip_string_literal (__first, __last);
      else if (*__first == '\'')
        __first = skip_char_literal (__first, __last);
      else if (*__first == '\\')
        {
          __first = skip_blanks (++__first, __last);

          if (__first != __last && *__first == '\n')
            ++__first;
        }
      else
        ++__first;
    }

  return __first;
}

template <typename _InputIterator>
bool pp<_InputIterator>::test_if_level()
{
  bool result = !_M_skipping[iflevel++];
  _M_skipping[iflevel] = _M_skipping[iflevel - 1];
  _M_true_test[iflevel] = false;
  return result;
}

template <typename _InputIterator>
inline int pp<_InputIterator>::skipping() const
{ return _M_skipping[iflevel]; }

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_primary(_InputIterator __first, _InputIterator __last, long *result)
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
      *result = 0;
      break;

    case '!':
      __first = eval_primary (__first, __last, result);
      *result = !*result;
      return __first;

    case '(':
      __first = eval_constant_expression(__first, __last, result);
      next_token (__first, __last, &token);

      if (token != ')')
        std::cerr << "** WARNING expected ``)'' = " << token << std::endl;
      else
        __first = next_token(__first, __last, &token);
      break;

    default:
      *result = 0;
    }

  return __first;
}

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_multiplicative(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_additive(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_shift(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_relational(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_equality(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_and(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_xor(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_or(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_logical_and(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_logical_or(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_constant_expression(_InputIterator __first, _InputIterator __last, long *result)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::eval_expression (_InputIterator __first, _InputIterator __last, long *result)
{
  return __first = eval_constant_expression (skip_blanks (__first, __last), __last, result);
}

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::handle_if (_InputIterator __first, _InputIterator __last)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::handle_else (_InputIterator __first, _InputIterator /*__last*/)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::handle_elif (_InputIterator __first, _InputIterator __last)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::handle_endif (_InputIterator __first, _InputIterator /*__last*/)
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

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::handle_ifdef (bool check_undefined, _InputIterator __first, _InputIterator __last)
{
  if (test_if_level())
    {
      _InputIterator end_macro_name = skip_identifier (__first, __last);
      std::string macro_name (__first, end_macro_name);
      bool value = env.resolve (pp_symbol::get (__first, end_macro_name)) != 0;
      __first = end_macro_name;

      if (check_undefined)
        value = !value;

      _M_true_test[iflevel] = value;
      _M_skipping[iflevel] = !value;
    }

  return __first;
}

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::handle_undef(_InputIterator __first, _InputIterator __last)
{
  __first = skip_white_spaces (__first, __last);
  _InputIterator end_macro_name = skip_identifier (__first, __last);
  assert (end_macro_name != __first);

  env.unbind (pp_symbol::get (__first, end_macro_name));
  __first = end_macro_name;

  return __first;
}

template <typename _InputIterator>
char pp<_InputIterator>::peek_char (_InputIterator __first, _InputIterator __last)
{
  if (__first == __last)
    return 0;

  return *++__first;
}

template <typename _InputIterator>
_InputIterator pp<_InputIterator>::next_token (_InputIterator __first, _InputIterator __last, int *kind)
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
        ++__first;
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
            token_name = pp_symbol::get (__first, end);
            __first = end;

            if (token_name == pp_symbol::get ("defined", 7))
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

template <typename _InputIterator>
bool pp<_InputIterator>::_S_initialized = false;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_define;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_include;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_elif;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_else;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_endif;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_if;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_ifdef;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_ifndef;

template <typename _InputIterator>
pp_fast_string const *pp<_InputIterator>::pp_undef;

#endif // PP_ENGINE_BITS_H
