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

#include "pp-engine.h"

#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>

#include <language/duchain/duchain.h>

#include "pp-internal.h"
#include "preprocessor.h"
#include "pp-environment.h"
#include "pp-location.h"
#include "chartools.h"
#include "macrorepository.h"

using namespace rpp;

#define RETURN_ON_FAIL(x) if(!(x)) { ++input; kDebug(9007) << "Preprocessor: Condition not satisfied"; return; }

pp::pp(Preprocessor* preprocessor)
  : m_environment(new Environment)
  , expand(this, 0, true)
  , m_preprocessor(preprocessor)
  , nextToken(0)
  , haveNextToken(false)
  , hideNext(false)
  , hadGuardCandidate(false)
  , checkGuardEnd(false)
{
  iflevel = 0;
  _M_skipping[iflevel] = 0;
  _M_true_test[iflevel] = 0;
}

pp::~pp()
{
  delete m_environment;
}

Preprocessor* pp::preprocessor() {
  return m_preprocessor;
}

PreprocessedContents pp::processFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        kWarning(9007) << "file '" << fileName << "' not found!" ;
        return PreprocessedContents();
    }

    PreprocessedContents result;
    processFileInternal(fileName, file.readAll(), result);
    return result;
}

PreprocessedContents pp::processFile(const QString& fileName, const QByteArray& data)
{
    PreprocessedContents result;
    processFileInternal(fileName, data, result);
    return result;
}

void pp::processFileInternal(const QString& fileName, const QByteArray& fileContents, PreprocessedContents& result)
{
    m_files.push(KDevelop::IndexedString(fileName));
    // Guestimate as to how much expansion will occur
    result.reserve(int(fileContents.length() * 1.2));
    PreprocessedContents contents = tokenizeFromByteArray(fileContents);
    {
      Stream is(&contents);
      Stream rs(&result, m_environment->locationTable());
      operator () (is, rs);
    }
    result.squeeze();
}

void pp::handle_directive(uint directive, Stream& input, Stream& output)
{
  static const uint ifDirective = KDevelop::IndexedString("if").index();
  static const uint elseDirective = KDevelop::IndexedString("else").index();
  static const uint elifDirective = KDevelop::IndexedString("elif").index();
  static const uint ifdefDirective = KDevelop::IndexedString("ifdef").index();
  static const uint undefDirective = KDevelop::IndexedString("undef").index();
  static const uint endifDirective = KDevelop::IndexedString("endif").index();
  static const uint ifndefDirective = KDevelop::IndexedString("ifndef").index();
  static const uint defineDirective = KDevelop::IndexedString("define").index();
  static const uint includeDirective = KDevelop::IndexedString("include").index();
  static const uint includeNextDirective = KDevelop::IndexedString("include_next").index();

  skip_blanks (input, output);
  while (!input.atEnd() && input != '\n' && input == '/' && input.peekNextCharacter() == '*')
  {
    skip_comment_or_divop(input, output);
    skip_blanks (input, output);
  }

  if(!(directive == ifndefDirective)) {
    hadGuardCandidate = true; //Too late, the guard must be the first directive
  }
  if(checkGuardEnd) {
    guardCandidate = KDevelop::IndexedString();
    checkGuardEnd = false;
  }
  
  if(directive == defineDirective)
      if (! skipping ())
        return handle_define(input);

  if(directive == includeDirective || directive == includeNextDirective)
      if (! skipping ())
        return handle_include (directive == includeNextDirective, input, output);

  if(directive == undefDirective)
      if (! skipping ())
        return handle_undef(input);

  if(directive == elifDirective)
      return handle_elif(input);

  if(directive == elseDirective)
      return handle_else(input.inputPosition().line);

  if(directive == endifDirective)
      return handle_endif(input, output);

  if(directive == ifDirective)
      return handle_if(input);

  if(directive == ifdefDirective)
      return handle_ifdef(false, input);

  if(directive == ifndefDirective)
      return handle_ifdef(true, input);
}

void pp::handle_include(bool skip_current_path, Stream& input, Stream& output)
{
  if (input.atEnd()) {
    createProblem(input, i18n("invalid include directive"));
    return;
  }
  QByteArray bytes = KDevelop::IndexedString::fromIndex(input.current()).byteArray();
  if (bytes.size() > 0 && (isLetter(bytes.at(0)) || bytes.at(0) == '_')) {
    pp_macro_expander expand_include(this);

    Anchor inputPosition = input.inputPosition();
    KDevelop::CursorInRevision originalInputPosition = input.originalInputPosition();
    PreprocessedContents includeString;
    {
      Stream cs(&includeString);
      expand_include(input, cs);
    }

    skip_blanks(input, devnull());
    RETURN_ON_FAIL(!includeString.isEmpty() && (includeString.first() == indexFromCharacter('<') || includeString.first() == indexFromCharacter('"')));

    // Filter out whitespaces
    PreprocessedContents filteredIncludeString;
    if(!includeString.empty() && includeString.front() == indexFromCharacter('"'))
    {
      // Don't filter if it is a string token
      filteredIncludeString = includeString;
    }
    else
    {
      // Filter out whitespaces, as the preprocessor adds them in random places to prevent implicit token merging
      foreach(uint index, includeString)
          if(index != indexFromCharacter(' '))
            filteredIncludeString.push_back(index);
    }
    
    Stream newInput(&filteredIncludeString, inputPosition);
    newInput.setOriginalInputPosition(originalInputPosition);
    handle_include(skip_current_path, newInput, output);
    return;
  }

  RETURN_ON_FAIL(input == '<' || input == '"');
  char quote((input == '"') ? '"' : '>');
  ++input;

  PreprocessedContents includeNameB;

  while (!input.atEnd() && input != quote) {
    RETURN_ON_FAIL(input != '\n');

      includeNameB.append(input);
    ++input;
  }

  QString includeName(QString::fromUtf8(stringFromContents(includeNameB)));

  Stream* include = m_preprocessor->sourceNeeded(includeName, quote == '"' ? Preprocessor::IncludeLocal : Preprocessor::IncludeGlobal, input.inputPosition().line, skip_current_path);
  Q_ASSERT(!include);

  /*if (include && !include->atEnd()) {
    m_files.push(includeName);

    output.mark(includeName, 0);

    operator()(*include, output);

    // restore the file name and sync the buffer
    output.mark(m_files.pop(), input.inputPosition().line);
  }*/

  delete include;
}

void pp::operator () (Stream& input, Stream& output)
{
  int previousIfLevel = iflevel;

  forever
  {
    haveNextToken = false;

    if (skipping()) {
      skip_blanks(input, devnull());

    } else {
      skip_blanks(input, output);
    }

    if (input.atEnd()) {
      break;

    } else if (input == '#') {
      skip_blanks(++input, devnull());

      uint directive = skip_identifier(input);

      skip_blanks(input, devnull());

      Anchor inputPosition = input.inputPosition();
      KDevelop::CursorInRevision originalInputPosition = input.originalInputPosition();

      PreprocessedContents skipped;
      {
        Stream ss(&skipped);
        skip (input, ss);
      }

      Stream ss(&skipped, inputPosition);
      ss.setOriginalInputPosition(originalInputPosition);
      handle_directive(directive, ss, output);

    } else if (input == '\n') {
      output << input;
      ++input;

    } else if (skipping ()) {
      skip (input, devnull());

    } else {
      output.mark(input.inputPosition());
      if(checkGuardEnd) {
        expand.startSignificantContentSearch();
      }
      
      expand (input, output);
      if(checkGuardEnd) {
        if(expand.foundSignificantContent() || !input.atEnd()) {
          guardCandidate = KDevelop::IndexedString();
        }
        checkGuardEnd = false;
      }
    }
  }
  
  if(!guardCandidate.isEmpty())
    preprocessor()->foundHeaderGuard(input, guardCandidate);

  if (iflevel != previousIfLevel && !input.skippedToEnd())
    createProblem(input, i18n("Unterminated #if statement"));
}

void pp::createProblem(Stream& input, const QString& description) {
    KDevelop::ProblemPointer problem(new KDevelop::Problem);
    problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 0)));
    problem->setDescription(description);
    problemEncountered(problem);
}

void pp::handle_define (Stream& input)
{
  pp_macro* macro = new pp_macro;
  macro->file = currentFileName();
  macro->sourceLine = input.originalInputPosition().line;

  skip_blanks (input, devnull());
  macro->name = KDevelop::IndexedString::fromIndex(skip_identifier(input)); //@todo make macros utf8 too

  if (!input.atEnd() && input == '(')
  {
    macro->function_like = true;

    skip_blanks (++input, devnull()); // skip '('
    uint formal = skip_identifier(input);

    skip_blanks(input, devnull());

    if (input == '.') {
      macro->variadics = true;

      do {
        ++input;

      } while (input == '.');
    }
    if (formal && !macro->variadics)
      macro->formalsList().append( KDevelop::IndexedString::fromIndex(formal) );


    while (!input.atEnd() && input == ',')
    {
      skip_blanks(++input, devnull());

      uint formal = skip_identifier(input);

      skip_blanks (input, devnull());

      if (input == '.') {
        macro->variadics = true;

        do {
          ++input;

        } while (input == '.');
      }

      if (formal && !macro->variadics)
        macro->formalsList().append( KDevelop::IndexedString::fromIndex(formal) );
    }

    RETURN_ON_FAIL(input == ')');
    ++input;
  }

  skip_blanks (input, devnull());

  while (!input.atEnd() && input != '\n')
  {
    if(input == '/' && (input.peekNextCharacter() == '/' || input.peekNextCharacter() == '*')) {
      skip_comment_or_divop(input, devnull());
      if(!input.atEnd() && input != '\n')
        skip_blanks (input, devnull());
      continue;
    }
    if (input == '\\')
    {
      int pos = input.offset();
      skip_blanks (++input, devnull());

      if (!input.atEnd() && input == '\n')
      {
        skip_blanks(++input, devnull());
        macro->definitionList().append(KDevelop::IndexedString::fromIndex(indexFromCharacter(' ')));
        continue;

      } else {
        // Whoops, rewind :)
        input.seek(pos);
      }
    }
    if (input == '"')
    {
      do {
        if (input == '\\' && input.peekNextCharacter() == '"') {
          // skip escaped close quote
          macro->definitionList().append(KDevelop::IndexedString::fromIndex(input.current()));
          ++input;
          if(input.atEnd())
            break;
        }
        macro->definitionList().append(KDevelop::IndexedString::fromIndex(input.current()));
        ++input;
      } while (!input.atEnd() && input != '"' && input != '\n');

      if(!input.atEnd())
      {
        macro->definitionList().append(KDevelop::IndexedString::fromIndex(input.current()));
        ++input;
      }
      continue;
    }

    macro->definitionList().append(KDevelop::IndexedString::fromIndex(input.current()));
    ++input;
  }

  m_environment->setMacro(macro);
}


void pp::skip (Stream& input, Stream& output, bool outputText)
{
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;

  while (!input.atEnd() && input != '\n')
  {
    if (input == '/')
    {
      skip_comment_or_divop (input, output, outputText);
    }
    else if (input == '"')
    {
      skip_string_literal (input, output);
    }
    else if (input == '\'')
    {
      skip_char_literal (input, output);
    }
    else if (input == '\\')
    {
      output << input;
      skip_blanks (++input, output);

      if (!input.atEnd() && input == '\n')
      {
        output << input;
        ++input;
      }
    }
    else
    {
      output << input;
      ++input;
    }
  }
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

Value pp::eval_primary(Stream& input)
{
  int start = input.offset();
  bool expect_paren = false;
  int token = next_token_accept(input);
  Value result;

  switch (token) {
    case TOKEN_NUMBER:
      result.set_long(token_value);
      break;

    case TOKEN_UNUMBER:
      result.set_ulong(token_uvalue);
      break;

    case TOKEN_DEFINED:
      token = next_token_accept(input);

      if (token == '(')
      {
        expect_paren = true;
        token = next_token_accept(input);
      }

      if (token != TOKEN_IDENTIFIER)
      {
        KDevelop::ProblemPointer problem(new KDevelop::Problem);
        problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 1)));
        QChar tk(token);
        problem->setDescription(i18n("Expected \"identifier\", found: %1", (tk < TOKENS_END && tk > TOKENS_START) ? QString(tk) : i18n("character %1", token)));
        problem->setExplanation(i18n("<h5>Token text</h5><pre>%1</pre><h5>Input</h5><pre>%2</pre>", token_text.str(), QString::fromUtf8(input.stringFrom(start))));
        problemEncountered(problem);
        break;
      }

      {
        pp_macro* m = m_environment->retrieveMacro(token_text, true);
        result.set_long( (m && !m->isUndef()) ? 1 : 0);
      }

      token = next_token(input); // skip '('

      if (expect_paren) {
        if (token != ')') {
          KDevelop::ProblemPointer problem(new KDevelop::Problem);
          problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 0)));
          QChar tk(token);
          problem->setDescription(i18n("Expected \")\", found %1", tk.isLetterOrNumber() ? QString(tk) : i18n("character %1", token)));
          problem->setExplanation(i18n("<h5>Token text</h5><pre>%1</pre><h5>Input</h5><pre>%2</pre>", token_text.str(), QString::fromUtf8(input.stringFrom(start))));
          problemEncountered(problem);
        } else {
          accept_token();
        }
      }
      break;

    case TOKEN_IDENTIFIER:
      break;

    case '-':
      result.set_long(- eval_primary(input).l);
      break;

    case '+':
      result.set_long(+ eval_primary(input).l);
      break;

    case '!':
      result.set_long(eval_primary(input).is_zero());
      break;

    case '~':
      result.set_long(~ eval_primary(input).l);
      break;

    case '(':
      result = eval_constant_expression(input);
      token = next_token(input);

      if (token != ')') {
        KDevelop::ProblemPointer problem(new KDevelop::Problem);
        problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 1)));
        QChar tk(token);
        problem->setDescription(i18n("Expected \")\", found %1", tk.isLetterOrNumber() ? QString(tk) : i18n("character %1", token)));
        problem->setExplanation(i18n("<h5>Token text</h5><pre>%1</pre><h5>Input</h5><pre>%2</pre>", token_text.str(), QString::fromUtf8(input.stringFrom(start))));
        problemEncountered(problem);
      } else {
        accept_token();
      }

      break;

    default:
      break;
  }

  return result;
}

Value pp::eval_multiplicative(Stream& input)
{
  int start = input.offset();

  Value result = eval_primary(input);

  int token = next_token(input);

  while (token == '*' || token == '/' || token == '%') {
    accept_token();

    Value value = eval_primary(input);

    if (token == '*') {
      result *= value;

    } else if (token == '/') {
      if (value.is_zero() || (!value.is_ulong() && !result.is_ulong() && value.l == -1 && result.l == LLONG_MIN)) {
        KDevelop::ProblemPointer problem(new KDevelop::Problem);
        problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 0)));
        problem->setDescription(i18n("Division by zero"));
        problem->setDescription(i18n("Input text: %1", QString::fromUtf8(input.stringFrom(start))));
        problemEncountered(problem);
        result.set_long(0);

      } else {
        result /= value;
      }

    } else {
      if (value.is_zero() || (!value.is_ulong() && !result.is_ulong() && value.l == -1 && result.l == LLONG_MIN)) {
        KDevelop::ProblemPointer problem(new KDevelop::Problem);
        problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 0)));
        problem->setDescription(i18n("Division by zero"));
        problem->setDescription(i18n("Input text: %1", QString::fromUtf8(input.stringFrom(start))));
        problemEncountered(problem);
        result.set_long(0);

      } else {
        result %= value;
      }
    }

    token = next_token(input);
  }

  return result;
}

Value pp::eval_additive(Stream& input)
{
  Value result = eval_multiplicative(input);

  int token = next_token(input);

  while (token == '+' || token == '-') {
    accept_token();

    Value value = eval_multiplicative(input);

    if (token == '+')
      result += value;
    else
      result -= value;

    token = next_token(input);
  }

  return result;
}


Value pp::eval_shift(Stream& input)
{
  Value result = eval_additive(input);

  int token;
  token = next_token(input);

  while (token == TOKEN_LT_LT || token == TOKEN_GT_GT) {
    accept_token();

    Value value = eval_additive(input);

    if (token == TOKEN_LT_LT)
      result <<= value;
    else
      result >>= value;

    token = next_token(input);
  }

  return result;
}


Value pp::eval_relational(Stream& input)
{
  Value result = eval_shift(input);

  int token = next_token(input);

  while (token == '<'
      || token == '>'
      || token == TOKEN_LT_EQ
      || token == TOKEN_GT_EQ)
  {
    accept_token();
    Value value = eval_shift(input);

    switch (token)
    {
      default:
        Q_ASSERT(0);
        break;

      case '<':
        result = result < value;
        break;

      case '>':
        result = result > value;
        break;

      case TOKEN_LT_EQ:
        result = result <= value;
        break;

      case TOKEN_GT_EQ:
        result = result >= value;
        break;
    }

    token = next_token(input);
  }

  return result;
}


Value pp::eval_equality(Stream& input)
{
  Value result = eval_relational(input);

  int token = next_token(input);

  while (token == TOKEN_EQ_EQ || token == TOKEN_NOT_EQ) {
    accept_token();
    Value value = eval_relational(input);

    if (token == TOKEN_EQ_EQ)
      result = result == value;
    else
      result = result != value;

    token = next_token(input);
  }

  return result;
}


Value pp::eval_and(Stream& input)
{
  Value result = eval_equality(input);

  int token = next_token(input);

  while (token == '&') {
    accept_token();
    Value value = eval_equality(input);
    result &= value;
    token = next_token(input);
  }

  return result;
}


Value pp::eval_xor(Stream& input)
{
  Value result = eval_and(input);

  int token;
  token = next_token(input);

  while (token == '^') {
    accept_token();
    Value value = eval_and(input);
    result ^= value;
    token = next_token(input);
  }

  return result;
}


Value pp::eval_or(Stream& input)
{
  Value result = eval_xor(input);

  int token = next_token(input);

  while (token == '|') {
    accept_token();
    Value value = eval_xor(input);
    result |= value;
    token = next_token(input);
  }

  return result;
}


Value pp::eval_logical_and(Stream& input)
{
  Value result = eval_or(input);

  int token = next_token(input);

  while (token == TOKEN_AND_AND) {
    accept_token();
    Value value = eval_or(input);
    result = result && value;
    token = next_token(input);
  }

  return result;
}


Value pp::eval_logical_or(Stream& input)
{
  Value result = eval_logical_and(input);

  int token = next_token(input);

  while (token == TOKEN_OR_OR) {
    accept_token();
    Value value = eval_logical_and(input);
    result = result || value;
    token = next_token(input);
  }

  return result;
}


Value pp::eval_constant_expression(Stream& input)
{
  Value result = eval_logical_or(input);

  int token = next_token(input);

  if (token == '?')
  {
    accept_token();
    Value left_value = eval_constant_expression(input);
    skip_blanks(input, devnull());

    token = next_token_accept(input);
    if (token == ':')
    {
      Value right_value = eval_constant_expression(input);

      result = !result.is_zero() ? left_value : right_value;
    }
    else
    {
      KDevelop::ProblemPointer problem(new KDevelop::Problem);
      problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 1)));
      problem->setDescription(i18n("expected ``:'' = %1", int(token)));
      problemEncountered(problem);
      result = left_value;
    }
  }

  return result;
}


Value pp::eval_expression(Stream& input)
{
  skip_blanks(input, devnull());
  return eval_constant_expression(input);
}


void pp::handle_if (Stream& input)
{
  if (test_if_level())
  {
    pp_macro_expander expand_condition(this);
    skip_blanks(input, devnull());

    Anchor inputPosition = input.inputPosition();
    KDevelop::CursorInRevision originalInputPosition = input.originalInputPosition();
    PreprocessedContents condition;
    {
      Stream cs(&condition);
      expand_condition(input, cs);
    }

    Stream cs(&condition, inputPosition);
    cs.setOriginalInputPosition(originalInputPosition);
    Value result = eval_expression(cs);

    _M_true_test[iflevel] = !result.is_zero();
    _M_skipping[iflevel] = result.is_zero();

  } else {
    // Capture info for precompiled macros
    pp_macro_expander expand_condition(this);
    skip_blanks(input, devnull());
    PreprocessedContents condition;
    {
      Stream cs(&condition);
      expand_condition(input, cs);
    }

    _M_true_test[iflevel] = true;
    _M_skipping[iflevel] = true;
  }
}


void pp::handle_else(int sourceLine)
{
  if(iflevel == 1)
    guardCandidate = KDevelop::IndexedString();

  if (iflevel == 0 && !skipping ())
  {
    KDevelop::ProblemPointer problem(new KDevelop::Problem);
    problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(KTextEditor::Cursor(sourceLine, 0), 0)));
    problem->setDescription(i18n("#else without #if"));
    problemEncountered(problem);
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


void pp::handle_elif(Stream& input)
{
  if(iflevel == 1)
    guardCandidate = KDevelop::IndexedString();
  
  RETURN_ON_FAIL(iflevel > 0);

  if (iflevel == 0 && !skipping())
  {
    KDevelop::ProblemPointer problem(new KDevelop::Problem);
    problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 0)));
    problem->setDescription(i18n("#else without #if"));
    problemEncountered(problem);
  }
  else
  {
    pp_macro_expander expand_condition(this);
    skip_blanks(input, devnull());

    Anchor inputPosition = input.inputPosition();
    KDevelop::CursorInRevision originalInputPosition = input.originalInputPosition();
    PreprocessedContents condition;
    {
      Stream cs(&condition);
      cs.setOriginalInputPosition(originalInputPosition);
      expand_condition(input, cs);
    }

    if (!_M_true_test[iflevel] && !_M_skipping[iflevel - 1])
    {
      Stream cs(&condition, inputPosition);
      Value result = eval_expression(cs);
      _M_true_test[iflevel] = !result.is_zero();
      _M_skipping[iflevel] = result.is_zero();
    }
    else
    {
      _M_skipping[iflevel] = true;
    }
  }
}


void pp::handle_endif(Stream& input, Stream& output)
{
  if (iflevel == 0 && !skipping())
  {
    KDevelop::ProblemPointer problem(new KDevelop::Problem);
    problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(input.originalInputPosition().castToSimpleCursor(), 0)));
    problem->setDescription(i18n("#endif without #if at output line %1", m_environment->locationTable()->anchorForOffset(output.offset()).anchor.line));
    problemEncountered(problem);
  }
  else
  {
    _M_skipping[iflevel] = 0;
    _M_true_test[iflevel] = 0;

    --iflevel;
    if(iflevel == 0) {
      if(!guardCandidate.isEmpty()) {
        checkGuardEnd = true;
      }
    }
  }
}

uint pp::branchingHash() const
{
  uint hash = 0;
  for( int a = 0; a <= iflevel; a++ ) {
    hash *= 19;
    if( _M_skipping[a] )
      hash += 3;
    if( _M_true_test[a] )
      hash += 7;
  }
  return hash;
}

void pp::handle_ifdef (bool check_undefined, Stream& input)
{
  KDevelop::IndexedString macro_name = KDevelop::IndexedString::fromIndex(skip_identifier(input));
///@todo eventually fix the block description
  if(check_undefined && expand.in_header_section() && guardCandidate.isEmpty() && !hadGuardCandidate && iflevel == 0) {
    //It's the first #ifndef and the header-section hasn't ended yet, assume it to be the header-guard
    guardCandidate = macro_name;
  }

  hadGuardCandidate = true;
  
  if (test_if_level())
  {
    pp_macro* macro = m_environment->retrieveMacro(macro_name, true);
    bool value = false;
    if( macro && macro->defined )
      value = true;

    if (check_undefined)
      value = !value;

    _M_true_test[iflevel] = value;
    _M_skipping[iflevel] = !value;
  }
}


void pp::handle_undef(Stream& input)
{
  skip_blanks (input, devnull());

  KDevelop::IndexedString macro_name = KDevelop::IndexedString::fromIndex(skip_identifier(input));
  RETURN_ON_FAIL(!macro_name.isEmpty());

  pp_macro* macro = new pp_macro;
  macro->file = currentFileName();
  macro->name = macro_name;
  macro->sourceLine = input.originalInputPosition().line;

  macro->defined = false;

  m_environment->setMacro(macro);
}

int pp::next_token (Stream& input)
{
  if (haveNextToken)
    return nextToken;

  skip_blanks(input, devnull());

  if (input.atEnd())
  {
    return 0;
  }

  char ch = 0;
  if(isCharacter(input.current()))
    ch = characterFromIndex(input.current());
  char ch2 = input.peekNextCharacter();

  nextToken = 0;

  switch (ch) {
    case '/':
      if (ch2 == '/' || ch2 == '*')
      {
        skip_comment_or_divop(input, devnull());
        return next_token(input);
      }
      ++input;
      nextToken = '/';
      break;

    case '<':
      ++input;
      if (ch2 == '<')
      {
        ++input;
        nextToken = TOKEN_LT_LT;
      }
      else if (ch2 == '=')
      {
        ++input;
        nextToken = TOKEN_LT_EQ;
      }
      else
        nextToken = '<';

      break;

    case '>':
      ++input;
      if (ch2 == '>')
      {
        ++input;
        nextToken = TOKEN_GT_GT;
      }
      else if (ch2 == '=')
      {
        ++input;
        nextToken = TOKEN_GT_EQ;
      }
      else
        nextToken = '>';

      break;

    case '!':
      ++input;
      if (ch2 == '=')
      {
        ++input;
        nextToken = TOKEN_NOT_EQ;
      }
      else
        nextToken = '!';

      break;

    case '=':
      ++input;
      if (ch2 == '=')
      {
        ++input;
        nextToken = TOKEN_EQ_EQ;
      }
      else
        nextToken = '=';

      break;

    case '|':
      ++input;
      if (ch2 == '|')
      {
        ++input;
        nextToken = TOKEN_OR_OR;
      }
      else
        nextToken = '|';

      break;

    case '&':
      ++input;
      if (ch2 == '&')
      {
        ++input;
        nextToken = TOKEN_AND_AND;
      }
      else
        nextToken = '&';

      break;

    default:
      if (isLetter(ch) || ch == '_' || !isCharacter(input.current()))
      {
        token_text = KDevelop::IndexedString::fromIndex( skip_identifier (input) );
        static const KDevelop::IndexedString definedText("defined");
        if (token_text == definedText)
          nextToken = TOKEN_DEFINED;
        else
          nextToken = TOKEN_IDENTIFIER;
      }
      else if (isNumber(ch))
      {
        KTextEditor::Cursor numericBeginPosition = input.inputPosition().castToSimpleCursor();

        PreprocessedContents byteNumber;
        {
          Stream ns(&byteNumber);
          skip_number(input, ns);
        }

        KTextEditor::Cursor numericEndPosition = input.inputPosition().castToSimpleCursor();

        QString number(QString::fromUtf8(stringFromContents(byteNumber)));
        int base = 10;
        if (number.startsWith("0x")) {
          base = 16;
        } else if (number.startsWith('0')) {
          base = 8;
        }

        uint countUnsigned = 0, countLong = 0, countLongLong = 0;
        while (true) {
          if (number.endsWith("u", Qt::CaseInsensitive)) {
                  ++countUnsigned;
            number.chop(1);
          } else if (number.endsWith("ll", Qt::CaseInsensitive)) { // "ll" before "l"
                  ++countLongLong;
            number.chop(2);
          } else if (number.endsWith("l", Qt::CaseInsensitive)) {
                  ++countLong;
            number.chop (1);
          } else {
            break;
          }
        }

        if ((countUnsigned > 1) || ((countLong + countLongLong) > 1)) {
          KDevelop::ProblemPointer problem(new KDevelop::Problem);
          problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(numericBeginPosition, numericEndPosition)));
          problem->setDescription(i18n("Invalid suffix combination"));
          problemEncountered(problem);
        }

        bool parsedOk = 0;
        if (countUnsigned) {
          token_uvalue = number.toULongLong (&parsedOk, base);
          nextToken = TOKEN_UNUMBER;
        } else {
          token_value = number.toLongLong (&parsedOk, base);
          nextToken = TOKEN_NUMBER;
        }

        if (!parsedOk) {
          KDevelop::ProblemPointer problem(new KDevelop::Problem);
          problem->setFinalLocation(KDevelop::DocumentRange(currentFileName(), KTextEditor::Range(numericBeginPosition, numericEndPosition)));
          problem->setDescription(i18n("Invalid numeric value"));
          problemEncountered(problem);
        }
      }
      else
      {
        if(isCharacter(input.current()))
          nextToken = characterFromIndex(input.current());
        else
          nextToken = TOKEN_IDENTIFIER;
        ++input;
      }
  }

  //kDebug(9007) << "Next token '" << ch << ch2 << "'" << nextToken << "txt" << token_text << "val" << token_value;

  haveNextToken = true;
  return nextToken;
}

int pp::next_token_accept (Stream& input)
{
  int result = next_token(input);
  accept_token();
  return result;
}

void pp::accept_token()
{
  haveNextToken = false;
  nextToken = 0;
}

bool pp::hideNextMacro( ) const
{
  return hideNext;
}

void pp::setHideNextMacro( bool h )
{
  hideNext = h;
}

Environment* pp::environment( ) const
{
  return m_environment;
}

void pp::setEnvironment(Environment* env)
{
  delete m_environment;
  m_environment = env;
}

const QList< KDevelop::ProblemPointer > & rpp::pp::problems() const
{
  return m_problems;
}

void rpp::pp::problemEncountered(const KDevelop::ProblemPointer & problem)
{
  m_problems.append(problem);
}
