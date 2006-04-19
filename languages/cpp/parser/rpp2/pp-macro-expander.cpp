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

#include "pp-macro-expander.h"

#include <kdebug.h>

#include "pp-internal.h"

pp_frame::pp_frame(pp_macro* __expandingMacro, const QList<QString>& __actuals)
  : expandingMacro(__expandingMacro)
  , actuals(__actuals)
{
}

QString pp_macro_expander::resolve_formal(const QString& name)
{
  Q_ASSERT(!name.isEmpty());

  if (!m_frame)
    return QString();

  Q_ASSERT(m_frame->expandingMacro != 0);

  const QStringList& formals = m_frame->expandingMacro->formals;

  for (int index = 0; index < formals.size(); ++index) {
    if (name == formals[index]) {
      if (index < m_frame->actuals.size())
        return m_frame->actuals[index];
      else
        Q_ASSERT(0); // internal error?
    }
  }

  return QString();
}

pp_macro_expander::pp_macro_expander(QHash<QString, pp_macro*>& environment, pp_frame* frame)
  : m_environment(environment)
  , m_frame(frame)
  , m_lines(0)
  , m_generatedLines(0)
{
}

void pp_macro_expander::operator()(Stream& input, Stream& output)
{
  m_generatedLines = 0;
  skip_blanks(input, output);

  m_lines = skip_blanks.linesSkipped();

  while (!input.atEnd())
  {
    if (input == '\n')
    {
      output << input;
      ++m_lines;

      skip_blanks(++input, output);
      m_lines += skip_blanks.linesSkipped();

      if (!input.atEnd() && input == '#')
        break;
    }
    else if (input == '#')
    {
      skip_blanks(++input, output);
      m_lines += skip_blanks.linesSkipped();

      QString identifier = skip_identifier(input);
      output << '\"';
      int was = m_lines;

      Stream is(&identifier);
      operator()(is, output);

      m_lines += was;
      output << '\"';
    }
    else if (input == '\"')
    {
      skip_string_literal(input, output);
      m_lines += skip_string_literal.linesSkipped();
    }
    else if (input == '\'')
    {
      skip_char_literal(input, output);
      m_lines += skip_char_literal.linesSkipped();
    }
    else if (PPInternal::isComment(input))
    {
      skip_comment_or_divop(input, output);
      int n = skip_comment_or_divop.linesSkipped();
      m_lines += n;

      output << QString(n, '\n');
    }
    else if (input.current().isSpace())
    {
      do {
        if (input == '\n' || !input.current().isSpace())
          break;

      } while (!(++input).atEnd());

      output << ' ';
    }
    else if (input.current().isNumber())
    {
      skip_number (input, output);
      m_lines += skip_number.linesSkipped();
    }
    else if (input.current().isLetter() || input == '_')
    {
      QString name = skip_identifier (input);

      // search for the paste token
      qint64 blankStart = input.pos();
      skip_blanks (input, PPInternal::devnull());
      if (!input.atEnd() && input == '#') {
        ++input;

        if (!input.atEnd() && input == '#')
          skip_blanks(++input, PPInternal::devnull());
        else
          input.seek(blankStart);

      } else {
        input.seek(blankStart);
      }

      Q_ASSERT(name.length() >= 0 && name.length() < 512);

      QString actual = resolve_formal(name);
      if (!actual.isEmpty()) {
        output << actual;
        continue;
      }

      static bool hide_next = false; // ### remove me

      pp_macro* macro = m_environment[name];
      if (! macro || macro->hidden || hide_next)
      {
        hide_next = name == "defined";
        output << name;
        continue;
      }

      if (!macro->function_like)
      {
        pp_macro_expander expand_macro(m_environment);
        macro->hidden = true;
        Stream ms(&macro->definition, QIODevice::ReadOnly);
        expand_macro(ms, output);
        macro->hidden = false;
        m_generatedLines += expand_macro.linesSkipped();
        continue;
      }

      // function like macro
      if (input.atEnd() || input != '(')
      {
        output << name;
        continue;
      }

      QList<QString> actuals;
      ++input; // skip '('

      pp_macro_expander expand_actual(m_environment, m_frame);

      qint64 before = input.pos();
      {
        actual.clear();

        {
          Stream as(&actual);
          skip_argument_variadics(actuals, macro, input, as);
        }

        if (input.pos() != before)
        {
          QString newActual;
          {
            Stream as(&actual);
            Stream nas(&newActual);
            expand_actual(as, nas);
          }
          actuals.append(newActual);
        }
      }

      // TODO: why separate from the above?
      while (!input.atEnd() && input == ',')
      {
        actual.clear();
        ++input; // skip ','

        {
          {
            Stream as(&actual);
            skip_argument_variadics(actuals, macro, input, as);
          }

          QString newActual;
          {
            Stream as(&actual);
            Stream nas(&newActual);
            expand_actual(as, nas);
          }
          actuals.append(newActual);
        }
      }

      Q_ASSERT(!input.atEnd() && input == ')');

      ++input; // skip ')'

#if 0 // ### enable me
      assert ((macro->variadics && macro->formals.size () >= actuals.size ())
                  || macro->formals.size() == actuals.size());
#endif

      pp_frame frame(macro, actuals);
      pp_macro_expander expand_macro(m_environment, &frame);
      macro->hidden = true;
      Stream ms(&macro->definition, QIODevice::ReadOnly);
      expand_macro(ms, output);
      macro->hidden = false;
      m_generatedLines += expand_macro.linesSkipped();

    } else {
      output << input;
      ++input;
    }
  }
}

void pp_macro_expander::skip_argument_variadics (const QList<QString>& __actuals, pp_macro *__macro, Stream& input, Stream& output)
{
  qint64 first;

  do {
    first = input.pos();
    skip_argument(input, output);

  } while ( __macro->variadics
            && first != input.pos()
            && !input.atEnd()
            && input == '.'
            && (__actuals.size() + 1) == __macro->formals.size());
}

int pp_macro_expander::generatedLines( ) const
{
  return m_generatedLines;
}

int pp_macro_expander::linesSkipped( ) const
{
  return m_lines;
}

// kate: indent-width 2;
