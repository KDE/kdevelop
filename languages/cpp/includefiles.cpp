#include"includefiles.h"

#include<ext/hash_set>
#include<algorithm>

namespace __gnu_cxx {
template<>
  struct hash<HashedString> {
    size_t operator () ( const HashedString& rhs ) const {
      return rhs.hash();
    }
  };
};

class HashedStringSetData : public KShared {
public:
  __gnu_cxx::hash_set<HashedString> m_files;
};

HashedStringSet::HashedStringSet() {
}

HashedStringSet::~HashedStringSet() {
}

HashedStringSet::HashedStringSet( const HashedString& file ) {
  insert( file );
}

HashedStringSet::HashedStringSet( const HashedStringSet& rhs ) : m_data( rhs.m_data ) {
}

HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs ) {
  HashedStringSet ret = lhs;
  ret += rhs;
  
  return ret;
}

HashedStringSet& HashedStringSet::operator = ( const HashedStringSet& rhs ) {
  m_data = rhs.m_data;
  return *this;
}
  
HashedStringSet& HashedStringSet::operator +=( const HashedStringSet& rhs ) {
  if( !rhs.m_data ) return *this;
  
  makeDataPrivate();
  m_data->m_files.insert( rhs.m_data->m_files.begin(), rhs.m_data->m_files.end() );
  return *this;
}

void HashedStringSet::makeDataPrivate() {
  if( !m_data ) {
    m_data = new HashedStringSetData();
    return;
  }
  if( m_data->_KShared_count() != 1 )
    m_data = new HashedStringSetData( *m_data );
}

bool HashedStringSet::operator[] ( const HashedString& rhs ) const {
  if( rhs.str() == "*" ) return true; /// * stands for "any file"
  if( !m_data ) return false;
  return m_data->m_files.find( rhs ) != m_data->m_files.end();
}

void HashedStringSet::insert( const HashedString& str ) {
  makeDataPrivate();
  m_data->m_files.insert( str );
}

bool HashedStringSet::operator <= ( const HashedStringSet& rhs ) const {
  if( !m_data ) return true;
  if( m_data->m_files.empty() ) return true;
  if( !rhs.m_data ) return false;
  return std::includes( rhs.m_data->m_files.begin(), rhs.m_data->m_files.end(), m_data->m_files.begin(), m_data->m_files.end() );
}

bool HashedStringSet::operator == ( const HashedStringSet& rhs ) const {
  bool empty1 = false;
  if( !m_data )
    empty1 = true;
  else if( m_data->m_files.empty() )
    empty1 = true;
  bool empty2 = false;
  if( !rhs.m_data )
    empty2 = true;
  else if( rhs.m_data->m_files.empty() )
    empty2 = true;
  
  if( empty1 && empty2 ) return true;
  if( empty1 || empty2 ) return false;
  
  return m_data->m_files == rhs.m_data->m_files;
}
