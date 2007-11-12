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

#include <QDate>
#include <QTime>

#include <kdebug.h>
#include <klocale.h>

#include <iproblem.h>

#include "pp-internal.h"
#include "pp-engine.h"
#include "pp-environment.h"
#include "preprocessor.h"

using namespace rpp;

pp_frame::pp_frame(pp_macro* __expandingMacro, const QList<QString>& __actuals)
  : expandingMacro(__expandingMacro)
  , actuals(__actuals)
{
}

QString pp_macro_expander::resolve_formal(const QString& name, Stream& input)
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
      else {
        KDevelop::Problem problem;
        problem.setFinalLocation(KDevelop::DocumentRange(m_engine->currentFileName(), KTextEditor::Range(input.inputPosition(), 0)));
        problem.setDescription(i18n("Call to macro %1 missing argument number %2", name, index));
        m_engine->problemEncountered(problem);
        // Triggers on deflate.c
        //Q_ASSERT(0); // internal error?
      }
    }
  }

  return QString();
}

pp_macro_expander::pp_macro_expander(pp* engine, pp_frame* frame, bool inHeaderSection)
  : m_engine(engine)
  , m_frame(frame)
  , m_in_header_section(inHeaderSection)
{
}

//A header-section ends when the first non-directive and non-comment occurs
#define check_header_section \
  if( m_in_header_section ) \
  { \
     \
    m_in_header_section = false; \
    m_engine->preprocessor()->headerSectionEnded(input); \
    if( input.atEnd() ) \
      continue; \
  } \


void pp_macro_expander::operator()(Stream& input, Stream& output)
{
  skip_blanks(input, output);

  while (!input.atEnd())
  {
    if (isComment(input))
    {
      skip_comment_or_divop(input, output, true);
    }else{
      if (input == '\n')
      {
        output << input;

        skip_blanks(++input, output);

        if (!input.atEnd() && input == '#')
          break;
      }
      else if (input == '#')
      {
        skip_blanks(++input, output);

        QString identifier = skip_identifier(input);

        KTextEditor::Cursor inputPosition = input.inputPosition();
        QString formal = resolve_formal(identifier, input);

        if (!formal.isEmpty()) {
          Stream is(&formal, inputPosition);
          skip_whitespaces(is, devnull());

          output << '\"';

          while (!is.atEnd()) {
            if (input == '"') {
              output << '\\' << is;

            } else if (input == '\n') {
              output << '"' << is << '"';

            } else {
              output << is;
            }

            skip_whitespaces(++is, output);
          }

          output << '\"';

        } else {
          output << '#'; // TODO ### warning message?
        }

      }
      else if (input == '\"')
      {
        check_header_section
        
        skip_string_literal(input, output);
      }
      else if (input == '\'')
      {
        check_header_section
        
        skip_char_literal(input, output);
      }
      else if (input.current().isSpace())
      {
        do {
          if (input == '\n' || !input.current().isSpace())
            break;

          output << input;

        } while (!(++input).atEnd());
      }
      else if (input.current().isNumber())
      {
        check_header_section
        
        skip_number (input, output);
      }
      else if (input.current().isLetter() || input == '_')
      {
        check_header_section
        
        KTextEditor::Cursor inputPosition = input.inputPosition();
        QString name = skip_identifier (input);

        // search for the paste token
        int blankStart = input.pos();
        skip_blanks (input, devnull());
        if (!input.atEnd() && input == '#') {
          ++input;

          if (!input.atEnd() && input == '#')
            skip_blanks(++input, devnull());
          else
            input.seek(blankStart);

        } else {
          input.seek(blankStart);
        }

        Q_ASSERT(name.length() >= 0 && name.length() < 512);

        KTextEditor::Cursor inputPosition2 = input.inputPosition();
        QString actual = resolve_formal(name, input);
        if (!actual.isEmpty()) {
          output.appendString(inputPosition2, actual);
          continue;
        }

        // TODO handle inbuilt "defined" etc functions

        pp_macro* macro = m_engine->environment()->retrieveMacro(name);
        if (!macro || !macro->defined || macro->hidden || m_engine->hideNextMacro())
        {
          m_engine->setHideNextMacro(name == "defined");

          if (name == "__LINE__")
            output.appendString(inputPosition, QString::number(input.inputPosition().line()));
          else if (name == "__FILE__")
            output.appendString(inputPosition, QString("\"%1\"").arg(m_engine->currentFile()));
          else if (name == "__DATE__")
            output.appendString(inputPosition, QDate::currentDate().toString("MMM dd yyyy"));
          else if (name == "__TIME__")
            output.appendString(inputPosition, QTime::currentTime().toString("hh:mm:ss"));
          else
            output.appendString(inputPosition, name);
          continue;
        }

        if (!macro->function_like)
        {
          pp_macro* m = 0;

          if (!macro->definition.isEmpty()) {
            macro->hidden = true;

            pp_macro_expander expand_macro(m_engine);
            Stream ms(&macro->definition, input.inputPosition());
            QString expanded;
            {
              Stream es(&expanded);
              expand_macro(ms, es);
            }

            if (!expanded.isEmpty())
            {
              Stream es(&expanded, input.inputPosition());
              skip_whitespaces(es, devnull());
              QString identifier = skip_identifier(es);

              pp_macro* m2 = 0;
              if (es.atEnd() && (m2 = m_engine->environment()->retrieveMacro(identifier)) && m2->defined) {
                m = m2;
              } else {
                output.appendString(inputPosition, expanded);
              }
            }

            macro->hidden = false;
          }

          if (!m)
            continue;

          macro = m;
        }

        skip_whitespaces(input, devnull());

        //In case expansion fails, we can skip back to this position
        int openingPosition = input.pos();
        KTextEditor::Cursor openingPositionCursor = input.inputPosition();
        
        // function like macro
        if (input.atEnd() || input != '(')
        {
          output.appendString(inputPosition, name);
          continue;
        }

        QList<QString> actuals;
        ++input; // skip '('

        pp_macro_expander expand_actual(m_engine, m_frame);

        int before = input.pos();
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
              Stream as(&actual, input.inputPosition());
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
            KTextEditor::Cursor inputPosition = input.inputPosition();
            {
              Stream as(&actual);
              skip_argument_variadics(actuals, macro, input, as);
            }

            QString newActual;
            {
              Stream as(&actual, inputPosition);
              Stream nas(&newActual);
              expand_actual(as, nas);
            }
            actuals.append(newActual);
          }
        }

        if( input != ')' ) {
          //Failed to expand the macro. Output the macro name and continue normal
          //processing behind it.(Code completion depends on this behavior when expanding
          //incomplete input-lines)
          output.appendString(inputPosition, name);
          input.seek(openingPosition);
          input.setInputPosition(openingPositionCursor);
          continue;
        }
        //Q_ASSERT(!input.atEnd() && input == ')');

        ++input; // skip ')'

#if 0 // ### enable me
        assert ((macro->variadics && macro->formals.size () >= actuals.size ())
                    || macro->formals.size() == actuals.size());
#endif

        pp_frame frame(macro, actuals);
        pp_macro_expander expand_macro(m_engine, &frame);
        macro->hidden = true;
        Stream ms(&macro->definition, input.inputPosition());
        expand_macro(ms, output);
        macro->hidden = false;

      } else {
        output << input;
        ++input;
      }
    }

  }
}

void pp_macro_expander::skip_argument_variadics (const QList<QString>& __actuals, pp_macro *__macro, Stream& input, Stream& output)
{
  int first;

  do {
    first = input.pos();
    skip_argument(input, output);

  } while ( __macro->variadics
            && first != input.pos()
            && !input.atEnd()
            && input == '.'
            && (__actuals.size() + 1) == __macro->formals.size());
}

