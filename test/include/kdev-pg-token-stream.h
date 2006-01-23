/* This file is part of kdev-pg
   Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef KDEV_PG_TOKEN_STREAM_H
#define KDEV_PG_TOKEN_STREAM_H

#include <cstdlib>

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
      _M_token_count(0)
  { 
    reset(); 
  }

  ~kdev_pg_token_stream()
  {
    if (_M_token_buffer)
      ::free(_M_token_buffer);
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

private:
  token_type *_M_token_buffer;
  std::size_t _M_token_buffer_size;
  std::size_t _M_index;
  std::size_t _M_token_count;

private:
  kdev_pg_token_stream(kdev_pg_token_stream const &other);
  void operator = (kdev_pg_token_stream const &other);
};


#endif // KDEV_PG_TOKEN_STREAM_H

