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

#ifndef KDEVHASH_H
#define KDEVHASH_H

//Partial one at a time hash, starting with fnv prime
class KDevHash
{
public:
  KDevHash() : m_hash(2166136261u) {}
  KDevHash(uint hash) : m_hash(hash) {}
  operator unsigned int()
  {
    return m_hash;
  }
  template<typename T>
  KDevHash& operator<<(T addition)
  {
    const char *_data = (const char*)&addition;
    for (unsigned int i = 0; i < sizeof(T); ++i)
    {
      m_hash += _data[i];
      m_hash += ( m_hash << 10 );
      m_hash ^= ( m_hash >> 6 );
    }
    return *this;
  }
private:
  KDevHash(const KDevHash&);
  KDevHash operator=(const KDevHash&);
  unsigned int m_hash;
};

#endif //KDEVHASH_H
