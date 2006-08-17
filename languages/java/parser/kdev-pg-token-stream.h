/*
  This file is part of kdev-pg
  Copyright 2005, 2006 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef KDEV_PG_TOKEN_STREAM_H
#define KDEV_PG_TOKEN_STREAM_H

#include <cstdlib>

#include "kdev-pg-location-table.h"

class kdev_pg_token_stream
{
public:
  struct token_type
  {
    int kind;
    std::size_t begin;
    std::size_t end;
    char const *text;
  };

public:
  kdev_pg_token_stream()
    : _M_token_buffer(0),
      _M_token_buffer_size(0),
      _M_index(0),
      _M_token_count(0),
      _M_line_table(0)
  {
    reset();
  }

  ~kdev_pg_token_stream()
  {
    if (_M_token_buffer)
      ::free(_M_token_buffer);
    if (_M_line_table)
      delete _M_line_table;
  }

  inline void reset()
  {
    _M_index = 0;
    _M_token_count = 0;
  }

  inline std::size_t size() const
  {
    return _M_token_count;
  }

  inline std::size_t index() const
  {
    return _M_index;
  }

  inline void rewind(std::size_t index)
  {
    _M_index = index;
  }

  inline token_type const &token(std::size_t index) const
  {
    return _M_token_buffer[index];
  }

  inline token_type &token(std::size_t index)
  {
    return _M_token_buffer[index];
  }

  inline int next_token()
  {
    return _M_token_buffer[_M_index++].kind;
  }

  inline token_type &next()
  {
    if (_M_token_count == _M_token_buffer_size)
      {
        if (_M_token_buffer_size == 0)
          _M_token_buffer_size = 1024;

        _M_token_buffer_size <<= 2;

        _M_token_buffer = reinterpret_cast<token_type*>
          (::realloc(_M_token_buffer, _M_token_buffer_size * sizeof(token_type)));
      }

    return _M_token_buffer[_M_token_count++];
  }

  inline kdev_pg_location_table *line_table()
  {
    if (!_M_line_table)
      _M_line_table = new kdev_pg_location_table();

    return _M_line_table;
  }

  inline void start_position(std::size_t index, int *line, int *column)
  {
    if (!_M_line_table)
      {
        *line = 0; *column = 0;
      }
    else
      _M_line_table->position_at(token(index).begin, line, column);
  }

  inline void end_position(std::size_t index, int *line, int *column)
  {
    if (!_M_line_table)
      {
        *line = 0; *column = 0;
      }
    else
      _M_line_table->position_at(token(index).end, line, column);
  }

private:
  token_type *_M_token_buffer;
  std::size_t _M_token_buffer_size;
  std::size_t _M_index;
  std::size_t _M_token_count;
  kdev_pg_location_table *_M_line_table;

private:
  kdev_pg_token_stream(kdev_pg_token_stream const &other);
  void operator = (kdev_pg_token_stream const &other);
};


#endif // KDEV_PG_TOKEN_STREAM_H

