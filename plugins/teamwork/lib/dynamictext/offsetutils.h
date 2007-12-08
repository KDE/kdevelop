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

#ifndef OFFSETUTILS_H
#define OFFSETUTILS_H

#include <map>
#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/map.hpp>
#include <string>
#include <iosfwd>
#include "dynamictextexport.h"

typedef int OffsetPosition;

using namespace std;

///Can be thrown by OffsetMap.
struct OffsetMapError {
  enum Type {
    Inconsistent
  };
  Type t;

  OffsetMapError( Type type );

  const char* asText() const;
};

///@todo there's a lot of room for optimizations in this.. but first make sure everything works correctly. Everything may throw DynamicTextError

class DYNAMICTEXT_EXPORT OffsetMap {
  public:
    void clear();

    OffsetMap( OffsetPosition position, int offset );

    OffsetMap();

    /**Returns true if the given map can be inserted without making the map inconsistent
     * */
    bool isCompatible( const OffsetMap& rhs ) const;

    /**Insert the given offset-map into this one(while transforming it through this one).
     * corresponds to the mathematical function-operator o (like g o f).
     * Let the function for the current map be f(x), the inserted be g(x), and the resulting map be h(x).
     * Then h(x) = f(g(x))
     *
     * This function may throw OffsetMapError if the insertion would make the map inconsistent.
     * 
     * */
    OffsetMap operator %( const OffsetMap& rhs ) const;

    OffsetMap& operator %=( const OffsetMap& rhs );

    /**Inverts the offset-map(like the mathematical inversion of a function)
     * */
  OffsetMap operator ~() const;

  OffsetMap engInvert() const;
  
    void printMap() const;

    std::string print() const;

    ///Applies the function defined by this offset-map to the given parameter
    int operator () ( const OffsetPosition position ) const;

    template <class Archive>
    void serialize( Archive& arch, const OffsetPosition ) {
      arch & m_offsets;
    }
  private:
  typedef std::map<OffsetPosition, int> OffsetInternalMap;
  
  ///Iterator may get deleted
  void makeConsistent( OffsetInternalMap::iterator position );

    int getOffset( OffsetPosition position ) const;

    void addOffset( OffsetPosition position, int offset );
    void mergeOffset( OffsetPosition position, int offset );

    OffsetInternalMap m_offsets;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
