/***************************************************************************
copyright            : (C) 2006 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "vectortimestamp.h"
#include <iostream>
#include <ext/hash_map>
#include <sstream>


std::ostream& operator << ( std::ostream& str, const VectorTimestamp& timestamp ) {
  str << timestamp.print();
  return str;
}

namespace __gnu_cxx {
template <>
struct hash<VectorTimestamp> {
  size_t operator() ( const VectorTimestamp& x ) {
    return x.hash();
  }
};
}

///Maximizes every stamp with the other side
void VectorTimestamp::maximize( const VectorTimestamp& rhs ) {
  if ( rhs.m_state.size() > m_state.size() )
    m_state.resize( rhs.m_state.size(), 0 );
  for ( uint a = 0; a < rhs.m_state.size(); a++ ) {
    if ( rhs.m_state[ a ] > m_state[ a ] )
      m_state[ a ] = rhs.m_state[ a ];
  }
}

bool VectorTimestamp::isZero() const {
  for ( uint a = 0; a < m_state.size(); a++ )
    if ( m_state[ a ] != 0 )
      return false;
  return true;
}



///copies the timestamp and increases on the given index by one
VectorTimestamp::VectorTimestamp( int primaryIndex, const VectorTimestamp& rhs ) : m_state( rhs.m_state ), m_primaryIndex( 0 ) {
  setPrimaryIndex( primaryIndex );
  increase();
}

Timestamp VectorTimestamp::primaryStamp() const {
	if ( m_state.size() <= m_primaryIndex )
    return 0;
  return ( *this ) [ m_primaryIndex ];
}
/**
 * @return whether every timestamp is smaller then the according index of rhs
* */
bool VectorTimestamp::smaller( const VectorTimestamp& rhs ) const {
  uint sz = size();
  if ( rhs.size() > sz )
    sz = rhs.size();
  if ( sz == 0 )
    return false;

  for ( uint a = 0; a < sz; a++ ) {
    Timestamp s1 = ( *this ) [ a ];
    Timestamp s2 = rhs[ a ];
    if ( s1 < s2 )
      continue;
    else
      return false;
  }
  return true;
}

/**
 * @return whether every timestamp is smaller or same then the according index of rhs
* */
bool VectorTimestamp::smallerOrSame( const VectorTimestamp& rhs ) const {
  uint sz = size();
  if ( rhs.size() > sz )
    sz = rhs.size();
  for ( uint a = 0; a < sz; a++ ) {
    Timestamp s1 = ( *this ) [ a ];
    Timestamp s2 = rhs[ a ];
    if ( s1 <= s2 )
      continue;
    else
      return false;
  }
  return true;
}

bool VectorTimestamp::isOneBigger( const VectorTimestamp& rhs ) const {
  uint sz = size();
  if ( rhs.size() > sz )
    sz = rhs.size();
  for ( uint a = 0; a < sz; a++ ) {
    Timestamp s1 = ( *this ) [ a ];
    Timestamp s2 = rhs[ a ];
    if ( s1 > s2 )
      return true;
  }
  return false;
}

bool VectorTimestamp::isOneSmaller( const VectorTimestamp& rhs ) const {
  uint sz = size();
  if ( rhs.size() > sz )
    sz = rhs.size();
  for ( uint a = 0; a < sz; a++ ) {
    Timestamp s1 = ( *this ) [ a ];
    Timestamp s2 = rhs[ a ];
    if ( s1 < s2 )
      return true;
  }
  return false;
}

size_t VectorTimestamp::hash() const {
  size_t s = 0;
  uint sz = m_state.size();
  for ( uint a = 0; a < sz; a++ ) {
    s += ( a * 17 ) * m_state[ a ];
  }
  return s;
}

bool VectorTimestamp::operator == ( const VectorTimestamp& rhs ) const {
  uint sz = size();
  if ( rhs.size() > sz )
    sz = rhs.size();
  for ( uint a = 0; a < sz; a++ ) {
    if ( ( *this ) [ a ] != rhs[ a ] )
      return false;
  }
  return true;
}

///The following two are useful for sorting in a map
///Very inefficient
bool VectorTimestamp::operator < ( const VectorTimestamp& rhs ) const {
  uint sz = size();
  if ( rhs.size() > sz )
    sz = rhs.size();
  for ( uint a = 0; a < sz; a++ ) {
    Timestamp s1 = ( *this ) [ a ];
    Timestamp s2 = rhs[ a ];
    if ( s1 < s2 )
      return true;
    else if ( s1 > s2 )
      return false;
  }
  return false;
}

///Very inefficient
bool VectorTimestamp::operator <= ( const VectorTimestamp& rhs ) const {
  uint sz = size();
  if ( rhs.size() > sz )
    sz = rhs.size();
  for ( uint a = 0; a < sz; a++ ) {
    Timestamp s1 = ( *this ) [ a ];
    Timestamp s2 = rhs[ a ];
    if ( s1 < s2 )
      return true;
    else if ( s1 > s2 )
      return false;
  }
  return true;
}

void VectorTimestamp::setPrimaryIndex( const uint index ) {
  m_primaryIndex = index;
  if ( m_state.size() < index + 1 )
    m_state.resize( index + 1, 0 );
}

std::string VectorTimestamp::print() const {
  std::string ret = "[";
  for ( std::vector<Timestamp>::const_iterator it = m_state.begin(); it != m_state.end(); ++it ) {
    if ( it != m_state.begin() )
      ret += ", ";
    std::ostringstream o;
    o << *it;
    ret += o.str();
  }
  ret += "]";

  return ret;
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
