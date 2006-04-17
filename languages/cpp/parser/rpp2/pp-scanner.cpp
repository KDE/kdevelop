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

void pp_skip::rewind(QTextStream& stream, qint64 offset) const
{
  stream.seek(stream.pos() - offset);
}

int pp_skip::linesSkipped() const
{
  return m_lines;
}

void pp_skip_blanks::operator()(QTextStream& input, QTextStream& output)
{
  m_lines = 0;

  QChar c;
  while (!input.atEnd()) {
    input >> c;

    if (c == '\\') {
      input >> c;
      if (c != '\n') {
        return rewind(input, 2);

      } else {
        ++m_lines;
        continue;
      }
    }

    if (c == '\n' || !c.isSpace())
      return rewind(input);

    output << c;
  }
}

void pp_skip_comment_or_divop::operator()(QTextStream& input, QTextStream& output)
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

  QChar c;
  while (!input.atEnd()) {
    input >> c;

    switch (state) {
      case MAYBE_BEGIN:
        if (c != '/')
          return rewind(input);

        state = BEGIN;
        break;

      case BEGIN:
        if (c == '*')
          state = IN_COMMENT;
        else if (c == '/')
          state = IN_CXX_COMMENT;
        else
          return rewind(input);
        break;

      case IN_COMMENT:
        if (c == '*')
          state = MAYBE_END;
        break;

      case IN_CXX_COMMENT:
        if (c == '\n')
          return rewind(input);
        break;

      case MAYBE_END:
        if (c == '/')
          state = END;
        else if (c != '*')
          state = IN_COMMENT;
        break;

      case END:
        return rewind(input);
    }

    output << c;
  }
}

void pp_skip_identifier::operator()(QTextStream& input, QTextStream& output);
{
  m_lines = 0;

  QChar c;
  while (!input.atEnd()) {
    input >> c;

    if (!c.isLetterOrNumber() && c != '_')
        return rewind(input);

    output << c;
  }
}

void pp_skip_number::operator()(QTextStream& input, QTextStream& output);
{
  m_lines = 0;

  QChar c;
  while (!input.atEnd()) {
    input >> c;

    if (!c.isLetterOrNumber() && c != '_')
        return rewind(input);

    output << c;
  }
}

void pp_skip_string_literal::operator()(QTextStream& input, QTextStream& output);
{
  enum {
    BEGIN,
    IN_STRING,
    QUOTE,
    END
  } state (BEGIN);

  m_lines = 0;

  QChar c;
  while (!input.atEnd()) {
    input >> c;

    switch (state) {
      case BEGIN:
        if (c != '\"')
          return rewind(input);
        state = IN_STRING;
        break;

      case IN_STRING:
        Q_ASSERT(c != '\n');

        if (c == '\"')
          state = END;
        else if (c == '\\')
          state = QUOTE;
        break;

      case QUOTE:
        state = IN_STRING;
        break;

      case END:
        return rewind(input);
    }

    output << c;
  }
}

void pp_skip_char_literal::operator()(QTextStream& input, QTextStream& output);
{
  enum {
    BEGIN,
    IN_STRING,
    QUOTE,
    END
  } state (BEGIN);

  m_lines = 0;

  while (!input.atEnd()) {
    input >> c;

    switch (state) {
      case BEGIN:
        if (c != '\'')
          return rewind(input);
        state = IN_STRING;
        break;

      case IN_STRING:
        Q_ASSERT(c != '\n');

        if (c == '\'')
          state = END;
        else if (c == '\\')
          state = QUOTE;
        break;

      case QUOTE:
        state = IN_STRING;
        break;
    }

    output << c;
  }
}

void pp_skip_argument::operator()(QTextStream& input, QTextStream& output);
{
  int depth = 0;
  m_lines = 0;

  while (!input.atEnd()) {
    input >> c;

    if (!depth && (c == ')' || c == ',')) {
      return rewind(input);

    } else if (c == '(') {
      ++depth;

    } else if (c == ')') {
      --depth;

    } else if (c == '\"') {
      rewind(input);
      skip_string_literal(input, output);
      lines += skip_string_literal.lines();
      continue;

    } else if (c == '\'') {
      rewind(input);
      __first = skip_char_literal (input, output);
      lines += skip_char_literal.lines;
      continue;

    } else if (c == '/') {
      rewind(input);
      __first = skip_comment_or_divop (input, output);
      lines += skip_comment_or_divop.lines;
      continue;

    } else if (c.isLetter() || c == '_') {
      rewind(input);
      __first = skip_identifier(input, output);
      lines += skip_identifier.lines;
      continue;

    } else if (c.isNumber()) {
      rewind(input);
      __first = skip_number(input, output);
      lines += skip_number.lines;
      continue;

    } else if (c == '\n') {
      ++lines;
    }

    output << c;
  }

  return rewind(input);
}
