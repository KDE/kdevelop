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

#ifndef PARSESESSION_H
#define PARSESESSION_H

#include <cstdlib>

#include <QtCore/QString>
#include <QtCore/QByteArray>

#include "rpp/preprocessor.h"

class pool;
class TokenStream;
class LocationTable;

/// Contains everything needed to keep an AST useful once the rest of the parser
/// has gone away.
class ParseSession
{
public:
  ParseSession();
  ~ParseSession();

  /**
   * Return the position (\a line%, \a column%) of the \a offset in \a filename
   *
   * \note the line starts from 0.
   */
  void positionAt(std::size_t offset, int *line, int *column,
                  QString *filename) const;

  void setContents(const QByteArray& contents);

  const char *contents() const;
  std::size_t size() const;
  pool* mempool;
  TokenStream* token_stream;
  LocationTable* location_table;
  LocationTable* line_table;
  QList<Preprocessor::MacroItem> macros;

private:
  void extract_line(int offset, int *line, QString *filename) const;
  QByteArray m_contents;
};

#endif
