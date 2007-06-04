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

#ifndef CROSSMAPIMPL_H
#define CROSSMAPIMPL_H

#include "metahelpers.h"

namespace Utils {
using namespace Meta;

template <class Value, class Key, class KeyExtractor, int num, int relativeNum>
struct KeyValue {
  KeyValue( const Value& value ) : val( KeyExtractor::template Extractor<Key, relativeNum, Value>::getKey( value ) ) {}
  Key val;
};

struct DummyChainEle {
  template <class Type>
  DummyChainEle( const Type& /*t*/ ) {}
}
;

///KeyValueChain stores a copy of all key-values in the given list(and can extract them during the constructor from the Value-type)
template <class Value, class Keys, class KeyExtractor, int num = 0, class AllKeys = Keys>
struct KeyValueChain : public If < ( Count< Keys >::value > 1 ), KeyValueChain < Value, typename RemoveOneLeft<Keys>::Result, KeyExtractor, num + 1, AllKeys>, DummyChainEle>::Result, public KeyValue< Value, typename GetOneLeft<Keys>::Result, KeyExtractor, num, FindRelativeInList<typename GetOneLeft<Keys>::Result, AllKeys, num>::relativeIndex > {
  typedef typename If< ( Count< Keys >::value > 1 ), KeyValueChain < Value, typename RemoveOneLeft<Keys>::Result, KeyExtractor, num + 1, AllKeys > , DummyChainEle > ::Result NextChain;
  typedef KeyValue< Value, typename GetOneLeft<Keys>::Result, KeyExtractor, num, FindRelativeInList<typename GetOneLeft<Keys>::Result, AllKeys, num>::relativeIndex > KeyValueItem;

  KeyValueChain( const Value& value ) : NextChain( value ), KeyValueItem( value ) {}
  KeyValueChain() {}

  ///Sets the value of the n'th key
  template<int n>
  void setValue( typename GetListItem< n, AllKeys >::Result& val ) {
    typedef typename GetListItem< n, AllKeys >::Result KeyType;
    static_cast<KeyValue< Value, KeyType, KeyExtractor, n, FindRelativeInList<KeyType, AllKeys, n>::relativeIndex >*>( this )->val = val;
  }

  ///Returns the value of the num'th key
  template<int n>
  typename GetListItem< n, AllKeys >::Result& getValue() {
    typedef typename GetListItem< n, AllKeys >::Result KeyType;
    return static_cast<KeyValue< Value, KeyType, KeyExtractor, n, FindRelativeInList<KeyType, AllKeys, n>::relativeIndex >*>( this )->val;
  }
};

struct ExtractKeyType {
  template<class Type>
  struct Action {
    typedef Type Result;
    typedef False isContainer;
  };
};


template <class Value, class Key, int num>
struct CrossMapData {
  enum {
    number = num
  };

  typedef typename ExtractKeyType::template Action< Key >::Result RealKey;
  typedef std::multimap< RealKey, uint > MapType;
  typedef typename ExtractKeyType::template Action< Key >::isContainer IsContainer;
  
  MapType map;
  
  template<class KeyExtractor, int relativeNum>
  void insert( const KeyValue< Value, Key, KeyExtractor, num, relativeNum>& keyVal, const Value & /*value*/, const uint id ) {
    insertInternal( keyVal.val, id, IsContainer() );
  }

  ///Not yet ported to the container-stuff
  template <class KeyExtractor, int relativeNum, class Validator>
  uint find( const KeyValue< Value, Key, KeyExtractor, num, relativeNum>& keyVal, const Value & value, const Validator & validator ) {
    return findInternal( keyVal.val, value, validator, IsContainer() );
  }

  template <class KeyExtractor, int relativeNum>
  void remove( const KeyValue< Value, Key, KeyExtractor, num, relativeNum>& keyVal, uint id ) {
    removeInternal( keyVal.val, id, IsContainer() );
  }
    
  void clear() {
    map.clear();
  }

  bool hasContent() const {
    return !map.empty();
  }

  template <class Archive>
  void serialize( Archive & arch, uint /*version*/ ) {
    arch & map;
  }

  private:

  template<class Validator>
  uint findInternal( const Key& key, const Value & value, const Validator & validator, const True ) {
    for( typename Key::const_iterator it = key.begin(); it != key.end(); ++it ) {
      uint ret = findInternal( *it, value, validator, False() );
      if( ret ) return ret;
    }
    return 0;
  }
  
  template<class Validator>
  uint findInternal( const RealKey& key, const Value & value, const Validator & validator, const False ) {
    std::pair<typename MapType::iterator, typename MapType::iterator>
    range = map.equal_range( key );

    while ( range.first != range.second ) {
      if ( validator.validateId( value, ( *range.first ).second ) )
        return ( *range.first ).second;

      ++range.first;
    }

    return 0;
  }


    void insertInternal( const Key& key, const uint id, const True ) {
      for( typename Key::const_iterator it = key.begin(); it != key.end(); ++it )
        insertInternal( *it, id, False() );
    }

    void insertInternal( const RealKey& key, const uint id, const False ) {
      map.insert( make_pair( key, id ) );
    }
    
    void removeInternal( const Key& key, const uint id, const True ) {
      for( typename Key::const_iterator it = key.begin(); it != key.end(); ++it )
        removeInternal( *it, id, False() );
    }

    void removeInternal( const RealKey& key, const uint id, const False ) {
      std::pair<typename MapType::iterator, typename MapType::iterator>
          range = map.equal_range( key );
  
  
      while ( range.first != range.second ) {
        if ( id == ( *range.first ).second ) {
          map.erase( range.first );
          return ;
        }
  
        ++range.first;
      }
    }
};

struct CrossMapChainDummy {

  template<class C, class D, class V>
  uint find( const C&, const D&, const V& ) {
    return 0;
  }

  template<class C>
  void remove( const C&, uint ) {}

  template<class C, class D>
  void insert( const C&, const D&, uint ) {}

  void clear() {}

  bool hasContent() const {
    return false;
  }
  template<class Archive>
  void serialize( Archive & /*arch*/, uint /*version*/ ) {}
};

template<class Value, class Keys, int num>
struct CrossMapChain : public If<( Count<Keys>::value > 1 ), CrossMapChain< Value, typename RemoveOneLeft<Keys>::Result, num + 1>, CrossMapChainDummy>::Result, public CrossMapData< Value, typename GetOneLeft<Keys>::Result, num >
{
  typedef typename If<( Count<Keys>::value > 1 ), CrossMapChain< Value, typename RemoveOneLeft<Keys>::Result, num + 1>, CrossMapChainDummy>::Result Next;
  typedef CrossMapData< Value, typename GetOneLeft<Keys>::Result, num > Data;

  template<class KeyExtractor, class AllKeys>
  void insert( const KeyValueChain< Value, Keys, KeyExtractor, num, AllKeys >& values, const Value & value, uint valueId ) {
    Next::template insert( values, value, valueId )
    ;
    Data::template insert( values, value, valueId )
    ;
  }
  template <class KeyExtractor, class AllKeys>
  void remove( const KeyValueChain< Value, Keys, KeyExtractor, num, AllKeys>& values, uint id ) {
    Next::template remove( values, id )
      ;
    Data::template remove( values, id )
      ;
  }

  template <class KeyExtractor, class AllKeys, class Validator>
  uint find( const KeyValueChain< Value, Keys, KeyExtractor, num, AllKeys>& values, const Value & value, const Validator & validator ) {
    uint ret = Data::template find( values, value, validator )
               ;
    if ( ret )
      return ret;

    return Next::template find( values, value, validator )
    ;
  }

  int hasContent() const {
    bool a = Next::hasContent()
    ;
    bool b = Data::hasContent()
    ;
    if ( a != b && Count<Keys>::value >
         1 )
      return 2;
    else if ( a && b )
      return 1;
    else
      return 0;
  }

  void clear() {
    Next::clear();
    Data::clear();
  }

  template <class Archive>
  void serialize( Archive & arch, uint /*version*/ ) {
    Data::serialize( arch, 0 );
    Next::serialize( arch, 0 );
  }
};

template <class Type>
class CrossMapHashTraits {
    enum {
      hashable = 0
  };
};

}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
