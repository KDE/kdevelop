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

#include "referencecountedindexedstring.h"
#include "repositories/stringrepository.h"

#include <kurl.h>

namespace KDevelop {
namespace {
  
struct ReferenceCountedStringData {
  unsigned short length;
  unsigned int refCount;
  unsigned int itemSize() const {
    return sizeof(ReferenceCountedStringData) + length;
  }
  unsigned int hash() const {
    IndexedString::RunningHash running;
    const char* str = ((const char*)this) + sizeof(ReferenceCountedStringData);
    for(int a = length-1; a >= 0; --a) {
      running.append(*str);
      ++str;
    }
    return running.hash;
    
  }
};

struct ReferenceCountedStringRepositoryItemRequest {

  //The text is supposed to be utf8 encoded
  ReferenceCountedStringRepositoryItemRequest(const char* text, unsigned int hash, unsigned short length) : m_hash(hash), m_length(length), m_text(text) {
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
    return sizeof(ReferenceCountedStringData) + m_length;
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  void createItem(ReferenceCountedStringData* item) const {
    item->length = m_length;
    item->refCount = 0;
    ++item;
    memcpy(item, m_text, m_length);
  }
  
  //Should return whether the here requested item equals the given item
  bool equals(const ReferenceCountedStringData* item) const {
    return item->length == m_length && (memcmp(++item, m_text, m_length) == 0);
  }
  unsigned int m_hash;
  unsigned short m_length;
  const char* m_text;
};

typedef ItemRepository<ReferenceCountedStringData, ReferenceCountedStringRepositoryItemRequest, ReferenceCounting, true> ReferenceCountedStringRepository;

///@param item must be valid(nonzero)
inline QString stringFromItem(const ReferenceCountedStringData* item) {
  const unsigned short* textPos = (unsigned short*)(item+1);
  return QString::fromUtf8((char*)textPos, item->length);
}

inline QByteArray arrayFromItem(const ReferenceCountedStringData* item) {
  const unsigned short* textPos = (unsigned short*)(item+1);
  return QByteArray((char*)textPos, item->length);
}  
}

RepositoryManager< ReferenceCountedStringRepository > globalReferenceCountedStringRepository("Reference Counted String Index");

ReferenceCountedIndexedString::ReferenceCountedIndexedString() : m_index(0) {
}

///@param str must be a utf8 encoded string, does not need to be 0-terminated.
///@param length must be its length in bytes.
ReferenceCountedIndexedString::ReferenceCountedIndexedString( const char* str, unsigned short length, unsigned int hash ) {
  if(!length)
    m_index = 0;
  else if(length == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    
    m_index = globalReferenceCountedStringRepository->index(ReferenceCountedStringRepositoryItemRequest(str, hash ? hash : hashString(str, length), length));
    ++globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
  }
}

ReferenceCountedIndexedString::ReferenceCountedIndexedString( char c ) {
  m_index = 0xffff0000 | c;
}

ReferenceCountedIndexedString::ReferenceCountedIndexedString( const KUrl& url ) {
  QByteArray array(url.pathOrUrl().toUtf8());

  const char* str = array.constData();

  int size = array.size();

  if(!size)
    m_index = 0;
  else if(size == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    m_index = globalReferenceCountedStringRepository->index(ReferenceCountedStringRepositoryItemRequest(str, hashString(str, size), size));
    ++globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
  }
}

ReferenceCountedIndexedString::ReferenceCountedIndexedString( const QString& string ) {
  QByteArray array(string.toUtf8());

  const char* str = array.constData();

  int size = array.size();

  if(!size)
    m_index = 0;
  else if(size == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    m_index = globalReferenceCountedStringRepository->index(ReferenceCountedStringRepositoryItemRequest(str, hashString(str, size), size));
    ++globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
  }
}

ReferenceCountedIndexedString::ReferenceCountedIndexedString( const char* str) {
  unsigned int length = strlen(str);
  if(!length)
    m_index = 0;
  else if(length == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    m_index = globalReferenceCountedStringRepository->index(ReferenceCountedStringRepositoryItemRequest(str, hashString(str, length), length));
    ++globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
  }
}

ReferenceCountedIndexedString::ReferenceCountedIndexedString( const IndexedString& str) : m_index(0) {
  *this = ReferenceCountedIndexedString(str.byteArray());
}

ReferenceCountedIndexedString::ReferenceCountedIndexedString( const QByteArray& str) {
  unsigned int length = str.length();
  if(!length)
    m_index = 0;
  else if(length == 1)
    m_index = 0xffff0000 | str[0];
  else {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    m_index = globalReferenceCountedStringRepository->index(ReferenceCountedStringRepositoryItemRequest(str, hashString(str, length), length));
    ++globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
  }
}

ReferenceCountedIndexedString::~ReferenceCountedIndexedString() {
  if(m_index && (m_index & 0xffff0000) != 0xffff0000) {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    uint refCount = --globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
    if(!refCount)
      globalReferenceCountedStringRepository->deleteItem(m_index);
  }
}

ReferenceCountedIndexedString::ReferenceCountedIndexedString( const ReferenceCountedIndexedString& rhs ) : m_index(rhs.m_index) {
  if(m_index && (m_index & 0xffff0000) != 0xffff0000) {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    ++globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
  }
}

ReferenceCountedIndexedString& ReferenceCountedIndexedString::operator=(const ReferenceCountedIndexedString& rhs) {
  if(m_index == rhs.m_index)
    return *this;
  if(m_index && (m_index & 0xffff0000) != 0xffff0000) {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    uint refCount = --globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
    if(!refCount)
      globalReferenceCountedStringRepository->deleteItem(m_index);
  }
  
  m_index = rhs.m_index;
  
  if(m_index && (m_index & 0xffff0000) != 0xffff0000) {
    QMutexLocker lock(globalReferenceCountedStringRepository->mutex());
    ++globalReferenceCountedStringRepository->dynamicItemFromIndex(m_index)->refCount;
  }
  
  return *this;
}


KUrl ReferenceCountedIndexedString::toUrl() const {
  KUrl url( str() );
  return url;
}

QString ReferenceCountedIndexedString::str() const {
  if(!m_index)
    return QString();
  else if((m_index & 0xffff0000) == 0xffff0000)
    return QString(QChar((char)m_index & 0xff));
  else
    return stringFromItem(globalReferenceCountedStringRepository->itemFromIndex(m_index));
}

int ReferenceCountedIndexedString::length() const {
  if(!m_index)
    return 0;
  else if((m_index & 0xffff0000) == 0xffff0000)
    return 1;
  else
    return globalReferenceCountedStringRepository->itemFromIndex(m_index)->length;
}

QByteArray ReferenceCountedIndexedString::byteArray() const {
  if(!m_index)
    return QByteArray();
  else if((m_index & 0xffff0000) == 0xffff0000)
    return QString(QChar((char)m_index & 0xff)).toUtf8();
  else
    return arrayFromItem(globalReferenceCountedStringRepository->itemFromIndex(m_index));
}

unsigned int ReferenceCountedIndexedString::hashString(const char* str, unsigned short length) {
  RunningHash running;
  for(int a = length-1; a >= 0; --a) {
    running.append(*str);
    ++str;
  }
  return running.hash;
}


}
