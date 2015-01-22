/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

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

#ifndef KDEVPLATFORM_KDEVHASH_H
#define KDEVPLATFORM_KDEVHASH_H

//Partial one at a time hash, starting with fnv prime
class KDevHash
{
public:
  enum {
      DEFAULT_SEED = 2166136261u
  };

  KDevHash(unsigned int hash = DEFAULT_SEED)
    : m_hash(hash)
  {}

  KDevHash(const KDevHash&) = delete;
  KDevHash& operator=(const KDevHash&) = delete;

  operator unsigned int() const
  {
    return m_hash;
  }

  template<typename T>
  KDevHash& operator<<(T addition)
  {
    m_hash = hash(reinterpret_cast<const char*>(&addition), sizeof(T), m_hash);
    return *this;
  }

  static unsigned int hash(const char* const data, const unsigned int size, unsigned int seed = DEFAULT_SEED)
  {
    for (unsigned int i = 0; i < size; ++i) {
      seed += data[i];
      seed += ( seed << 10 );
      seed ^= ( seed >> 6 );
    }
    return seed;
  }

private:
  unsigned int m_hash;
};

#endif //KDEVPLATFORM_KDEVHASH_H
