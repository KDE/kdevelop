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

#if (_MSC_VER >= 1400)
#  define FILENO _fileno
#else
#  define FILENO fileno
#endif

#if defined (PP_OS_WIN)
#  define PATH_SEPARATOR '\\'
#else
#  define PATH_SEPARATOR '/'
#endif

inline std::string pp::fix_file_path(std::string const &filename) const
{
#if defined (PP_OS_WIN)
    std::string s = filename;
    for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
        if (*it == '/')
            *it = '\\';
    }
    return s;
#else
    return filename;
#endif
}

inline bool pp::is_absolute(std::string const &filename) const
{
#if defined(PP_OS_WIN)
  return filename.length() >= 3
      && filename.at(1) == ':'
      && (filename.at(2) == '\\' || filename.at(2) == '/');
#else
  return filename.length() >= 1
          && filename.at(0) == '/';
#endif
}

template <typename _OutputIterator>
void pp::file (std::string const &filename, _OutputIterator __result)
{
  FILE *fp = fopen (filename.c_str(), "rb");
  if (fp != 0)
    {
      std::string was = _M_current_file;
      _M_current_file = filename;
      output_line (_M_current_file, 1, __result);
      file (fp, __result);
      _M_current_file = was;
    }
  else
    std::cerr << "** WARNING file ``" << filename << " not found!" << std::endl;
}

template <typename _OutputIterator>
void pp::file (FILE *fp, _OutputIterator __result)
{
   assert (fp != 0);

#if defined (HAVE_MMAP)
  struct stat st;
  fstat(FILENO (fp), &st);
  std::size_t size = st.st_size;
  char *buffer = 0;
  buffer = (char *) ::mmap(0, size, PROT_READ, MAP_SHARED, FILENO (fp), 0);
  fclose (fp);
  if (!buffer || buffer == (char*) -1)
    return;
  this->operator () (buffer, buffer + size, __result);
  ::munmap(buffer, size);
#else
  std::string buffer;
  while (!feof(fp)) {
      char tmp[1024];
      int read = (int) fread (tmp, sizeof(char), 1023, fp);
      tmp[read] = '\0';
      buffer += tmp;
  }
  fclose (fp);
  this->operator () (buffer.c_str(), buffer.c_str() + buffer.size(), __result);
#endif
}

template <typename _InputIterator>
bool pp::find_header_protection (_InputIterator __first, _InputIterator __last, std::string *__prot)
{
  int was = lines;

  while (__first != __last)
    {
      if (pp_isspace (*__first))
        {
          if (*__first == '\n')
            ++lines;

          ++__first;
        }
      else if (_PP_internal::comment_p (__first, __last))
        {
          __first = skip_comment_or_divop (__first, __last);
          lines += skip_comment_or_divop.lines;
        }
      else if (*__first == '#')
        {
          __first = skip_blanks (++__first, __last);
          lines += skip_blanks.lines;

          if (__first != __last && *__first == 'i')
            {
              _InputIterator __begin = __first;
              __first = skip_identifier (__begin, __last);
              lines += skip_identifier.lines;

              std::string __directive (__begin, __first);

              if (__directive == "ifndef")
                {
                  __first = skip_blanks (__first, __last);
                  lines += skip_blanks.lines;

                  __begin = __first;
                  __first = skip_identifier (__first, __last);
                  lines += skip_identifier.lines;

                  if (__begin != __first && __first != __last)
                    {
                      __prot->assign (__begin, __first);
                      return true;
                    }
                }
            }
          break;
        }
      else
        break;
    }

 lines = was;
 return false;
}

inline pp::PP_DIRECTIVE_TYPE pp::find_directive (char const *__directive, std::size_t __size) const
{
  switch (__size)
    {
      case 2:
        if (__directive[0] == 'i'
            && __directive[1] == 'f')
          return PP_IF;
        break;

      case 4:
        if (__directive[0] == 'e' && !strcmp (__directive, "elif"))
          return PP_ELIF;
        else if (__directive[0] == 'e' && !strcmp (__directive, "else"))
          return PP_ELSE;
        break;

      case 5:
        if (__directive[0] == 'i' && !strcmp (__directive, "ifdef"))
          return PP_IFDEF;
        else if (__directive[0] == 'u' && !strcmp (__directive, "undef"))
          return PP_UNDEF;
        else if (__directive[0] == 'e' && !strcmp (__directive, "endif"))
          return PP_ENDIF;
        break;

      case 6:
        if (__directive[0] == 'i' && !strcmp (__directive, "ifndef"))
          return PP_IFNDEF;
        else if (__directive[0] == 'd' && !strcmp (__directive, "define"))
          return PP_DEFINE;
        break;

      case 7:
        if (__directive[0] == 'i' && !strcmp (__directive, "include"))
          return PP_INCLUDE;
        break;

      default:
        break;
    }

  return PP_UNKNOWN_DIRECTIVE;
}

inline bool pp::file_exists (std::string const &__filename) const
{
  struct stat __st;
#if defined(_MSC_VER)
  return stat(__filename.c_str (), &__st) == 0;
#else
  return lstat (__filename.c_str (), &__st) == 0;
#endif
}

inline FILE *pp::find_include_file(std::string const &__input_filename, std::string *__filepath, INCLUDE_POLICY __include_policy) const
{
  assert (! __input_filename.empty() && __filepath);

  std::string __filename = fix_file_path(__input_filename);

  if (is_absolute(__filename))
    {
      *__filepath = __filename;

      return fopen (__filename.c_str(), "r");
    }

  if (__include_policy == INCLUDE_LOCAL && !_M_current_file.empty ())
    {
      std::string::size_type __index = _M_current_file.rfind (PATH_SEPARATOR);

      if (__index != std::string::npos)
        {
          std::string &__path = *__filepath;

          __path.assign (_M_current_file, __index, std::string::npos);

          if (! __path.empty () && __path[__path.size () - 1] != PATH_SEPARATOR)
            __path += PATH_SEPARATOR;

          __path += __filename;

          if (file_exists (__path))
            return fopen (__path.c_str (), "r");
        }
    }

  for (std::vector<std::string>::const_iterator it = include_paths.begin ();
      it != include_paths.end (); ++it)
    {
      std::string &__path = *__filepath;

      __path = *it;
      if (! __path.empty () && __path[__path.size () - 1] != PATH_SEPARATOR)
        __path += PATH_SEPARATOR;

      __path += __filename;

      if (file_exists (__path))
        return fopen (__path.c_str(), "r");
    }

  return 0;
}

template <typename _InputIterator, typename _OutputIterator>
_InputIterator pp::handle_directive(char const *__directive, std::size_t __size,
        _InputIterator __first, _InputIterator __last, _OutputIterator __result)
{
  __first = skip_blanks (__first, __last);

  switch (find_directive (__directive, __size))
    {
      case PP_DEFINE:
        if (! skipping ())
          return handle_define (__first, __last);
        break;

      case PP_INCLUDE:
        if (! skipping ())
          return handle_include (__first, __last, __result);
        break;

      case PP_UNDEF:
        if (! skipping ())
          return handle_undef(__first, __last);
        break;

      case PP_ELIF:
        return handle_elif (__first, __last);

      case PP_ELSE:
        return handle_else (__first, __last);

      case PP_ENDIF:
        return handle_endif (__first, __last);

      case PP_IF:
        return handle_if (__first, __last);

      case PP_IFDEF:
        return handle_ifdef (false, __first, __last);

      case PP_IFNDEF:
        return handle_ifdef (true, __first, __last);

      default:
        break;
    }

  return __first;
}

template <typename _OutputIterator>
void pp::output_line(const std::string &__filename, int __line, _OutputIterator __result)
{
  std::string __msg;

  __msg += "# ";

  char __line_descr[16];
  int n = snprintf (__line_descr, 16, "%d", __line);
  __msg += __line_descr;

  __msg += " \"";

  if (__filename.empty ())
    __msg += "<internal>";
  else
    __msg += __filename;

  __msg += "\"\n";
  std::copy (__msg.begin (), __msg.end (), __result);
}

template <typename _InputIterator, typename _OutputIterator>
_InputIterator pp::handle_include (_InputIterator __first, _InputIterator __last,
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

  std::string filename (__first, end_name);

  std::string filepath;
  FILE *fp = find_include_file (filename, &filepath, quote == '<' ? INCLUDE_GLOBAL : INCLUDE_LOCAL);
#if defined (PP_HOOK_ON_FILE_INCLUDED)
      PP_HOOK_ON_FILE_INCLUDED (_M_current_file, fp ? filepath : filename, fp);
#endif
  if (fp != 0)
    {
      std::string old_file = _M_current_file;
      _M_current_file = filepath;
      int __saved_lines = lines;

      lines = 1;
      output_line (_M_current_file, 1, __result);

      file (fp, __result);

      // restore the file name and the line position
      _M_current_file = old_file;
      lines = __saved_lines;

      // sync the buffer
      output_line (_M_current_file, lines, __result);
    }

  return __first;
}

template <typename _InputIterator, typename _OutputIterator>
void pp::operator () (_InputIterator __first, _InputIterator __last, _OutputIterator __result)
{
#ifndef PP_NO_SMART_HEADER_PROTECTION
  std::string __prot;
  __prot.reserve (255);

  if (find_header_protection (__first, __last, &__prot) && env.resolve (pp_symbol::get (__prot.c_str (), __prot.size ())) != 0)
    {
      // std::cerr << "** DEBUG found header protection:" << __prot << std::endl;
      return;
    }
#endif

  lines = 1;
  char __buffer[512];

  while (true)
    {
      __first = skip_blanks (__first, __last);
      lines += skip_blanks.lines;

      if (__first == __last)
        break;
      else if (*__first == '#')
        {
          assert (*__first == '#');
          __first = skip_blanks (++__first, __last);
          lines += skip_blanks.lines;

          _InputIterator end_id = skip_identifier (__first, __last);
          lines += skip_identifier.lines;
          std::size_t __size = end_id - __first;

          assert (__size < 512);
          char *__cp = __buffer;
          std::copy (__first, end_id, __cp);
          __cp[__size] = '\0';

          end_id = skip_blanks (end_id, __last);
          __first = skip (end_id, __last);

          (void) handle_directive (__buffer, __size, end_id, __first, __result);
        }
      else if (*__first == '\n')
        {
          // ### compress the line
          *__result++ = *__first++;
          ++lines;
        }
      else if (skipping ())
        __first = skip (__first, __last);
      else
        {
          __first = expand (__first, __last, __result);
          lines += expand.lines;
          // ### sync the lines
        }
    }
}

inline pp::pp (pp_environment &__env):
  env (__env), expand (env)
{
  iflevel = 0;
  _M_skipping[iflevel] = 0;
  _M_true_test[iflevel] = 0;
}

inline std::back_insert_iterator<std::vector<std::string> > pp::include_paths_inserter ()
{ return std::back_inserter (include_paths); }

inline std::vector<std::string>::iterator pp::include_paths_begin ()
{ return include_paths.begin (); }

inline std::vector<std::string>::iterator pp::include_paths_end ()
{ return include_paths.end (); }

inline std::vector<std::string>::const_iterator pp::include_paths_begin () const
{ return include_paths.begin (); }

inline std::vector<std::string>::const_iterator pp::include_paths_end () const
{ return include_paths.end (); }

inline void pp::push_include_path (std::string const &__path)
{ include_paths.push_back (__path); }

template <typename _InputIterator>
_InputIterator pp::handle_define (_InputIterator __first, _InputIterator __last)
{
  pp_macro macro;
#if defined (PP_WITH_MACRO_POSITION)
  macro.file = pp_symbol::get (_M_current_file);
#endif
  std::string definition;

  __first = skip_blanks (__first, __last);
  _InputIterator end_macro_name = skip_identifier (__first, __last);
  pp_fast_string const *macro_name = pp_symbol::get (__first, end_macro_name);
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
          macro.variadics = true;
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
              macro.variadics = true;
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
          _InputIterator __begin = __first;
          __begin = skip_blanks (++__begin, __last);

          if (__begin != __last && *__begin == '\n')
            {
              __first = skip_blanks (++__begin, __last);
              definition += ' ';
              continue;
            }
        }

      definition += *__first++;
    }

  macro.definition = pp_symbol::get (definition);
  env.bind (macro_name, macro);

  return __first;
}

template <typename _InputIterator>
_InputIterator pp::skip (_InputIterator __first, _InputIterator __last)
{
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;

  while (__first != __last && *__first != '\n')
    {
      if (*__first == '/')
        {
          __first = skip_comment_or_divop (__first, __last);
          lines += skip_comment_or_divop.lines;
        }
      else if (*__first == '"')
        {
          __first = skip_string_literal (__first, __last);
          lines += skip_string_literal.lines;
        }
      else if (*__first == '\'')
        {
          __first = skip_char_literal (__first, __last);
          lines += skip_char_literal.lines;
        }
      else if (*__first == '\\')
        {
          __first = skip_blanks (++__first, __last);
          lines += skip_blanks.lines;

          if (__first != __last && *__first == '\n')
            {
              ++__first;
              ++lines;
            }
        }
      else
        ++__first;
    }

  return __first;
}

inline bool pp::test_if_level()
{
  bool result = !_M_skipping[iflevel++];
  _M_skipping[iflevel] = _M_skipping[iflevel - 1];
  _M_true_test[iflevel] = false;
  return result;
}

inline int pp::skipping() const
{ return _M_skipping[iflevel]; }

template <typename _InputIterator>
_InputIterator pp::eval_primary(_InputIterator __first, _InputIterator __last, long *result)
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

      *result = env.resolve (token_text->c_str (), token_text->size ()) != 0;

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
_InputIterator pp::eval_multiplicative(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_additive(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_shift(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_relational(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_equality(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_and(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_xor(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_or(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_logical_and(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_logical_or(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_constant_expression(_InputIterator __first, _InputIterator __last, long *result)
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
_InputIterator pp::eval_expression (_InputIterator __first, _InputIterator __last, long *result)
{
  return __first = eval_constant_expression (skip_blanks (__first, __last), __last, result);
}

template <typename _InputIterator>
_InputIterator pp::handle_if (_InputIterator __first, _InputIterator __last)
{
  if (test_if_level())
    {
      pp_macro_expander expand_condition (env);
      std::string condition;
      condition.reserve (255);
      expand_condition (skip_blanks (__first, __last), __last, std::back_inserter (condition));

      long result = 0;
      eval_expression(condition.c_str (), condition.c_str () + condition.size (), &result);

      _M_true_test[iflevel] = result;
      _M_skipping[iflevel] = !result;
    }

  return __first;
}

template <typename _InputIterator>
_InputIterator pp::handle_else (_InputIterator __first, _InputIterator /*__last*/)
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
_InputIterator pp::handle_elif (_InputIterator __first, _InputIterator __last)
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
_InputIterator pp::handle_endif (_InputIterator __first, _InputIterator /*__last*/)
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
_InputIterator pp::handle_ifdef (bool check_undefined, _InputIterator __first, _InputIterator __last)
{
  if (test_if_level())
    {
      _InputIterator end_macro_name = skip_identifier (__first, __last);
      pp_fast_string const *macro_name = pp_symbol::get (__first, end_macro_name);
      bool value = env.resolve (macro_name) != 0;
      __first = end_macro_name;

      if (check_undefined)
        value = !value;

      _M_true_test[iflevel] = value;
      _M_skipping[iflevel] = !value;
    }

  return __first;
}

template <typename _InputIterator>
_InputIterator pp::handle_undef(_InputIterator __first, _InputIterator __last)
{
  __first = skip_blanks (__first, __last);
  _InputIterator end_macro_name = skip_identifier (__first, __last);
  assert (end_macro_name != __first);

  env.unbind (pp_symbol::get (__first, end_macro_name));
  __first = end_macro_name;

  return __first;
}

template <typename _InputIterator>
char pp::peek_char (_InputIterator __first, _InputIterator __last)
{
  if (__first == __last)
    return 0;

  return *++__first;
}

template <typename _InputIterator>
_InputIterator pp::next_token (_InputIterator __first, _InputIterator __last, int *kind)
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
        if (pp_isalpha (ch) || ch == '_')
          {
            _InputIterator end = skip_identifier (__first, __last);
            _M_current_text.assign (__first, end);

            token_text = &_M_current_text;
            __first = end;

            if (*token_text == "defined")
              *kind = TOKEN_DEFINED;
            else
              *kind = TOKEN_IDENTIFIER;
          }
        else if (pp_isdigit (ch))
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

#endif // PP_ENGINE_BITS_H

// kate: indent-width 2;
