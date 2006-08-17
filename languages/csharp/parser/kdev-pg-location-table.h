/* This file is part of kdev-pg
   Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEV_PG_LOCATION_TABLE_H
#define KDEV_PG_LOCATION_TABLE_H

#include <cstdlib>

struct kdev_pg_location_table
{
  inline kdev_pg_location_table(std::size_t size = 1024)
    : lines(0), line_count(0), current_line(0)
  {
    resize(size);
  }

  inline ~kdev_pg_location_table()
  {
    free(lines);
  }

  inline std::size_t size() const
  { return line_count; }

  void resize(std::size_t size)
  {
    assert(size > 0);
    lines = (std::size_t*) ::realloc(lines, sizeof(std::size_t) * size);
    line_count = size;
  }

  /**
   * Returns the \a line and \a column of the given \a offset in this table.
   */
  void position_at(std::size_t offset, int *line, int *column) const
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

  /**
   * Marks an \a offset as the character before the first one in the next line.
   * The position_at() function relies on newline() being called properly.
   */
  inline void newline(std::size_t offset)
  {
    if (current_line == line_count)
      resize(current_line * 2);

    lines[current_line++] = offset;
  }

  inline std::size_t &operator[](int index)
  { return lines[index]; }

private:
  /// An array of input buffer offsets
  std::size_t *lines;
  /// The size of the allocated array
  std::size_t line_count;
  /// The index to the next index in the lines array
  std::size_t current_line;

private:
  kdev_pg_location_table(kdev_pg_location_table const &other);
  void operator=(kdev_pg_location_table const &other);
};


#endif // KDEV_PG_LOCATION_TABLE_H

