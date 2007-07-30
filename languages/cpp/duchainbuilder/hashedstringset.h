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

#ifndef HASHED_STRING_SET_H
#define HASHED_STRING_SET_H

#include<QString>
#include<ksharedptr.h>
#include<set>
#include <ext/hash_map>
#include <ext/hash_set>
#include <duchain/hashedstring.h>
#include <string>
#include "cppduchainbuilderexport.h"

class HashedStringSetData;
class HashedStringSetGroup;

///This is a reference-counting string-set optimized for fast lookup of hashed strings
class KDEVCPPDUCHAINBUILDER_EXPORT HashedStringSet {
  public:
    HashedStringSet();

    ~HashedStringSet();

    ///Constructs a string-set from one single file
    HashedStringSet( const KDevelop::HashedString& file );

    HashedStringSet( const HashedStringSet& rhs );

    int size() const;

    HashedStringSet& operator = ( const HashedStringSet& rhs );
    ///@return whether the given file-name was included
    bool operator[] ( const KDevelop::HashedString& rhs ) const;

    void insert( const KDevelop::HashedString& str );

    HashedStringSet& operator +=( const HashedStringSet& );
    
    HashedStringSet& operator -=( const HashedStringSet& );

    ///intersection-test
    ///Returns true if all files that are part of this set are also part of the given set
    bool operator <= ( const HashedStringSet& rhs ) const;

    bool operator == ( const HashedStringSet& rhs ) const;

    void read( QDataStream& stream );
    void write( QDataStream& stream ) const;

    std::string print() const;

    KDevelop::HashType hash() const;
  private:
    friend class HashedStringSetGroup;
    void makeDataPrivate();
    KSharedPtr<HashedStringSetData> m_data; //this implies some additional cost because KShared's destructor is virtual. Maybe change that by copying KShared without the virtual destructor.
    friend HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs );
};

HashedStringSet operator + ( const HashedStringSet& lhs, const HashedStringSet& rhs );

namespace __gnu_cxx {
template<>
struct KDEVCPPDUCHAINBUILDER_EXPORT hash<KDevelop::HashedString> {
  KDevelop::HashType operator () ( const KDevelop::HashedString& str ) const {
    return str.hash();
  }
};
}

///Used to find all registered HashedStringSet's that contain all strings given to findGroups(..)
class KDEVCPPDUCHAINBUILDER_EXPORT HashedStringSetGroup {
  public:
    typedef std::set<unsigned int> ItemSet;
    void addSet( unsigned int id, const HashedStringSet& set );
    void enableSet( unsigned int id );
    bool isDisabled( unsigned int id ) const;
    void disableSet( unsigned int id );
    void removeSet( unsigned int id );

    //Writes the ids of all registered and not disabled HashedStringSet's that are completely included in the given HashedStringSet efficiently)
    void findGroups( HashedStringSet strings, ItemSet& target ) const;

  private:
    typedef __gnu_cxx::hash_map<KDevelop::HashedString, ItemSet> GroupMap;
    typedef __gnu_cxx::hash_map<unsigned int, unsigned int> SizeMap;
    GroupMap m_map;
    SizeMap m_sizeMap;
    ItemSet m_disabled;
    ItemSet m_global;
};

class HashedStringSubset;

///@todo convert to d-pointer once ready
class KDEVCPPDUCHAINBUILDER_EXPORT HashedStringRepository {
  public:
    typedef __gnu_cxx::hash_map<KDevelop::HashedString, HashedStringSubset*> AtomicSubsetMap;
    typedef __gnu_cxx::hash_map<KDevelop::HashType, HashedStringSubset*> HashMap;

    HashedStringSubset* getAtomicSubset( const KDevelop::HashedString& str );

    /**
     * Takes a list of atomic sub-sets(hashed-strings), and construct a master-subset
     * The returned set will be registered in the repository
     * @param atomics list of atomic subsets
     * @return the created set, or 0 on failure
     * */
    HashedStringSubset* buildSet( const QList<HashedStringSubset*> atomics );

    /**
     * @return a HashedStringSubset that represents the union of the given sets.
     * */
    HashedStringSubset* merge( HashedStringSubset* left, HashedStringSubset* right, bool anon = false );
    HashedStringSubset* intersection( HashedStringSubset* left, HashedStringSubset* right );
    void dump( HashedStringSubset* subset );

    QString dumpDotGraph();
  private:
    //After a new HashedStringSubset was created, this must be called to correctly connect it correctly to it's master-sets
    void connectToMasterSets( HashedStringSubset* set );

    /**
     * @return intersection of left and right
     * */
    HashedStringSubset* intersection( HashedStringSubset* set, const __gnu_cxx::hash_set<HashedStringSubset*>& allStringSets );
    
    AtomicSubsetMap m_atomicSubsets;
    HashMap m_allSubsets;
};

#endif
