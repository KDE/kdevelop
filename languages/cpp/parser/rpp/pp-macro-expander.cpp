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
#include <indexedstring.h>

#include "pp-internal.h"
#include "pp-engine.h"
#include "pp-environment.h"
#include "pp-location.h"
#include "preprocessor.h"
#include "chartools.h"

using namespace KDevelop;

QString joinIndexVector(const uint* arrays, uint size, QString between) {
  QString ret;
  FOREACH_CUSTOM(uint item, arrays, size) {
    if(!ret.isEmpty())
      ret += between;
    ret += IndexedString(item).str();
  }
  return ret;
}

void trim(QVector<uint>& array) {
  int lastValid = array.size()-1;
  for(; lastValid >= 0; --lastValid)
    if(array[lastValid] != indexFromCharacter(' '))
      break;
  
  array.resize(lastValid+1);
  
  int firstValid = 0;
  for(; firstValid < array.size(); ++firstValid)
    if(array[firstValid] != indexFromCharacter(' '))
      break;
  array = array.mid(firstValid);
}

using namespace rpp;              

pp_frame::pp_frame(pp_macro* __expandingMacro, const QList<pp_actual>& __actuals)
  : expandingMacro(__expandingMacro)
  , actuals(__actuals)
{
}

pp_actual pp_macro_expander::resolve_formal(IndexedString name, Stream& input)
{
  if (!m_frame)
    return pp_actual();

  Q_ASSERT(m_frame->expandingMacro != 0);

  const uint* formals = m_frame->expandingMacro->formals();
  uint formalsSize = m_frame->expandingMacro->formalsSize();

  if(name.isEmpty()) {
    KDevelop::Problem problem;
    problem.setFinalLocation(KDevelop::DocumentRange(m_engine->currentFileNameString(), KTextEditor::Range(input.originalInputPosition().textCursor(), 0)));
    problem.setDescription(i18n("Macro error"));
    m_engine->problemEncountered(problem);
    return pp_actual();
  }
  
  for (uint index = 0; index < formalsSize; ++index) {
    if (name.index() == formals[index]) {
      if (index < (uint)m_frame->actuals.size())
        return m_frame->actuals[index];
      else {
        KDevelop::Problem problem;
        problem.setFinalLocation(KDevelop::DocumentRange(m_engine->currentFileNameString(), KTextEditor::Range(input.originalInputPosition().textCursor(), 0)));
        problem.setDescription(i18n("Call to macro %1 missing argument number %2", IndexedString(name).str(), index));
        problem.setExplanation(i18n("Formals: %1", joinIndexVector(formals, formalsSize, ", ")));
        m_engine->problemEncountered(problem);
      }
    }
  }

  return pp_actual();
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

struct EnableMacroExpansion {
  EnableMacroExpansion(Stream& _input, const KDevelop::SimpleCursor& expansionPosition) : input(_input), hadMacroExpansion(_input.macroExpansion().isValid()) {
    
    if(!hadMacroExpansion)
      _input.setMacroExpansion(expansionPosition);
  }
  ~EnableMacroExpansion() {
    if(!hadMacroExpansion)
      input.setMacroExpansion(KDevelop::SimpleCursor::invalid());
  }
  Stream& input;
  bool hadMacroExpansion;
};

IndexedString definedIndex = IndexedString("defined");
IndexedString lineIndex = IndexedString("__LINE__");
IndexedString fileIndex = IndexedString("__FILE_");
IndexedString dateIndex = IndexedString("__DATE__");
IndexedString timeIndex= IndexedString("__TIME__");

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
        Q_ASSERT(isCharacter(input.current()));
        Q_ASSERT(IndexedString(input.current()).str() == "#");
        skip_blanks(++input, output);

        IndexedString identifier( skip_identifier(input) );

        Anchor inputPosition = input.inputPosition();
        KDevelop::SimpleCursor originalInputPosition = input.originalInputPosition();
        PreprocessedContents formal = resolve_formal(identifier, input).mergeText();
        
        //Escape so we don't break on '"'
        for(int a = formal.count()-1; a >= 0; --a)
          if(formal[a] == indexFromCharacter('\"'))
            formal.insert(a, indexFromCharacter('\\'));

        if (!formal.isEmpty()) {
          Stream is(&formal, inputPosition);
          is.setOriginalInputPosition(originalInputPosition);
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
      else if (isSpace(input.current()))
      {
        do {
          if (input == '\n' || !isSpace(input.current()))
            break;

          output << input;

        } while (!(++input).atEnd());
      }
      else if (isNumber(input.current()))
      {
        check_header_section
        
        skip_number (input, output);
      }
      else if (isLetter(input.current()) || input == '_' || !isCharacter(input.current()))
      {
        check_header_section
        
        Anchor inputPosition = input.inputPosition();
        IndexedString name(skip_identifier (input));
        
        // search for the paste token
        int blankStart = input.offset();
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

        Anchor inputPosition2 = input.inputPosition();
        pp_actual actual = resolve_formal(name, input);
        if (actual.isValid()) {
          Q_ASSERT(actual.text.size() == actual.inputPosition.size());
          
          QList<PreprocessedContents>::const_iterator textIt = actual.text.begin();
          QList<Anchor>::const_iterator cursorIt = actual.inputPosition.begin();

          for( ; textIt != actual.text.end(); ++textIt, ++cursorIt )
          {
            output.appendString(*cursorIt, *textIt);
          }
          output.mark(input.inputPosition());
          continue;
        }

        // TODO handle inbuilt "defined" etc functions

        pp_macro* macro = m_engine->environment()->retrieveMacro(name);
        
        if (!macro || !macro->defined || macro->hidden || m_engine->hideNextMacro())
        {
          m_engine->setHideNextMacro(name == definedIndex);

          if (name == lineIndex)
            output.appendString(inputPosition, convertFromByteArray(QString::number(input.inputPosition().line).toUtf8()));
          else if (name == fileIndex)
            output.appendString(inputPosition, convertFromByteArray(QString("\"%1\"").arg(m_engine->currentFileNameString()).toUtf8()));
          else if (name == dateIndex)
            output.appendString(inputPosition, convertFromByteArray(QDate::currentDate().toString("MMM dd yyyy").toUtf8()));
          else if (name == timeIndex)
            output.appendString(inputPosition, convertFromByteArray(QTime::currentTime().toString("hh:mm:ss").toUtf8()));
          else
            output.appendString(inputPosition, name);
          continue;
        }
        
        if (!macro->function_like)
        {
          EnableMacroExpansion enable(output, input.inputPosition()); //Configure the output-stream so it marks all stored input-positions as transformed through a macro
          pp_macro* m = 0;

          if (macro->definitionSize()) {
            macro->hidden = true;

            pp_macro_expander expand_macro(m_engine);
            Stream ms(macro->definition(), macro->definitionSize(), Anchor(input.inputPosition(), true));
            ms.setOriginalInputPosition(input.originalInputPosition());
            PreprocessedContents expanded;
            {
              Stream es(&expanded);
              expand_macro(ms, es);
            }

            if (!expanded.isEmpty())
            {
              Stream es(&expanded, Anchor(input.inputPosition(), true));
              es.setOriginalInputPosition(input.originalInputPosition());
              skip_whitespaces(es, devnull());
              IndexedString identifier( skip_identifier(es) );

              pp_macro* m2 = 0;
              if (es.atEnd() && (m2 = m_engine->environment()->retrieveMacro(identifier)) && m2->defined) {
                m = m2;
              } else {
                output.appendString(Anchor(input.inputPosition(), true), expanded);
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
        int openingPosition = input.offset();
        Anchor openingPositionCursor = input.inputPosition();
        
        // function like macro
        if (input.atEnd() || input != '(')
        {
          output.appendString(inputPosition, name);
          continue;
        }

        QList<pp_actual> actuals;
        ++input; // skip '('

        pp_macro_expander expand_actual(m_engine, m_frame);

        int before = input.offset();
        {
          actual.clear();

          PreprocessedContents actualText;
          skip_whitespaces(input, devnull());
          Anchor actualStart = input.inputPosition();
          {
            Stream as(&actualText);
            skip_argument_variadics(actuals, macro, input, as);
          }
          trim(actualText);

          if (input.offset() != before)
          {
            pp_actual newActual;
            {
              PreprocessedContents newActualText;
              Stream as(&actualText, actualStart);
              as.setOriginalInputPosition(input.originalInputPosition());

              rpp::LocationTable table;
              table.anchor(0, actualStart, 0);
              Stream nas(&newActualText, actualStart, &table);
              expand_actual(as, nas);
              
              table.splitByAnchors(newActualText, actualStart, newActual.text, newActual.inputPosition);
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
            PreprocessedContents actualText;
            skip_whitespaces(input, devnull());
            Anchor actualStart = input.inputPosition();
            {
              Stream as(&actualText);
              skip_argument_variadics(actuals, macro, input, as);
            }
            trim(actualText);

            pp_actual newActual;
            {
              PreprocessedContents newActualText;
              Stream as(&actualText, actualStart);
              as.setOriginalInputPosition(input.originalInputPosition());

              PreprocessedContents actualText;
              rpp::LocationTable table;
              table.anchor(0, actualStart, 0);
              Stream nas(&newActualText, actualStart, &table);
              expand_actual(as, nas);
              
              table.splitByAnchors(newActualText, actualStart, newActual.text, newActual.inputPosition);
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
        EnableMacroExpansion enable(output, input.inputPosition()); //Configure the output-stream so it marks all stored input-positions as transformed through a macro

        pp_frame frame(macro, actuals);
        pp_macro_expander expand_macro(m_engine, &frame);
        macro->hidden = true;
        Stream ms(macro->definition(), macro->definitionSize(), Anchor(input.inputPosition(), true));
        ms.setOriginalInputPosition(input.originalInputPosition());
        expand_macro(ms, output);
        macro->hidden = false;
      } else {
        output << input;
        ++input;
      }
    }

  }
}

void pp_macro_expander::skip_argument_variadics (const QList<pp_actual>& __actuals, pp_macro *__macro, Stream& input, Stream& output)
{
  int first;

  do {
    first = input.offset();
    skip_argument(input, output);

  } while ( __macro->variadics
            && first != input.offset()
            && !input.atEnd()
            && input == '.'
            && (__actuals.size() + 1) == (int)__macro->formalsSize());
}

