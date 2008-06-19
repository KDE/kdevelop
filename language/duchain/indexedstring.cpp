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
#include "repositories/stringrepository.h"

namespace KDevelop {
//Using a function makes sure that initialization order cannot break anything
static Repositories::StringRepository& globalStringRepository() {
  ///@todo make the string-repository dynamically sizable
  static Repositories::StringRepository theGlobalStringRepository("String Index");
  return theGlobalStringRepository;
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
  else
    m_index = globalStringRepository().index(Repositories::StringRepositoryItemRequest(str, hash ? hash : hashString(str, length), length));
}

IndexedString::IndexedString( char c ) {
  m_index = 0xffff0000 | c;
}

IndexedString::IndexedString( const QString& string ) {
  QByteArray array(string.toUtf8());
  
  const char* str = array.constData();
  
  int size = array.size();
  
  if(!size)
    m_index = 0;
  else if(size == 1)
    m_index = 0xffff0000 | str[0];
  else
    m_index = globalStringRepository().index(Repositories::StringRepositoryItemRequest(str, hashString(str, size), size));
}

IndexedString::IndexedString( const char* str) {
  unsigned int length = strlen(str);
  if(!length)
    m_index = 0;
  else if(length == 1)
    m_index = 0xffff0000 | str[0];
  else
    m_index = globalStringRepository().index(Repositories::StringRepositoryItemRequest(str, hashString(str, length), length));
}

IndexedString::IndexedString( const QByteArray& str) {
  unsigned int length = str.length();
  if(!length)
    m_index = 0;
  else if(length == 1)
    m_index = 0xffff0000 | str[0];
  else
    m_index = globalStringRepository().index(Repositories::StringRepositoryItemRequest(str, hashString(str, length), length));
}

QString IndexedString::str() const {
  if(!m_index)
    return QString();
  else if((m_index & 0xffff0000) == 0xffff0000)
    return QString(QChar((char)m_index & 0xff));
  else
    return Repositories::stringFromItem(globalStringRepository().itemFromIndex(m_index));
}

int IndexedString::length() const {
  if(!m_index)
    return 0;
  else if((m_index & 0xffff0000) == 0xffff0000)
    return 1;
  else
    return *globalStringRepository().itemFromIndex(m_index);
}

QByteArray IndexedString::byteArray() const {
  if(!m_index)
    return QByteArray();
  else if((m_index & 0xffff0000) == 0xffff0000)
    return QString(QChar((char)m_index & 0xff)).toUtf8();
  else
    return Repositories::arrayFromItem(globalStringRepository().itemFromIndex(m_index));
}

unsigned int IndexedString::hashString(const char* str, unsigned short length) {
  RunningHash running;
  for(int a = length-1; a >= 0; --a) {
    running.append(*str);
    ++str;
  }
  return running.hash;
}


}
