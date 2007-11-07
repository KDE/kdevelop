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

#include "pp-engine.h"

#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>

#include <duchain.h>

#include "pp-internal.h"
#include "preprocessor.h"
#include "pp-environment.h"
#include "pp-location.h"

#define PP_NO_SMART_HEADER_PROTECTION

using namespace rpp;

#define RETURN_ON_FAIL(x) if(!(x)) { ++input; kDebug(9007) << "Preprocessor: Condition not satisfied"; return; }

pp::pp(Preprocessor* preprocessor)
  : m_environment(new Environment(this))
  , expand(this, 0, true)
  , m_preprocessor(preprocessor)
  , nextToken(0)
  , haveNextToken(false)
  , hideNext(false)
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

QString pp::processFile(const QString& fileName, StringType type, const QString& data)
{
  if ( type == File  )
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
      m_files.push(fileName);

      QByteArray contents = file.readAll();
      QString contentsDecoded = QString::fromUtf8(contents);

      Stream is(&contentsDecoded);
      QString result;
      // Guestimate as to how much expansion will occur
      result.reserve(int(contents.length() * 1.2));

      {
        Stream rs(&result, m_environment->locationTable());
        operator () (is, rs);
      }

      result.squeeze();
      return result;
    }

    kWarning(9007) << "file '" << fileName << "' not found!" ;
    return QString();
  }
  else
  {
    QString result;
    // Guestimate as to how much expansion will occur
    result.reserve(int(data.length() * 1.2));
    m_files.push(fileName);

    {
      Stream is(&const_cast<QString&>(data));
      Stream rs(&result, m_environment->locationTable());
      operator () (is, rs);
    }

    result.squeeze();
    return result;
  }

}

QString pp::processFile(QIODevice* device)
{
  Q_ASSERT(device);

  QString result;

  m_files.push("<internal>");

  {
    QTextStream input(device);
    QString contents = input.readAll();
    Stream is(&contents);
    Stream rs(&result, m_environment->locationTable());
    operator () (is, rs);
  }

  return result;
}

QString pp::find_header_protection(Stream& input)
{
  while (!input.atEnd())
  {
    if (input.current().isSpace())
    {
      ++input;
    }
    else if (isComment(input))
    {
      skip_comment_or_divop (input, devnull());
    }
    else if (input == '#')
    {
      skip_blanks (++input, devnull());

      if (!input.atEnd() && input == 'i')
      {
        QString directive = skip_identifier(input);

        if (directive == "ifndef")
        {
          skip_blanks (input, devnull());

          QString define = skip_identifier(input);

          if (!define.isEmpty() && !input.atEnd())
          {
            input.reset();
            return define;
          }
        }
      }
      break;

    } else {
      break;
    }
  }

  input.reset();
  return QString();
}

pp::PP_DIRECTIVE_TYPE pp::find_directive (const QString& directive) const
{
  // TODO profile - had to be switched away from a static hash
  switch (directive.length()) {
    case 2:
      if (directive == "if")
        return PP_IF;
      break;

    case 4:
      if (directive == "elif")
        return PP_ELIF;
      if (directive == "else")
        return PP_ELSE;
      break;

    case 5:
      if (directive == "ifdef")
        return PP_IFDEF;
      if (directive == "undef")
        return PP_UNDEF;
      if (directive == "endif")
        return PP_ENDIF;
      break;

    case 6:
      if (directive == "ifndef")
        return PP_IFNDEF;
      if (directive == "define")
        return PP_DEFINE;
      break;

    case 7:
      if (directive == "include")
        return PP_INCLUDE;
      break;

    case 12:
      if (directive == "include_next")
        return PP_INCLUDE_NEXT;
      break;

    default:
      break;
  }

  return PP_UNKNOWN_DIRECTIVE;
}

void pp::handle_directive(const QString& directive, Stream& input, Stream& output)
{
  skip_blanks (input, output);

  PP_DIRECTIVE_TYPE d = find_directive(directive);
  switch (d)
  {
    case PP_DEFINE:
      if (! skipping ())
        return handle_define(input);
      break;

    case PP_INCLUDE:
    case PP_INCLUDE_NEXT:
      if (! skipping ())
        return handle_include (d == PP_INCLUDE_NEXT, input, output);
      break;

    case PP_UNDEF:
      if (! skipping ())
        return handle_undef(input);
      break;

    case PP_ELIF:
      return handle_elif(input);

    case PP_ELSE:
      return handle_else(input.inputPosition().line());

    case PP_ENDIF:
      return handle_endif(input, output);

    case PP_IF:
      return handle_if(input);

    case PP_IFDEF:
      return handle_ifdef(false, input);

    case PP_IFNDEF:
      return handle_ifdef(true, input);

    default:
      break;
  }
}

void pp::handle_include(bool skip_current_path, Stream& input, Stream& output)
{
  if (input.current().isLetter() || input == '_') {
    pp_macro_expander expand_include(this);

    KTextEditor::Cursor inputPosition = input.inputPosition();
    QString includeString;
    {
      Stream cs(&includeString);
      expand_include(input, cs);
    }

    skip_blanks(input, devnull());
    RETURN_ON_FAIL(!includeString.isEmpty() && (includeString.startsWith('<') || includeString.startsWith('"')));

    Stream newInput(&includeString, inputPosition);
    handle_include(skip_current_path, newInput, output);
    return;
  }

  RETURN_ON_FAIL(input == '<' || input == '"');
  QChar quote((input == '"') ? '"' : '>');
  ++input;

  QString includeName;

  while (!input.atEnd() && input != quote) {
    RETURN_ON_FAIL(input != '\n');

    includeName.append(input);
    ++input;
  }

  Stream* include = m_preprocessor->sourceNeeded(includeName, quote == '"' ? Preprocessor::IncludeLocal : Preprocessor::IncludeGlobal, input.inputPosition().line(), skip_current_path);
  Q_ASSERT(!include);

  /*if (include && !include->atEnd()) {
    m_files.push(includeName);

    output.mark(includeName, 0);

    operator()(*include, output);

    // restore the file name and sync the buffer
    output.mark(m_files.pop(), input.inputPosition().line());
  }*/

  delete include;
}

void pp::operator () (Stream& input, Stream& output)
{
#ifndef PP_NO_SMART_HEADER_PROTECTION
  QString headerDefine = find_header_protection(input);
  if (m_environment->retrieveMacro(headerDefine))
  {
    //kDebug(9007) << "found header protection:" << headerDefine;
    return;
  }
#endif

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

      QString directive = skip_identifier(input);

      Q_ASSERT(directive.length() < 512);

      skip_blanks(input, devnull());

      KTextEditor::Cursor inputPosition = input.inputPosition();
      
      QString skipped;
      {
        Stream ss(&skipped);
        skip (input, ss);
      }

      Stream ss(&skipped, inputPosition);
      handle_directive(directive, ss, output);

    } else if (input == '\n') {
      output << input;
      ++input;

    } else if (skipping ()) {
      skip (input, devnull());

    } else {
      output.mark(input.inputPosition());
      expand (input, output);
    }
  }

  if (iflevel != previousIfLevel) {
    KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 0), "Unterminated #if statement");
  }
}

void pp::handle_define (Stream& input)
{
  pp_macro* macro = new pp_macro();
  macro->file = currentFileName();
  QString definition;

  skip_blanks (input, devnull());
  macro->name = skip_identifier(input);

  if (!input.atEnd() && input == '(')
  {
    macro->function_like = true;

    skip_blanks (++input, devnull()); // skip '('
    QString formal = skip_identifier(input);
    if (!formal.isEmpty())
      macro->formals << formal;

    skip_blanks(input, devnull());

    if (input == '.') {
      macro->variadics = true;

      do {
        ++input;

      } while (input == '.');
    }

    while (!input.atEnd() && input == ',')
    {
      skip_blanks(++input, devnull());

      QString formal = skip_identifier(input);
      if (!formal.isEmpty())
        macro->formals << formal;

      skip_blanks (input, devnull());

      if (input == '.') {
        macro->variadics = true;

        do {
          ++input;

        } while (input == '.');
      }
    }

    RETURN_ON_FAIL(input == ')');
    ++input;
  }

  skip_blanks (input, devnull());

  while (!input.atEnd() && input != '\n')
  {
    if (input == '\\')
    {
      int pos = input.pos();
      skip_blanks (++input, devnull());

      if (!input.atEnd() && input == '\n')
      {
        skip_blanks(++input, devnull());
        definition += ' ';
        continue;

      } else {
        // Whoops, rewind :)
        input.seek(pos);
      }
    }

    definition += input;
    ++input;
  }

  macro->definition = definition;

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
        KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 1), i18n("expected ``identifier'' found: %1", char(token)));
        break;
      }

      {
        pp_macro* m = m_environment->retrieveMacro(token_text);
        result.set_long( (m && !m->isUndef()) ? 1 : 0);
      }

      token = next_token(input); // skip '('

      if (expect_paren) {
        if (token != ')')
          KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 0), i18n("expected ``)''"));
        else
          accept_token();
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

    case '(':
      result = eval_constant_expression(input);
      token = next_token(input);

      if (token != ')')
        KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 1), i18n("expected ``)'' = %1", char(token)));
      else
        accept_token();

      break;

    default:
      break;
  }

  return result;
}

Value pp::eval_multiplicative(Stream& input)
{
  Value result = eval_primary(input);

  int token = next_token(input);

  while (token == '*' || token == '/' || token == '%') {
    accept_token();

    Value value = eval_primary(input);

    if (token == '*') {
      result *= value;

    } else if (token == '/') {
      if (value.is_zero()) {
        KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 0), i18n("division by zero"));
        result.set_long(0);

      } else {
        result /= value;
      }

    } else {
      if (value.is_zero()) {
        KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 0), i18n("division by zero"));
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
      KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 1), i18n("expected ``:'' = %1", int(token)));
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

    KTextEditor::Cursor inputPosition = input.inputPosition();
    QString condition;
    {
      Stream cs(&condition);
      expand_condition(input, cs);
    }

    environment()->enterBlock(input.inputPosition().line(), condition);

    Stream cs(&condition, inputPosition);
    Value result = eval_expression(cs);

    _M_true_test[iflevel] = !result.is_zero();
    _M_skipping[iflevel] = result.is_zero();

  } else {
    // Capture info for precompiled macros
    pp_macro_expander expand_condition(this);
    skip_blanks(input, devnull());
    QString condition;
    {
      Stream cs(&condition);
      expand_condition(input, cs);
    }

    environment()->enterBlock(input.inputPosition().line(), condition);

    _M_true_test[iflevel] = true;
    _M_skipping[iflevel] = true;
  }
}


void pp::handle_else(int sourceLine)
{
  if (iflevel == 0 && !skipping ())
  {
    KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(KTextEditor::Cursor(sourceLine, 0), 0), i18n("#else without #if"));
  }
  else if (iflevel > 0 && _M_skipping[iflevel - 1])
  {
    _M_skipping[iflevel] = true;
    environment()->elseBlock(sourceLine);
  }
  else
  {
    _M_skipping[iflevel] = _M_true_test[iflevel];
    environment()->elseBlock(sourceLine);
  }
}


void pp::handle_elif(Stream& input)
{
  RETURN_ON_FAIL(iflevel > 0);

  if (iflevel == 0 && !skipping())
  {
    KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 0), i18n("#else without #if"));
  }
  else
  {
    pp_macro_expander expand_condition(this);
    skip_blanks(input, devnull());

    KTextEditor::Cursor inputPosition = input.inputPosition();
    QString condition;
    {
      Stream cs(&condition);
      expand_condition(input, cs);
    }

    environment()->elseBlock(input.inputPosition().line(), condition);

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
    KDevelop::DUChain::problemEncountered(currentFileName(), KTextEditor::Range(input.inputPosition(), 0), i18n("#endif without #if at output line %1", m_environment->locationTable()->positionForOffset(output.pos()).line()));
  }
  else
  {
    environment()->leaveBlock();

    _M_skipping[iflevel] = 0;
    _M_true_test[iflevel] = 0;

    --iflevel;
  }
}


void pp::handle_ifdef (bool check_undefined, Stream& input)
{
  QString macro_name = skip_identifier(input);

  environment()->enterBlock(input.inputPosition().line(), QString("%1defined(%2)").arg(check_undefined ? "!" : "").arg(macro_name));

  if (test_if_level())
  {
    pp_macro* macro = m_environment->retrieveMacro(macro_name);
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

  QString macro_name = skip_identifier(input);
  RETURN_ON_FAIL(!macro_name.isEmpty());

  pp_macro* macro = new pp_macro();
  macro->file = currentFileName();
  macro->name = macro_name;

  macro->defined = false;

  m_environment->setMacro(macro);
  
  //m_environment->clearMacro(macro_name);
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

  char ch = input.current().toLatin1();
  char ch2 = input.peek().toLatin1();

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
      if (QChar(ch).isLetter() || ch == '_')
      {
        token_text = skip_identifier (input);

        if (token_text == "defined")
          nextToken = TOKEN_DEFINED;
        else
          nextToken = TOKEN_IDENTIFIER;
      }
      else if (QChar(ch).isNumber())
      {
        QString number;
        {
          Stream ns(&number);
          skip_number(input, ns);
        }

        if (number.endsWith('u', Qt::CaseInsensitive)) {
          token_uvalue = number.toULong();
          nextToken = TOKEN_UNUMBER;

        } else {
          token_value = number.toLong();
          nextToken = TOKEN_NUMBER;
        }
      }
      else
      {
        nextToken = input.current().toLatin1();
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

QString pp::currentFile() const
{
  if (m_files.count())
    return currentFileName();

  Q_ASSERT(false);
  return "<internal>";
}

