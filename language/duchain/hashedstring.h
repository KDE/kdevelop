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

#ifndef HASHED_STRING_H
#define HASHED_STRING_H

//krazy:exclude-all=dpointer,inline

#include<QString>
#include <languageexport.h>

class QDataStream;

namespace KDevelop {
class HashedString;
}

KDEVPLATFORMLANGUAGE_EXPORT QDataStream& operator << ( QDataStream& stream, const KDevelop::HashedString& str );

KDEVPLATFORMLANGUAGE_EXPORT QDataStream& operator >> ( QDataStream& stream, KDevelop::HashedString& str );

namespace KDevelop {

typedef uint HashType; ///@todo use at least 64 bit hash

/**
 * A simple class that stores a string together with it's appropriate hash-key
 *
 * The difference to a normal QString is that the hash is stored, used for sorting, and for equality-comparison.
 *
 * Since most of the member-functions boil down to a simply integer-comparison in the most common case, they are inlined for performance-reasons.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT HashedString {
 public:
  HashedString();

  HashedString( const QString& str );

  HashedString( const char* str );

  inline HashType hash() const {
    return m_hash;
  }

  QString str() const;

  HashedString& operator= ( const QString& str );

  HashedString& operator +=( const QString& str );

  HashedString& operator +=( const char* str );

  bool operator == ( const HashedString& rhs ) const;

  ///Does not compare alphabetically, uses the hash-key for ordering.
  bool operator < ( const HashedString& rhs ) const;

  static HashType hashString( const QString& str );

 private:
  void initHash();

  QString m_str;
  HashType m_hash;

  friend QDataStream& ::operator << ( QDataStream& stream, const HashedString& str );
  friend QDataStream& ::operator >> ( QDataStream& stream, HashedString& str );
};

}

inline uint qHash( const KDevelop::HashedString& str ) {
  return str.hash();
}

#endif
