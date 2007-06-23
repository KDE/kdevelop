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

#include "hashedstring.h"
#include <QDataStream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include<ext/hash_set>
#include<set>
#include<list>
#include<algorithm>

//It needs to be measured whether this flag should be turned on or off. It seems just to move the complexity from one position to the other, without any variant being really better.
#define USE_HASHMAP
    
HashType fastHashString( const QString& str );

HashType hashStringSafe( const QString& str ) {
  HashType hash = 0;
  int len = str.length();
  for( int a = 0; a < len; a++ ) {
    hash = str[a].unicode() + (hash * 17);
  }
  return hash;
}

HashType HashedString::hashString(  const QString& str )
{
	return fastHashString( str );
}

HashType fastHashString( const QString& str ) {
  HashType hash = 0;
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


class HashedStringSetData : public KShared {
  public:
#ifdef USE_HASHMAP
      typedef __gnu_cxx::hash_set<HashedString> StringSet;
#else
      typedef std::set<HashedString> StringSet; //must be a set, so the set-algorithms work
#endif
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

#ifndef USE_HASHMAP
  KSharedPtr<HashedStringSetData> oldData = m_data;
  if( !oldData ) oldData = new HashedStringSetData();
  m_data = new HashedStringSetData();
  std::set_union( oldData->m_files.begin(), oldData->m_files.end(), rhs.m_data->m_files.begin(), rhs.m_data->m_files.end(), std::insert_iterator<HashedStringSetData::StringSet>( m_data->m_files, m_data->m_files.end() ) );
#else
  makeDataPrivate();
  m_data->m_files.insert( rhs.m_data->m_files.begin(), rhs.m_data->m_files.end() );
  /*HashedStringSetData::StringSet::const_iterator end = rhs.m_data->m_files.end();
  HashedStringSetData::StringSet& mySet( m_data->m_files );
  for( HashedStringSetData::StringSet::const_iterator it = rhs.m_data->m_files.begin(); it != end; ++it ) {
    mySet.insert( *it );
  }*/
  
#endif
  return *this;
}

HashedStringSet& HashedStringSet::operator -=( const HashedStringSet& rhs ) {
  if( !m_data ) return *this;
  if( !rhs.m_data ) return *this;
#ifndef USE_HASHMAP
  KSharedPtr<HashedStringSetData> oldData = m_data;
  m_data = new HashedStringSetData();
  std::set_difference( oldData->m_files.begin(), oldData->m_files.end(), rhs.m_data->m_files.begin(), rhs.m_data->m_files.end(), std::insert_iterator<HashedStringSetData::StringSet>( m_data->m_files, m_data->m_files.end() ) );
#else
  makeDataPrivate();
  HashedStringSetData::StringSet::const_iterator end = rhs.m_data->m_files.end();
  HashedStringSetData::StringSet::const_iterator myEnd = m_data->m_files.end();
  HashedStringSetData::StringSet& mySet( m_data->m_files );
  for( HashedStringSetData::StringSet::const_iterator it = rhs.m_data->m_files.begin(); it != end; ++it ) {
    mySet.erase( *it );
  }
  
#endif
  return *this;
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
#ifndef USE_HASHMAP
  return std::includes( rhs.m_data->m_files.begin(), rhs.m_data->m_files.end(), m_data->m_files.begin(), m_data->m_files.end() );
#else
  const HashedStringSetData::StringSet& otherSet( rhs.m_data->m_files );
  HashedStringSetData::StringSet::const_iterator end = rhs.m_data->m_files.end();
  HashedStringSetData::StringSet::const_iterator myEnd = m_data->m_files.end();

  for( HashedStringSetData::StringSet::const_iterator it = m_data->m_files.begin(); it != myEnd; ++it ) {
    HashedStringSetData::StringSet::const_iterator i = otherSet.find( *it );
    if( i == end ) return false;
  }
  return true;
#endif
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
  __gnu_cxx::hash_map<unsigned int, int> hitCounts;
  
  for( HashedStringSetData::StringSet::const_iterator it = strings.m_data->m_files.begin(); it != strings.m_data->m_files.end(); ++it ) {
    GroupMap::const_iterator itr = m_map.find( *it );
      if( itr == m_map.end() ) {
        //There are no string-sets that include the currently searched for string
        continue;
      }

      for( ItemSet::const_iterator it2 = itr->second.begin(); it2 != itr->second.end(); ++it2 ) {
        __gnu_cxx::hash_map<unsigned int, int>::iterator v = hitCounts.find( *it2 );
        if( v != hitCounts.end() ) {
          ++(*v).second;
        } else {
          hitCounts[*it2] = 1;
        }
      }
  }

  //Now count together all groups that are completely within the given string-set(their hitCount equals their size)
  ItemSet found;
  for( __gnu_cxx::hash_map<unsigned int, int>::const_iterator it = hitCounts.begin(); it != hitCounts.end(); ++it ) {
    if( (*it).second == (*m_sizeMap.find( (*it).first )).second )
      found.insert( (*it).first );
  }

  
  std::set_union( found.begin(), found.end(), m_global.begin(), m_global.end(), std::insert_iterator<ItemSet>( target, target.end() ) );
  
  target.swap( found );
  target.clear();
  std::set_difference( found.begin(), found.end(), m_disabled.begin(), m_disabled.end(), std::insert_iterator<ItemSet>( target, target.end() ) );
}








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////HashedStringRepository/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**

WARNING: These are notes I have taken while thinking about the hashed-string-set-repository. They are obselete, inconsistent, ugly, etc. but may be helpful
for understanding the whole thing. The layering is not implemented.

Structure of the repository:
Everything is built from sets.
One set may be built from either:
  - A string(then it is an atomic set)
  - From two other string-sets

Each set manages:
 - either pointers to the two sets it is built from, or to the one string it contains
 - An arbitrary list of master-sets(sets that were constructed by merging this set with another)

The general idea is that each set can reach ALL sets it is contained in by following it's master-set pointers.
That needs to be made sure, see the end of these notes to see how it is achieved.

Old notes:

Every string-set has a pointer to each string-set it is contained in.
Together with that pointer, there is a pointer to the other string-set that is merged with this one.


How to build the perfect structure of string-sets?

Important: A string-set MUST be linked to all sub-sets that it contains <=> A sub-set MUST be linked to all bigger sets it is contained in.


How to build a string-set:
- Insert all missing atomic strings into the string-repository
- Put all atomic string-sets into a set currentStringSets
- newStringSets = set()
    - for stringSet in currentStringSets:
        nextStringSets = all follower string-sets of next that are contained in the current set(easy to compute: test whether the merged string-set is in currentStringSets)
        
        if nextStringSets.empty():
            nextStringSets << new string-set that merges stringSet with the most popular other string-set from currentStringSets, that is not contained within this string-set
        
        newStringSets += nextStringSets  #newStringSets should contain no string-set twice
    
    currentStringSets = newStringSets
  loop until currentStringSets only contains one string-set

Test:
1 - david
2 - ich
3 - bin
4 - hallo
5 - naja
6 - blabla

7 - david,ich
8 - bin,hallo
9 - hallo,ich,bin,david


neues set:
ich bin david

Durchlauf 1:
1 - david
2 - ich
3 - bin
Durchlauf 2:
7 - david,ich
10 - bin,david
Durchlauf 3:
11 - ich,bin,david

Problem:

1 - a
2 - b
3 - c
4 - d

5 - a,b
6 - c,d
7 - a,c
8 - b,d
9 - a,b,c,d(5,6)
10 - a,b,c,d(7,8)

Same sets may be separated differently. Since we want each set unique, we use a symmetric hash-sum(just a sum of the hashes of all involved strings) to find the sets.

What if I later want a set a,b,c? 

Run 1:
1 - a
2 - b
3 - c
Run 2:
5 - a,b
11 - c,a

-> Create the set a,b,c as a master-set of a,b and c,a, then:
- Find all positions where follower-sets of a,b intersect follower-sets of c,a, and insert a,b,c there

Then, a,b,c is correctly inserted.

Problem:

1 - a
2 - b
3 - c
4 - d
5 - e
6 - f
7 - g

construct set: abde
Run 1:
a,b
d,e
Run 2:
a,b,d,e

construct set: afcg
Run 1:
a,f
c,g
Run 2:
a,f,c,g

construct set abcdeg: (merge both sets above)
Run 1:
a,b
a,f
d,e
c,g
Run 2:
a,b,d,e
a,f,c,g
Run 3:
a,b,c,d,e,g


Now construct set a,b,c (it is not slave of abde and afcg, but of abcdeg)
start at slaves of a,b,c:
a,b
c,g

Notice that a,b and c,g merge 2 levels lower at abcdeg which is based on abde and afcg
Solution: Instead of letting abcdeg be merged from abde and afcg, change it to be merged from abde and a new node (afcg + abc)

This is needed because every set must have a connection to all it's master-sets, and every master-set must have a connection to all contained sets.

Question: How to efficiently find the intersecting sub-nodes?

 */

inline HashType symmetricMergeHash( HashType left, HashType right ) {
  return left + right;
}
/**
 * Manages recursive sets of hashed-string-sets in a way that is highly optimized for intersection-tests, and set-union
 * */

typedef __gnu_cxx::hash_set<HashedString> StdHashedStringSet;

class HashedStringSubset;
 namespace __gnu_cxx {
   template<> struct hash<HashedStringSubset*> {
     size_t operator() (const HashedStringSubset* set) const {
       return (size_t)set;
     }
   };
 }
     

class HashedStringSubset {
  public:
    typedef std::set<HashedStringSubset*> MasterSetList;
    typedef __gnu_cxx::hash_set<HashedStringSubset*> MasterHashSet;
    
    HashedStringSubset( const HashedString& str ) : left_(0), right_(0), string_(new HashedString(str)) {
      computeHash();
    }
    HashedStringSubset( HashedStringSubset* left, HashedStringSubset* right ) :  left_(left), right_(right), string_(0) {

      left_->masterSets_.insert(this);
      right_->masterSets_.insert(this);
      computeHash();
    }

    ~HashedStringSubset() {
      delete string_;
    }

    inline HashType hash() const {
      return hash_;
    };

    void setLeft( HashedStringSubset* left ) {
      left_->masterSets_.erase( left );
      left_ = left;
      left_->masterSets_.insert( left );
    }

    ///Returns all directly connected master-sets(they each represent a union of this set with another set, all sets that contain this set should be reachable through them)
    const MasterSetList& masterSets() {
      return masterSets_;
    }
    
    struct PointerCompare {
      bool operator() ( const HashedStringSubset* lhs, const HashedStringSubset* rhs ) const {
        return lhs->hash() < rhs->hash();
      }
    };
    struct PointerHash {
      bool operator() ( const HashedStringSubset* lhs ) const {
        return lhs->hash();
      }
    };

    ///@return left slave(is contained within this set)
    HashedStringSubset* left() const {
      return left_;
    }

    ///@return right slave-set(is contained within this set)
    HashedStringSubset* right() const {
      return left_;
    }
    
    ///Returns the slave-set that is not the given one
    HashedStringSubset* otherSlave( HashedStringSubset* pre ) {
      if( pre == left_ )
        return right_;
      else
        return left_;
    }
    
    ///Collect all master-sets across all levels
    void collectMasterSets( MasterHashSet& masters ) {
      for( MasterSetList::const_iterator it = masterSets_.begin(); it != masterSets_.end(); ++it ) {
        masters.insert( *it );
        (*it)->collectMasterSets(masters);
      }
    }

    ///Collect the first master-sets that intersect with the given otherMasters
    void collectIntersectingMasterSets( MasterSetList& masters , const MasterHashSet& otherMasters ) {
      for( MasterSetList::const_iterator it = masterSets_.begin(); it != masterSets_.end(); ++it ) {
        if( otherMasters.find( *it ) != otherMasters.end() ) {
          //We have found an intersecting master-set
          masters.insert( *it );
          continue; //do not recurse, because all masters of the intersecting master will intersect anyway
        }
        masters.insert( *it );
        (*it)->collectIntersectingMasterSets(masters, otherMasters);
      }
    }
    
  private:

    void computeHash() {
      if( string_ )
        hash_ = string_->hash();
      else
        hash_ = symmetricMergeHash(left_->hash(), right_->hash());
    }
    
    MasterSetList masterSets_; //All subsets on the next layer that contain this subset

    ///Either a subset has left_ and right_ set, or it has string_ set
    HashedStringSubset* left_, *right_; ///These are set when 2 subsets from the lower layer were merged
    HashedString* string_; ///If string_ is set, it is an atomic subset.

    HashType hash_;
};


void HashedStringRepository::connectToMasterSets( HashedStringSubset* set ) {
  ///@todo make efficient, after functionality is verified
  HashedStringSubset::MasterHashSet allLeftMasters;
  HashedStringSubset::MasterSetList intersectingMasters;

  set->left()->collectMasterSets(allLeftMasters);
  set->collectIntersectingMasterSets(intersectingMasters, allLeftMasters);

  for( HashedStringSubset::MasterSetList::const_iterator it = intersectingMasters.begin(); it != intersectingMasters.end(); ++it ) {
    //The set->left is contained in *it, and set->right is contained in *it,
    //while in each of *it's slave-sets, exactly either set->left or set->right is connected.
    //That means that *it is the point where they meet.

    ///Create an intermediate set that merges set and (*it)->left, and use that as slave-set of *it. That way a connection is established.

    //Create the intermediate set, it will automatically add itself as master-set to (*it)->left()
    HashedStringSubset* newSet = new HashedStringSubset( (*it)->left(), set );

    //Connect the intermediate set and *it
    (*it)->setLeft(newSet); //setLeft will automatically add itself to the master-list of newSet, and remove itself from the old master-list of it's old left()

    m_allSubsets.insert( std::make_pair(newSet->hash(), newSet) );
    connectToMasterSets( newSet ); //Think whether this is needed
    
  }
}


HashedStringSubset* HashedStringRepository::getAtomicSubset( const HashedString& str )
{
  AtomicSubsetMap::const_iterator it = m_atomicSubsets.find(str);
  if( it != m_atomicSubsets.end() ) {
    return (*it).second;
  } else {
    HashedStringSubset* subset = new HashedStringSubset(str);
    m_atomicSubsets.insert( std::make_pair(str, subset) );
    m_allSubsets.insert( std::make_pair(subset->hash(), subset ) );
  }
}

HashedStringSubset* HashedStringRepository::buildSet( const QList<HashedStringSubset*> atomics )
{
  if( atomics.size() == 0 )
    return 0;
  
  __gnu_cxx::hash_set<HashedStringSubset*, HashedStringSubset::PointerHash> currentStringSets;
  __gnu_cxx::hash_set<HashedStringSubset*, HashedStringSubset::PointerHash> newStringSets;

  foreach( HashedStringSubset* atomic, atomics )
    currentStringSets.insert(atomic);

  while( currentStringSets.size() > 1 ) {
    newStringSets.clear();
    __gnu_cxx::hash_set<HashedStringSubset*, HashedStringSubset::PointerHash>::iterator currentStringSetsEnd = currentStringSets.end();
    for( __gnu_cxx::hash_set<HashedStringSubset*, HashedStringSubset::PointerHash>::iterator it = currentStringSets.begin(); it != currentStringSetsEnd; ++it ) {

      bool addedMasterSet = false; //Every sub-set must have at least one masterSet
      
      ///Insert all masterSets that are in the set we build
      HashedStringSubset::MasterSetList::const_iterator masterSetEnd = (*it)->masterSets().end();
      for( HashedStringSubset::MasterSetList::const_iterator masterSetIt = (*it)->masterSets().begin(); masterSetIt != masterSetEnd; ++masterSetIt ) {
        if( currentStringSets.find( (*masterSetIt)->otherSlave(*it) ) != currentStringSetsEnd )  {
          ///The merge-partner is within the set we build, so the masterSet is within the set too
          newStringSets.insert( *masterSetIt );
        }
      }

      if( !addedMasterSet ) {
        ///Every sub-set must have at least one masterSet, so it is represented in the final set

        ///We should create a new masterSet, that merges the current set with another one from currentStringSets. Maybe we should be more clever about choosing that one.
        __gnu_cxx::hash_set<HashedStringSubset*, HashedStringSubset::PointerHash>::iterator mergePartnerIt = it;

        //Simply take the next set as merge-partner
        mergePartnerIt++;
        if( mergePartnerIt == currentStringSetsEnd ) //If it is the last set, take the first
          mergePartnerIt = currentStringSets.begin();

        newStringSets.insert( merge( *it, *mergePartnerIt ) );
          
      }
    }
    currentStringSets.swap(newStringSets);
  }

  //Only one total set should be left in the end
  Q_ASSERT(currentStringSets.size() == 1);
  return *currentStringSets.begin();
}

HashedStringSubset* HashedStringRepository::merge( HashedStringSubset* left, HashedStringSubset* right )
{
  HashMap::iterator it = m_allSubsets.find( symmetricMergeHash(left->hash(), right->hash()) );
  if( it != m_allSubsets.end() ) {
    return (*it).second;
  } else {
    HashedStringSubset* subset = new HashedStringSubset( left, right );
    m_allSubsets.insert( std::make_pair(subset->hash(), subset ) );

    connectToMasterSets(subset);
    
    return subset;
  }
}

