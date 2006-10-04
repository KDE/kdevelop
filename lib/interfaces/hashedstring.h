#ifndef HASHED_STRING_H
#define HASHED_STRING_H

#include<qstring.h>

///A simple class that stores a string together with it's appropriate hash-key
class HashedString {
public:
HashedString() : m_hash(0) {
}
  
explicit HashedString( const QString& str ) : m_str( str ) {
  initHash();
}
  
  inline size_t hash() const {
    return m_hash;
  }

  QString str() const {
    return m_str;
  }

  bool operator == ( const HashedString& rhs ) const {
    if( m_hash != rhs.m_hash ) return false;
    return m_str == rhs.m_str;
  }

  ///Does not compare alphabetically, uses the hash-key for ordering.
  bool operator < ( const HashedString& rhs ) const {
    if( m_hash < rhs.m_hash ) return true;
    if( m_hash == rhs.m_hash )
      return m_str < rhs.m_str;
    return false;
  }
  
private:
  void initHash();
  
  QString m_str;
  size_t m_hash;
};

#endif
