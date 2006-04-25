/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "pp-scanner.h"

int pp_skip::linesSkipped() const
{
  return m_lines;
}

void pp_skip_blanks::operator()(Stream& input, Stream& output)
{
  m_lines = 0;

  while (!input.atEnd()) {
    if (input == '\\') {
      ++input;
      if (input != '\n') {
        --input;
        return;

      } else {
        ++input;
        ++m_lines;
        continue;
      }
    }

    if (input == '\n' || !input.current().isSpace())
      return;

    output << input;
    ++input;
  }
}

void pp_skip_comment_or_divop::operator()(Stream& input, Stream& output)
{
  enum {
    MAYBE_BEGIN,
    BEGIN,
    MAYBE_END,
    END,
    IN_COMMENT,
    IN_CXX_COMMENT
  } state (MAYBE_BEGIN);

  m_lines = 0;

  while (!input.atEnd()) {
    switch (state) {
      case MAYBE_BEGIN:
        if (input != '/')
          return;

        state = BEGIN;
        break;

      case BEGIN:
        if (input == '*')
          state = IN_COMMENT;
        else if (input == '/')
          state = IN_CXX_COMMENT;
        else
          return;
        break;

      case IN_COMMENT:
        if (input == '*')
          state = MAYBE_END;
        break;

      case IN_CXX_COMMENT:
        if (input == '\n')
          return;
        break;

      case MAYBE_END:
        if (input == '/')
          state = END;
        else if (input != '*')
          state = IN_COMMENT;
        break;

      case END:
        return;
    }

    if (input == '\n')
      output << '\n';
    else
      output << ' ';
    ++input;
  }
}

QString pp_skip_identifier::operator()(Stream& input)
{
  m_lines = 0;

  QString identifier;

  while (!input.atEnd()) {
    if (!input.current().isLetterOrNumber() && input != '_')
        break;

    identifier.append(input);
    ++input;
  }

  return identifier;
}

void pp_skip_number::operator()(Stream& input, Stream& output)
{
  m_lines = 0;

  while (!input.atEnd()) {
    if (!input.current().isLetterOrNumber() && input != '_')
        return;

    output << input;
    ++input;
  }
}

void pp_skip_string_literal::operator()(Stream& input, Stream& output)
{
  enum {
    BEGIN,
    IN_STRING,
    QUOTE,
    END
  } state (BEGIN);

  m_lines = 0;

  while (!input.atEnd()) {
    switch (state) {
      case BEGIN:
        if (input != '\"')
          return;
        state = IN_STRING;
        break;

      case IN_STRING:
        Q_ASSERT(input != '\n');

        if (input == '\"')
          state = END;
        else if (input == '\\')
          state = QUOTE;
        break;

      case QUOTE:
        state = IN_STRING;
        break;

      case END:
        return;
    }

    output << input;
    ++input;
  }
}

void pp_skip_char_literal::operator()(Stream& input, Stream& output)
{
  enum {
    BEGIN,
    IN_STRING,
    QUOTE,
    END
  } state (BEGIN);

  m_lines = 0;

  while (!input.atEnd()) {
    if (state == END)
      break;

    switch (state) {
      case BEGIN:
        if (input != '\'')
          return;
        state = IN_STRING;
        break;

      case IN_STRING:
        Q_ASSERT(input != '\n');

        if (input == '\'')
          state = END;
        else if (input == '\\')
          state = QUOTE;
        break;

      case QUOTE:
        state = IN_STRING;
        break;

      default:
        Q_ASSERT(0);
        break;
    }

    output << input;
    ++input;
  }
}

void pp_skip_argument::operator()(Stream& input, Stream& output)
{
  int depth = 0;
  m_lines = 0;

  while (!input.atEnd()) {
    if (!depth && (input == ')' || input == ',')) {
      return;

    } else if (input == '(') {
      ++depth;
      ++input;

    } else if (input == ')') {
      --depth;
      ++input;

    } else if (input == '\"') {
      skip_string_literal(input, output);
      m_lines += skip_string_literal.linesSkipped();
      continue;

    } else if (input == '\'') {
      skip_char_literal (input, output);
      m_lines += skip_char_literal.linesSkipped();
      continue;

    } else if (input == '/') {
      skip_comment_or_divop (input, output);
      m_lines += skip_comment_or_divop.linesSkipped();
      continue;

    } else if (input.current().isLetter() || input == '_') {
      output << skip_identifier(input);
      m_lines += skip_identifier.linesSkipped();
      continue;

    } else if (input.current().isNumber()) {
      output << skip_number(input);
      m_lines += skip_number.linesSkipped();
      continue;

    } else if (input == '\n') {
      ++m_lines;
    }

    output << input;
    ++input;
  }

  return;
}
