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

#ifndef REFERENCECOUNTEDINDEXEDSTRING_H
#define REFERENCECOUNTEDINDEXEDSTRING_H

//krazy:excludeall=dpointer,inline

#include <QtCore/QString>
#include "../languageexport.h"

class QDataStream;
class KUrl;

namespace KDevelop {
class ReferenceCountedIndexedString;
}

namespace KDevelop {
class IndexedString;
///This is a version of IndexedString that features reference-counting.
///This means that data is removed from the repository again, once it is not used by any
///ReferenceCountedIndexedString. This should always be used when the strings are very long,
///when the strings are only temporary, or when you generally consider them to be not needed any more from some point.
///
///The price is slightly worse performance, much more disk-activity for storing/loading because the
///static reference-counts need to be updated, and 4 additional bytes used for each string.
///Empty strings have an index of zero.
///Strings of length one are not put into the repository, but are encoded directly within the index:
///They are encoded like 0xffff00bb where bb is the byte of the character.
class KDEVPLATFORMLANGUAGE_EXPORT ReferenceCountedIndexedString {
 public:
  ReferenceCountedIndexedString();
  ///@param str must be a utf8 encoded string, does not need to be 0-terminated.
  ///@param length must be its length in bytes.
  ///@param hash must be a hash as constructed with the here defined hash functions. If it is zero, it will be computed.
  explicit ReferenceCountedIndexedString( const char* str, unsigned short length, unsigned int hash = 0 );

  ///Needs a zero terminated string. When the information is already available, try using the other constructor.
  explicit ReferenceCountedIndexedString( const char* str );

  explicit ReferenceCountedIndexedString( char c );

  ///When the information is already available, try using the other constructor. This is expensive.
  explicit ReferenceCountedIndexedString( const QString& str );

  ///When the information is already available, try using the other constructor. This is expensive.
  explicit ReferenceCountedIndexedString( const QByteArray& str );
  
  explicit ReferenceCountedIndexedString( const IndexedString& str );

  explicit ReferenceCountedIndexedString( unsigned int index ) : m_index(index) {
  }
  
  ~ReferenceCountedIndexedString();
  
  ReferenceCountedIndexedString( const ReferenceCountedIndexedString& str );
  
  ReferenceCountedIndexedString& operator=(const ReferenceCountedIndexedString& rhs);

  ///Creates an indexed string from a KUrl, this is expensive.
  explicit ReferenceCountedIndexedString( const KUrl& url );

  ///Re-construct a KUrl from this indexed string, the result can be used with the
  ///KUrl-using constructor. This is expensive.
  KUrl toUrl() const;

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

  bool operator == ( const ReferenceCountedIndexedString& rhs ) const {
    return m_index == rhs.m_index;
  }

  bool operator != ( const ReferenceCountedIndexedString& rhs ) const {
    return m_index != rhs.m_index;
  }

  ///Does not compare alphabetically, uses the index  for ordering.
  bool operator < ( const ReferenceCountedIndexedString& rhs ) const {
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

KDEVPLATFORMLANGUAGE_EXPORT inline uint qHash( const KDevelop::ReferenceCountedIndexedString& str ) {
  return str.index();
}
}

#endif
