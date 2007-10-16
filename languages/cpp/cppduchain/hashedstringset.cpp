/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "hashedstringset.h"
#include <QDataStream>
#include <QSet>
#include <QTextStream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include<list>
#include<algorithm>
#include <iostream>
#include "kdebug.h"

//It needs to be measured whether this flag should be turned on or off. It seems just to move the complexity from one position to the other, without any variant being really better.
#define USE_HASHMAP

using namespace KDevelop;

class HashedStringSetData : public KShared {
  public:
      typedef QSet<HashedString> StringSet;
      
      StringSet m_files;
      mutable bool m_hashValid;
      mutable HashType m_hash;
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
  m_data->m_files += rhs.m_data->m_files;
  
  return *this;
}

HashedStringSet& HashedStringSet::operator -=( const HashedStringSet& rhs ) {
  if( !m_data ) return *this;
  if( !rhs.m_data ) return *this;

  makeDataPrivate();

  m_data->m_files -= rhs.m_data->m_files;

  return *this;
}

QString HashedStringSet::dump() const {
  QString ret;
  if( m_data ) {
  foreach( const HashedString& str, m_data->m_files )
    ret += str.str();
  ret += " ";
  }
  return ret;
}

void HashedStringSet::makeDataPrivate() {
  if ( !m_data ) {
    m_data = new HashedStringSetData();
    return ;
  }
  if ( ((int)m_data->ref) != 1 )
    m_data = new HashedStringSetData( *m_data );
}

bool HashedStringSet::operator[] ( const HashedString& rhs ) const {
  //if ( rhs.str() == "*" )
    //return true; /// * stands for "any file"
  if ( !m_data )
    return false;
  return m_data->m_files.find( rhs ) != m_data->m_files.end();
}

void HashedStringSet::insert( const HashedString& str ) {
  if( str.str().isEmpty() ) return;
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
  
  const HashedStringSetData::StringSet& otherSet( rhs.m_data->m_files );
  HashedStringSetData::StringSet::const_iterator end = rhs.m_data->m_files.end();
  HashedStringSetData::StringSet::const_iterator myEnd = m_data->m_files.end();

  for( HashedStringSetData::StringSet::const_iterator it = m_data->m_files.begin(); it != myEnd; ++it ) {
    HashedStringSetData::StringSet::const_iterator i = otherSet.find( *it );
    if( i == end ) return false;
  }
  return true;
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

HashType HashedStringSet::hash() const {
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
	  s << (*it).str().toAscii().data() << "\n";
	}
  }
  return s.str();
}

void HashedStringSetGroup::addSet( unsigned int id, const HashedStringSet& set ) {
  if( set.m_data && !set.m_data->m_files.empty() ) {
    m_sizeMap[ id ] = set.size();
    for( HashedStringSetData::StringSet::const_iterator it = set.m_data->m_files.begin(); it != set.m_data->m_files.end(); ++it ) {
      GroupMap::iterator itr = m_map.find( *it );
      if( itr == m_map.end() ) {
        itr = m_map.insert( std::make_pair( *it, ItemSet() ) ).first;
      }
      itr->second.insert( id );
    }
  } else {
    m_global.insert( id );
  }
}

void HashedStringSetGroup::disableSet( unsigned int id ) {
  m_disabled.insert( id );
}

void HashedStringSetGroup::enableSet( unsigned int id ) {
  m_disabled.erase( id );
}

bool HashedStringSetGroup::isDisabled( unsigned int id ) const {
  return m_disabled.find( id ) != m_disabled.end();
}

void HashedStringSetGroup::removeSet( unsigned int id ) {
  m_disabled.erase( id );
  m_global.erase( id );
  m_sizeMap.erase( id );
  for( GroupMap::iterator it = m_map.begin(); it != m_map.end(); ++it ) {
    it->second.erase( id );
  }
}

void HashedStringSetGroup::findGroups( HashedStringSet strings, ItemSet& target ) const {
  target.clear();
  if( !strings.m_data ) {
    std::set_difference( m_global.begin(), m_global.end(), m_disabled.begin(), m_disabled.end(), std::insert_iterator<ItemSet>( target, target.end() ) );
    return;
  }
  //This might yet be optimized by sorting the sets according to their size, and starting the intersectioning with the smallest ones.
  __gnu_cxx::hash_map<unsigned int, unsigned int> hitCounts;

  for( HashedStringSetData::StringSet::const_iterator it = strings.m_data->m_files.begin(); it != strings.m_data->m_files.end(); ++it ) {
    GroupMap::const_iterator itr = m_map.find( *it );
      if( itr == m_map.end() ) {
        //There are no string-sets that include the currently searched for string
        continue;
      }

      for( ItemSet::const_iterator it2 = itr->second.begin(); it2 != itr->second.end(); ++it2 ) {
        __gnu_cxx::hash_map<unsigned int, unsigned int>::iterator v = hitCounts.find( *it2 );
        if( v != hitCounts.end() ) {
          ++(*v).second;
        } else {
          hitCounts[*it2] = 1;
        }
      }
  }

  //Now count together all groups that are completely within the given string-set(their hitCount equals their size)
  ItemSet found;
  for( __gnu_cxx::hash_map<unsigned int, unsigned int>::const_iterator it = hitCounts.begin(); it != hitCounts.end(); ++it ) {
    if( (*it).second == (*m_sizeMap.find( (*it).first )).second )
      found.insert( (*it).first );
  }


  std::set_union( found.begin(), found.end(), m_global.begin(), m_global.end(), std::insert_iterator<ItemSet>( target, target.end() ) );

  target.swap( found );
  target.clear();
  std::set_difference( found.begin(), found.end(), m_disabled.begin(), m_disabled.end(), std::insert_iterator<ItemSet>( target, target.end() ) );
}


