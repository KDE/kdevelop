/* This file is part of KDevelop
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef SYMBOL_H
#define SYMBOL_H

#include <cstring>
#include <ext/hash_set>

struct NameSymbol
{
  const char *data;
  std::size_t count;

  inline bool operator == (const NameSymbol &other) const
  { return count == other.count && memcmp(data, other.data, count) == 0; }

protected:
  inline NameSymbol() {}
  inline NameSymbol(const char *d, std::size_t c)
    : data(d), count(c) {}

private:
  void operator = (const NameSymbol &);

  friend class NameTable;
};

namespace __gnu_cxx
{

template<>
struct hash<const NameSymbol*>
{
  inline std::size_t operator() (const NameSymbol * const &name) const
  { return reinterpret_cast<std::size_t>(name); }
};


template <>
struct hash<NameSymbol>
{
  inline std::size_t operator()(const NameSymbol &r) const
  {
    std::size_t hash_value = 0;

    for (unsigned i=0; i<r.count; ++i)
      hash_value = (hash_value << 5) - hash_value + r.data[i];

    return hash_value;
  }
};

} // namespace __gnu_cxx



struct NameTable
{
  inline const NameSymbol *findOrInsert(const char *str, std::size_t len)
  {
    NameSymbol key(str, len);
    __gnu_cxx::hash_set<NameSymbol>::iterator it = storage.find(key);

    if (it != storage.end())
      return &*it;

    return &*storage.insert(key).first;
  }

  inline std::size_t count() const
  { return storage.size(); }

private:
  __gnu_cxx::hash_set<NameSymbol> storage;
};

inline void printSymbol(const NameSymbol *n)
{
  std::size_t c = n->count;
  const char *s = n->data;

  while (c-- != 0)
    putchar(*s++);
}

#endif // SYMBOL_H
