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

#include "offsetutils.h"
#include <iostream>
#include <sstream>
#include "verify.h"

OffsetMapError::OffsetMapError( Type type ) : t( type ) {}

const char* OffsetMapError::asText() const {
  switch ( t ) {
    case Inconsistent:
    return "the map is inconsistent";
    default:
    return "unknown";
  }
}

void OffsetMap::clear() {
  m_offsets.clear();
}

OffsetMap::OffsetMap( OffsetPosition position, int offset ) {
  addOffset( position, offset );
}

OffsetMap::OffsetMap() {};

/**Returns true if the given map can be inserted without making the map inconsistent
 * */
bool OffsetMap::isCompatible( const OffsetMap& rhs ) const {
  OffsetMap rhsInv = ~rhs;

  if ( m_offsets.empty() )
    return true;

  for ( std::map<OffsetPosition, int>::const_iterator it = m_offsets.begin(); it != m_offsets.end(); ++it ) {
    OffsetPosition i = rhsInv( ( *it ).first );
    if ( rhs( i ) != ( *it ).first )
      return false;
  }

  for ( std::map<OffsetPosition, int>::const_reverse_iterator it = rhs.m_offsets.rbegin(); it != rhs.m_offsets.rend(); ++it ) {
    int endPos = ( *it ).first + ( *it ).second;
    if ( endPos < 0 )
      return false;

    std::map< OffsetPosition, int >::const_iterator it2 = m_offsets.lower_bound( ( OffsetPosition ) endPos );

    if ( it2 != m_offsets.begin() ) {
      --it2;
      if ( ( *it2 ).second + ( *it2 ).first > endPos )
        return false;
    }
  }
  return true;
}

OffsetMap OffsetMap::operator %( const OffsetMap& rhs ) const {
  OffsetMap ret;
  OffsetMap rhsInv = ~rhs;

  for ( std::map<OffsetPosition, int>::const_iterator it = m_offsets.begin(); it != m_offsets.end(); ++it ) {
    OffsetPosition i = rhsInv( ( *it ).first ); // - rhs.getOffset( ( *it ).first );
    ret.m_offsets[ i ] = ( *it ).second;
  }

  for ( std::map<OffsetPosition, int>::const_reverse_iterator it = rhs.m_offsets.rbegin(); it != rhs.m_offsets.rend(); ++it )
    ret.addOffset( ( *it ).first, ( *it ).second );
  return ret;
}

OffsetMap& OffsetMap::operator %=( const OffsetMap& rhs ) {
  OffsetMap rhsInv = ~rhs;
  std::map<OffsetPosition, int> oldOffsets;
  oldOffsets.swap( m_offsets );

  for ( std::map<OffsetPosition, int>::const_iterator it = oldOffsets.begin(); it != oldOffsets.end(); ++it ) {
    OffsetPosition i = rhsInv( ( *it ).first );
    DYN_VERIFY_SAME( rhs( i ), ( *it ).first );
    m_offsets[ i ] = ( *it ).second;
  }

  for ( std::map<OffsetPosition, int>::const_reverse_iterator it = rhs.m_offsets.rbegin(); it != rhs.m_offsets.rend(); ++it )
    addOffset( ( *it ).first, ( *it ).second );
  return *this;
}

/**Inverts the offset-map(like the mathematical inversion of a function)
 * */
OffsetMap OffsetMap::operator ~() const {
  OffsetMap ret;
  std::map<OffsetPosition, int>& newMap( ret.m_offsets );
  for ( std::map<OffsetPosition, int>::const_iterator it = m_offsets.begin(); it != m_offsets.end(); ++it ) {
    if( (*it).second > 0 ) {
      for( int a = 1; a <= (*it).second; ++a ) {
        int npos = ( *it ).first + a;
        std::map<OffsetPosition, int>::iterator it2 = newMap.find( npos );
        if ( it2 == newMap.end() ) {
          newMap[ npos ] = -1;
        } else {
          ( *it2 ).second -= 1;
        }
      }
    } else {
      for( int a = -1; a >= (*it).second; --a ) {
        int npos = ( *it ).first + a;
        std::map<OffsetPosition, int>::iterator it2 = newMap.find( npos );
        if ( it2 == newMap.end() ) {
          newMap[ npos ] = -a;
        } else {
          ( *it2 ).second -= a;
        }
      }
    }
    
    /*int npos = ( *it ).first + ( *it ).second;
    std::map<OffsetPosition, int>::iterator it2 = newMap.find( npos );
    if ( it2 == newMap.end() ) {
      newMap[ npos ] = -( *it ).second;
    } else {
      ( *it2 ).second -= ( *it ).second;
    }*/
  }

  //cout << "inverting. old map size: " << newMap.size() << " new map size: " << m_offsets.size() << endl;
  return ret;
}

OffsetMap OffsetMap::engInvert() const {
  OffsetMap ret;
  std::map<OffsetPosition, int>& newMap( ret.m_offsets );
  for ( std::map<OffsetPosition, int>::const_iterator it = m_offsets.begin(); it != m_offsets.end(); ++it ) {
    
  int npos = ( *it ).first;
  if( ( *it ).second < 0 )
    --npos;
  else if( ( *it ).second > 0 )
    ++npos;
  
    std::map<OffsetPosition, int>::iterator it2 = newMap.find( npos );
    if ( it2 == newMap.end() ) {
      newMap[ npos ] = -( *it ).second;
    } else {
      ( *it2 ).second -= ( *it ).second;
    }
  }
  
  //cout << "inverting. old map size: " << newMap.size() << " new map size: " << m_offsets.size() << endl;
  return ret;
}

void OffsetMap::printMap() const {
  cout << "offsets: " << print() << endl;
}

std::string OffsetMap::print() const {
  ostringstream o;
  for ( std::map<OffsetPosition, int>::const_iterator it = m_offsets.begin(); it != m_offsets.end(); ++it )
    o << "(" << ( *it ).first << ": " << ( *it ).second << ") ";
  return o.str();
}

int OffsetMap::getOffset( OffsetPosition position ) const {

	std::map<OffsetPosition, int>::const_iterator it = m_offsets.begin();

	int ret = 0;
	while ( it != m_offsets.end() && ( *it ).first <= position ) {
		ret += ( *it ).second;
		++it;
	}
	return ret;
}

int OffsetMap::operator () ( const OffsetPosition position ) const {
  return getOffset( position ) + position;
}

void OffsetMap::addOffset( OffsetPosition position, int offset ) {
  /*if ( ( offset + int( position ) ) < 0 || offset == 0 )
	return ;*/
  //cout << "inserting offset " << offset << " at position " << position << endl;
	OffsetInternalMap::iterator it = m_offsets.find( position );
  if ( it == m_offsets.end() ) {
		it = m_offsets.insert( OffsetInternalMap::value_type( position, offset ) ).first;
  } else {
		(*it).second += offset;
		if ( (*it).second == 0 )
			m_offsets.erase( it );
  }
}

void OffsetMap::mergeOffset( OffsetPosition position, int offset ) {
  OffsetInternalMap::iterator it = m_offsets.find( ( OffsetPosition ) position );
  if( it != m_offsets.end() ) {
    (*it).second += offset;
    makeConsistent( it );
  } else {
    makeConsistent( m_offsets.insert( OffsetInternalMap::value_type( position,  offset ) ).first );
  }
}

void OffsetMap::makeConsistent( OffsetInternalMap::iterator position ) {
  std::map< OffsetPosition, int >::iterator it = position;
  bool forward = (*position).second > 0;

  OffsetPosition until = (*position).first + (*position).second;

  if( forward ) {
    ///seek forwards and include all overlapped offets into this one
    ++it;
    while( it != m_offsets.end() && (*it).first <= until ) {
      (*position).second += (*it).second;
      m_offsets.erase( it++ );
    }
  } else {
    ///seek backwards and include this one into any overlapped offset, deleting this one.

    if( it != m_offsets.begin() ) {
      --it;
      while( (*it).first >= until ) {
        (*it).second += (*position).second;
        m_offsets.erase( position );
        position = it;
        if( it != m_offsets.begin() )
          --it;
        else
          break;
      }
    }
  }

  if( (*position).second == 0 )
    m_offsets.erase( position );
}


// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
