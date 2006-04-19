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

#include <kdebug.h>

#include "pp-internal.h"
#include "preprocessor.h"

pp::pp(Preprocessor* preprocessor, QHash<QString, pp_macro*>& environment)
  : m_environment(environment)
  , expand(environment)
  , m_preprocessor(preprocessor)
  , lines(0)
  , nextToken(0)
  , haveNextToken(false)
{
  iflevel = 0;
  _M_skipping[iflevel] = 0;
  _M_true_test[iflevel] = 0;
}

QString pp::processFile(const QString& filename)
{
  QFile file(filename);
  if (file.open(QIODevice::ReadOnly))
  {
    m_files.push(filename);

    Stream is(&file);
    QString result;

    {
      Stream rs(&result);
      operator () (is, rs);
    }

    return result;
  }

  kWarning() << k_funcinfo << "file '" << filename << "' not found!" << endl;
  return QString();
}

QString pp::processFile(QIODevice* device)
{
   Q_ASSERT(device);

  QString result;
  m_files.push("<internal>");

  {
    Stream is(device);
    Stream rs(&result);
    operator () (is, rs);
  }

  return result;
}

QString pp::find_header_protection(Stream& input)
{
  int was = lines;
  qint64 inputOffset = input.pos();

  while (!input.atEnd())
  {
    if (input.current().isSpace())
    {
      if (input == '\n')
        ++lines;

      ++input;
    }
    else if (PPInternal::isComment(input))
    {
      skip_comment_or_divop (input, PPInternal::devnull());
      lines += skip_comment_or_divop.linesSkipped();
    }
    else if (input == '#')
    {
      skip_blanks (++input, PPInternal::devnull());
      lines += skip_blanks.linesSkipped();

      if (!input.atEnd() && input == 'i')
      {
        QString directive = skip_identifier(input);
        lines += skip_identifier.linesSkipped();

        if (directive == "ifndef")
        {
          skip_blanks (input, PPInternal::devnull());
          lines += skip_blanks.linesSkipped();

          QString define = skip_identifier(input);
          lines += skip_identifier.linesSkipped();

          if (!define.isEmpty() && !input.atEnd())
          {
            input.seek(inputOffset);
            return define;
          }
        }
      }
      break;

    } else {
      break;
    }
  }

  lines = was;
  input.seek(inputOffset);
  return QString();
}

pp::PP_DIRECTIVE_TYPE pp::find_directive (const QString& directive) const
{
  static QHash<QString, PP_DIRECTIVE_TYPE> directiveHash;
  if (directiveHash.isEmpty()) {
    directiveHash.insert("if", PP_IF);
    directiveHash.insert("elif", PP_ELIF);
    directiveHash.insert("else", PP_ELSE);
    directiveHash.insert("ifdef", PP_IFDEF);
    directiveHash.insert("undef", PP_UNDEF);
    directiveHash.insert("endif", PP_ENDIF);
    directiveHash.insert("ifndef", PP_IFNDEF);
    directiveHash.insert("define", PP_DEFINE);
    directiveHash.insert("include", PP_INCLUDE);
  }

  if (directiveHash.contains(directive))
    return directiveHash[directive];

  return PP_UNKNOWN_DIRECTIVE;
}

void pp::handle_directive(const QString& directive, Stream& input, Stream& output)
{
  skip_blanks (input, output);

  switch (find_directive(directive))
  {
    case PP_DEFINE:
      if (! skipping ())
        return handle_define(input);
      break;

    case PP_INCLUDE:
      if (! skipping ())
        return handle_include (input, output);
      break;

    case PP_UNDEF:
      if (! skipping ())
        return handle_undef(input);
      break;

    case PP_ELIF:
      return handle_elif(input);

    case PP_ELSE:
      return handle_else();

    case PP_ENDIF:
      return handle_endif();

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

QString pp::createLineMark(const QString& filename, int line)
{
  return QString("# %1 \"%2\"\n").arg(line).arg(filename.isEmpty() ? QString("<internal>") : filename);
}

void pp::handle_include(Stream& input, Stream& output)
{
  Q_ASSERT(input == '<' || input == '"');
  QChar quote((input == '"') ? '"' : '>');
  ++input;

  QString includeName;

  while (!input.atEnd() && input != quote) {
    Q_ASSERT(input != '\n');

    includeName.append(input);
    ++input;
  }

  Stream* include = m_preprocessor->sourceNeeded(includeName, quote == '"' ? Preprocessor::IncludeLocal : Preprocessor::IncludeGlobal);
  if (include && !include->atEnd()) {
    m_files.push(includeName);
    m_includeLineNumbers.push(lines);

    lines = 0;
    output << createLineMark(includeName, 0);

    operator()(*include, output);

    // restore the file name and the line position and
    // sync the buffer
    m_files.pop();
    output << createLineMark(m_files.top(), m_includeLineNumbers.pop());
  }
}

void pp::operator () (Stream& input, Stream& output)
{
#ifndef PP_NO_SMART_HEADER_PROTECTION
  QString headerDefine = find_header_protection(input);
  if (m_environment.contains(headerDefine))
  {
    kDebug() << k_funcinfo << "found header protection: " << headerDefine << endl;
    return;
  }
#endif

  forever
  {
    skip_blanks(input, skipping() ? PPInternal::devnull() : output);
    lines += skip_blanks.linesSkipped();

    if (input.atEnd()) {
      break;

    } else if (input == '#') {
      skip_blanks(++input, PPInternal::devnull());
      lines += skip_blanks.linesSkipped();

      QString directive = skip_identifier(input);
      lines += skip_identifier.linesSkipped();

      Q_ASSERT(directive.length() < 512);

      skip_blanks(input, PPInternal::devnull());

      QString skipped;
      {
        Stream ss(&skipped);
        skip (input, ss);
      }

      int was = lines;
      Stream ss(&skipped);
      handle_directive(directive, ss, output);

      if (lines != was)
      {
        lines = was;
        output << createLineMark(m_files.top(), lines);
      }

    } else if (input == '\n') {
      // ### compress the line
      // Is that really necessary? seems like it would just make the lookup slower later
      output << input;
      ++input;
      ++lines;

    } else if (skipping ()) {
      skip (input, PPInternal::devnull());

    } else {
      output << createLineMark(m_files.top(), lines);
      expand (input, output);
      lines += expand.linesSkipped();

      if (expand.generatedLines())
        output << createLineMark(m_files.top(), lines);
    }
  }
}


void pp::handle_define (Stream& input)
{
  pp_macro* macro = new pp_macro();
#if defined (PP_WITH_MACRO_POSITION)
  macro->file = m_files.top();
#endif
  QString definition;

  skip_blanks (input, PPInternal::devnull());
  QString macro_name = skip_identifier(input);

  if (!input.atEnd() && input == '(')
  {
    macro->function_like = true;

    skip_blanks (++input, PPInternal::devnull()); // skip '('
    QString formal = skip_identifier(input);
    if (!formal.isEmpty())
      macro->formals << formal;

    skip_blanks(input, PPInternal::devnull());

    if (input == '.') {
      macro->variadics = true;

      do {
        ++input;

      } while (input == '.');
    }

    while (!input.atEnd() && input == ',')
    {
      skip_blanks(++input, PPInternal::devnull());

      QString formal = skip_identifier(input);
      if (!formal.isEmpty())
        macro->formals << formal;

      skip_blanks (input, PPInternal::devnull());

      if (input == '.') {
        macro->variadics = true;

        do {
          ++input;

        } while (input == '.');
      }
    }

    Q_ASSERT(input == ')');
    ++input;
  }

  skip_blanks (input, PPInternal::devnull());

  while (!input.atEnd() && input != '\n')
  {
    if (input == '\\')
    {
      qint64 pos = input.pos();
      skip_blanks (++input, PPInternal::devnull());

      if (!input.atEnd() && input == '\n')
      {
        ++macro->lines;
        skip_blanks(++input, PPInternal::devnull());
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

  m_environment.insert(macro_name, macro);
}


void pp::skip (Stream& input, Stream& output)
{
  pp_skip_string_literal skip_string_literal;
  pp_skip_char_literal skip_char_literal;

  while (!input.atEnd() && input != '\n')
  {
    if (input == '/')
    {
      skip_comment_or_divop (input, output);
      lines += skip_comment_or_divop.linesSkipped();
    }
    else if (input == '"')
    {
      skip_string_literal (input, output);
      lines += skip_string_literal.linesSkipped();
    }
    else if (input == '\'')
    {
      skip_char_literal (input, output);
      lines += skip_char_literal.linesSkipped();
    }
    else if (input == '\\')
    {
      output << input;
      skip_blanks (++input, output);
      lines += skip_blanks.linesSkipped();

      if (!input.atEnd() && input == '\n')
      {
        output << input;
        ++input;
        ++lines;
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


long pp::eval_primary(Stream& input)
{
  bool expect_paren = false;
  int token = next_token_accept(input);
  long result = 0;

  switch (token) {
    case TOKEN_NUMBER:
      return token_value;

    case TOKEN_DEFINED:
      token = next_token_accept(input);

      if (token == '(')
      {
        expect_paren = true;
        token = next_token_accept(input);
      }

      if (token != TOKEN_IDENTIFIER)
      {
        kWarning() << k_funcinfo << "expected ``identifier'' found:" << token << endl;
        break;
      }

      result = m_environment.contains(token_text);

      token = next_token(input); // skip '('

      if (expect_paren) {
        token = next_token(input);

        if (token != ')')
          kWarning() << k_funcinfo << "expected ``)''" << endl;
        else
          accept_token();
      }
      break;

    case TOKEN_IDENTIFIER:
      break;

    case '!':
      return !eval_primary(input);

    case '(':
      result = eval_constant_expression(input);
      token = next_token(input);

      if (token != ')')
        kWarning() << k_funcinfo << "expected ``)'' = " << token << endl;
      else
        accept_token();

      break;

    default:
      break;
  }

  return result;
}

long pp::eval_multiplicative(Stream& input)
{
  long result = eval_primary(input);

  int token = next_token(input);

  while (token == '*' || token == '/' || token == '%') {
    accept_token();

    long value = eval_primary(input);

    if (token == '*') {
      result *= value;

    } else if (token == '/') {
      if (value == 0) {
        kWarning() << k_funcinfo << "division by zero" << endl;
        result = 0;

      } else {
        result = result / value;
      }

    } else {
      if (value == 0) {
        kWarning() << k_funcinfo << "division by zero" << endl;
        result = 0;

      } else {
        result %= value;
      }
    }

    token = next_token(input);
  }

  return result;
}

long pp::eval_additive(Stream& input)
{
  long result = eval_multiplicative(input);

  int token = next_token(input);

  while (token == '+' || token == '-') {
    accept_token();

    long value = eval_multiplicative(input);

    if (token == '+')
      result += value;
    else
      result -= value;

    token = next_token(input);
  }

  return result;
}


long pp::eval_shift(Stream& input)
{
  long result = eval_additive(input);

  int token;
  token = next_token(input);

  while (token == TOKEN_LT_LT || token == TOKEN_GT_GT) {
    accept_token();

    long value = eval_additive(input);

    if (token == TOKEN_LT_LT)
      result <<= value;
    else
      result >>= value;

    token = next_token(input);
  }

  return result;
}


long pp::eval_relational(Stream& input)
{
  long result = eval_shift(input);

  int token = next_token(input);

  while (token == '<'
      || token == '>'
      || token == TOKEN_LT_EQ
      || token == TOKEN_GT_EQ)
  {
    accept_token();
    long value = eval_shift(input);

    switch (token)
    {
      default:
        Q_ASSERT(0);
        break;

      case '<':
        result = result < value;
        break;

      case '>':
        result = result < value;
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


long pp::eval_equality(Stream& input)
{
  long result = eval_relational(input);

  int token = next_token(input);

  while (token == TOKEN_EQ_EQ || token == TOKEN_NOT_EQ) {
    accept_token();
    long value = eval_relational(input);

    if (token == TOKEN_EQ_EQ)
      result = result == value;
    else
      result = result != value;

    token = next_token(input);
  }

  return result;
}


long pp::eval_and(Stream& input)
{
  long result = eval_equality(input);

  int token = next_token(input);

  while (token == '&') {
    accept_token();
    long value = eval_equality(input);
    result = result & value;
    token = next_token(input);
  }

  return result;
}


long pp::eval_xor(Stream& input)
{
  long result = eval_and(input);

  int token;
  token = next_token(input);

  while (token == '^') {
    accept_token();
    long value = eval_and(input);
    result = result ^ value;
    token = next_token(input);
  }

  return result;
}


long pp::eval_or(Stream& input)
{
  long result = eval_xor(input);

  int token = next_token(input);

  while (token == '|') {
    accept_token();
    long value = eval_xor(input);
    result = result | value;
    token = next_token(input);
  }

  return result;
}


long pp::eval_logical_and(Stream& input)
{
  long result = eval_or(input);

  int token = next_token(input);

  while (token == TOKEN_AND_AND) {
    accept_token();
    long value = eval_or(input);
    result = result && value;
    token = next_token(input);
  }

  return result;
}


long pp::eval_logical_or(Stream& input)
{
  long result = eval_logical_and(input);

  int token = next_token(input);

  while (token == TOKEN_OR_OR) {
    accept_token();
    long value = eval_logical_and(input);
    result = result || value;
    token = next_token(input);
  }

  return result;
}


long pp::eval_constant_expression(Stream& input)
{
  long result = eval_logical_or(input);

  int token = next_token(input);

  if (token == '?')
  {
    accept_token();
    long left_value = eval_constant_expression(input);
    skip_blanks(input, PPInternal::devnull());

    token = next_token_accept(input);
    if (token == ':')
    {
      long right_value = eval_constant_expression(input);

      result = result ? left_value : right_value;
    }
    else
    {
      kWarning() << k_funcinfo << "expected ``:'' = " << int (token) << endl;
      result = left_value;
    }
  }

  return result;
}


long pp::eval_expression(Stream& input)
{
  skip_blanks(input, PPInternal::devnull());
  return eval_constant_expression(input);
}


void pp::handle_if (Stream& input)
{
  if (test_if_level())
  {
    pp_macro_expander expand_condition(m_environment);
    skip_blanks(input, PPInternal::devnull());
    QString condition;
    {
      Stream cs(&condition);
      expand_condition(input, cs);
    }

    Stream cs(&condition);
    long result = eval_expression(cs);

    _M_true_test[iflevel] = result;
    _M_skipping[iflevel] = !result;
  }
}


void pp::handle_else()
{
  if (iflevel == 0 && !skipping ())
  {
    kWarning() << k_funcinfo << "#else without #if" << endl;
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
  Q_ASSERT(iflevel > 0);

  if (iflevel == 0 && !skipping())
  {
    kWarning() << k_funcinfo << "#else without #if" << endl;
  }
  else if (!_M_true_test[iflevel] && !_M_skipping[iflevel - 1])
  {
    long result = eval_expression(input);
    _M_true_test[iflevel] = result;
    _M_skipping[iflevel] = !result;
  }
  else
  {
    _M_skipping[iflevel] = true;
  }
}


void pp::handle_endif()
{
  if (iflevel == 0 && !skipping())
  {
    kWarning() << k_funcinfo << "#endif without #if" << endl;
  }
  else
  {
    _M_skipping[iflevel] = 0;
    _M_true_test[iflevel] = 0;

    --iflevel;
  }
}


void pp::handle_ifdef (bool check_undefined, Stream& input)
{
  if (test_if_level())
  {
    QString macro_name = skip_identifier(input);
    bool value = m_environment.contains(macro_name);

    if (check_undefined)
      value = !value;

    _M_true_test[iflevel] = value;
    _M_skipping[iflevel] = !value;
  }
}


void pp::handle_undef(Stream& input)
{
  skip_blanks (input, PPInternal::devnull());
  QString macro_name = skip_identifier(input);
  Q_ASSERT(!macro_name.isEmpty());

  m_environment.remove(macro_name);
}

int pp::next_token (Stream& input)
{
  if (haveNextToken)
    return nextToken;

  skip_blanks(input, PPInternal::devnull());

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
        skip_comment_or_divop(input, PPInternal::devnull());
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
        token_value = number.toLong();

        nextToken = TOKEN_NUMBER;
      }
      else
      {
        nextToken = input.current().toLatin1();
        ++input;
      }
  }

  //kDebug() << "Next token '" << ch << ch2 << "' " << nextToken << " txt " << token_text << " val " << token_value << endl;

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

// kate: indent-width 2;
