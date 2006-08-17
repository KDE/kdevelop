/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "parsesession.h"

#include "lexer.h"
#include "memorypool.h"

ParseSession::ParseSession(const char *_contents, std::size_t _size, pool* _mempool)
  : contents(_contents)
  , size(_size)
  , mempool(_mempool)
  , token_stream(0)
  , location_table(0)
  , line_table(0)
{
}

ParseSession::~ParseSession()
{
  delete contents;
  delete mempool;
  delete token_stream;
  delete location_table;
  delete line_table;
}

void ParseSession::positionAt(std::size_t offset, int *line, int *column,
                       QString *filename) const
{
  int ppline, ppcolumn;
  // Retrieve the line of the first preprocessor statement before this offset
  line_table->positionAt(offset, &ppline, &ppcolumn);

  int base_line = 1;

  // Extract the actual line number and filename from the preprocessor statement
  extract_line((*line_table)[ppline-1], &base_line, filename);

  int line2, column2;
  // Retrieve the line (and column) of the preprocessor statement above
  location_table->positionAt((*line_table)[ppline-1], &line2, &column2);

  // Retrieve the line and column of the offset in the non-preprocessed source
  location_table->positionAt(offset, line, column);

  /*
    NPPL == non-preprocessed line
    NPPL =
      actual line of the first preceeding preprocessor statement +
      NPPL of the token - (NPPL of the first preceeding preprocessor statement + 1 to account for the preprocessor statement)
  */
  //kDebug() << k_funcinfo << offset << ": line " << (base_line + *line - line2  - 1) << ", column " << *column << " == " << base_line << " + " << *line << " - " << line2 << " - 1" << endl;
  *line = base_line + *line - line2  - 1;

  if (*filename == "<internal>")
    filename->clear();
}

void ParseSession::extract_line(int offset, int *line, QString *filename) const
{
  const char *cursor = contents + offset;

  if (*cursor != '#')
    {
      // nothing to do
      return;
    }

  ++cursor; // skip '#'
  if (std::isspace(*cursor) && std::isdigit(*(cursor + 1)))
    {
      ++cursor;
      char buffer[1024], *cp = buffer;
      do { *cp++ = *cursor++; }  // ### FIXME unsafe!
      while (std::isdigit(*cursor));
      *cp = '\0';
      int line_number = strtol(buffer, 0, 0);

      if (! std::isspace(*cursor))
        {
          /*Problem p = createProblem();
          p.setMessage("expected white space");
          control->reportProblem(p);*/
          goto skip_line;
        }

      ++cursor; // skip the white space

      if (*cursor != '"')
        {
          /*Problem p = createProblem();
          p.setMessage("expected \"");
          control->reportProblem(p);*/
          goto skip_line;
        }

      ++cursor;

      cp = buffer;
      while (*cursor && *cursor != '"')
      { *cp++ = *cursor++; } // ### FIXME unsafe!
      *cp = '\0';

      if (*cursor != '"')
        {
          /*Problem p = createProblem();
          p.setMessage("expected \"");
          control->reportProblem(p);*/
          goto skip_line;
        }

      ++cursor;

      *filename = buffer;
      *line = line_number;
      //kDebug() << k_funcinfo << "filename: " << buffer << " line: " << line << endl;
      return;
    }

skip_line:
  // skip the line
  while (*cursor && *cursor != '\n')
    ++cursor;
}
