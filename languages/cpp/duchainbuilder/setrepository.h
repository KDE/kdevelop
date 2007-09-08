/***************************************************************************
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SETREPOSITORY_H
#define SETREPOSITORY_H

#include <ksharedptr.h>
#include <ext/hash_map>
#include <set>
#include <vector>
#include "cppduchainbuilderexport.h"

/**
 * These set-repositories are optimized by the fact that they can only grow.
 *
 * The general idea is that new items can only be appended, and the ranges of the items are used to
 * represent sets of these items by binary trees, that at least share their end-nodes.
 *
 * These trees can then be used for optimized intersection- and union-computation.
 * 
 * This will have the biggest advantage when items that were added contiguously are commonly used within the same sets, and work fastest if the intersections between different sets are contiguously long.
 * */

class QString;

namespace Utils {

///@todo create the nodes from a memory-pool(rxx_allocator)

class SetNode;
class BasicSetRepository;

/**
 * This object is copyable
 * */
class KDEVCPPDUCHAINBUILDER_EXPORT Set {
public:
  class Iterator;
  
  Set();
  ~Set();
  Set(const Set& rhs);
  Set& operator=(const Set& rhs);

  QString dumpDotGraph() const;

  //Returns an itrator that can be used to iterate over the contained indices
  Iterator iterator() const;

  //Returns this set converted to a standard set that contains all indices contained by this set.
  std::set<unsigned int> stdSet() const;
  
  /**
   * Use this to iterate over the indices contained by a set. See bottom of the file
   * */
  
private:
  friend class BasicSetRepository;
  
  class Private;
  KSharedPtr<Private> d;
};

/**
 * This is a repository that can be used to efficiently manage generic sets
 * that are represented by interweaved binary trees.
 *
 * All strings are based on items that are contained in one master-repository,
 * starting at one.
 *
 * An index of zero is interpreted as invalid.
 *
 * The master-repository can only grow, never shrink.
 * */

class KDEVCPPDUCHAINBUILDER_EXPORT BasicSetRepository {
public:
  BasicSetRepository();
  ~BasicSetRepository();
  typedef unsigned int Index;

  /**
   * Adds a group of new indices to the repository.
   * @param count Count of indices to add.
   * @return The first index created.
   * The created indices will be the range from return-value to return-value + count
   * */
  Index appendIndices(int count);


  /**
   * Takes a sorted list of index-ranges, and creates a set from them.
   * The ranges must be sorted. Each range consists of 2 items in the vector, [start, end)
   * */
  Set createSet(const std::vector<Index>& indices);

  /**
   * Takes a simple set of indices, not ranges.
   * */
  Set createSet(const std::set<Index>& indices);
  
  QString dumpDotGraph() const;

  /**
   * Set-union operation
   * @warning not implemented yet
   * */
  Set setUnion(const Set& first, const Set& second);
  
  /**
   * Set-intersection operation
   * */
  Set intersect(const Set& first, const Set& second);

  /**
   * Return first - second (The returned set will contain all items that are in first, but not in second)
   * @warning not implemented yet
   * */
  Set subtract(const Set& first, const Set& second);
  
private:
  class Private;
  Private* d;
};

/**
 * Manage a set-repository, where each item in the set is of type T.
 * @param Hash Should be a functor-object with an operator() that returns the hash for a given T
 * */
template<class T, class Hash>
  class KDEVCPPDUCHAINBUILDER_EXPORT SetRepository : public BasicSetRepository {
  typedef __gnu_cxx::hash_map<T, Index, Hash> ElementHash; ///@todo use a pool allocator, @see rxx_allocator
public:
  /**
   * @param item The item to be searched for.
   * @param repositoryIndex Target will be filled with the index repositoryItem has in the repository.
   * @param repositoryItem Target will be filled with the item found in the repository that equalis item. If there is none, item will be added to repository.
   * @param set The item will be added to the set. @note This set must not be in the repository yet, and it will not be in the repository afterwards.
   *
   * This can also be used to unify the items, to save memory(by using repositoryItem instead of item).
   * */
  void getItem(const T& item, Index* repositoryIndex, T* repositoryItem = 0)
  {
    typename ElementHash::const_iterator it = m_elements.find(item);
    if(it != m_elements.end()) {
      
      if(repositoryItem)
        *repositoryItem = (*it).first;
      
      if(repositoryIndex)
        *repositoryIndex = (*it).second;
    }else{
      Index ret = appendIndices(1);
      Q_ASSERT(ret);
      m_elements.insert(item, ret);
      
      if(repositoryItem)
        *repositoryItem = item;
      
      if(repositoryIndex)
        *repositoryIndex = ret;
    }
  }

private:
  // Tracks what elements have been assigned to what indices
  ElementHash m_elements;
};

class Set::Iterator {
public:
  Iterator(const Iterator& rhs);
  Iterator& operator=(const Iterator& rhs);
  
  Iterator();
  ~Iterator();
  operator bool() const;
  Iterator& operator++();
  BasicSetRepository::Index operator*() const;
private:
  friend class Set;
  class IteratorPrivate;
  KSharedPtr<IteratorPrivate> d;
};

}

#endif

