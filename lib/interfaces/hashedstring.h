#ifndef HASHED_STRING_H
#define HASHED_STRING_H

#include<qstring.h>
#include<qdatastream.h>
#include<ksharedptr.h>
#include <ext/hash_map>


///A simple class that stores a string together with it's appropriate hash-key
class HashedString {
  public:
    HashedString() : m_hash( 0 ) {}

    HashedString( const QString& str ) : m_str( str ) {
      initHash();
    }
    
    HashedString( const char* str ) : m_str( str ) {
      initHash();
    }

    inline size_t hash() const {
      return m_hash;
    }

    QString str() const {
      return m_str;
    }

    bool operator == ( const HashedString& rhs ) const {
      if ( m_hash != rhs.m_hash )
        return false;
      return m_str == rhs.m_str;
    }

    ///Does not compare alphabetically, uses the hash-key for ordering.
    bool operator < ( const HashedString& rhs ) const {
      if ( m_hash < rhs.m_hash )
        return true;
      if ( m_hash == rhs.m_hash )
        return m_str < rhs.m_str;
      return false;
    }

    static size_t hashString( const QString& str );

  private:
    void initHash();

    QString m_str;
    size_t m_hash;

    friend QDataStream& operator << ( QDataStream& stream, const HashedString& str );
    friend QDataStream& operator >> ( QDataStream& stream, HashedString& str );
};

QDataStream& operator << ( QDataStream& stream, const HashedString& str );

QDataStream& operator >> ( QDataStream& stream, HashedString& str );

QDataStream& operator << ( QDataStream& stream, const bool& b );

QDataStream& operator >> ( QDataStream& stream, bool& b );

class HashedStringSetData;

///This is a reference-counting string-set optimized for fast lookup of hashed strings
class HashedStringSet {
  public:
    HashedStringSet();

    ~HashedStringSet();

    ///Constructs a string-set from one single file
    HashedStringSet( const HashedString& file );

    HashedStringSet( const HashedStringSet& rhs );

    HashedStringSet& operator = ( const HashedStringSet& rhs );
    ///@return whether the given file-name was included
    bool operator[] ( const HashedString& rhs ) const;

    void insert( const HashedString& str );

    ///Returns true if all files that are part of this set are also part of the given set
    HashedStringSet& operator +=( const HashedStringSet& );

    ///intersection-test
    bool operator <= ( const HashedStringSet& rhs ) const;

    bool operator == ( const HashedStringSet& rhs ) const;

    void read( QDataStream& stream );
    void write( QDataStream& stream ) const;

  size_t hash() const;
  private:
    void makeDataPrivate();
    KSharedPtr<HashedStringSetData> m_data; //this implies some additional cost because KShared's destructor is virtual. Maybe change that by copying KShared without the virtual destructor.
    friend HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs );
};

HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs );

namespace __gnu_cxx {
template<>
struct hash<HashedString> {
  size_t operator () ( const HashedString& str ) const {
    return str.hash();
  }
};
};

#endif
