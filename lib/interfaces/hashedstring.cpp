#include "hashedstring.h"
#include <kdatastream.h>
#include <sstream>
size_t fastHashString( const QString& str );

size_t hashStringSafe( const QString& str ) {
  size_t hash = 0;
  int len = str.length();
  for( int a = 0; a < len; a++ ) {
    hash = str[a].unicode() + (hash * 17);
  }
  return hash;
}

size_t HashedString::hashString(  const QString& str )
{
	return fastHashString( str );
}

size_t fastHashString( const QString& str ) {
  size_t hash = 0;
  if( !str.isEmpty() ) {
    const QChar* curr = str.unicode();
    const QChar* end = curr + str.length();
    QChar c;
    for(; curr < end ;) {
      c = *curr;
      hash = c.unicode() + ( hash * 17 );
      ++curr;
    }
  }
  return hash;
}

void HashedString::initHash() {
    m_hash = hashString( m_str );
}

#include<ext/hash_set>
#include<set>
#include<algorithm>

class HashedStringSetData : public KShared {
  public:
      //typedef __gnu_cxx::hash_set<HashedString> StringSet;
      typedef std::set<HashedString> StringSet; //must be a set, so the set-algorithms work
      StringSet m_files;
  mutable bool m_hashValid;
      mutable size_t m_hash;
      HashedStringSetData() : m_hashValid( false ) {
      }
  inline void invalidateHash() {
    m_hashValid = false;
  }

  void computeHash() const;
};

void HashedStringSetData::computeHash() const {
  int num = 1;
  m_hash = 0;
  for( StringSet::const_iterator it = m_files.begin(); it != m_files.end(); ++it ) {
    num *= 7;
    m_hash += num * (*it).hash();
  }
  m_hashValid = true;
}

HashedStringSet::HashedStringSet() {}

HashedStringSet::~HashedStringSet() {}

HashedStringSet::HashedStringSet( const HashedString& file ) {
  insert( file );
}

HashedStringSet::HashedStringSet( const HashedStringSet& rhs ) : m_data( rhs.m_data ) {}

HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs ) {
  HashedStringSet ret = lhs;
  ret += rhs;

  return ret;
}

int HashedStringSet::size() const {
    if( !m_data ) return 0;
    return m_data->m_files.size();
}

HashedStringSet& HashedStringSet::operator = ( const HashedStringSet& rhs ) {
  m_data = rhs.m_data;
  return *this;
}

HashedStringSet& HashedStringSet::operator +=( const HashedStringSet& rhs ) {
  if ( !rhs.m_data )
    return * this;

  makeDataPrivate();
  m_data->m_files.insert( rhs.m_data->m_files.begin(), rhs.m_data->m_files.end() );
  m_data->invalidateHash();
  return *this;
}

void HashedStringSet::makeDataPrivate() {
  if ( !m_data ) {
    m_data = new HashedStringSetData();
    return ;
  }
  if ( m_data->_KShared_count() != 1 )
    m_data = new HashedStringSetData( *m_data );
}

bool HashedStringSet::operator[] ( const HashedString& rhs ) const {
  if ( rhs.str() == "*" )
    return true; /// * stands for "any file"
  if ( !m_data )
    return false;
  return m_data->m_files.find( rhs ) != m_data->m_files.end();
}

void HashedStringSet::insert( const HashedString& str ) {
  makeDataPrivate();
  m_data->m_files.insert( str );
  m_data->invalidateHash();
}

bool HashedStringSet::operator <= ( const HashedStringSet& rhs ) const {
  if ( !m_data )
    return true;
  if ( m_data->m_files.empty() )
    return true;
  if ( !rhs.m_data )
    return false;
  return std::includes( rhs.m_data->m_files.begin(), rhs.m_data->m_files.end(), m_data->m_files.begin(), m_data->m_files.end() );
}

bool HashedStringSet::operator == ( const HashedStringSet& rhs ) const {
  if( hash() != rhs.hash() ) return false;
  
  bool empty1 = false;
  if ( !m_data )
    empty1 = true;
  else if ( m_data->m_files.empty() )
    empty1 = true;
  bool empty2 = false;
  if ( !rhs.m_data )
    empty2 = true;
  else if ( rhs.m_data->m_files.empty() )
    empty2 = true;

  if ( empty1 && empty2 )
    return true;
  if ( empty1 || empty2 )
    return false;

  return m_data->m_files == rhs.m_data->m_files;
}

size_t HashedStringSet::hash() const {
  if( !m_data ) return 0;
  if( !m_data->m_hashValid ) m_data->computeHash();
  return m_data->m_hash;
}

void HashedStringSet::read( QDataStream& stream ) {
    bool b;
    stream >> b;
    if( b ) {
        m_data = new HashedStringSetData();
        int cnt;
        stream >> cnt;
        HashedString s;
        for( int a = 0; a < cnt; a++ ) {
            stream >> s;
            m_data->m_files.insert( s );
        }
    } else {
        m_data = 0;
    }
}

void HashedStringSet::write( QDataStream& stream ) const {
    bool b = m_data;
    stream << b;
    if( b ) {
        int cnt = m_data->m_files.size();
        stream << cnt;
        for( HashedStringSetData::StringSet::const_iterator it = m_data->m_files.begin(); it != m_data->m_files.end(); ++it ) {
            stream << *it;
        }
    }
}

std::string HashedStringSet::print() const {
  std::ostringstream s;
  if( m_data ) {
        for( HashedStringSetData::StringSet::const_iterator it = m_data->m_files.begin(); it != m_data->m_files.end(); ++it ) {
	  s << (*it).str().ascii() << "\n";
	}
  }
  return s.str();
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

