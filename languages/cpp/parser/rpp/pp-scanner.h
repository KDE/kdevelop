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

#ifndef PP_SCANNER_H
#define PP_SCANNER_H

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
                return __first;
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

#endif // PP_SCANNER_H
