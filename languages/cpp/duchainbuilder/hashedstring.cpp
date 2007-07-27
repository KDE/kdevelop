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

#include "hashedstring.h"
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

uint qHash ( const HashedString& str ) {
  return str.hash();
}

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

New Problem:
Have sets:
a,b
c,d
a,c
b,d
create set:
-> a,b,c,d

a,b,c,d should be a unique set, but it can be constructed from a,b + c,d and from a,c + b,d
Solution:
Merge a,b+a,c, a,c+b,d and then merge those to a,b,c,d

@todo first compute the hash correctly, then solve the problem above
@todo implement intersection

Intersection assumption:
If there is set A and set B, there is also a set that represents the intersection of A and B

test:
atomics are: a,b,c,d,e
Buld set a,b,c and b,c,d:
a,b,c:
a,c
a,b
b,c,d:
b,d
c,d

- no intersection-set b,c

How to make sure that intersection-sets exist:
When creating new sets in the building-algorithm, create the new set by choosing the merge-partner in such a way
that as many of it's master-sets as possible are master-sets of the new set, while still making sure that the set
is bigger then before.
*/

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

    HashedStringSubset( const HashedString& str ) : left_(0), right_(0), string_(new HashedString(str)), anonymous_(false), hash_(0), size_(0) {
      computeLocalAttributes();
      kDebug() << "created set " << string() << " of size " << size_ << endl;
    }
    HashedStringSubset( HashedStringSubset* left, HashedStringSubset* right, bool anonymous = false ) :  left_(left), right_(right), string_(0), anonymous_(anonymous), hash_(0), size_(0) {

      Q_ASSERT(left != right );
      Q_ASSERT( left_ != this && right_ != this );
      left_->addMasterSet(this);
      right_->addMasterSet(this);
      computeLocalAttributes();
      kDebug() << "created set " << string() << " of size " << size_ << endl;
    }

    ///An anonymous set is just a connection between different sets. It may have the same hash as another set. Anonymous sets should be ignored by most algorithms. Instead their one master-set should be used.
    bool anonymous() const {
      return anonymous_;
    }

    ~HashedStringSubset() {
      delete string_;
    }

    inline HashType hash() const {
      return hash_;
    };

    QString string(bool shortl=false) const {
      QString str;
      makeString(str, shortl);
      return str;
    }

    void makeString( QString& str, bool shortl=false ) const {
      std::set<HashedString*> allStrings;
      makeStringInternal(allStrings);
      for( std::set<HashedString*>::const_iterator it = allStrings.begin(); it != allStrings.end(); ++it ) {
        if( !str.isEmpty() && !shortl )
          str += ',' + (*it)->str();
        else
          str += (*it)->str();
      }
      if( anonymous_ ) {
        str+="_anon_"+QString("%1").arg((short)((size_t)this));
      }
    }

    void setLeft( HashedStringSubset* left ) {
      Q_ASSERT(left != this);
      left_->masterSets_.erase( this );
      left_ = left;
      left_->addMasterSet( this );
      Q_ASSERT( left_ != right_ );
      computeLocalAttributes();
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
      return right_;
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
        if( masters.find(*it) != masters.end() ) {
          //The master is already in the set
        } else {
          masters.insert( *it );
          (*it)->collectMasterSets(masters);
        }
      }
    }

    ///Collect the first master-sets that intersect with the given otherMasters
    void collectIntersectingMasterSets( MasterSetList& masters , const MasterHashSet& otherMasters ) {
      for( MasterSetList::const_iterator it = masterSets_.begin(); it != masterSets_.end(); ++it ) {
        if( !(*it)->anonymous() ) {
          if( otherMasters.find( *it ) != otherMasters.end() ) {
            //We have found an intersecting master-set
            masters.insert( *it );
            continue; //do not recurse, because all masters of the intersecting master will intersect anyway
          }
        }
        (*it)->collectIntersectingMasterSets(masters, otherMasters);
      }
    }

    ///Returns the public representation of this set.
    HashedStringSubset* getNonAnonymous() {
      if( anonymous_ ) {
        Q_ASSERT(masterSets_.size() == 1);
        return (*masterSets_.begin())->getNonAnonymous();
      } else {
        return this;
      }
    }

    HashedStringSubset* intersection( const HashedStringSubset* other ) {
      HashedStringSubset* intersect = intersectionInternal( other );
      kDebug() << "intersection of " << string() << "(" << this << ") and " << other->string() << "("<< other << ")" <<": " << (intersect ? intersect->string() : QString("none")) << endl;
      return intersect;
    }

    ///Returns the intersection of this set with the given other, or 0 on fail
    HashedStringSubset* intersectionInternal( const HashedStringSubset* other ) {
      ///The intersection is the lowest(by set-size) non-anonymous point on the shortest path from this to other

      if( isContainedIn(other) )
        return this;
      if( !left_ ) return 0;

      HashedStringSubset* leftIntersection = left_->intersectionInternal(other);
      HashedStringSubset* rightIntersection = right_->intersectionInternal(other);
      if( leftIntersection && rightIntersection ) {
        return leftIntersection->size_ > rightIntersection->size_ ? leftIntersection : rightIntersection;
      } else if( leftIntersection ) {
        return leftIntersection;
      } else if( rightIntersection ) {
        return rightIntersection;
      }
      return 0;
    }

    bool isContainedIn( const HashedStringSubset* other ) const {
      if( other == this )
        return true;
      ///@todo find a way to do this efficiently
      for( MasterSetList::const_iterator it = masterSets_.begin(); it != masterSets_.end(); ++it ) {
        if( (*it)->isContainedIn(other) )
          return true;
      }
      return false;
    }

  private:

    void addMasterSet( HashedStringSubset* master ) {
      masterSets_.insert(master);
      kDebug() << string() << ": adding master " << master->string() << endl;
      Q_ASSERT( !anonymous_ || masterSets_.size() == 1 );
    }

    void makeStringInternal( std::set<HashedString*>& allStrings ) const {
      if( left_ ) {
        left_->makeStringInternal(allStrings);
        right_->makeStringInternal(allStrings);
      }else{
        allStrings.insert( string_ );
      }
    }

    void computeLocalAttributes() {
      if( string_ ) {
        hash_ = string_->hash();
        size_ = 1;
      } else {
        hash_ = left_->hash_ + right_->hash_;
        size_ = left_->size_ + right_->size_;
        HashedStringSubset* intersection = left_->intersection(right_);
        if( intersection ) {
          hash_ -= intersection->hash();
          size_ -= intersection->size_;
        }
      }
    }

    MasterSetList masterSets_; //All subsets on the next layer that contain this subset

    ///Either a subset has left_ and right_ set, or it has string_ set
    HashedStringSubset* left_, *right_; ///These are set when 2 subsets from the lower layer were merged
    HashedString* string_; ///If string_ is set, it is an atomic subset.

    bool anonymous_;

    HashType hash_;
    unsigned int size_;
};

inline HashType symmetricMergeHash( HashedStringSubset* left, HashedStringSubset* right ) {
  HashType ret = left->hash() + right->hash();
  HashedStringSubset* intersection = left->intersection(right);
  if( intersection )
    ret -= intersection->hash();
  return ret;
}

void HashedStringRepository::connectToMasterSets( HashedStringSubset* set ) {
  ///@todo make efficient, after functionality is verified
  HashedStringSubset::MasterHashSet allLeftMasters;
  HashedStringSubset::MasterSetList intersectingMasters;

  set->left()->collectMasterSets(allLeftMasters);
  set->right()->collectIntersectingMasterSets(intersectingMasters, allLeftMasters);
  kDebug() << "connecting " << set->string() << " to its masters. m: " << set->left()->masterSets().size() << " " << set->right()->masterSets().size() << " lm: " << allLeftMasters.size() << " isect: " << intersectingMasters.size() << endl;

  for( HashedStringSubset::MasterSetList::const_iterator it = intersectingMasters.begin(); it != intersectingMasters.end(); ++it ) {
    //The set->left is contained in *it, and set->right is contained in *it,
    //while in each of *it's slave-sets, exactly either set->left or set->right is connected.
    //That means that *it is the point where they meet.

    if( *it == set ) continue;
    kDebug() << "while connecting " << set->string() << ": found master-set " << (*it)->string() << endl;
    kDebug() << "all masters of " << set->left()->string() << ":\n";

    for( HashedStringSubset::MasterHashSet::const_iterator it2 = allLeftMasters.begin(); it2 != allLeftMasters.end(); ++it2 )
      kDebug() << "  " << (*it2)->string() << endl;

    HashedStringSubset::MasterHashSet allRightMasters;
    set->right()->collectMasterSets(allRightMasters);
    kDebug() << "all masters of " << set->right()->string() << ":\n";

    for( HashedStringSubset::MasterHashSet::const_iterator it3 = allRightMasters.begin(); it3 != allRightMasters.end(); ++it3 )
      kDebug() << "  " << (*it3)->string() << endl;


    ///Create an intermediate set that merges set and (*it)->left, and use that as slave-set of *it. That way a connection is established.

    //Create the intermediate set, it will automatically add itself as master-set to (*it)->left() and "set"
    HashedStringSubset* newSet = merge( (*it)->left(), set );
    if( newSet == *it ) {
      //Create an intermediate anonymous connector-set
      newSet = new HashedStringSubset( (*it)->left(), set, true );
      kDebug() << "inserting anonymous set" << endl;
    }

    Q_ASSERT(*it != newSet);
    //Connect the intermediate set and *it
    HashType oldHash = (*it)->hash();
    (*it)->setLeft(newSet); //setLeft will automatically add itself to the master-list of newSet, and remove itself from the old master-list of it's old left()
    Q_ASSERT( (*it)->hash() == oldHash );

    //m_allSubsets.insert( std::make_pair(newSet->hash(), newSet) );
    //connectToMasterSets( newSet ); //Think whether this is needed

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
    return subset;
  }
}

HashedStringSubset* HashedStringRepository::buildSet( const QList<HashedStringSubset*> atomics )
{
  if( atomics.size() == 0 )
    return 0;
  kDebug() << "BUILDING NEW SET" << endl;

  __gnu_cxx::hash_set<HashedStringSubset*> allStringSets;

  __gnu_cxx::hash_set<HashedStringSubset*> currentStringSets;
  __gnu_cxx::hash_set<HashedStringSubset*> newStringSets;

  foreach( HashedStringSubset* atomic, atomics ) {
    currentStringSets.insert(atomic);
    allStringSets.insert(atomic);
  }

  while( currentStringSets.size() > 1 ) {
    newStringSets.clear();
    __gnu_cxx::hash_set<HashedStringSubset*>::iterator currentStringSetsEnd = currentStringSets.end();
    for( __gnu_cxx::hash_set<HashedStringSubset*>::iterator it = currentStringSets.begin(); it != currentStringSetsEnd; ++it ) {

      QString totalString;
      (*it)->makeString(totalString);
      kDebug() << "Processing set: " << totalString.toAscii().data() << endl;

      bool addedMasterSet = false; //Every sub-set must have at least one masterSet

      ///Add/create intersections with all master-sets
      HashedStringSubset::MasterSetList::const_iterator masterSetEnd = (*it)->masterSets().end();
      for( HashedStringSubset::MasterSetList::const_iterator masterSetIt = (*it)->masterSets().begin(); masterSetIt != masterSetEnd; ++masterSetIt )
      {
        HashedStringSubset* added = (*masterSetIt)->otherSlave(*it);
        if( !added )
          continue; //maybe an atomic set

        //Check if the added subset intersects with the set we are building. If it does, create the intersections of that set with the one we are building.
        HashedStringSubset* intersect = intersection(added, allStringSets);
        if( intersect && allStringSets.find(intersect) == allStringSets.end() ) {
          //probably the intersection was created
          HashedStringSubset* newSet = merge(intersect,*it);
          newStringSets.insert( newSet );
          allStringSets.insert( newSet );
          addedMasterSet = true;
        }
      }

      if( !addedMasterSet ) {
        ///Every sub-set must have at least one masterSet, so it is represented in the final set

        ///We should create a new masterSet, that merges the current set with another one from currentStringSets

        __gnu_cxx::hash_set<HashedStringSubset*>::iterator mergePartnerIt = it;

        //Simply take the next set as merge-partner
        mergePartnerIt++;
        if( mergePartnerIt == currentStringSetsEnd ) //If it is the last set, take the first
          mergePartnerIt = currentStringSets.begin();

        HashedStringSubset* newSubStringSet = merge( *it, *mergePartnerIt );
        newStringSets.insert( newSubStringSet );
        allStringSets.insert( newSubStringSet );
      }
    }
    currentStringSets.swap(newStringSets);
  }

  //Only one total set should be left in the end
  Q_ASSERT(currentStringSets.size() == 1);
  return *currentStringSets.begin();
}

HashedStringSubset* HashedStringRepository::intersection( HashedStringSubset* set, const __gnu_cxx::hash_set<HashedStringSubset*>& allStringSets )
{
  ///@todo make efficient, the whole tree is walked atm

  if( allStringSets.find(set) != allStringSets.end() )
    return set;

  if( set->left() ) {
    HashedStringSubset* leftIntersection = intersection(set->left(), allStringSets);
    HashedStringSubset* rightIntersection = intersection(set->right(), allStringSets);
    if( leftIntersection && rightIntersection ) {
      return merge( leftIntersection, rightIntersection );
    } else if( leftIntersection ) {
      return leftIntersection;
    } else if( rightIntersection ) {
      return rightIntersection;
    }
  }
  Q_UNUSED(left)
  Q_UNUSED(right)
  return 0;
}

HashedStringSubset* HashedStringRepository::merge( HashedStringSubset* left, HashedStringSubset* right, bool /*anon */)
{
  QString leftStr, rightStr;
  left->makeString(leftStr);
  right->makeString(rightStr);
  kDebug() << "merge: merging \"" << leftStr.toAscii().data() << "\" and \"" << rightStr.toAscii().data() << "\"" << endl;
  HashMap::iterator it = m_allSubsets.find( symmetricMergeHash(left, right) );
  if( it != m_allSubsets.end() ) {
    kDebug() << "merge: got pre-merged node" << endl;
    return (*it).second;
  } else {
    HashedStringSubset* subset = new HashedStringSubset( left, right );
    m_allSubsets.insert( std::make_pair(subset->hash(), subset ) );

    connectToMasterSets(subset);

    QString totalString;
    subset->makeString(totalString);
    kDebug() << "merge: string: " << totalString.toAscii().data() << endl;


    return subset;
  }
}

HashedStringSubset* HashedStringRepository::intersection( HashedStringSubset* left, HashedStringSubset* right ) {
  return left->intersection(right);
}

QString HashedStringRepository::dumpDotGraph() {
  QTextStream stream;
  QString ret;
  stream.setString(&ret, QIODevice::WriteOnly);
  stream << "digraph Repository {\n";
  for( HashMap::const_iterator it = m_allSubsets.begin(); it != m_allSubsets.end(); ++it ) {
    QString label;
    (*it).second->makeString(label);
    QString shortLabel;
    (*it).second->makeString(shortLabel,true);

    for( HashedStringSubset::MasterSetList::const_iterator masterIt = (*it).second->masterSets().begin(); masterIt != (*it).second->masterSets().end(); ++masterIt ) {
      QString label2;
      (*masterIt)->makeString(label2,true);
      Q_ASSERT( (*it).second->hash() != (*masterIt)->hash() );
      Q_ASSERT( (*it).second != *masterIt );
      if( shortLabel == label2 ) {
        kDebug() << "double label " << shortLabel << endl;
        kDebug() << "hashes: " << (*it).second->hash() << " " << (*masterIt)->hash() << " objects: " << (*it).second << " " << (*masterIt) << endl;
        Q_ASSERT(0);
      }
      stream  << shortLabel << " -> "  << label2 << ";\n";
    }

    if( !(*it).second->left() ) {
      //It is an atomic subset(string)
      stream << shortLabel << " [label=\"" << label << "\"];\n";
    } else if( (*it).second->masterSets().empty() ) {
      //It is a terminating master-set(without other master-sets)
      stream << shortLabel << " [shape=invtriangle,label=\"" << label << "\"];\n";
    } else {
      //It is a normal node
      stream << shortLabel << " [shape=polygon,sides=4,label=\"" << label << "\"];\n";
    }
  }

  stream << "}\n";
  return ret;
}

