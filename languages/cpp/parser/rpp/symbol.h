/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SYMBOL_H
#define SYMBOL_H

#include <cstring>
#include <utility>
#include <set>

struct name_symbol
{
  char const *data;
  std::size_t size;

  inline bool operator == (name_symbol const &other) const
  {
    return size == other.size
      && ::strncmp(data, other.data, size) == 0;
  }

  inline bool operator < (name_symbol const &other) const
  {
    return std::lexicographical_compare(data, data + size,
					other.data, other.data + other.size);
  }

protected:
  inline name_symbol() {}
  inline name_symbol(char const *d, std::size_t c)
    : data(d), size(c) {}

private:
  void operator = (name_symbol const &);

  friend class name_table;
};

class name_table
{
public:
  ~name_table()
  {
    for (std::set<name_symbol>::iterator it = _M_storage.begin();
	 it != _M_storage.end(); ++it)
      {
	char *data = const_cast<char*>((*it).data);
	delete[] data;
      }
  }

  inline name_symbol const *find(char const *data, std::size_t size) const
  {
    name_symbol key(data, size);
    std::set<name_symbol>::iterator it = _M_storage.find(key);

    if (it != _M_storage.end())
      return &*it;

    return 0;
  }

  inline name_symbol const *install(char const *data, std::size_t size)
  {
    name_symbol key(data, size);
    std::set<name_symbol>::iterator it = _M_storage.find(key);

    if (it != _M_storage.end())
      return &*it;

    char *allocated = new char[size + 1];
    allocated[size] = '\0';
    key.data = ::strncpy(allocated, data, size);

    return &*_M_storage.insert(key).first;
  }

  inline std::size_t size() const
  { return _M_storage.size(); }

private:
  std::set<name_symbol> _M_storage;
};

#endif // SYMBOL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
