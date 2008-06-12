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

#ifndef INDEXED_STRING_H
#define INDEXED_STRING_H

//krazy:excludeall=dpointer,inline

#include <QString>
#include "language/languageexport.h"

class QDataStream;

namespace KDevelop {
class IndexedString;
}

namespace KDevelop {

//Empty strings have an index of zero.
//Strings of length one are not put into the repository, but are encoded directly within the index:
//They are encoded like 0xffff00bb where bb is the byte of the character.
class KDEVPLATFORMLANGUAGE_EXPORT IndexedString {
 public:
  IndexedString();
  ///@param str must be a utf8 encoded string, does not need to be 0-terminated.
  ///@param length must be its length in bytes.
  ///@param hash must be a hash as constructed with the here defined hash functions. If it is zero, it will be computed.
  explicit IndexedString( const char* str, unsigned short length, unsigned int hash = 0 );

  ///Needs a zero terminated string. When the information is already available, try using the other constructor.
  explicit IndexedString( const char* str );

  explicit IndexedString( char c );
  
  ///When the information is already available, try using the other constructor. This is expensive.
  explicit IndexedString( const QString& str );

  ///When the information is already available, try using the other constructor. This is expensive.
  explicit IndexedString( const QByteArray& str );
  
  explicit IndexedString( unsigned int index ) : m_index(index) {
  }
  
  inline unsigned int hash() const {
    return m_index;
  }

  //The string is uniquely identified by this index. You can use it for comparison.
  inline unsigned int index() const {
    return m_index;
  }
  
  bool isEmpty() const {
    return m_index == 0;
  }
  
  //This is relatively expensive(needs a mutex lock, hash lookups, and eventual loading), so avoid it when possible.
  int length() const;

  ///Convenience function, avoid using it, it's relatively expensive
  QString str() const;
  
  ///Convenience function, avoid using it, it's relatively expensive(les expensive then str() though)
  QByteArray byteArray() const;

  bool operator == ( const IndexedString& rhs ) const {
    return m_index == rhs.m_index;
  }

  bool operator != ( const IndexedString& rhs ) const {
    return m_index != rhs.m_index;
  }
  
  ///Does not compare alphabetically, uses the index  for ordering.
  bool operator < ( const IndexedString& rhs ) const {
    return m_index < rhs.m_index;
  }
  
  //Use this to construct a hash-value on-the-fly
  //To read it, just use the hash member, and when a new string is started, call clear(.
  //This needs very fast performance(per character operation), so it must stay inlined.
  struct RunningHash {
    enum {
      HashInitialValue = 5381
    };
    
    RunningHash() : hash(HashInitialValue) { //We initialize the hash with zero, because we want empty strings to create a zero hash(invalid)
    }
    inline void append(const char c) {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    inline void clear() {
      hash = HashInitialValue;
    }
    unsigned int hash;
  };

  static unsigned int hashString(const char* str, unsigned short length);

 private:
   uint m_index;
};

KDEVPLATFORMLANGUAGE_EXPORT inline uint qHash( const KDevelop::IndexedString& str ) {
  return str.index();
}
}

#endif
