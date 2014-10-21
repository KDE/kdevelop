/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "lexer.h"
#include "tokens.h"
#include "control.h"
#include "parsesession.h"
#include "rpp/pp-scanner.h"

#include <cctype>
#include <util/kdevvarlengtharray.h>

#include <klocalizedstring.h>

void TokenStream::splitRightShift(uint index)
{
  Q_ASSERT(kind(index) == Token_rightshift);

  // change kind of current token and adapt size
  Token &current_token = (*this)[index];
  Q_ASSERT(current_token.size == 2);
  current_token.size = 1;
  current_token.kind = '>';

  // copy to next token (i.e. the new one) and adapt position
  Token next_token = current_token;
  next_token.position += 1;

  insert(index+1, next_token);
}

KDevelop::IndexedString TokenStream::symbol(const Token& t) const
{
  if(t.size == 1)
    return KDevelop::IndexedString::fromIndex(session->contents()[t.position]);
  else
    return KDevelop::IndexedString();
}

uint TokenStream::symbolIndex(const Token& t) const
{
  return session->contents()[t.position];
}

QByteArray TokenStream::symbolByteArray(const Token& t) const
{
  if (t.size == 0) // esp. for EOF
    return QByteArray();

  return stringFromContents(session->contentsVector(), t.position, t.size);
}

QString TokenStream::symbolString(const Token& t) const
{
  return QString::fromUtf8(symbolByteArray(t));
}

uint TokenStream::symbolLength(const Token& t) const
{
  uint ret = 0;
  for(uint a = t.position; a < t.position+t.size; ++a) {
    ret += KDevelop::IndexedString::lengthFromIndex(session->contents()[a]);
  }
  return ret;
}

QString Lexer::SpecialCursor::toString() const
{
  return KDevelop::IndexedString::fromIndex(*current).str();
}

/**
 * Returns the character BEHIND the found comment
 * */
void Lexer::skipComment()
{
  ///A nearly exact copy of rpp::pp_skip_comment_or_divop::operator()
  enum {
    MAYBE_BEGIN,
    BEGIN,
    MAYBE_END,
    END,
    IN_COMMENT,
    IN_CXX_COMMENT
  } state (MAYBE_BEGIN);

  while (cursor < endCursor && *cursor) {
    switch (state) {
      case MAYBE_BEGIN:
        if (*cursor != '/')
          return;

        state = BEGIN;
        break;

      case BEGIN:
        if (*cursor == '*')
          state = IN_COMMENT;
        else if (*cursor == '/')
          state = IN_CXX_COMMENT;
        else
          return;
        break;

      case IN_COMMENT:
        if( *cursor == '\n' ) {
          scan_newline();
          continue;
        }
        if (*cursor == '*')
          state = MAYBE_END;
        break;

      case IN_CXX_COMMENT:
        if (*cursor == '\n')
          return;
        break;

      case MAYBE_END:
        if (*cursor == '/')
          state = END;
        else if (*cursor != '*')
          state = IN_COMMENT;
        if( *cursor == '\n' ) {
          scan_newline();
          continue;
        }
        break;

      case END:
        return;
    }

    ++cursor;
  }
  return;
}

const uint index_size = 200;

KDevVarLengthArray<KDevVarLengthArray<QPair<uint, TOKEN_KIND>, 10 >, index_size > createIndicesForTokens() {
  KDevVarLengthArray<KDevVarLengthArray<QPair<uint, TOKEN_KIND>, 10 >, index_size > ret;
  ret.resize(index_size);
  #define ADD_TOKEN(string) ret[KDevelop::IndexedString(#string).index() % index_size].append(qMakePair(KDevelop::IndexedString(#string).index(), Token_ ## string));
  #define ADD_TOKEN2(string, tok) ret[KDevelop::IndexedString(#string).index() % index_size].append(qMakePair(KDevelop::IndexedString(#string).index(), Token_ ## tok));
  ADD_TOKEN(K_DCOP);
  ADD_TOKEN(Q_OBJECT);
  ADD_TOKEN(__typeof);
  ADD_TOKEN2(__typeof__, __typeof);
  ADD_TOKEN2(typeof, __typeof);
  ADD_TOKEN(and);
  ADD_TOKEN(and_eq);
  ADD_TOKEN(asm);
  ADD_TOKEN2(__asm, asm);
  ADD_TOKEN2(__asm__, asm)
  ADD_TOKEN(auto);
  ADD_TOKEN(bitand);
  ADD_TOKEN(bitor);
  ADD_TOKEN(bool);
  ADD_TOKEN(break);
  ADD_TOKEN(case);
  ADD_TOKEN(catch);
  ADD_TOKEN(char);
  ADD_TOKEN(char16_t);
  ADD_TOKEN(char32_t);
  ADD_TOKEN(class);
  ADD_TOKEN(compl);
  ADD_TOKEN(const);
  ADD_TOKEN(constexpr);
  ADD_TOKEN(const_cast);
  ADD_TOKEN(continue);
  ADD_TOKEN(decltype);
  ADD_TOKEN2(__decltype, decltype);
  ADD_TOKEN(default);
  ADD_TOKEN(delete);
  ADD_TOKEN(do);
  ADD_TOKEN(double);
  ADD_TOKEN(dynamic_cast);
  ADD_TOKEN(else);
  //ADD_TOKEN(emit);
  ADD_TOKEN(enum);
  ADD_TOKEN(explicit);
  ADD_TOKEN(export);
  ADD_TOKEN(extern);
  ADD_TOKEN(false);
  ADD_TOKEN(float);
  ADD_TOKEN(final);
  ADD_TOKEN(for);
  ADD_TOKEN(friend);
  ADD_TOKEN(goto);
  ADD_TOKEN(if);
  ADD_TOKEN(inline);
  ADD_TOKEN2(__inline__, inline);
  ADD_TOKEN2(__inline, inline);
  ADD_TOKEN(int);
  ADD_TOKEN(k_dcop);
  ADD_TOKEN(k_dcop_signals);
  ADD_TOKEN(long);
  ADD_TOKEN(mutable);
  ADD_TOKEN(namespace);
  ADD_TOKEN(new);
  ADD_TOKEN(noexcept);
  ADD_TOKEN(not);
  ADD_TOKEN(not_eq);
  ADD_TOKEN(nullptr);
  ADD_TOKEN(operator);
  ADD_TOKEN(or);
  ADD_TOKEN(or_eq);
  ADD_TOKEN(override);
  ADD_TOKEN(private);
  ADD_TOKEN(protected);
  ADD_TOKEN(public);
  ADD_TOKEN(register);
  ADD_TOKEN(reinterpret_cast);
  ADD_TOKEN(return);
  ADD_TOKEN(short);
  ADD_TOKEN(__qt_signals__);
  ADD_TOKEN(signed);
  ADD_TOKEN2(__signed__, signed);
  ADD_TOKEN(sizeof);
  ADD_TOKEN(__qt_slots__);
  ADD_TOKEN(static);
  ADD_TOKEN(static_assert);
  ADD_TOKEN(static_cast);
  ADD_TOKEN(struct);
  ADD_TOKEN(switch);
  ADD_TOKEN(template);
  ADD_TOKEN(this);
  ADD_TOKEN(thread_local);
  ADD_TOKEN2(__thread, thread_local);
  ADD_TOKEN2(__thread__, thread_local);
  ADD_TOKEN(throw);
  ADD_TOKEN(true);
  ADD_TOKEN(try);
  ADD_TOKEN(typedef);
  ADD_TOKEN(typeid);
  ADD_TOKEN(typename);
  ADD_TOKEN(union);
  ADD_TOKEN(unsigned);
  ADD_TOKEN2(__unsigned__, unsigned);
  ADD_TOKEN(using);
  ADD_TOKEN(virtual);
  ADD_TOKEN(void);
  ADD_TOKEN(volatile);
  ADD_TOKEN2(__volatile__, volatile);
  ADD_TOKEN(wchar_t);
  ADD_TOKEN(while);
  ADD_TOKEN(xor);
  ADD_TOKEN(xor_eq);
  ADD_TOKEN(__qt_signal__);
  ADD_TOKEN(__qt_slot__);
  ADD_TOKEN(__qt_property__);
  return ret;
}

scan_fun_ptr Lexer::s_scan_table[256];
bool Lexer::s_initialized = false;

Lexer::Lexer(Control *c)
  : session(0),
    control(c),
    m_leaveSize(false)
{
}

void Lexer::tokenize(ParseSession* _session)
{
  session = _session;
  TokenStream* stream = session->token_stream;
  Q_ASSERT(stream->isEmpty());

  if (!s_initialized)
    initialize_scan_table();

  m_canMergeComment = false;
  m_firstInLine = true;
  m_leaveSize = false;

  {
  Token eof;
  eof.kind = Token_EOF;
  eof.position = 0;
  eof.size = 0;
  stream->append(eof);
  index = 1;
  }

  cursor.current = session->contents();
  endCursor = session->contents() + session->contentsVector().size();
  while(endCursor-1 >= session->contents() && (*(endCursor-1)) == 0)
    --endCursor;

  while (cursor < endCursor) {
    Q_ASSERT(static_cast<uint>(stream->size()) == index);

    size_t previousIndex = index;

    {
    Token token{cursor.offsetIn(session->contents()), 0, Token_EOF};
    stream->append(token);
    }
    Token* current_token = &(session->token_stream->last());

    if(cursor.isChar()) {
      (this->*s_scan_table[((uchar)*cursor)])();
    }else{
      //check for utf8 strings
      static const uint u8Index = KDevelop::IndexedString("u8").index();
      //check for raw strings
      static const uint u8RIndex = KDevelop::IndexedString("u8R").index();
      static const uint uRIndex = KDevelop::IndexedString("uR").index();
      static const uint URIndex = KDevelop::IndexedString("UR").index();
      static const uint LRIndex = KDevelop::IndexedString("LR").index();

      if (*cursor.current == u8Index) {
        // check for utf8 string
        // not calling scan_identifier_or_literal as u8
        // is only supported for strings, not characters
        if (*(cursor+1) == '"') {
          ++cursor;
          scan_string_constant();
        } else {
          scan_identifier_or_keyword();
        }
      } else if (*cursor.current == u8RIndex || *cursor.current == uRIndex
                  || *cursor.current == URIndex || *cursor.current == LRIndex)
      {
        // probably raw string
        scan_raw_string_or_identifier();
      } else {
          //The cursor represents an identifier
          scan_identifier_or_keyword();
      }
    }


    if(!m_leaveSize)
      current_token->size = cursor.offsetIn( session->contents() ) - current_token->position;

    Q_ASSERT(m_leaveSize || (cursor.current == session->contents() + current_token->position + current_token->size));
    Q_ASSERT(current_token->position + current_token->size <= (uint)session->contentsVector().size());
    Q_ASSERT(previousIndex == index-1 || previousIndex == index); //Never parse more than 1 token, because that won't be initialized correctly

    m_leaveSize = false;

    if(previousIndex != index)
      m_firstInLine = false;
    else // skipped index, remove last appended token again
      stream->pop_back();
  }

  {
  Token eof;
  eof.kind = Token_EOF;
  eof.position = cursor.offsetIn( session->contents() );
  eof.size = 0;
  stream->append(eof);
  }

  stream->squeeze();
}

void Lexer::initialize_scan_table()
{
  s_initialized = true;

  for (int i=0; i<256; ++i)
    {
      if (isspace(i))
	s_scan_table[i] = &Lexer::scan_white_spaces;
      else if (isalpha(i) || i == '_')
	s_scan_table[i] = &Lexer::scan_identifier_or_keyword;
      else if (isdigit(i))
	s_scan_table[i] = &Lexer::scan_int_constant;
      else
	s_scan_table[i] = &Lexer::scan_invalid_input;
    }

  s_scan_table[int('L')] = &Lexer::scan_identifier_or_literal;
  s_scan_table[int('u')] = &Lexer::scan_identifier_or_literal;
  s_scan_table[int('U')] = &Lexer::scan_identifier_or_literal;
  s_scan_table[int('R')] = &Lexer::scan_raw_string_or_identifier;
  s_scan_table[int('\n')] = &Lexer::scan_newline;
  s_scan_table[int('#')] = &Lexer::scan_preprocessor;

  s_scan_table[int('\'')] = &Lexer::scan_char_constant;
  s_scan_table[int('"')]  = &Lexer::scan_string_constant;

  s_scan_table[int('.')] = &Lexer::scan_int_constant;

  s_scan_table[int('!')] = &Lexer::scan_not;
  s_scan_table[int('%')] = &Lexer::scan_remainder;
  s_scan_table[int('&')] = &Lexer::scan_and;
  s_scan_table[int('(')] = &Lexer::scan_left_paren;
  s_scan_table[int(')')] = &Lexer::scan_right_paren;
  s_scan_table[int('*')] = &Lexer::scan_star;
  s_scan_table[int('+')] = &Lexer::scan_plus;
  s_scan_table[int(',')] = &Lexer::scan_comma;
  s_scan_table[int('-')] = &Lexer::scan_minus;
  s_scan_table[int('/')] = &Lexer::scan_divide;
  s_scan_table[int(':')] = &Lexer::scan_colon;
  s_scan_table[int(';')] = &Lexer::scan_semicolon;
  s_scan_table[int('<')] = &Lexer::scan_less;
  s_scan_table[int('=')] = &Lexer::scan_equal;
  s_scan_table[int('>')] = &Lexer::scan_greater;
  s_scan_table[int('?')] = &Lexer::scan_question;
  s_scan_table[int('[')] = &Lexer::scan_left_bracket;
  s_scan_table[int(']')] = &Lexer::scan_right_bracket;
  s_scan_table[int('^')] = &Lexer::scan_xor;
  s_scan_table[int('{')] = &Lexer::scan_left_brace;
  s_scan_table[int('|')] = &Lexer::scan_or;
  s_scan_table[int('}')] = &Lexer::scan_right_brace;
  s_scan_table[int('~')] = &Lexer::scan_tilde;

  s_scan_table[0] = &Lexer::scan_EOF;
}

void Lexer::scan_preprocessor()
{
  while (cursor < endCursor && *cursor && *cursor != '\n')
    ++cursor;

  if (*cursor != '\n')
    {
      KDevelop::ProblemPointer p = createProblem();
      p->setDescription("expected end of line");
      control->reportProblem(p);
    }
}

void Lexer::scan_char_constant()
{
  //const char *begin = cursor;

  ++cursor;
  while (cursor < endCursor && *cursor && *cursor != '\'')
    {
      if (*cursor == '\n')
        {
          KDevelop::ProblemPointer p = createProblem();
          p->setDescription("unexpected new line");
          control->reportProblem(p);
          break;
        }

      if (*cursor == '\\' && (cursor.current + 1) < endCursor)
        ++cursor;

      ++cursor;
    }

  if (*cursor != '\'')
    {
      KDevelop::ProblemPointer p = createProblem();
      p->setDescription("expected '");
      control->reportProblem(p);
    }
  else
    {
      ++cursor;
    }

  (*session->token_stream)[index++].kind = Token_char_literal;
}

void Lexer::scan_string_constant()
{
  //const char *begin = cursor;

  ++cursor;
  while (cursor < endCursor && *cursor && *cursor != '"')
    {
       if (*cursor == '\n')
        {
          KDevelop::ProblemPointer p = createProblem();
          p->setDescription("unexpected new line");
          control->reportProblem(p);
          break;
        }

      if (*cursor == '\\' && (cursor.current + 1) < endCursor)
        ++cursor;

      ++cursor;
    }

  if (*cursor != '"')
    {
      KDevelop::ProblemPointer p = createProblem();
      p->setDescription("expected \"");
      control->reportProblem(p);
    }
  else
    {
      ++cursor;
    }

  (*session->token_stream)[index++].kind = Token_string_literal;
}

void Lexer::scan_raw_string_constant()
{
  Q_ASSERT(*cursor == '"');
  ++cursor;

  (*session->token_stream)[index++].kind = Token_string_literal;

  // find delimiter
  KDevVarLengthArray<uint, 16> delim;
  // NOTE: actually the spec says the delim should not be longer
  //       than 16 *chars* but due to string concatenation
  //       we don't really care about that and only look for
  //       max. 16 - chars or strings - until we find a proper '(' char
  int length = 0;
  while (cursor < endCursor && *cursor && *cursor != '(' && length < 16)
    {
      delim.append(*cursor.current);
      ++cursor;
      ++length;
    }

  if (*cursor != '(')
    {
      KDevelop::ProblemPointer p = createProblem();
      p->setDescription("expected R\"delim(");
      control->reportProblem(p);
      return;
    }
  ++cursor;

  // parse raw string
  bool delimFound = false;
  while (cursor < endCursor && *cursor)
    {
      if (*cursor == ')') {
        ++cursor;
        // check for end delimiter
        int i = 0;
        while(i < delim.size() && cursor < endCursor && *cursor && *cursor.current == delim[i]) {
          ++cursor;
          ++i;
        }
        if (i == delim.size() && cursor < endCursor && *cursor == '"') {
          ++cursor;
          delimFound = true;
          break;
        }
      } else {
        ++cursor;
      }
    }

  if (!delimFound)
    {
      KDevelop::ProblemPointer p = createProblem();
      p->setDescription("expected )delim\"");
      control->reportProblem(p);
    }
}

void Lexer::scan_newline()
{
  ++cursor;
  m_firstInLine = true;
}

void Lexer::scan_white_spaces()
{
  while (cursor < endCursor && isspace(*cursor))
    {
      if (*cursor == '\n')
	scan_newline();
      else
	++cursor;
    }
}

void Lexer::scan_identifier_or_literal()
{
  switch (*(cursor + 1))
    {
    case '\'':
      ++cursor;
      scan_char_constant();
      break;

    case '\"':
      ++cursor;
      scan_string_constant();
      break;

    default:
      scan_identifier_or_keyword();
      break;
    }
}

void Lexer::scan_raw_string_or_identifier()
{
  if (*(cursor+1) == '"')
    {
      ++cursor;
      scan_raw_string_constant();
    }
  else
    {
      scan_identifier_or_keyword();
    }
}

void Lexer::scan_identifier_or_keyword()
{
  if(!(cursor < endCursor))
    return;

  //We have to merge symbols tokenized separately, they may have been contracted using ##
  SpecialCursor nextCursor(cursor);
  ++nextCursor;

  while(nextCursor < endCursor && (!isCharacter(*(nextCursor.current)) || isLetterOrNumber(*nextCursor.current) || characterFromIndex(*nextCursor.current) == '_')) {
    //Fortunately this shouldn't happen too often, only when ## is used within the preprocessor
    KDevelop::IndexedString mergedSymbol(KDevelop::IndexedString::fromIndex(*(cursor.current)).byteArray() + KDevelop::IndexedString::fromIndex(*(nextCursor.current)).byteArray());

    (*cursor.current) = mergedSymbol.index();
    (*nextCursor.current) = 0;
    ++nextCursor;
  }

  uint bucket = (*cursor.current) % index_size;

  //A very simple lookup table: First level contains all pairs grouped by with (index % index_size), then there is a simple list
  static const KDevVarLengthArray<KDevVarLengthArray<QPair<uint, TOKEN_KIND>, 10 >, index_size > indicesForTokens = createIndicesForTokens();
  for(int a = 0; a < indicesForTokens[bucket].size(); ++a) {
    if(indicesForTokens[bucket][a].first == *cursor.current) {
      (*session->token_stream)[index++].kind = indicesForTokens[bucket][a].second;
      ++cursor;
      return;
    }
  }

  if(*cursor.current != 0) // If the index is zero, then the string is empty. Never create empty identifier tokens.
  {
    m_leaveSize = true; //Since we may have skipped input tokens while mergin, we have to make sure that the size stays 1(the merged tokens will be empty)
    (*session->token_stream)[index].size = 1;
    (*session->token_stream)[index++].kind = Token_identifier;
  }

  cursor = nextCursor;
}

void Lexer::scan_int_constant()
{
  if (*cursor == '.' && !std::isdigit(*(cursor + 1)))
    {
      scan_dot();
      return;
    }

  //const char *begin = cursor;

  while (cursor < endCursor &&  (isalnum(*cursor) || *cursor == '.'))
    ++cursor;

  (*session->token_stream)[index++].kind = Token_number_literal;
}

void Lexer::scan_not()
{
  /*
    '!'		::= not
    '!='		::= not_equal
  */

  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_not_eq;
    }
  else
    {
      (*session->token_stream)[index++].kind = '!';
    }
}

void Lexer::scan_remainder()
{
  /*
    '%'		::= remainder
    '%='		::= remainder_equal
  */

  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_remainder_eq;
    }
  else
    {
      (*session->token_stream)[index++].kind = '%';
    }
}

void Lexer::scan_and()
{
  /*
    '&&'		::= and_and
    '&'		::= and
    '&='		::= and_equal
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_and_eq;
    }
  else if (*cursor == '&')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_and;
    }
  else
    {
      (*session->token_stream)[index++].kind = '&';
    }
}

void Lexer::scan_left_paren()
{
  ++cursor;
  (*session->token_stream)[index++].kind = '(';
}

void Lexer::scan_right_paren()
{
  ++cursor;
  (*session->token_stream)[index++].kind = ')';
}

void Lexer::scan_star()
{
  /*
    '*'		::= star
    '*='		::= star_equal
  */

  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_star_eq;
    }
  else
    {
      (*session->token_stream)[index++].kind = '*';
    }
}

void Lexer::scan_plus()
{
  /*
    '+'		::= plus
    '++'		::= incr
    '+='		::= plus_equal
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_plus_eq;
    }
  else if (*cursor == '+')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_incr;
    }
  else
    {
      (*session->token_stream)[index++].kind = '+';
    }
}

void Lexer::scan_comma()
{
  ++cursor;
  (*session->token_stream)[index++].kind = ',';
}

void Lexer::scan_minus()
{
  /*
    '-'		::= minus
    '--'		::= decr
    '-='		::= minus_equal
    '->'		::= left_arrow
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_minus_eq;
    }
  else if (*cursor == '-')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_decr;
    }
  else if (*cursor == '>')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_arrow;
    }
  else
    {
      (*session->token_stream)[index++].kind = '-';
    }
}

void Lexer::scan_dot()
{
  /*
    '.'		::= dot
    '...'		::= ellipsis
  */

  ++cursor;
  if (*cursor == '.' && *(cursor + 1) == '.')
    {
      cursor += 2;
      (*session->token_stream)[index++].kind = Token_ellipsis;
    }
  else if (*cursor == '.' && *(cursor + 1) == '*')
    {
      cursor += 2;
      (*session->token_stream)[index++].kind = Token_ptrmem;
    }
  else
    (*session->token_stream)[index++].kind = '.';
}

void Lexer::scan_divide()
{
  /*
    '/'		::= divide
    '/='	::= divide_equal
  */

  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_div_eq;
    }
  else if( *cursor == '*' || *cursor == '/' )
    {
      ///It is a comment
      --cursor; //Move back to the '/'
      SpecialCursor commentBegin = cursor;
      skipComment();
      if( cursor != commentBegin ) {
        ///Store the comment
        if(!m_canMergeComment || (*session->token_stream)[index-1].kind != Token_comment) {

          //Only allow appending to comments that are behind a newline, because else they may belong to the item on their left side.
          //If index is 1, this comment is the first token, which should be the translation-unit comment. So do not merge following comments.
          if(m_firstInLine && index != 1)
            m_canMergeComment = true;
          else
            m_canMergeComment = false;

          (*session->token_stream)[index++].kind = Token_comment;
          (*session->token_stream)[index-1].size = (size_t)(cursor - commentBegin);
          (*session->token_stream)[index-1].position = commentBegin.offsetIn( session->contents() );
        }else{
          //Merge with previous comment
          (*session->token_stream)[index-1].size = cursor.offsetIn(session->contents()) - (*session->token_stream)[index-1].position;
        }
      }
    }
  else
    {
      (*session->token_stream)[index++].kind = '/';
    }
}

void Lexer::scan_colon()
{
  ++cursor;
  if (*cursor == ':')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_scope;
    }
  else
    {
      (*session->token_stream)[index++].kind = ':';
    }
}

void Lexer::scan_semicolon()
{
  ++cursor;
  (*session->token_stream)[index++].kind = ';';
}

void Lexer::scan_less()
{
  /*
    '<'			::= less
    '<<'		::= left_shift
    '<<='		::= left_shift_equal
    '<='		::= less_equal
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_leq;
    }
  else if (*cursor == '<')
    {
      ++cursor;
      if (*cursor == '=')
      {
        ++cursor;
        (*session->token_stream)[index++].kind = Token_leftshift_eq;
      }
      else
      {
        (*session->token_stream)[index++].kind = Token_leftshift;
      }
    }
  else
    {
      (*session->token_stream)[index++].kind = '<';
    }
}

void Lexer::scan_equal()
{
  /*
    '='			::= equal
    '=='		::= equal_equal
  */
  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_eq;
    }
  else
    {
      (*session->token_stream)[index++].kind = '=';
    }
}

void Lexer::scan_greater()
{
  /*
    '>'			::= greater
    '>='		::= greater_equal
    '>>'		::= right_shift
    '>>='		::= right_shift_equal
  */

  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_geq;
    }
  else if (*cursor == '>')
    {
      ++cursor;
      if (*cursor == '=')
	{
	  ++cursor;
	  (*session->token_stream)[index++].kind = Token_rightshift_eq;
	}
      else
	{
	  (*session->token_stream)[index++].kind = Token_rightshift;
	}
    }
  else
    {
      (*session->token_stream)[index++].kind = '>';
    }
}

void Lexer::scan_question()
{
  ++cursor;
  (*session->token_stream)[index++].kind = '?';
}

void Lexer::scan_left_bracket()
{
  ++cursor;
  (*session->token_stream)[index++].kind = '[';
}

void Lexer::scan_right_bracket()
{
  ++cursor;
  (*session->token_stream)[index++].kind = ']';
}

void Lexer::scan_xor()
{
  /*
    '^'			::= xor
    '^='		::= xor_equal
  */
  ++cursor;

  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_xor_eq;
    }
  else
    {
      (*session->token_stream)[index++].kind = '^';
    }
}

void Lexer::scan_left_brace()
{
  ++cursor;
  (*session->token_stream)[index++].kind = '{';
}

void Lexer::scan_or()
{
  /*
    '|'			::= or
    '|='		::= or_equal
    '||'		::= or_or
  */
  ++cursor;
  if (*cursor == '=')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_or_eq;
    }
  else if (*cursor == '|')
    {
      ++cursor;
      (*session->token_stream)[index++].kind = Token_or;
    }
  else
    {
    (*session->token_stream)[index++].kind = '|';
  }
}

void Lexer::scan_right_brace()
{
  ++cursor;
  (*session->token_stream)[index++].kind = '}';
}

void Lexer::scan_tilde()
{
  ++cursor;
  (*session->token_stream)[index++].kind = '~';
}

void Lexer::scan_EOF()
{
  ++cursor;
  (*session->token_stream)[index++].kind = Token_EOF;
}

void Lexer::scan_invalid_input()
{
  KDevelop::ProblemPointer p = createProblem();
  p->setDescription(i18n("invalid input: %1", KDevelop::IndexedString::fromIndex(*cursor.current).str()));
  control->reportProblem(p);

  ++cursor;
}

KDevelop::ProblemPointer Lexer::createProblem() const
{
  Q_ASSERT(index > 0);

  KDevelop::ProblemPointer p(new KDevelop::Problem);

  KDevelop::CursorInRevision position = session->positionAt(cursor.offsetIn(session->contents()), true);
  p->setSource(KDevelop::ProblemData::Lexer);
  p->setFinalLocation(KDevelop::DocumentRange(session->url(), KTextEditor::Range(position.castToSimpleCursor(), 1)));

  return p;
}

