/***************************************************************************
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "hashedstring.h"
#include <QDataStream>
#include <QHash>

using namespace KDevelop;

HashType HashedString::hashString(  const QString& str )
{
  return qHash( str );
}

void HashedString::initHash() {
  m_hash = hashString( m_str );
}

HashedString::HashedString() : m_hash( 0 ) {}

HashedString::HashedString( const QString& str ) : m_str( str ) {
  initHash();
}

HashedString::HashedString( const char* str ) : m_str( str ) {
  initHash();
}

HashedString& HashedString::operator= ( const QString& str ) {
  operator=(HashedString(str));
  return *this;
}

HashedString& HashedString::operator +=( const QString& str ) {
  m_str += str;
  initHash();
  return *this;
}

HashedString& HashedString::operator +=( const char* str ) {
  m_str += str;
  initHash();
  return *this;
}

QDataStream& operator << ( QDataStream& stream, const HashedString& str ) {
    stream << str.m_str;
    stream << str.m_hash;
    return stream;
}

QDataStream& operator >> ( QDataStream& stream, HashedString& str ) {
    stream >> str.m_str;
    stream >> str.m_hash;
    return stream;
}
