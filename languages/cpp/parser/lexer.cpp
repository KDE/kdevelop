/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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

#include <cctype>

#include <kdebug.h>

scan_fun_ptr Lexer::s_scan_keyword_table[] = {
  &Lexer::scanKeyword0, &Lexer::scanKeyword0,
  &Lexer::scanKeyword2, &Lexer::scanKeyword3,
  &Lexer::scanKeyword4, &Lexer::scanKeyword5,
  &Lexer::scanKeyword6, &Lexer::scanKeyword7,
  &Lexer::scanKeyword8, &Lexer::scanKeyword9,
  &Lexer::scanKeyword10, &Lexer::scanKeyword11,
  &Lexer::scanKeyword12, &Lexer::scanKeyword13,
  &Lexer::scanKeyword14, &Lexer::scanKeyword0,
  &Lexer::scanKeyword16
};

scan_fun_ptr Lexer::s_scan_table[256];
bool Lexer::s_initialized = false;

Lexer::Lexer(Control *c)
  : session(0),
    control(c)
{
}

void Lexer::tokenize(ParseSession* _session)
{
  session = _session;

  if (!s_initialized)
    initialize_scan_table();

  session->token_stream->resize(1024);
  (*session->token_stream)[0].kind = Token_EOF;

  index = 1;

  cursor = session->contents;

  session->location_table->resize(1024);
  (*session->location_table)[0] = 0;
  session->location_table->current_line = 1;

  session->line_table->resize(1024);
  (*session->line_table)[0] = 0;
  session->line_table->current_line = 1;

  do {
    if (index == session->token_stream->size())
      session->token_stream->resize(session->token_stream->size() * 2);

    Token *current_token = &(*session->token_stream)[index];
    current_token->text = session->contents;
    current_token->position = cursor - session->contents;
    (this->*s_scan_table[*cursor])();
    current_token->size = cursor - session->contents - current_token->position;
  } while (cursor < session->contents + session->size-1);

    if (index == session->token_stream->size())
      session->token_stream->resize(session->token_stream->size() * 2);

  (*session->token_stream)[index].position = cursor - session->contents;
  (*session->token_stream)[index].kind = Token_EOF;
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
  if (session->line_table->current_line == session->line_table->size())
    session->line_table->resize(session->line_table->current_line * 2);

  (*session->line_table)[session->line_table->current_line++] = (cursor - session->contents);

  while (*cursor && *cursor != '\n')
    ++cursor;

  if (*cursor != '\n')
    {
      Problem p = createProblem();
      p.setMessage("expected end of line");
      control->reportProblem(p);
    }
}

void Lexer::scan_char_constant()
{
  const char *begin = cursor;

  ++cursor;
  while (*cursor && *cursor != '\'')
    {
       if (*cursor == '\n')
        {
          Problem p = createProblem();
          p.setMessage("unexpected new line");
          control->reportProblem(p);
          break;
        }

      if (*cursor == '\\')
	++cursor;

      ++cursor;
    }

  if (*cursor != '\'')
    {
      Problem p = createProblem();
      p.setMessage("expected '");
      control->reportProblem(p);
    }
  else
    {
      ++cursor;
    }

  (*session->token_stream)[index].extra.symbol =
    control->findOrInsertName((const char*) begin, cursor - begin);

  (*session->token_stream)[index++].kind = Token_char_literal;
}

void Lexer::scan_string_constant()
{
  const char *begin = cursor;

  ++cursor;
  while (*cursor && *cursor != '"')
    {
       if (*cursor == '\n')
        {
          Problem p = createProblem();
          p.setMessage("unexpected new line");
          control->reportProblem(p);
          break;
        }

      if (*cursor == '\\')
	++cursor;

      ++cursor;
    }

  if (*cursor != '"')
    {
      Problem p = createProblem();
      p.setMessage("expected \"");
      control->reportProblem(p);
    }
  else
    {
      ++cursor;
    }

  (*session->token_stream)[index].extra.symbol =
    control->findOrInsertName((const char*) begin, cursor - begin);

  (*session->token_stream)[index++].kind = Token_string_literal;
}

void Lexer::scan_newline()
{
  if (session->location_table->current_line == session->location_table->size())
    session->location_table->resize(session->location_table->current_line * 2);

  (*session->location_table)[session->location_table->current_line++] = (cursor - session->contents);
  ++cursor;
}

void Lexer::scan_white_spaces()
{
  while (isspace(*cursor))
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

void Lexer::scan_identifier_or_keyword()
{
  const char *skip = cursor;
  while (isalnum(*skip) || *skip== '_')
    ++skip;

  int n = skip - cursor;
  Token *current_token = &(*session->token_stream)[index];
  (this->*s_scan_keyword_table[n < 17 ? n : 0])();

  if (current_token->kind == Token_identifier)
    {
      current_token->extra.symbol =
	control->findOrInsertName((const char*) cursor, n);
    }

  cursor = skip;
}

void Lexer::scan_int_constant()
{
  if (*cursor == '.' && !std::isdigit(*(cursor + 1)))
    {
      scan_dot();
      return;
    }

  const char *begin = cursor;

  while (isalnum(*cursor) || *cursor == '.')
    ++cursor;

  (*session->token_stream)[index].extra.symbol =
    control->findOrInsertName((const char*) begin, cursor - begin);

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
      (*session->token_stream)[index++].kind = Token_assign;
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
      (*session->token_stream)[index++].kind = Token_assign;
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
      (*session->token_stream)[index++].kind = Token_assign;
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
      (*session->token_stream)[index++].kind = Token_assign;
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
      (*session->token_stream)[index++].kind = Token_assign;
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
      if (*cursor == '*')
	{
	  ++cursor;
	  (*session->token_stream)[index++].kind = Token_ptrmem;
	}
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
      (*session->token_stream)[index++].kind = Token_assign;
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
	  (*session->token_stream)[index++].kind = Token_assign;
	}
      else
	{
	  (*session->token_stream)[index++].kind = Token_shift;
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
	  (*session->token_stream)[index++].kind = Token_assign;
	}
      else
	{
	  (*session->token_stream)[index++].kind = Token_shift;
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
      (*session->token_stream)[index++].kind = Token_assign;
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
      (*session->token_stream)[index++].kind = Token_assign;
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
  Problem p = createProblem();
  p.setMessage("invalid input");
  control->reportProblem(p);

  ++cursor;
}

void LocationTable::positionAt(std::size_t offset,
                               int *line, int *column) const
{
  int first = 0;
  // len is assigned the position 1 past the current set position
  int len = current_line;
  int half;
  int middle;

  while (len > 0)
    {
      // Half of the way through the array
      half = len >> 1;
      // The starting point
      middle = first;

      middle += half;

      if (lines[middle] < offset)
        {
          first = middle;
          ++first;
          len = len - half - 1;
        }
      else
        len = half;
    }

  *line = std::max(first, 1);
  *column = offset - lines[*line - 1] - 1;
}

void Lexer::scanKeyword0()
{
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword2()
{
  switch (*cursor)
    {
    case 'i':
      if (*(cursor + 1) == 'f')
	{
	  (*session->token_stream)[index++].kind = Token_if;
	  return;
	}
      break;

    case 'd':
      if (*(cursor + 1) == 'o')
	{
	  (*session->token_stream)[index++].kind = Token_do;
	  return;
	}
      break;

    case 'o':
      if (*(cursor + 1) == 'r')
	{
	  (*session->token_stream)[index++].kind = Token_or;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword3()
{
  switch (*cursor)
    {
    case 'a':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'd')
	{
	  (*session->token_stream)[index++].kind = Token_and;
	  return;
	}
      if (*(cursor + 1) == 's' &&
	  *(cursor + 2) == 'm')
	{
	  (*session->token_stream)[index++].kind = Token_asm;
	  return;
	}
      break;

    case 'f':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'r')
	{
	  (*session->token_stream)[index++].kind = Token_for;
	  return;
	}
      break;

    case 'i':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_int;
	  return;
	}
      break;

    case 'n':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'w')
	{
	  (*session->token_stream)[index++].kind = Token_new;
	  return;
	}
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_not;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'y')
	{
	  (*session->token_stream)[index++].kind = Token_try;
	  return;
	}
      break;

    case 'x':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'r')
	{
	  (*session->token_stream)[index++].kind = Token_xor;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword4()
{
  switch (*cursor)
    {
    case 'a':
      if (*(cursor + 1) == 'u' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'o')
	{
	  (*session->token_stream)[index++].kind = Token_auto;
	  return;
	}
      break;

    case 'c':
      if (*(cursor + 1) == 'a' &&
	  *(cursor + 2) == 's' &&
	  *(cursor + 3) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_case;
	  return;
	}
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 'r')
	{
	  (*session->token_stream)[index++].kind = Token_char;
	  return;
	}
      break;

    case 'b':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 'l')
	{
	  (*session->token_stream)[index++].kind = Token_bool;
	  return;
	}
      break;

    case 'e':
      if (*(cursor + 1) == 'l' &&
	  *(cursor + 2) == 's' &&
	  *(cursor + 3) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_else;
	  return;
	}
      if (*(cursor + 1) == 'm' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_emit;
	  return;
	}
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'u' &&
	  *(cursor + 3) == 'm')
	{
	  (*session->token_stream)[index++].kind = Token_enum;
	  return;
	}
      break;

    case 'g':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'o')
	{
	  (*session->token_stream)[index++].kind = Token_goto;
	  return;
	}
      break;

    case 'l':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 'g')
	{
	  (*session->token_stream)[index++].kind = Token_long;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 's')
	{
	  (*session->token_stream)[index++].kind = Token_this;
	  return;
	}
      break;

    case 'v':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'd')
	{
	  (*session->token_stream)[index++].kind = Token_void;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword5()
{
  switch (*cursor)
    {
    case 'c':
      if (*(cursor + 1) == 'a' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'c' &&
	  *(cursor + 4) == 'h')
	{
	  (*session->token_stream)[index++].kind = Token_catch;
	  return;
	}
      if (*(cursor + 1) == 'l' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 's' &&
	  *(cursor + 4) == 's')
	{
	  (*session->token_stream)[index++].kind = Token_class;
	  return;
	}
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'm' &&
	  *(cursor + 3) == 'p' &&
	  *(cursor + 4) == 'l')
	{
	  (*session->token_stream)[index++].kind = Token_compl;
	  return;
	}
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 's' &&
	  *(cursor + 4) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_const;
	  return;
	}
      break;

    case 'b':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'o' &&
	  *(cursor + 4) == 'r')
	{
	  (*session->token_stream)[index++].kind = Token_bitor;
	  return;
	}
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'e' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'k')
	{
	  (*session->token_stream)[index++].kind = Token_break;
	  return;
	}
      break;

    case 'f':
      if (*(cursor + 1) == 'l' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_float;
	  return;
	}
      break;

    case 'o':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == '_' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'q')
	{
	  (*session->token_stream)[index++].kind = Token_or_eq;
	  return;
	}
      break;

    case 's':
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 'r' &&
	  *(cursor + 4) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_short;
	  return;
	}
      if (*(cursor + 1) == 'l' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 's')
	{
	  (*session->token_stream)[index++].kind = Token_slots;
	  return;
	}
      break;

    case 'u':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'o' &&
	  *(cursor + 4) == 'n')
	{
	  (*session->token_stream)[index++].kind = Token_union;
	  return;
	}
      if (*(cursor + 1) == 's' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'n' &&
	  *(cursor + 4) == 'g')
	{
	  (*session->token_stream)[index++].kind = Token_using;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'r' &&
	  *(cursor + 3) == 'o' &&
	  *(cursor + 4) == 'w')
	{
	  (*session->token_stream)[index++].kind = Token_throw;
	  return;
	}
      break;

    case 'w':
      if (*(cursor + 1) == 'h' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'l' &&
	  *(cursor + 4) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_while;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword6()
{
  switch (*cursor)
    {
    case 'a':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'd' &&
	  *(cursor + 3) == '_' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'q')
	{
	  (*session->token_stream)[index++].kind = Token_and_eq;
	  return;
	}
      break;

    case 'b':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'n' &&
	  *(cursor + 5) == 'd')
	{
	  (*session->token_stream)[index++].kind = Token_bitand;
	  return;
	}
      break;

    case 'e':
      if (*(cursor + 1) == 'x' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'o' &&
	  *(cursor + 4) == 'r' &&
	  *(cursor + 5) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_export;
	  return;
	}
      if (*(cursor + 1) == 'x' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'r' &&
	  *(cursor + 5) == 'n')
	{
	  (*session->token_stream)[index++].kind = Token_extern;
	  return;
	}
      break;

    case 'd':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'l' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_delete;
	  return;
	}
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'u' &&
	  *(cursor + 3) == 'b' &&
	  *(cursor + 4) == 'l' &&
	  *(cursor + 5) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_double;
	  return;
	}
      break;

    case 'f':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'n' &&
	  *(cursor + 5) == 'd')
	{
	  (*session->token_stream)[index++].kind = Token_friend;
	  return;
	}
      break;

    case 'i':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 'l' &&
	  *(cursor + 3) == 'i' &&
	  *(cursor + 4) == 'n' &&
	  *(cursor + 5) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_inline;
	  return;
	}
      break;

    case 'K':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'D' &&
	  *(cursor + 3) == 'C' &&
	  *(cursor + 4) == 'O' &&
	  *(cursor + 5) == 'P')
	{
	  (*session->token_stream)[index++].kind = Token_K_DCOP;
	  return;
	}
      break;

    case 'n':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == '_' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'q')
	{
	  (*session->token_stream)[index++].kind = Token_not_eq;
	  return;
	}
      break;

    case 'p':
      if (*(cursor + 1) == 'u' &&
	  *(cursor + 2) == 'b' &&
	  *(cursor + 3) == 'l' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'c')
	{
	  (*session->token_stream)[index++].kind = Token_public;
	  return;
	}
      break;

    case 's':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 'g' &&
	  *(cursor + 3) == 'n' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'd')
	{
	  (*session->token_stream)[index++].kind = Token_signed;
	  return;
	}
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 'z' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'o' &&
	  *(cursor + 5) == 'f')
	{
	  (*session->token_stream)[index++].kind = Token_sizeof;
	  return;
	}
      if (*(cursor + 1) == 't' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'c')
	{
	  (*session->token_stream)[index++].kind = Token_static;
	  return;
	}
      if (*(cursor + 1) == 't' &&
	  *(cursor + 2) == 'r' &&
	  *(cursor + 3) == 'u' &&
	  *(cursor + 4) == 'c' &&
	  *(cursor + 5) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_struct;
	  return;
	}
      if (*(cursor + 1) == 'w' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'c' &&
	  *(cursor + 5) == 'h')
	{
	  (*session->token_stream)[index++].kind = Token_switch;
	  return;
	}
      break;

    case 'r':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'u' &&
	  *(cursor + 4) == 'r' &&
	  *(cursor + 5) == 'n')
	{
	  (*session->token_stream)[index++].kind = Token_return;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'y' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'd')
	{
	  (*session->token_stream)[index++].kind = Token_typeid;
	  return;
	}
      break;

    case 'x':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'r' &&
	  *(cursor + 3) == '_' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'q')
	{
	  (*session->token_stream)[index++].kind = Token_xor_eq;
	  return;
	}
      break;

    case 'k':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'd' &&
	  *(cursor + 3) == 'c' &&
	  *(cursor + 4) == 'o' &&
	  *(cursor + 5) == 'p')
	{
	  (*session->token_stream)[index++].kind = Token_k_dcop;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword7()
{
  switch (*cursor)
    {
    case 'd':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'f' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'u' &&
	  *(cursor + 5) == 'l' &&
	  *(cursor + 6) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_default;
	  return;
	}
      break;

    case 'm':
      if (*(cursor + 1) == 'u' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'b' &&
	  *(cursor + 5) == 'l' &&
	  *(cursor + 6) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_mutable;
	  return;
	}
      break;

    case 'p':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'v' &&
	  *(cursor + 4) == 'a' &&
	  *(cursor + 5) == 't' &&
	  *(cursor + 6) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_private;
	  return;
	}
      break;
#if 0
    case 's':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 'g' &&
	  *(cursor + 3) == 'n' &&
	  *(cursor + 4) == 'a' &&
	  *(cursor + 5) == 'l' &&
	  *(cursor + 6) == 's')
	{
	  (*session->token_stream)[index++].kind = Token_signals;
	  return;
	}
      break;
#endif
    case 't':
      if (*(cursor + 1) == 'y' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'd' &&
	  *(cursor + 5) == 'e' &&
	  *(cursor + 6) == 'f')
	{
	  (*session->token_stream)[index++].kind = Token_typedef;
	  return;
	}
      break;

    case 'v':
      if (*(cursor + 1) == 'i' &&
	  *(cursor + 2) == 'r' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'u' &&
	  *(cursor + 5) == 'a' &&
	  *(cursor + 6) == 'l')
	{
	  (*session->token_stream)[index++].kind = Token_virtual;
	  return;
	}
      break;
    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword8()
{
  switch (*cursor)
    {
    case '_':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 't' &&
	  *(cursor + 3) == 'y' &&
	  *(cursor + 4) == 'p' &&
	  *(cursor + 5) == 'e' &&
	  *(cursor + 6) == 'o' &&
	  *(cursor + 7) == 'f')
	{
	  (*session->token_stream)[index++].kind = Token___typeof;
	  return;
	}
      break;

    case 'c':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'n' &&
	  *(cursor + 6) == 'u' &&
	  *(cursor + 7) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_continue;
	  return;
	}
      break;

    case 'e':
      if (*(cursor + 1) == 'x' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'l' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'c' &&
	  *(cursor + 6) == 'i' &&
	  *(cursor + 7) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_explicit;
	  return;
	}
      break;

    case 'o':
      if (*(cursor + 1) == 'p' &&
	  *(cursor + 2) == 'e' &&
	  *(cursor + 3) == 'r' &&
	  *(cursor + 4) == 'a' &&
	  *(cursor + 5) == 't' &&
	  *(cursor + 6) == 'o' &&
	  *(cursor + 7) == 'r')
	{
	  (*session->token_stream)[index++].kind = Token_operator;
	  return;
	}
      break;

    case 'Q':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'O' &&
	  *(cursor + 3) == 'B' &&
	  *(cursor + 4) == 'J' &&
	  *(cursor + 5) == 'E' &&
	  *(cursor + 6) == 'C' &&
	  *(cursor + 7) == 'T')
	{
	  (*session->token_stream)[index++].kind = Token_Q_OBJECT;
	  return;
	}
      break;

    case 'r':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'g' &&
	  *(cursor + 3) == 'i' &&
	  *(cursor + 4) == 's' &&
	  *(cursor + 5) == 't' &&
	  *(cursor + 6) == 'e' &&
	  *(cursor + 7) == 'r')
	{
	  (*session->token_stream)[index++].kind = Token_register;
	  return;
	}
      break;

    case 'u':
      if (*(cursor + 1) == 'n' &&
	  *(cursor + 2) == 's' &&
	  *(cursor + 3) == 'i' &&
	  *(cursor + 4) == 'g' &&
	  *(cursor + 5) == 'n' &&
	  *(cursor + 6) == 'e' &&
	  *(cursor + 7) == 'd')
	{
	  (*session->token_stream)[index++].kind = Token_unsigned;
	  return;
	}
      break;

    case 't':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'm' &&
	  *(cursor + 3) == 'p' &&
	  *(cursor + 4) == 'l' &&
	  *(cursor + 5) == 'a' &&
	  *(cursor + 6) == 't' &&
	  *(cursor + 7) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_template;
	  return;
	}
      if (*(cursor + 1) == 'y' &&
	  *(cursor + 2) == 'p' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 'n' &&
	  *(cursor + 5) == 'a' &&
	  *(cursor + 6) == 'm' &&
	  *(cursor + 7) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_typename;
	  return;
	}
      break;

    case 'v':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'l' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == 'i' &&
	  *(cursor + 6) == 'l' &&
	  *(cursor + 7) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_volatile;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword9()
{
  switch (*cursor)
    {
    case 'p':
      if (*(cursor + 1) == 'r' &&
	  *(cursor + 2) == 'o' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'e' &&
	  *(cursor + 5) == 'c' &&
	  *(cursor + 6) == 't' &&
	  *(cursor + 7) == 'e' &&
	  *(cursor + 8) == 'd')
	{
	  (*session->token_stream)[index++].kind = Token_protected;
	  return;
	}
      break;

    case 'n':
      if (*(cursor + 1) == 'a' &&
	  *(cursor + 2) == 'm' &&
	  *(cursor + 3) == 'e' &&
	  *(cursor + 4) == 's' &&
	  *(cursor + 5) == 'p' &&
	  *(cursor + 6) == 'a' &&
	  *(cursor + 7) == 'c' &&
	  *(cursor + 8) == 'e')
	{
	  (*session->token_stream)[index++].kind = Token_namespace;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword10()
{
  switch (*cursor)
    {
    case 'c':
      if (*(cursor + 1) == 'o' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 's' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == '_' &&
	  *(cursor + 6) == 'c' &&
	  *(cursor + 7) == 'a' &&
	  *(cursor + 8) == 's' &&
	  *(cursor + 9) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_const_cast;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword11()
{
  switch (*cursor)
    {
    case 's':
      if (*(cursor + 1) == 't' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 'i' &&
	  *(cursor + 5) == 'c' &&
	  *(cursor + 6) == '_' &&
	  *(cursor + 7) == 'c' &&
	  *(cursor + 8) == 'a' &&
	  *(cursor + 9) == 's' &&
	  *(cursor + 10) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_static_cast;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword12()
{
  switch (*cursor)
    {
    case 'd':
      if (*(cursor + 1) == 'y' &&
	  *(cursor + 2) == 'n' &&
	  *(cursor + 3) == 'a' &&
	  *(cursor + 4) == 'm' &&
	  *(cursor + 5) == 'i' &&
	  *(cursor + 6) == 'c' &&
	  *(cursor + 7) == '_' &&
	  *(cursor + 8) == 'c' &&
	  *(cursor + 9) == 'a' &&
	  *(cursor + 10) == 's' &&
	  *(cursor + 11) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_dynamic_cast;
	  return;
	}
      break;

    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword13()
{
  switch (*cursor)
    {
    case '_':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'a' &&
	  *(cursor + 3) == 't' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == 'r' &&
	  *(cursor + 6) == 'i' &&
	  *(cursor + 7) == 'b' &&
	  *(cursor + 8) == 'u' &&
	  *(cursor + 9) == 't' &&
	  *(cursor + 10) == 'e' &&
	  *(cursor + 11) == '_' &&
	  *(cursor + 12) == '_')
	{
	  (*session->token_stream)[index++].kind = Token___attribute__;
	  return;
	}
      break;
    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword14()
{
  switch (*cursor)
    {
    case 'k':
      if (*(cursor + 1) == '_' &&
	  *(cursor + 2) == 'd' &&
	  *(cursor + 3) == 'c' &&
	  *(cursor + 4) == 'o' &&
	  *(cursor + 5) == 'p' &&
	  *(cursor + 6) == '_' &&
	  *(cursor + 7) == 's' &&
	  *(cursor + 8) == 'i' &&
	  *(cursor + 9) == 'g' &&
	  *(cursor + 10) == 'n' &&
	  *(cursor + 11) == 'a' &&
	  *(cursor + 12) == 'l' &&
	  *(cursor + 13) == 's')
	{
	  (*session->token_stream)[index++].kind = Token_k_dcop_signals;
	  return;
	}
      break;
    }
  (*session->token_stream)[index++].kind = Token_identifier;
}

void Lexer::scanKeyword16()
{
  switch (*cursor)
    {
    case 'r':
      if (*(cursor + 1) == 'e' &&
	  *(cursor + 2) == 'i' &&
	  *(cursor + 3) == 'n' &&
	  *(cursor + 4) == 't' &&
	  *(cursor + 5) == 'e' &&
	  *(cursor + 6) == 'r' &&
	  *(cursor + 7) == 'p' &&
	  *(cursor + 8) == 'r' &&
	  *(cursor + 9) == 'e' &&
	  *(cursor + 10) == 't' &&
	  *(cursor + 11) == '_' &&
	  *(cursor + 12) == 'c' &&
	  *(cursor + 13) == 'a' &&
	  *(cursor + 14) == 's' &&
	  *(cursor + 15) == 't')
	{
	  (*session->token_stream)[index++].kind = Token_reinterpret_cast;
	  return;
	}
      break;
    }

  (*session->token_stream)[index++].kind = Token_identifier;
}

Problem Lexer::createProblem() const
{
  Q_ASSERT(index > 0);

  Problem p; // ### fill me

  int line = 0, column = 0;
  QString fileName;

  session->positionAt(index - 1, &line, &column, &fileName);
  p.setLine(line);
  p.setColumn(column);
  p.setFileName(fileName);

  return p;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
