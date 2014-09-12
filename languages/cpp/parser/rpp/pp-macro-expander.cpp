/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2006 Hamish Rodda <rodda@kde.org>
  Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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
#include <KLocalizedString> 

#include <language/duchain/problem.h>
#include <serialization/indexedstring.h>

#include "pp-internal.h"
#include "pp-engine.h"
#include "pp-environment.h"
#include "pp-location.h"
#include "preprocessor.h"
#include "chartools.h"

const int maxMacroExpansionDepth = 70;

using namespace KDevelop;

QString joinIndexVector(const uint* arrays, uint size, QString between) {
  QString ret;
  FOREACH_CUSTOM(uint item, arrays, size) {
    if(!ret.isEmpty())
      ret += between;
    ret += IndexedString::fromIndex(item).str();
  }
  return ret;
}

QString joinIndexVector(const IndexedString* arrays, uint size, QString between) {
  QString ret;
  FOREACH_CUSTOM(const IndexedString& item, arrays, size) {
    if(!ret.isEmpty())
      ret += between;
    ret += item.str();
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
  : depth(0)
  , expandingMacro(__expandingMacro)
  , actuals(__actuals)
{
}

pp_actual pp_macro_expander::resolve_formal(const IndexedString& name, Stream& input)
{
  if (!m_frame)
    return pp_actual();

  Q_ASSERT(m_frame->expandingMacro != 0);

  const IndexedString* formals = m_frame->expandingMacro->formals();
  uint formalsSize = m_frame->expandingMacro->formalsSize();

  if(name.isEmpty()) {
    KDevelop::ProblemPointer problem(new KDevelop::Problem);
    problem->setFinalLocation(KDevelop::DocumentRange(IndexedString(m_engine->currentFileNameString()), RangeInRevision(input.originalInputPosition(), 0).castToSimpleRange()));
    problem->setDescription(i18n("Macro error"));
    m_engine->problemEncountered(problem);
    return pp_actual();
  }
  
  for (uint index = 0; index < formalsSize; ++index) {
    if (name.index() == formals[index].index()) {
      if (index < (uint)m_frame->actuals.size()) {
        return m_frame->actuals[index];
      }
      else {
        KDevelop::ProblemPointer problem(new KDevelop::Problem);
        problem->setFinalLocation(KDevelop::DocumentRange(IndexedString(m_engine->currentFileNameString()), RangeInRevision(input.originalInputPosition(), 0).castToSimpleRange()));
        problem->setDescription(i18n("Call to macro %1 missing argument number %2", name.str(), index));
        problem->setExplanation(i18n("Formals: %1", joinIndexVector(formals, formalsSize, ", ")));
        m_engine->problemEncountered(problem);
      }
    }
  }

  return pp_actual();
}

#define RETURN_IF_INPUT_BROKEN    if(input.atEnd()) { kDebug() << "too early end while expanding" << macro->name.str(); return; }


pp_macro_expander::pp_macro_expander(pp* engine, pp_frame* frame, bool inHeaderSection)
  : m_engine(engine)
  , m_frame(frame)
  , m_in_header_section(inHeaderSection)
  , m_search_significant_content(false)
  , m_found_significant_content(false)
{
  if(m_in_header_section)
    m_search_significant_content = true; //Find the end of the header section
}

//A header-section ends when the first non-directive and non-comment occurs
#define check_header_section \
  if( m_search_significant_content ) \
  { \
     \
    if(m_in_header_section) { \
      m_in_header_section = false; \
      m_engine->preprocessor()->headerSectionEnded(input); \
    } \
    m_found_significant_content = true;  \
    m_search_significant_content = false; \
    if( input.atEnd() ) \
      continue; \
  } \

struct EnableMacroExpansion {
  EnableMacroExpansion(Stream& _input, const KDevelop::CursorInRevision& expansionPosition) : input(_input), hadMacroExpansion(_input.macroExpansion().isValid()) {
    
    if(!hadMacroExpansion)
      _input.setMacroExpansion(expansionPosition);
  }
  ~EnableMacroExpansion() {
    if(!hadMacroExpansion)
      input.setMacroExpansion(KDevelop::CursorInRevision::invalid());
  }
  Stream& input;
  bool hadMacroExpansion;
};

//A helper class that temporary hides a macro in the environment
class MacroHider {
  public:
  MacroHider(pp_macro* macro, Environment* environment) : m_macro(macro), m_environment(environment) {
    
    m_hideMacro.name = macro->name;
    m_hideMacro.hidden = true;
    environment->insertMacro(&m_hideMacro);
  }
  ~MacroHider() {
    m_environment->insertMacro(m_macro);
  }
  private:
    pp_macro m_hideMacro;
    pp_macro* m_macro;
    Environment* m_environment;
};

void pp_macro_expander::operator()(Stream& input, Stream& output, bool substitute, LocationTable* table)
{
  skip_blanks(input, output);

  while (!input.atEnd())
  {
    if (isComment(input))
    {
      skip_comment_or_divop(input, output, true);
    }else{
      if (input == '\\' && input.peekNextCharacter() == '\n')
      {
        ++input;
        ++input;
        skip_blanks(input, output);
        if (!input.atEnd() && input == '#')
          break;
      }
      else if (input == '\n')
      {
        output << input;

        skip_blanks(++input, output);

        if (!input.atEnd() && input == '#')
          break;
      }
      else if (input == '#')
      {
        Q_ASSERT(isCharacter(input.current()));
        Q_ASSERT(IndexedString::fromIndex(input.current()).str() == "#");
        
        ++input;
        
        // search for the paste token
        if(input == '#') {
          ++input;
          skip_blanks (input, devnull());
          
          // Need to extract previous identifier in case there are spaces in front of current output position
          // May happen if parameter to the left of ## was expanded to an empty string
          IndexedString previous;
          if (output.offset() > 0) {
            previous = IndexedString::fromIndex(output.popLastOutput()); //Previous already has been expanded
            while(output.offset() > 0 && isSpace(previous.index()))
              previous = IndexedString::fromIndex(output.popLastOutput());   
          }
          output.appendString(output.currentOutputAnchor(), previous);
          // OK to put the merged tokens into stream separately, because the stream in character-based
          Anchor nextStart = input.inputPosition();
          IndexedString next = IndexedString::fromIndex(skip_identifier (input));
          pp_actual actualNext = resolve_formal(next, input);
          if (!actualNext.isValid()) {
            output.appendString(nextStart, next);
          }else{
            output.appendString(actualNext.sourcePosition, actualNext.sourceText);
          }
          output.mark(input.inputPosition());
          continue;
        }
        
        skip_blanks(input, output);

        IndexedString identifier = IndexedString::fromIndex( skip_identifier(input) );

        Anchor inputPosition = input.inputPosition();
        KDevelop::CursorInRevision originalInputPosition = input.originalInputPosition();
        PreprocessedContents formal = resolve_formal(identifier, input).sourceText;
        
        //Escape so we don't break on '"'
        for(int a = formal.count()-1; a >= 0; --a) {
          if(formal[a] == indexFromCharacter('\"') || formal[a] == indexFromCharacter('\\'))
            formal.insert(a, indexFromCharacter('\\'));
            else if(formal[a] == indexFromCharacter('\n')) {
              //Replace newlines with "\n"
              formal[a] = indexFromCharacter('n');
              formal.insert(a, indexFromCharacter('\\'));
            }
              
        }
        Stream is(&formal, inputPosition);
        is.setOriginalInputPosition(originalInputPosition);
        skip_whitespaces(is, devnull());

        output << '\"';

        while (!is.atEnd()) {
          output << is;

          skip_whitespaces(++is, output);
        }

        output << '\"';
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
        int offset = input.offset();
        IndexedString name = IndexedString::fromIndex(skip_identifier (input));
        
        // peek forward to check for ##
        int start = input.offset();
        skip_blanks(input, devnull());
        if (!input.atEnd()) {
          if(input == '#' && (++input) == '#') {
              ++input;
              //We have skipped a paste token
              skip_blanks(input, devnull());
              pp_actual actualFirst = resolve_formal(name, input);

              if (!actualFirst.isValid()) {
                output.appendString(inputPosition, name);
              } else {
                output.appendString(actualFirst.sourcePosition, actualFirst.sourceText);
              }

              input.seek(start); // will need to process the second argument too
              output.mark(input.inputPosition());
              continue;
          }else{
            input.seek(start);
          }
        }

        if (substitute) {
        pp_actual actual = resolve_formal(name, input);
        if (actual.isValid()) {
          Q_ASSERT(actual.text.size() == actual.inputPosition.size());
          
          QList<PreprocessedContents>::const_iterator textIt = actual.text.constBegin();
          QList<Anchor>::const_iterator cursorIt = actual.inputPosition.constBegin();

          for( ; textIt != actual.text.constEnd(); ++textIt, ++cursorIt )
          {
            output.appendString(*cursorIt, *textIt);
          }
          output << ' '; //Insert a whitespace to omit implicit token merging
            
            }else{
            output.appendString(inputPosition, name);
          }
          
          continue;
        }

        // TODO handle inbuilt "defined" etc functions

        pp_macro* macro = m_engine->environment()->retrieveMacro(name, false);
        
        if (!macro || !macro->defined || macro->hidden || macro->function_like || m_engine->hideNextMacro())
        {
          static const IndexedString definedIndex = IndexedString("defined");
          m_engine->setHideNextMacro(name == definedIndex);


        static const IndexedString lineIndex = IndexedString("__LINE__");
        static const IndexedString fileIndex = IndexedString("__FILE__");
        static const IndexedString dateIndex = IndexedString("__DATE__");
        static const IndexedString timeIndex = IndexedString("__TIME__");
          if (name == lineIndex)
            output.appendString(inputPosition, convertFromByteArray(QString::number(input.inputPosition().line).toUtf8()));
          else if (name == fileIndex)
            output.appendString(inputPosition, convertFromByteArray(QString("\"%1\"").arg(m_engine->currentFileNameString()).toUtf8()));
          else if (name == dateIndex)
            output.appendString(inputPosition, convertFromByteArray(QDate::currentDate().toString("\"MMM dd yyyy\"").toUtf8()));
          else if (name == timeIndex)
            output.appendString(inputPosition, convertFromByteArray(QTime::currentTime().toString("\"hh:mm:ss\"").toUtf8()));
          else {
            if (table) {
              // In case of a merged token, find some borders for it inside a macro invocation
              Anchor leftmost = table->positionAt(offset, *input.source(), true).first;
              Anchor rightmost = table->positionAt(input.offset(), *input.source(), true).first;
              // The order of parameters inside macro body may be different from its declaration
              if (rightmost < leftmost) {
                qSwap(rightmost, leftmost);
              }
              output.appendString(leftmost, name);
              if (rightmost != leftmost) {
                output.mark(rightmost);
              }
            } else {
            output.appendString(inputPosition, name);
            }
          }
          continue;
        }
        
        EnableMacroExpansion enable(output, input.inputPosition()); //Configure the output-stream so it marks all stored input-positions as transformed through a macro

          if (macro->definitionSize()) {
            //Hide the expanded macro to prevent endless expansion
            MacroHider hideMacro(macro, m_engine->environment());
            
            pp_macro_expander expand_macro(m_engine);
            ///@todo UGLY conversion
            Stream ms((uint*)macro->definition(), macro->definitionSize(), Anchor(input.inputPosition(), true));
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
              IndexedString identifier = IndexedString::fromIndex( skip_identifier(es) );

              output.appendString(Anchor(input.inputPosition(), true), expanded);
              output << ' '; //Prevent implicit token merging
            }
          }
        }else if(input == '(' && !substitute) {

        //Eventually execute a function-macro
          
        IndexedString previous = IndexedString::fromIndex(indexFromCharacter(' ')); //Previous already has been expanded
        uint stepsBack = 0;
        while(isSpace(previous.index()) && output.peekLastOutput(stepsBack)) {
          previous = IndexedString::fromIndex(output.peekLastOutput(stepsBack));
          ++stepsBack;
        }
        pp_macro* macro = m_engine->environment()->retrieveMacro(previous, false);
        if(!macro || !macro->function_like || !macro->defined || macro->hidden) {
          output << input;
          ++input;
          continue;
        }
        
        //In case expansion fails, we can skip back to this position
        int openingPosition = input.offset();
        Anchor openingPositionCursor = input.inputPosition();
        
        QList<pp_actual> actuals;
        ++input; // skip '('
        
        if(input.atEnd())
        {
          // If the input has ended too early, seek back, and flush the input into the output
          input.seek(openingPosition);
          input.setInputPosition(openingPositionCursor);
          while(!input.atEnd())
          {
            output << input;
            ++input;
          }
          
          kDebug() << "too early end while expanding" << macro->name.str();
          return;
        }

        pp_macro_expander expand_actual(m_engine, m_frame);
        skip_actual_parameter(input, *macro, actuals, expand_actual);

        while (!input.atEnd() && input == ',')
        {
          ++input; // skip ','
          
          if(input.atEnd())
          {
            // If the input has ended too early, seek back, and flush the input into the output
            input.seek(openingPosition);
            input.setInputPosition(openingPositionCursor);
            while(!input.atEnd())
            {
              output << input;
              ++input;
            }
            
            kDebug() << "too early end while expanding" << macro->name.str();
            return;
          }
          
          skip_actual_parameter(input, *macro, actuals, expand_actual);
        }

        if( input != ')' ) {
          //Failed to expand the macro. Output the macro name and continue normal
          //processing behind it.(Code completion depends on this behavior when expanding
          //incomplete input-lines)
          input.seek(openingPosition);
          input.setInputPosition(openingPositionCursor);
          //Move one character into the output, so we don't get an endless loop
          output << input;
          ++input;
          
          continue;
        }
        
        //Remove the name of the called macro
        while(stepsBack) {
          --stepsBack;
          output.popLastOutput();
        }
        
        //Q_ASSERT(!input.atEnd() && input == ')');

        ++input; // skip ')'
        
#if 0 // ### enable me
        assert ((macro->variadics && macro->formals.size () >= actuals.size ())
                    || macro->formals.size() == actuals.size());
#endif
        EnableMacroExpansion enable(output, input.inputPosition()); //Configure the output-stream so it marks all stored input-positions as transformed through a macro

        pp_frame frame(macro, actuals);
        if(m_frame)
          frame.depth = m_frame->depth + 1;
        
        if(frame.depth >= maxMacroExpansionDepth) 
        {
          kDebug() << "reached maximum macro-expansion depth while expanding" << macro->name.str();
          RETURN_IF_INPUT_BROKEN
          
          output << input;
          ++input;
        }else{
          pp_macro_expander expand_macro(m_engine, &frame);
          
          //Hide the expanded macro to prevent endless expansion
          MacroHider hideMacro(macro, m_engine->environment());
          
          ///@todo UGLY conversion
          Stream ms((uint*)macro->definition(), macro->definitionSize(), Anchor(input.inputPosition(), true));

          PreprocessedContents expansion_text;
          rpp::LocationTable table;
          Stream expansion_stream(&expansion_text, Anchor(input.inputPosition(), true), &table);
          expand_macro(ms, expansion_stream, true);

          Stream ns(&expansion_text, Anchor(input.inputPosition(), true));
          ns.setOriginalInputPosition(input.originalInputPosition());
          expand_macro(ns, output, false, &table);
          output << ' '; //Prevent implicit token merging
        }
      } else {
        output << input;
        ++input;
      }
    }

  }
}

void pp_macro_expander::skip_actual_parameter(Stream& input, rpp::pp_macro& macro, QList< pp_actual >& actuals, pp_macro_expander& expander)
{
  PreprocessedContents actualText;
  skip_whitespaces(input, devnull());
  Anchor actualStart = input.inputPosition();
  {
    Stream as(&actualText);
    skip_argument_variadics(actuals, &macro, input, as);
  }
  trim(actualText);

  pp_actual newActual;
  newActual.sourceText = actualText;
  newActual.sourcePosition = actualStart;
  {
    PreprocessedContents newActualText;
    Stream as(&actualText, actualStart);
    as.setOriginalInputPosition(input.originalInputPosition());

    rpp::LocationTable table;
    table.anchor(0, actualStart, 0);
    Stream nas(&newActualText, actualStart, &table);
    expander(as, nas);
    table.splitByAnchors(newActualText, actualStart, newActual.text, newActual.inputPosition);
  }
  newActual.forceValid = true;

  actuals.append(newActual);
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
