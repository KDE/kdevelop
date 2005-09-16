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

#include "preprocessor.h"
#include "symbol.h"

#include <cstdlib>
#include <fcntl.h>
#include <iostream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

preprocessor::scan_fun_type preprocessor::_S_scan_table[128];
bool preprocessor::_S_scan_table_initialized = false;

namespace
{
  inline bool __is_space(int ch)
  {
    switch (ch)
      {
      default:
        return false;

      case ' ': case '\t': case '\r':
        return true;
      }
  }

  inline bool __is_digit(int ch)
  {
    switch (ch)
      {
      default:
        return false;

      case '0' ... '9':
        return true;
      }
  }

  inline bool __is_alpha(int ch)
  {
    switch (ch)
      {
      default:
        return false;

      case 'a' ... 'z': case 'A' ... 'Z': case '_':
        return true;
      }
  }

  inline bool __is_alnum(int ch)
  {
    switch (ch)
      {
      default:
        return false;

      case 'a' ... 'z': case 'A' ... 'Z': case '_':
      case '0' ... '9':
        return true;
      }
  }
} // namespace

bool buffered_file::open(std::string const &fn)
{
  file_size = pos = 0;
  filename = fn;

  int fd = ::open(filename.c_str(), O_RDONLY, S_IRUSR);
  if (fd == -1)
    return false;

  struct stat st;
  fstat(fd, &st);
  file_size = st.st_size;

#ifdef USE_MMAP
  buffer = (char *) ::mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0);
  if (!buffer || buffer == (char*) -1)
    {
      buffer = 0;
      file_size = 0;
    }
#else
  buffer = new char[file_size + 1];
  ::read(fd, buffer, file_size);
  buffer[file_size] = '\0';
#endif

  ::close(fd);

  return true;
}

buffered_file::~buffered_file()
{
#ifdef USE_MMAP
  ::munmap(buffer, file_size);
#else
  delete[] buffer;
#endif
  buffer = 0;
}

preprocessor::preprocessor()
{
  if (!_S_scan_table_initialized)
    {
      for (int i=0; i<128; ++i)
        {
          if (__is_space(i))
            _S_scan_table[i] = &preprocessor::scan_whitespaces;
          else if (__is_alpha(i))
            _S_scan_table[i] = &preprocessor::scan_identifier;
          else if (__is_digit(i))
            _S_scan_table[i] = &preprocessor::scan_number;
          else
            _S_scan_table[i] = &preprocessor::scan_char;
        }

      _S_scan_table[int('"')]  = &preprocessor::scan_quote;
      _S_scan_table[int('#')]  = &preprocessor::scan_preproc;
      _S_scan_table[int('/')]  = &preprocessor::scan_slash;
      _S_scan_table[int('\'')] = &preprocessor::scan_quote;
      _S_scan_table[int('\\')] = &preprocessor::scan_backslash;
      _S_scan_table[int('\n')] = &preprocessor::scan_newline;

      _S_scan_table_initialized = true;
    }

  iflevel = 0;
  _M_current_file = 0;
  _M_skipping[iflevel] = 0;
  _M_true_test[iflevel] = 0;
  _M_macro_expr = false;

  system_include_paths
    .push_back("/usr/include");

  system_include_paths
    .push_back("/usr/lib/gcc/" GCC_MACHINE "/" GCC_VERSION "/include");

  system_include_paths
    .push_back("/usr/include/c++/" GCC_VERSION);

  system_include_paths
    .push_back("/usr/include/c++/" GCC_VERSION "/" GCC_MACHINE);
}

preprocessor::~preprocessor()
{
}

void preprocessor::scan_backslash()
{
  inp();

  if (ch == '\n' || ch == '\r')
    {
      if (ch == '\r')
        inp();

      inp();
      ++_M_current_file->lineno;
      _M_accept_preproc = true;
    }
  else
    {
      token = '\\';
      produced = true;
    }
}

void preprocessor::scan_char()
{
  token = ch;
  produced = true;

  if (!token)
    return;

  inp();

  if (_M_macro_expr)
    {
      switch (token)
        {
        case 0:
        case '(':
        case ')':
        case ',':
          break;

        case '+':
          token = PP_ADD;
          break;

        case '-':
          token = PP_SUB;
          break;

        case '*':
          token = PP_STAR;
          break;

        case '/':
          token = PP_DIVIDE;
          break;

        case '%':
          token = PP_REMAINDER;
          break;

        case '?':
          token = PP_QUESTION;
          break;

        case ':':
          token = PP_COLON;
          break;

        case '^':
          token = PP_XOR;

        case '!':
          if (ch == '=')
            {
              inp();
              token = PP_NOT_EQ;
            }
          else
            token = PP_NOT;
          break;

        case '<':
          if (ch == '=')
            {
              inp();
              token = PP_LT_EQ;
            }
          else
            token = PP_LT;
          break;

        case '>':
          if (ch == '=')
            {
              inp();
              token = PP_GT_EQ;
            }
          else
            token = PP_GT;
          break;

        case '&':
          if (ch == '&')
            {
              inp();
              token = PP_AND_AND;
            }
          else
            token = PP_AND;
          break;

        case '|':
          if (ch == '|')
            {
              inp();
              token = PP_OR_OR;
            }
          else
            token = PP_OR;
          break;

        case '=':
          if (ch == '=')
            {
              inp();
              token = PP_EQ_EQ;
              break;
            }
          // no break here :-)

        default:
          std::cerr << "** WARNING unknown operator ``" << char(token) << "''"
                    << std::endl;
          break;
        }

      return;
    }
  else
    {
      switch (token)
        {
        case 0: case '#':
        case '(':
        case ')':
        case ',':
          break;

        default:
          while (ch && ch != '\n' && ch != '(' && ch != ')' // ### this is not correct :(
                 && ch != ',' && ch != '"' && ch != '\''
                 && !__is_alnum(ch)
                 && !__is_space(ch))
            {
              inp();
            }
          token = PP_WORD;
          break;
        }
    }

  _M_accept_preproc = false;
}

void preprocessor::scan_slash()
{
  inp();

  if (ch == '/')
    {
      do { inp(); }
      while (ch && ch != '\n');
    }
  else if (ch == '*')
    {
      inp(); // skip '*'

      int tk;
      do
        {
          tk = ch;

          if (tk == '\n')
            {
              ++_M_current_file->lineno;
            }

          inp();
        }
      while (ch && (tk != '*' || ch != '/'));

      if (ch != '/')
        {
          std::cerr << "** WARNING unexpected eof" << std::endl;
        }
      else
        {
          inp();
        }
    }
  else
    {
      _M_accept_preproc = false;
      token = '/';
      produced = true;
    }
}

void preprocessor::scan_identifier()
{
  _M_accept_preproc = false;

  do { inp(); }
  while (__is_alnum(ch));

  token = PP_IDENTIFIER;
  produced = true;
}

void preprocessor::scan_number()
{
  _M_accept_preproc = false;

  if (_M_macro_expr)
    {
      do { inp(); }
      while (__is_digit(ch));

      if (ch == 'l' || ch == 'L')
        inp();

      token = PP_NUMBER;
    }
  else
    {
      do { inp(); }
      while (__is_alnum(ch));

      token = PP_WORD;
    }

  produced = true;
}

void preprocessor::scan_whitespaces()
{
  do { inp(); }
  while (__is_space(ch));

  if (!_M_macro_expr)
    {
      token = PP_WHITESPACES;
      produced = true;
    }
}

void preprocessor::scan_newline()
{
  _M_accept_preproc = true;
  inp();

  token = '\n';
  ++_M_current_file->lineno;
  produced = true;
}

void preprocessor::scan_preproc()
{
  if (_M_accept_preproc)
    {
      token = PP_PREPROC;
    }
  else
    {
      token = '#';
    }

  inp();
  produced = true;
  _M_accept_preproc = false;
}

void preprocessor::scan_quote()
{
  _M_accept_preproc = false;

  int quote = ch;

  inp(); // skip the quote

  while (ch && ch != '\n' && ch != quote)
    {
      if (ch == '\\')
        {
          inp();
        }

      inp();
    }

  if (ch != quote)
    {
      std::cerr << "** WARNING unexpected newline or end-of-file"
                << std::endl;
    }
  else
    {
      inp();
    }

  token = (quote == '"') ? PP_STRING_LITERAL : PP_CHAR_LITERAL;
  produced = true;
}

void preprocessor::handle_directive(DIRECTIVE_ENUM directive)
{
  switch (directive)
    {
    case PP_DEFINE:
      if (!skipping())
        handle_define();
      break;

    case PP_ELIF:
      handle_elif();
      break;

    case PP_ELSE:
      handle_else();
      break;

    case PP_ENDIF:
      handle_endif();
      break;

    case PP_IF:
      handle_if();
      break;

    case PP_IFDEF:
      handle_ifdef();
      break;

    case PP_IFNDEF:
      handle_ifndef();
      break;

    case PP_INCLUDE:
      if (!skipping())
        handle_include();
      break;

    case PP_UNDEF:
      if (!skipping())
        handle_undef();
      break;

    case PP_UNKNOWN:
      skip_line();
      break;
    } // switch
}

void preprocessor::handle_include()
{
  assert(ch == '<' || ch == '"');
  int quote = (ch == '"') ? ch : '>';
  inp(); // skip the quote character

  std::size_t start = _M_current_file->pos - 1;
  while (ch && ch != '\n' && ch != quote)
    {
      inp();
    }
  std::size_t end = _M_current_file->pos - 1;

  _String ide(_M_current_file->buffer + start, end - start);
  std::string include_file = ide.as_string();

  skip_line();
  process_file(include_file);
}

void preprocessor::handle_define()
{
  assert(__is_alpha(ch));
  _String ide = read_identifier();

  // ### warning for macro redefinition

  name_symbol const *sym = _M_symbol_table.install(ide.data, ide.size);
  _Macro &m = _M_macro_table[sym];
  m.enabled = false;
  m.fun_like = (ch == '(');
  m.formals = 0;

  if (m.fun_like)
    {
      // collect formals
      _M_macro_expr = true;
      next_token(); // scan '('

      next_token(); // skip '('

      if (token == PP_IDENTIFIER)
        {
          char const *data;
          std::size_t size;
          token_string_value(&data, &size);
          m.formals = snoc(m.formals, _String(data, size), &_M_memory_pool);
          next_token();
        }

      while (token == ',')
        {
          next_token(); // skip ','

          if (token == PP_IDENTIFIER)
            {
              char const *data;
              std::size_t size;
              token_string_value(&data, &size);
              m.formals = snoc(m.formals, _String(data, size), &_M_memory_pool);
              next_token();
            }
          else
            next_token();
        }

      if (token == ')')
        next_token(); // skip ')'

      _M_macro_expr = false;

      if (m.formals)
        m.formals = m.formals->toFront();
    }

  m.enabled = true;

// ### cleanup
  std::size_t begin = _M_current_file->pos;
  std::size_t end = begin;

  buffered_file *orig_buffered_file = _M_current_file;

  while (token && token != '\n')
    {
      next_token();
      end = _M_token_end;
    }

  m.value.data = _M_current_file->buffer + begin;
  m.value.size = end - begin;

  assert(orig_buffered_file == _M_current_file);
  assert((end - begin) < 2000 && begin <= end);
}

void preprocessor::handle_if()
{
  if (test_if_level())
    {
      bool result = eval_expression() != 0;

      _M_true_test[iflevel] = result;
      _M_skipping[iflevel] = !result;
    }
}

void preprocessor::handle_else()
{
  if (iflevel == 0 && !skipping())
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
}

void preprocessor::handle_elif()
{
  assert(iflevel > 0);

  if (iflevel == 0 && !skipping())
    {
      std::cerr << "** WARNING #else without #if" << std::endl;
    }
  else if (!_M_true_test[iflevel] && !_M_skipping[iflevel - 1])
    {
      bool result = (eval_expression() != 0);
      _M_true_test[iflevel] = result;
      _M_skipping[iflevel] = !result;
    }
  else
    {
      _M_skipping[iflevel] = true;
    }
}

void preprocessor::handle_endif()
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
}

preprocessor::_String preprocessor::read_identifier()
{
  std::size_t start = _M_current_file->pos - 1;

  while (__is_alnum(ch))
    {
      inp();
    }

  std::size_t size = _M_current_file->pos - start - 1;

  return _String(_M_current_file->buffer + start, size);
}

void preprocessor::handle_ifdef(DIRECTIVE_ENUM pp)
{
  if (test_if_level())
    {
      _String ide = read_identifier();
      bool value = macro_defined(ide.data, ide.size);

      if (pp == PP_IFNDEF)
        {
          value = !value;
        }

      _M_true_test[iflevel] = value;
      _M_skipping[iflevel] = !value;

      if (pp == PP_IFNDEF && _M_check_header_protection)
        {
          name_symbol const *sym = _M_symbol_table.install(ide.data, ide.size);

          std::pair<std::string, name_symbol const *>
            hp(_M_current_file->filename, sym);

          _M_header_protection.insert(hp);
        }
    }
}

void preprocessor::handle_undef()
{
  assert(__is_alpha(ch));

  _String ide = read_identifier();
  if (name_symbol const *sym = _M_symbol_table.install(ide.data, ide.size))
    {
      _M_macro_table.erase(sym);
    }
}

void preprocessor::skip_line()
{
  while (token && token != '\n')
    {
      next_token();
    }
}

void preprocessor::tokenize()
{
  if (_M_current_file->file_size == 0)
    {
      std::cerr << "** WARNING empty file" << std::endl;
      return; // nothing to do
    }

  ::fprintf(stdout, "\n# %d \"%s\"\n",
            _M_current_file->lineno,
            _M_current_file->filename.c_str());

  inp();

  _M_accept_preproc = true;
  _M_check_header_protection = true;

  int line = 1;
  int last_tk = '\n';
  while (int tk = next_token())
    {
      if (tk == PP_PREPROC)
        {
          // skip white spaces
          while (__is_space(ch))
            inp();

          if (!__is_alpha(ch))
            continue;

          _String ide = read_identifier();

          // skip white spaces
          while (__is_space(ch))
            inp();

          handle_directive(preproc_type(ide.data, ide.size));
          _M_check_header_protection = false;
        }
      else if (!skipping())
        {
          if (tk == '\n' && last_tk == '\n')
            continue;

          last_tk = tk;

          if (tk == '\n')
            ++line;
          else
            _M_check_header_protection = false;

          if (line != _M_current_file->lineno)
            {
              line = _M_current_file->lineno;

              ::fprintf(stdout, "\n# %d \"%s\"\n",
                        line,
                        _M_current_file->filename.c_str());
            }

          if (tk == PP_IDENTIFIER)
            {
              char const *data;
              std::size_t size;
              token_string_value(&data, &size);

              if (_Macro *m = resolve_macro(data, size))
                {
                  next_token(); // skip the macro name

                  if (m->fun_like)
                    {
                      // collect arguments
                      ListNode<ListNode<_String> const *> const *arguments = 0;
                      collect_arguments(&arguments);
#ifdef __GNUC__
#  warning "macro substitution -- implement me"
#endif
                    }
                  else if (m->value.size > 0)
                    {
                      _M_current_file->saved_ch = ch;

                      std::string buf = m->value.as_string(); // ### save a copy removing the as_string()

                      // create a new buffer with the macro contents
                      buffered_file _temp_buf;
                      _temp_buf.buffer = (char*) buf.c_str();
                      _temp_buf.file_size = m->value.size;
                      _temp_buf.filename = _M_current_file->filename;
                      _temp_buf.lineno = _M_current_file->lineno;

                      buffered_file *saved = _M_current_file;
                      _M_current_file = &_temp_buf;

                      m->enabled = false;
                      tokenize();
                      m->enabled = true;

                      _temp_buf.buffer = 0;

                      _M_current_file = saved; // restore the buffer
                      ch = _M_current_file->saved_ch; // and the current `ch'
                      assert(ch >= 0);
                    }
                  continue;
                }
            }

          ::fwrite(_M_current_file->buffer + _M_token_start,
                   1, _M_token_end - _M_token_start, stdout);
        }
    }

  if (0 && _M_current_file->pos != _M_current_file->file_size + 1)
    {
      assert(_M_current_file->pos == _M_current_file->file_size + 1);
    }
}

int preprocessor::next_token()
{
  produced = false;

  while (!produced)
    {
      _M_token_start = _M_current_file->pos - 1;
      (this->*_S_scan_table[ch])();
    }

  _M_token_end = _M_current_file->pos - 1;

  return token;
}

void preprocessor::process_file(std::string const &filename)
{
  std::string include_file = find_include_file(filename);

  std::map<std::string, name_symbol const*>::iterator
    it = _M_header_protection.find(include_file);

  if (it != _M_header_protection.end())
    {
      name_symbol const *sym = (*it).second;

      if (_M_macro_table.find(sym) != _M_macro_table.end())
        {
          return;
        }
    }

  if (include_file.empty())
    {
      std::cerr << "** WARNING file " << filename << " not found"
                << std::endl;
      return;
    }

  buffered_file *saved_buffered_file = _M_current_file;

  _M_current_file = new buffered_file();
  _M_current_file->open(include_file);
  _M_current_file->saved_ch = ch;

  tokenize();

  ch = _M_current_file->saved_ch;
  assert(ch >= 0);
  _M_accept_preproc = true;

  delete _M_current_file;

  _M_current_file = saved_buffered_file;
}

std::size_t preprocessor::file_size(std::string const &filename)
{
  static std::set<std::string> _S_visited;

  std::set<std::string>::iterator it = _S_visited.find(filename);
  if (it != _S_visited.end())
    return std::size_t(-1);

  struct stat st;
  if (lstat(filename.c_str(), &st) == 0)
    return st.st_size;

  _S_visited.insert(filename);

  return std::size_t(-1);
}

std::string preprocessor::find_include_file(std::string const &filename,
                              INCLUDE_POLICY include_policy) const
{
  assert(!filename.empty());

  if (filename[0] == '/')
    return filename;

  if (include_policy == LocalInclude)
    {
      if (file_size(filename) != std::size_t(-1))
        return filename;
    }

  // system paths
  for (paths_const_iterator it = system_include_paths.begin();
       it != system_include_paths.end(); ++it)
    {
      std::string fn = *it;
      fn += '/';
      fn += filename;

      if (file_size(fn) != std::size_t(-1))
        return fn;
    }

  // user paths
  for (paths_const_iterator it = include_paths.begin();
       it != include_paths.end(); ++it)
    {
      std::string fn = *it;
      fn += '/';
      fn += filename;

      if (file_size(fn) != std::size_t(-1))
        return fn;
    }

  if (include_policy == GlobaInclude)
    {
      if (file_size(filename) != std::size_t(-1))
        return filename;
    }

  return std::string();
}

preprocessor::DIRECTIVE_ENUM preprocessor::preproc_type(char const *data,
                                                      std::size_t size)
{
  switch (size)
    {
    default:
      return PP_UNKNOWN;

    case 2:
      return !strncmp("if", data, 2)
        ? PP_IF
        : PP_UNKNOWN;

    case 4:
      return !strncmp("else", data, 4)
        ? PP_ELSE
        : (!strncmp("elif", data, 4) ? PP_ELIF : PP_UNKNOWN);

    case 5:
      return !strncmp("ifdef", data, 5)
        ? PP_IFDEF
        : (!strncmp("endif", data, 5) ? PP_ENDIF : PP_UNKNOWN);

    case 6:
      return !strncmp("ifndef", data, 6)
        ? PP_IFNDEF
        : (!strncmp("define", data, 6) ? PP_DEFINE : PP_UNKNOWN);

    case 7:
      return !strncmp("include", data, 7)
        ? PP_INCLUDE
        : PP_UNKNOWN;
    }
}

long preprocessor::eval_primary()
{
  switch (token)
    {
    case PP_NUMBER:
      {
        long result = token_long_value();
        next_token();
        return result;
      }

    case PP_IDENTIFIER:
      {
        long result = 0;
        char const *data;
        std::size_t size;

        token_string_value(&data, &size);

        if (size == 7 && !::strncmp(data, "defined", 7))
          {
            next_token();

            bool skip_paren = (token == '(');

            if (skip_paren)
              {
                next_token();
              }

            if (token == PP_IDENTIFIER)
              {
                token_string_value(&data, &size);
                result = macro_defined(data, size);

                next_token();

                if (skip_paren && token != ')')
                  {
                    std::cerr << "** WARNING expected ``)''" << std::endl;
                  }
                else if (skip_paren)
                  {
                    next_token(); // skip ')'
                  }
              }
            else
              {
                std::cerr << "** WARNING expected an identifier" << std::endl;
              }

            return result;
          }
        else
          {
            next_token();

            std::cerr << "** WARNING macro substitution not yet implemented"
                      << std::endl;
          }
      }
      return 0;

    case PP_NOT:
      next_token();
      return !eval_constant_expression();

    case '(':
      {
        next_token();

        long result = eval_constant_expression();

        if (token != ')')
          {
            std::cerr << "** WARNING expected ``)''" << std::endl;
          }
        else
          {
            next_token();
          }

        return result;
      }

    default:
      return 0;
    }
}

long preprocessor::eval_multiplicative()
{
  long result = eval_primary();

  while (token == PP_STAR
         || token == PP_DIVIDE
         || token == PP_REMAINDER)
    {
      int op = token;
      next_token();
      long value = eval_primary();

      switch (op)
        {
        case PP_STAR:
          result = result * value;
          break;

        case PP_DIVIDE:
          if (value == 0)
            {
              std::cerr << "** WARNING division by zero" << std::endl;
              result = 0;
              break;
            }
          else
            {
              result = result / value;
            }
          break;

        case PP_REMAINDER:
          result = result % value;
          break;
        } // switch
    }

  return result;
}

long preprocessor::eval_additive()
{
  long result = eval_multiplicative();

  while (token == PP_ADD
         || token == PP_SUB)
    {
      int op = token;
      next_token();
      long value = eval_multiplicative();

      switch (op)
        {
        case PP_ADD:
          result = result + value;
          break;

        case PP_SUB:
          result = result - value;
          break;
        } // switch
    }

  return result;
}

long preprocessor::eval_relational()
{
  long result = eval_additive();

  while (token == PP_LT
         || token == PP_LT_EQ
         || token == PP_GT
         || token == PP_GT_EQ)
    {
      int op = token;
      next_token();
      long value = eval_additive();

      switch (op)
        {
        case PP_LT:
          result = result < value;
          break;

        case PP_LT_EQ:
          result = result <= value;
          break;

        case PP_GT:
          result = result > value;
          break;

        case PP_GT_EQ:
          result = result >= value;
          break;
        } // switch
    }

  return result;
}

long preprocessor::eval_equality()
{
  long result = eval_relational();

  while (token == PP_EQ_EQ || token == PP_NOT_EQ)
    {
      int op = token;
      next_token();
      long value = eval_relational();

      if (op == PP_EQ_EQ)
        result = result == value;
      else
        result = result != value;
    }

  return result;
}

long preprocessor::eval_and()
{
  long result = eval_equality();

  while (token == PP_AND)
    {
      next_token();

      long value = eval_equality();
      result = result & value;
    }

  return result;
}

long preprocessor::eval_xor()
{
  long result = eval_and();

  while (token == PP_XOR)
    {
      next_token();

      long value = eval_and();
      result = result ^ value;
    }

  return result;
}

long preprocessor::eval_or()
{
  long result = eval_xor();

  while (token == PP_OR)
    {
      next_token();

      long value = eval_xor();
      result = result | value;
    }

  return result;
}

long preprocessor::eval_logical_and()
{
  long result = eval_or();

  while (token == PP_AND_AND)
    {
      next_token();

      long value = eval_or();
      result = result && value;
    }

  return result;
}

long preprocessor::eval_logical_or()
{
  long result = eval_logical_and();

  while (token == PP_OR_OR)
    {
      next_token();

      long value = eval_logical_and();
      result = result || value;
    }

  return result;
}

long preprocessor::eval_constant_expression()
{
  long result = eval_logical_or();

  if (token == '?')
    {
      next_token();
      long left_value = eval_constant_expression();

      if (token == ':')
        {
          next_token();
          long right_value = eval_constant_expression();

          result = result ? left_value : right_value;
        }
      else
        {
          std::cerr << "** WARNING expected ``:''" << std::endl;
          result = left_value;
        }
    }

  return result;
}

long preprocessor::eval_expression()
{
  bool saved_macro_expr = _M_macro_expr;
  _M_macro_expr = true;

  next_token();
  long result = eval_constant_expression();
  skip_line();

  _M_macro_expr = saved_macro_expr;

  return result;
}

long preprocessor::token_long_value() const
{
  return ::strtol(_M_current_file->buffer + _M_token_start, 0, 0);
}

void preprocessor::token_string_value(char const **data,
                                      std::size_t *size) const
{
  *data = _M_current_file->buffer + _M_token_start;
  *size = _M_token_end - _M_token_start;
}

bool preprocessor::test_if_level()
{
  bool result = !_M_skipping[iflevel++];
  _M_skipping[iflevel] = _M_skipping[iflevel - 1];
  _M_true_test[iflevel] = false;
  return result;
}

int preprocessor::index_of_formal(const ListNode<_String> *formals, _String const &ide) const
{
  if (!formals)
    return -1;

  _String_compare cmp;

  const ListNode<_String> *it = formals->toFront(), *end = it;
  do
    {
      _String const &s = it->element;
      if (cmp(ide, s))
        return it->index;

      it = it->next;
    }
  while (it != end);

  return -1;
}

bool preprocessor::macro_defined(char const *data, std::size_t size) const
{
  if (name_symbol const *sym = _M_symbol_table.find(data, size))
    {
      return _M_macro_table.find(sym) != _M_macro_table.end();
    }

  return false;
}

preprocessor::_Macro *preprocessor::resolve_macro(char const *data, std::size_t size)
{
  if (name_symbol const *sym = _M_symbol_table.find(data, size))
    {
      macro_table::iterator it = _M_macro_table.find(sym);

      if (it != _M_macro_table.end() && (*it).second.enabled)
        return &(*it).second;
    }

  return 0;
}

void preprocessor::collect_one_argument(ListNode<_String> const **argument)
{
  int count = 0;

  while (token != 0)
    {
      if (!count && (token == ')' || token == ','))
        {
          break;
        }
      else if (token == '(')
        {
          ++count;
        }
      else if (token == ')')
        {
          --count;
        }

      char const *data;
      std::size_t size;
      token_string_value(&data, &size);

      *argument = snoc(*argument, _String(data, size), &_M_memory_pool);
      next_token();
    }
}

void preprocessor::collect_arguments(ListNode<ListNode<_String> const*> const **arguments)
{
  if (token != '(')
    {
      std::cerr << "** WARNING expected arguments for the macro definition"
                << std::endl;
      return;
    }

  next_token();  // skip '('

  ListNode<_String> const *argument = 0;
  collect_one_argument(&argument);
  if (!argument)
    {
      // nothing to do
      return;
    }

  *arguments = snoc(*arguments, argument, &_M_memory_pool);

  while (token == ',')
    {
      next_token(); // skip ','

      collect_one_argument(&argument);
      if (!argument)
        break;

      *arguments = snoc(*arguments, argument, &_M_memory_pool);
    }

  assert(token == ')');
  next_token();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
