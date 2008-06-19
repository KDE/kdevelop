/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef STRINGREPOSITORY_H
#define STRINGREPOSITORY_H

#include "itemrepository.h"
#include <QString>

namespace Repositories {
using namespace KDevelop;
struct StringRepositoryItemRequest {

  //The text is supposed to be utf8 encoded
  StringRepositoryItemRequest(const char* text, unsigned int hash, unsigned short length) : m_hash(hash), m_length(length), m_text(text) {
  }
  
  enum {
    AverageSize = 10 //This should be the approximate average size of an Item
  };

  typedef unsigned int HashType;
  
  //Should return the hash-value associated with this request(For example the hash of a string)
  HashType hash() const {
    return m_hash;
  }
  
  //Should return the size of an item created with createItem
  size_t itemSize() const {
    return sizeof(unsigned short) + m_length;
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  void createItem(unsigned short* item) const {
    *item = m_length;
    ++item;
    memcpy(item, m_text, m_length);
  }
  
  //Should return whether the here requested item equals the given item
  bool equals(const unsigned short* item) const {
    return *item == m_length && (memcmp(++item, m_text, m_length) == 0);
  }
  unsigned int m_hash;
  unsigned short m_length;
  const char* m_text;
};

typedef ItemRepository<unsigned short, StringRepositoryItemRequest, true> StringRepository;

///@param item must be valid(nonzero)
QString stringFromItem(const unsigned short* item) {
  const unsigned short* textPos = item;
  ++textPos;
  return QString::fromUtf8((char*)textPos, (int)*item);
}

QByteArray arrayFromItem(const unsigned short* item) {
  const unsigned short* textPos = item;
  ++textPos;
  return QByteArray((char*)textPos, (int)*item);
}

}
#endif
