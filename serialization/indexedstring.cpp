/***************************************************************************
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "indexedstring.h"
#include "serialization/stringrepository.h"

#include <kurl.h>
#include "referencecounting.h"

namespace KDevelop {
namespace {

struct IndexedStringData {
  unsigned short length;
  unsigned int refCount;
  unsigned int itemSize() const {
    return sizeof(IndexedStringData) + length;
  }
  unsigned int hash() const {
    IndexedString::RunningHash running;
    const char* str = ((const char*)this) + sizeof(IndexedStringData);
    for(int a = length-1; a >= 0; --a) {
      running.append(*str);
      ++str;
    }
    return running.hash;

  }
};

inline void increase(uint& val) {
  ++val;
}

inline void decrease(uint& val) {
  --val;
}

struct IndexedStringRepositoryItemRequest {

  //The text is supposed to be utf8 encoded
  IndexedStringRepositoryItemRequest(const char* text, unsigned int hash, unsigned short length) : m_hash(hash), m_length(length), m_text(text) {
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
  uint itemSize() const {
    return sizeof(IndexedStringData) + m_length;
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  void createItem(IndexedStringData* item) const {
    item->length = m_length;
    item->refCount = 0;
    ++item;
    memcpy(item, m_text, m_length);
  }

  static void destroy(IndexedStringData* item, KDevelop::AbstractItemRepository&) {
    Q_UNUSED(item);
    //Nothing to do here (The object is not intelligent)
  }

  static bool persistent(const IndexedStringData* item) {
    return (bool)item->refCount;
  }

  //Should return whether the here requested item equals the given item
  bool equals(const IndexedStringData* item) const {
    return item->length == m_length && (memcmp(++item, m_text, m_length) == 0);
  }
  unsigned int m_hash;
  unsigned short m_length;
  const char* m_text;
};

typedef ItemRepository<IndexedStringData, IndexedStringRepositoryItemRequest, false, true> IndexedStringRepository;

///@param item must be valid(nonzero)
inline QString stringFromItem(const IndexedStringData* item) {
  const unsigned short* textPos = (unsigned short*)(item+1);
  return QString::fromUtf8((char*)textPos, item->length);
}

inline QByteArray arrayFromItem(const IndexedStringData* item) {
  const unsigned short* textPos = (unsigned short*)(item+1);
  return QByteArray((char*)textPos, item->length);
}

inline const char* c_strFromItem(const IndexedStringData* item) {
  return (const char*)(item+1);
}
}

namespace {
  RepositoryManager< IndexedStringRepository >& getGlobalIndexedStringRepository(){
    static RepositoryManager< IndexedStringRepository > globalIndexedStringRepository("String Index");
    return globalIndexedStringRepository;
  }
}

IndexedString::IndexedString() : m_index(0) {
}

///@param str must be a utf8 encoded string, does not need to be 0-terminated.
///@param length must be its length in bytes.
IndexedString::IndexedString( const char* str, unsigned short length, unsigned int hash ) {
  if(!length)
    m_index = 0;
  else if(length == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());

    m_index = getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(str, hash ? hash : hashString(str, length), length));

    if(shouldDoDUChainReferenceCounting(this))
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
  }
}

IndexedString::IndexedString( char c ) {
  m_index = 0xffff0000 | c;
}

IndexedString::IndexedString( const KUrl& url ) {
  QByteArray array(url.pathOrUrl().toUtf8());

  const char* str = array.constData();

  int size = array.size();

  if(!size)
    m_index = 0;
  else if(size == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
    m_index = getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(str, hashString(str, size), size));

    if(shouldDoDUChainReferenceCounting(this))
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
  }
}

IndexedString::IndexedString( const QString& string ) {
  QByteArray array(string.toUtf8());

  const char* str = array.constData();

  int size = array.size();

  if(!size)
    m_index = 0;
  else if(size == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
    m_index = getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(str, hashString(str, size), size));

    if(shouldDoDUChainReferenceCounting(this))
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
  }
}

IndexedString::IndexedString( const char* str) {
  unsigned int length = strlen(str);
  if(!length)
    m_index = 0;
  else if(length == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
    m_index = getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(str, hashString(str, length), length));

    if(shouldDoDUChainReferenceCounting(this))
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
  }
}

IndexedString::IndexedString( const QByteArray& str) {
  unsigned int length = str.length();
  if(!length)
    m_index = 0;
  else if(length == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
    m_index = getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(str, hashString(str, length), length));

    if(shouldDoDUChainReferenceCounting(this))
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
  }
}

IndexedString::~IndexedString() {
  if(m_index && (m_index & 0xffff0000) != 0xffff0000) {
    if(shouldDoDUChainReferenceCounting(this)) {
      QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());

      decrease(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
  }
}

IndexedString::IndexedString( const IndexedString& rhs ) : m_index(rhs.m_index) {
  if(m_index && (m_index & 0xffff0000) != 0xffff0000) {
    if(shouldDoDUChainReferenceCounting(this)) {
      QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
  }
}

IndexedString& IndexedString::operator=(const IndexedString& rhs) {
  if(m_index == rhs.m_index)
    return *this;
  if(m_index && (m_index & 0xffff0000) != 0xffff0000) {

    if(shouldDoDUChainReferenceCounting(this)) {
      QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
      decrease(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
  }

  m_index = rhs.m_index;

  if(m_index && (m_index & 0xffff0000) != 0xffff0000) {
    if(shouldDoDUChainReferenceCounting(this)) {
      QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
  }

  return *this;
}


KUrl IndexedString::toUrl() const {
  KUrl url( str() );
  return url;
}

QString IndexedString::str() const {
  if(!m_index)
    return QString();
  else if((m_index & 0xffff0000) == 0xffff0000)
    return QString(QChar((char)m_index & 0xff));
  else
    return stringFromItem(getGlobalIndexedStringRepository()->itemFromIndex(m_index));
}

int IndexedString::length() const {
  return lengthFromIndex(m_index);
}

int IndexedString::lengthFromIndex(unsigned int index) {
  if(!index)
    return 0;
  else if((index & 0xffff0000) == 0xffff0000)
    return 1;
  else
    return getGlobalIndexedStringRepository()->itemFromIndex(index)->length;
}

const char* IndexedString::c_str() const
{
  if(!m_index)
    return 0;
  else if((m_index & 0xffff0000) == 0xffff0000)
    return ((char*)&m_index)+3; //The last byte contains the character
  else
    return c_strFromItem(getGlobalIndexedStringRepository()->itemFromIndex(m_index));

}

QByteArray IndexedString::byteArray() const {
  if(!m_index)
    return QByteArray();
  else if((m_index & 0xffff0000) == 0xffff0000)
    return QString(QChar((char)m_index & 0xff)).toUtf8();
  else
    return arrayFromItem(getGlobalIndexedStringRepository()->itemFromIndex(m_index));
}

unsigned int IndexedString::hashString(const char* str, unsigned short length) {
  RunningHash running;
  for(int a = length-1; a >= 0; --a) {
    running.append(*str);
    ++str;
  }
  return running.hash;
}

uint IndexedString::indexForString( const char* str, short unsigned length, uint hash) {
  if(!length)
    return 0;
  else if(length == 1)
    return 0xffff0000 | str[0];
  else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
    return getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(str, hash ? hash : hashString(str, length), length));
  }
}

uint IndexedString::indexForString(const QString& str, uint hash) {
  const QByteArray array(str.toUtf8());
  return indexForString(array.constBegin(), array.size(), hash);
}

}

QDebug operator<<(QDebug s, const KDevelop::IndexedString& string)
{
  s.nospace() << string.str();
  return s.space();
}
