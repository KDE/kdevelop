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

#ifndef VECTORTIMESTAMP_H
#define VECTORTIMESTAMP_H

#include <vector>
#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/level.hpp>
#include <iosfwd>

typedef int Timestamp;

class VectorTimestamp {
    std::vector<Timestamp> m_state;
    uint m_primaryIndex;
  public:
    VectorTimestamp( int primaryIndex = 0 ) : m_primaryIndex( 0 ) {
      setPrimaryIndex( primaryIndex );
    }

    ///copies the timestamp and increases on the given index by one
    VectorTimestamp( int primaryIndex, const VectorTimestamp& rhs );

    Timestamp primaryStamp() const;

    Timestamp operator[] ( const uint index ) const {
      if ( index < m_state.size() ) {
        return m_state[ index ];
      } else {
        return 0;
      }
    }

    void setStamp( uint index, int value ) {
      if ( index >= m_state.size() ) {
        m_state.resize( index + 1, 0 );
      }

      m_state[ index ] = value;
    }

    ///Maximizes every stamp with the other side
    void maximize( const VectorTimestamp& rhs );

    bool isZero() const;

    template <class Archive>
    void serialize( Archive& arch, const uint /*version*/ ) {
      arch & m_state;
      arch & m_primaryIndex;
    }

    /**
     * @return whether every timestamp is smaller then the according index of rhs
    * */
    bool smaller( const VectorTimestamp& rhs ) const ;

    /**
    * @return whether every timestamp is smaller or same then the according index of rhs
    * */
    bool smallerOrSame( const VectorTimestamp& rhs ) const ;

    bool isOneBigger( const VectorTimestamp& rhs ) const ;

    bool isOneSmaller( const VectorTimestamp& rhs ) const ;

    size_t hash() const ;

    bool operator == ( const VectorTimestamp& rhs ) const ;
    
    inline bool operator != ( const VectorTimestamp& rhs ) const  {
      return ! operator==( rhs );
    }

    ///The following two are useful for sorting in a map
    ///Very inefficient
    bool operator < ( const VectorTimestamp& rhs ) const ;

    ///Very inefficient
    bool operator <= ( const VectorTimestamp& rhs ) const ;

    void setPrimaryIndex( const uint index );

    uint primaryIndex() const {
      return m_primaryIndex;
    }

    void decrease() {
      --m_state[ m_primaryIndex ];
    }

    void increase() {
      ++m_state[ m_primaryIndex ];
    }

    void resize( uint sz ) {
      m_state.resize( sz, 0 );
    }

    uint size() const {
      return m_state.size();
    }

    std::string print() const;
};

std::ostream& operator << ( std::ostream& str, const VectorTimestamp& timestamp );

BOOST_CLASS_IMPLEMENTATION(VectorTimestamp, object_serializable)

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
