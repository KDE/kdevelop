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

///This file defines a multi-indexed set(called CrossMap) similar to boost's multi-index, but with simpler api, with support for index-lists, and less efficient.

#ifndef CROSSMAP_H
#define CROSSMAP_H


#include "statictree.h"
#include <map>
#include <string>
#include <ext/hash_map>
#include <iostream>
#include <list>
#include "crossmapimpl.h"


#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/hash_map.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/map.hpp> 

namespace Utils {

using namespace Meta;
using namespace Tree;
using namespace __gnu_cxx;
using namespace std;
//#define hash_map map

///This can be specialized to get the keys non-intrusively.
template <class KeyType, int number, class ItemType>
KeyType getCrossMapKey( const ItemType& item ) {
  return item.template getKey<KeyType, number>();
}

/**A similar structure may be given to CrossMap to extract the keys in a different way, or the
 * Extractor-member may be specialized. Advantage: partial specialization is possible. */

struct StandardCrossMapKeyExtractor {
  template <class KeyType, int number, class ItemType>
  struct Extractor {
    static KeyType getKey( const ItemType& item ) {
      return getCrossMapKey<KeyType, number, ItemType>( item );
    }
  };
};

///For the trivial identity no special extraction has to be specialized if StandardCrossMapKeyExtractor is used.
template <class Type>
struct StandardCrossMapKeyExtractor::Extractor< Type, 0, Type > {
  static const Type& getKey( const Type& item ) {
    return item;
  }
};

/**For very simple cases, specialization of getCrossMapKey can be simplified by using this macro.
 * For writing the expression the value is available as a const reference named "value", and the result
 * of the expression is used as key.
 * Example:
 * CROSSMAP_KEY_EXTRACTOR( MyValueType, MyValueType::Id, 0, value.id() );
 * */
#define CROSSMAP_KEY_EXTRACTOR( ValueType, KeyType, Number, Expression ) \
namespace Utils { \
template <>     \
KeyType getCrossMapKey< KeyType, Number, ValueType>( const ValueType& value ) {    \
  return Expression;    \
}           \
}          \


/**This macro can be used to create small wrapper-types that hold another type,

* to access the type in the crossmap in a unique way
* @example:
* NAMED_TYPE( UserId, uint );
* ...;
* User v = map.value<UserId>( 5 ); ///Gets the user with user-id 5*/

#define NAMED_TYPE(Name, type) \
struct Name { type value; operator type&() { return value; }; static const char* name() { return #Name; }; Name( const type& v ) : value(v) {}; /*Name& operator = ( const type& t ) { value = t; };*/ Name() {}; bool operator < ( const Name& rhs )  const { return value < rhs.value; } template<class T> Name( const T& t ) : value( t ) {} template<class T> bool operator == ( const T& rhs ) const { return value == rhs.value; } }


/**This may be specialized to define own container-types that should not be used as keys directly, but from which subkeys should be extracted
 * */

#define CROSSMAP_DEFINE_CONTAINER( Cnt ) \
namespace Utils {  \
template<class Type>                                  \
struct ExtractKeyType::Action<Cnt<Type> > {         \
  typedef Type Result;                              \
  typedef True isContainer;                         \
};                                                  \
}    \


template<class KeyList>
struct RealKeys {
  typedef typename ForEachInList<ExtractKeyType, KeyList>::Result Result;
};


///Not used yet, but should be
#define SET_HASHABLE(Type) template<> Utils::CrossMapHashTraits<Type> { enum { hashable = 1 } };

///This can be used to mark types that should be managed using a hash_multimap instead of a sorted multimap
template <class Type>
struct Hashed {}
;

class CrossMapId {
    uint id_;
  public:
    CrossMapId() : id_( 0 ) {}

    CrossMapId( uint id ) : id_( id ) {}

    uint id() {
      return id_;
    }

    bool operator == ( const CrossMapId & rhs ) const {
      return id_ == rhs.id_;
    }

    CrossMapId& operator = ( const CrossMapId & rhs ) {
      id_ = rhs.id_;
      return *this;
    }

    bool operator < ( const CrossMapId & rhs ) const {
      return id_ < rhs.id_;
    }

    bool operator <= ( const CrossMapId & rhs ) const {
      return id_ <= rhs.id_;
    }

    template <class Archive>
    void serialize( Archive & arch, uint version ) {
      arch & id_;
    }

    operator uint() const {
      return id_;
    }
};

/**This is a kind of in-memory database that allows storing/sorting items by an arbitrary count of keys
 * and key-types. The keys are extracted in
 * the moment an item is inserted. If the keys change, update(..) should be called on the item.
 * 
 *
 * It is very similar to the boost multi-index, but the api is a bit easier.
 *
 * Advantage against boost's multi-index: One item can be reachable by many indices in the same category,
 * by using CROSSMAP_DEFINE_CONTAINER(..) and giving such a key-container as to the key-list.
 * Then the item will be searchable by each key in the list.
 * (If the value-type is used as key, it will be filled with the key's value by default and no key-extractor needs to be defined for that case)
 * (Example: multiple filenames may be associated to users, and it is possible to get a list of users for each filename)
 *
 * Disadvantage: Less efficient, bigger binary
 *
 * @param Value The value-type, should be a type with a default-constructor(is used as Dummy for Functions like operator[] if the key cannot be found)
 * @param Keys a meta-list containing all keys(using BIND_LIST or Binder<...,...>
 * @param KeyExtractor (optional) a class that helps extracting the keys from the values
 * @param rebuildAfterLoad whether serialization should only store a list of values, without the additional maps and the cross-link-data etc.(then every key has to be re-extracted after the archive was loaded, and the structure may be different)
 */

template <class Value, class Keys, class KeyExtractor = StandardCrossMapKeyExtractor>
class CrossMap {
    typedef CrossMap< Value, Keys, KeyExtractor> Self;
    typedef KeyValueChain< Value, Keys, KeyExtractor> KeyValues;
    typedef typename RealKeys<Keys>::Result RealKeys;
    typedef CrossMapId Id;

    template <bool cacheKeys>
    struct ValueStore {
      Value value;
      KeyValues keyValues;
      ValueStore( const Value & v ) : value( v ), keyValues( v ) {}
      ValueStore& operator = ( const Value & v ) {
        value = v;
        keyValues = KeyValues( v )
                    ;
        return *this;
      }

      operator Value& () {
        return value;
      }
    };

    typedef ValueStore< true > MyValueStore;

    bool rebuildAfterLoad_; ///@todo Not yet used

  public:

    typedef hash_map< uint, MyValueStore > ValueMap;

    /**Special care has to be taken when a ValueEditor-object is copied: When a ValueEditor is copied,
     * the source will not update the map on it's destruction, and all copies of a ValueEditor that does
     * not update the map won't do it neither. So generally it's safer to use a ValueEditor in-place, without copying it around. */

    class ValueEditor {
        Id id_;
        Self* map_;
        Value* value_;
      public:
        ValueEditor( Self* map, const Id id, Value* value ) : id_( id ), map_( map ), value_( value ) {}

        ValueEditor( const ValueEditor& e ) : id_( e.id_ ), map_( e.map_ ), value_( e.value_ ) {
          const_cast<ValueEditor&>( e ).id_ = 0; ///This is done so the destruction of the other ValueEditor doesn't update the map
        }

        ValueEditor& operator = ( const ValueEditor& rhs ) {
          map_ = rhs.map_;
          value_ = rhs.value_;
          id_ = rhs.id_;
          const_cast<ValueEditor&>( rhs ).id_ = 0; ///This is done so the destruction of the other ValueEditor doesn't update the map
        }

        ~ValueEditor() {
          if ( id_ )
            map_->update( id_ );
        }

        ///It is safe to edit a value through this operator. It is not safe to take the address or reference for further use.
        Value* operator ->() {
          return value_;
        }
    };

    ///An iterator represents a list of Items that can be iterated over. It depends on the map that created it.
    class Iterator {
        typedef std::list<uint> IdList;
      public:
        Iterator( Self * map = 0 ) : it_( ids_.begin() ), map_( map ) {}

        operator bool() const {
          return map_ && it_ != ids_.end();
        }

        Iterator& operator ++ () {
          ++it_;
          return *this;
        }

        const Value& operator *() const {
          return map_->valueFromId( *it_ );
        }

        const Value* operator ->() const {
          return & map_->valueFromId( *it_ );
        }

        /**Returns a ValueEditor-object that can be used to directly edit the values in-place comfortably and automatically update the map.
         * @see ValueEditor::operator ->           */
        ValueEditor edit() {
          return ValueEditor( map_, *it_, &map_->valueFromId( *it_ ) );
        }

        ///expensive
        uint count() const {
          return ids_.size();
        }

        Id id() const {
          return * it_;
        }

        ///Must be called after the first ids are added, and can be called any time again to jump back to the beginning
        void restart() {
          it_ = ids_.begin();
        }

        operator Id() const {
          return * it_;
        }

        ///Compares the current value pointed to
        bool operator == ( const Iterator & rhs ) const {
          bool end1 = it_ == ids_.end();
          bool end2 = rhs.it_ == rhs.ids_.end();
          if ( end1 != end2 )
            return false;
          if ( end1 )
            return true;
          return *it_ == *rhs.it_;
        }

      private:
        void addId( uint id ) {
          ids_.push_back( id );
        }


        IdList::const_iterator it_;
        Self* map_;
        IdList ids_;
        friend class CrossMap< Value, Keys, KeyExtractor>;
    };


    /**
     * @param rebuildAfterLoad Whether the set should be stored compact(only the values themselves, without all the helper-maps). Not used yet.
     * */
    CrossMap( bool rebuildAfterLoad = true ) : rebuildAfterLoad_( rebuildAfterLoad ), currentId_( 1 ) {}

    /**The Functions that directly return a Value-Reference return a default-constructed dummy if the item cannot be found.
     * These are very simple convenience-functions that directly return the first matching item. iterator is much more powerful.
     * */
    template <class KeyType>
    const Value & operator[] ( const KeyType& key ) const {
      return value<0, KeyType>( key );
    }

    /**This is an expensive convenience-function. It returns a temporary object that allows access to the value, and once that object is destroyed
     * automatically updated the map. (That allows assignment-operations like mymap[5].name = "Honk").
     * But it is very expensive since every time the temporary object is destroyed, all keys are reevaluated and all maps are updated.
     * */

    /*
    template <class KeyType>
    ValueEditor<Value> & operator[] ( const KeyType& key ) {

      return value<0, KeyType>( key );
    }*/


    template <int keyNum, class KeyType>
    const Value& value( const KeyType & key ) const {
      typedef std::multimap<KeyType, uint> Map;
      const Map& map = getMap<keyNum, KeyType>();
      typename Map::const_iterator it = map.find( key );
      if ( it != map.end() ) {
        typename ValueMap::const_iterator itt = values_.find( ( *it ).second );
        if ( itt != values_.end() ) {
          return ( *itt ).second.value;
        }
      }
      return dummy_;
    }

    Iterator values() const {
      Iterator ret( const_cast<Self*>( this ) );
      for ( typename ValueMap::const_iterator it = values_.begin(); it != values_.end(); ++it )
        ret.addId( ( *it ).first );

      ret.restart();
      return ret;
    };

    template <int keyNum, class KeyType>
    Iterator values( const KeyType & key ) const {
      typedef std::multimap<KeyType, uint> Map;
      const Map& map = getMap<keyNum, KeyType>();
      std::pair<typename Map::const_iterator, typename Map::const_iterator> range = map.equal_range( key );
      Iterator ret( const_cast<Self*>
                    ( this ) );

      while ( range.first != range.second ) {
        ret.addId( ( *range.first ).second )
        ;
        ++range.first;
      }

      ret.restart();
      return ret;
    }

    template <class KeyType>
    Iterator values( const KeyType & key ) const {
      return values<0> ( key );
    }

    ///For none-hashed value-types, this is no additional expense agains unordered retrieval. Returns them ordered by the given key.
    template <int keyNum, class KeyType>
    Iterator orderedValues( const KeyType & key ) const {
      typedef std::multimap<KeyType, uint> Map;
      const Map& map = getMap<keyNum, KeyType>();
      std::pair<typename Map::const_iterator, typename Map::const_iterator> range = map.equal_range( key );
      Iterator ret( const_cast<Self*>
                    ( this ) );

      while ( range.first != range.second ) {
        ret.addId( ( *range.first ).second )
        ;
        ++range.first;
      }

      ret.restart();
      return ret;
    }

    ///For none-hashed value-types, this is no additional expense agains unordered retrieval.
    template <class KeyType>
    Iterator orderedValues( const KeyType & key ) const {
      return orderedValues<0> ( key );
    }

    template <int keyNum, class KeyType>
    Iterator orderedValues() const {
      Iterator ret( const_cast<Self*>( this ) );
      typedef std::multimap<KeyType, uint> Map;

      const Map& m = getMap<keyNum, KeyType>();
      for( typename Map::const_iterator it = m.begin(); it != m.end(); ++it )
        ret.addId( (*it).second );
      
      ret.restart();
      return ret;
    }

    template<class KeyType>
    Iterator orderedValues() const {
      return orderedValues<0, KeyType>();
    }
    
    
    const Value& valueFromId( Id id ) const {
      typename ValueMap::const_iterator itt = values_.find( id );
      if ( itt != values_.end()
         )
        return ( *itt ).second.value;
      else
        return dummy_;
    }

    template <class KeyType>
    const Value& value( const KeyType & key ) const {
      return value< 0, KeyType >( key );
    }

    ///Creates a copy of all key-values which can be used to reference items. This might be used to store a consistent representation of the keys a value was stored under.
    static KeyValues copyKeyValues( const Value & value ) {
      return KeyValues( value );
    }

    ///When KeyValues are given from outside, they do not have to be recomputed by using the KeyExtractor.
    Id insert( const KeyValues & keyValues, const Value & value ) {
      uint id = currentId_++;
      values_.insert( make_pair( id, value ) );
      insertToData( keyValues, value, id );
      return id;
    }

    Id insert( const Value & value ) {
      KeyValues keys( value );
      return insert( keys, value );
    }

    ///Returns the Id of the removed Value, or zero on fail
    Id remove
      ( Id id ) {
      if ( id ) {
        typename ValueMap::iterator it = values_.find( id );
        if ( it == values_.end() )
          return 0;

        removeFromData( ( *it ).second.keyValues, id );
        values_.erase( id );
        return id;
      } else {
        return 0;
      }
    }

    ///Removes the item the iterator currently points at
    bool remove
      ( const Iterator& it ) {
      if ( !it )
        return false;
      return ( bool ) remove
               ( it.id() );
    }

    ///Removes all items the iterator iterates over
    uint removeAll( const Iterator& it ) {
      if ( !it )
        return false;
      Iterator i = it;
      i.restart();
      uint ret = 0;
      while ( i ) {
        if ( remove
               ( i.id() ) ) ret++;
      }
      return ret;
    }

    ///Returns the Id of the removed Value, or zero on fail
    Id remove
      ( const KeyValues & keyValues, const Value & value ) {
      Id id = find( keyValues, value );
      return remove
               ( id );
    }

    ///Returns the Id of the removed Value, or zero on fail
    Id remove
      ( const Value & value ) {
      KeyValues keys( value );
      return remove
               ( keys, value );
    }

    ///@return 0 if the item was not found, else the id of a matching value */
    Id find( const KeyValues & values, const Value & value ) {
      return data_.template find( values, value, *this );
    }

    /**Tries to get the internal entry-Id of the Value by extracting the value's keys,
     * searching, and using the identity-test to verify that the found value is the same.
     * If all key-values changed since the item was inserted, or it changed in a way that
     * the identity-test(operator ==) fails, it cannot be found.(For that reason
     * it may be useful to choose an additional key above the really necessary ones,
     * that never changes, as the first key.(For example the address of a pointer).
     * @return 0 if the item was not found, else the id of a matching value */
    Id find( const Value & value ) {
      KeyValues keys( value );
      return find( keys, value );
    }

    ///@return 0 if there is no content, 1 if there is content, and 2 if the set is inconsistent(some maps have content, others don't)
    int hasContent() const {
      return !values_.empty() && data_.hasContent();
    }

    bool empty() const {
      return !hasContent();
    }

    void clear() {
      values_.clear();
      data_.clear();
    }

    uint count() const {
      return values_.size();
    }

    ///@return if the given id is bound to the specified value
    bool validateId( const Value & value, Id id ) const {
      typename ValueMap::const_iterator it = values_.find( id );

      if ( it != values_.end() && ( *it ).second.value == value )
        return true;
      return false;
    }

    template <class Archive>
    void load( Archive & arch, uint version ) {
      arch & rebuildAfterLoad_;
      arch & currentId_;
      arch & dummy_;
      if ( rebuildAfterLoad_ ) {
        uint size;
        arch & size;
        values_.clear();
        data_.clear();
        for ( uint a = 0; a < size; a++ ) {
          typename ValueMap::value_type t;
          arch & t;
          values_.insert( t );
          insertToData( t.second.keyValues, t.second.value, t.first );
        }
      } else {
        arch & values_;
        arch & data_;
      }

    }
    template <class Archive>
    void save( Archive & arch, uint version ) {
      arch & rebuildAfterLoad_;
      arch & currentId_;
      arch & dummy_;
      if ( rebuildAfterLoad_ ) {
        uint size = values_.size();
        arch & size;
        for ( typename ValueMap::const_iterator it = values_.begin(); it != values_.end(); ++it ) {
          arch & *it;
        }
      } else {
        arch & values_;
        arch & data_;
      }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    /**If the Id is known, that should be used. "operator ==" is used while searching,
     * so if something that affects that operator changed, the item cannot be found and updated.
     * the value has to be findable with at least one of the keys. If all keys changed, it cannot be found.*/
    bool update( const Value& value ) {
      return update( find( value ) );
    }

    /**Tries to find oldValue, and replaces it with newValue. If oldValue is not found, the item is not inserted. */
    bool update( const Value& oldValue, const Value& newValue ) {
      Id i = find( value );
      if ( !i )
        return false;
      remove
        ( i );
      insert( newValue );
    }

    ///Can be called after some keys changed
    bool update( Id id ) {
      if ( id.id() == 0 )
        return false;
      typename ValueMap::iterator it = values_.find( id )
                                       ;
      if ( it != values_.end() ) {
        removeFromData( ( *it ).second.keyValues, id );
        ( *it ).second.keyValues = KeyValues( ( *it ).second );
        insertToData( ( *it ).second.keyValues, ( *it ).second, id );
        return true;
      } else {
        return false;
      }
    }

    typename ValueMap::iterator begin() {
      return values_.begin();
    }

    typename ValueMap::iterator end() {
      return values_.end();
    }

    typename ValueMap::const_iterator begin() const {
      return values_.begin();
    }

    typename ValueMap::const_iterator end() const {
      return values_.end();
    }

  private:
    friend class Iterator;

    Value& valueFromId( Id id ) {
      typename ValueMap::iterator itt = values_.find( id );
      if ( itt != values_.end()
         )
        return ( *itt ).second.value;
      else
        return dummy_;
    }


    void removeFromData( const KeyValues & keyValues, Id id ) {
      data_.template remove
        ( keyValues, id );
    }

    void insertToData( const KeyValues & keyValues, const Value & value, Id id ) {
      data_.template insert( keyValues, value, id );
    }

    ///These functions allow efficient access to the underlying maps
    template <int keyNum>
    const std::multimap< typename GetListItem< keyNum, RealKeys >::Result, Value > & getMap() const {
      return data_. CrossMapData< Value, typename GetListItem< keyNum, Keys >::Result, keyNum> :: map;
    }

    template <class KeyType>
    const std::multimap< KeyType, uint >& getMap() const {
      return data_. CrossMapData< Value, KeyType, FindInList<KeyType, Keys>::value > :: map;
    }

    template <int occurrenceNum, class KeyType>
    const std::multimap< KeyType, uint >& getMap() const {
      enum {
        num = FindInList<KeyType, RealKeys, occurrenceNum>::value
      };
      return data_. CrossMapData< Value, typename GetListItem< num, Keys >::Result, num > :: map;
    }

    template <int keyNum>
    std::multimap< typename GetListItem< keyNum, RealKeys >::Result, Value >& getMap() {
      return data_. CrossMapData< Value, typename GetListItem< keyNum, Keys >::Result, keyNum> :: map;
    }

    template <class KeyType>
    std::multimap< KeyType, uint >& getMap() {
      return data_. CrossMapData< Value, KeyType, FindInList<KeyType, Keys>::value > :: map;
    }

    template <int occurrenceNum, class KeyType>
    std::multimap< KeyType, uint >& getMap() {
      enum {
        num = FindInList<KeyType, RealKeys, occurrenceNum>::value
      };
      return data_. CrossMapData< Value, typename GetListItem< num, Keys >::Result, num > :: map;
    }

    uint currentId_;

    Value dummy_;

    ValueMap values_;

    CrossMapChain< Value, Keys, 0 > data_;
};

}

CROSSMAP_DEFINE_CONTAINER( std::list )

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
