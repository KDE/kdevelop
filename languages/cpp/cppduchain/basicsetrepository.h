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

#ifndef BASICSETREPOSITORY_H
#define BASICSETREPOSITORY_H

#include <set>
#include <vector>
#include "cppduchainexport.h"

/**
 * This file provides a set system that can be used to efficiently manage sub-sets of a set of global objects.
 * Each global object must be mapped to a natural number.
 *
 * The efficiency comes from:
 * 1. The sets are represented by binary trees, where every single tree node can be shared across multiple sets.
 *    For that reason, intersecting sets will share large parts of their internal data structures, which leads to
 *    extremely low memory-usage compared to using for example std::set.
 *
 * The more common intersections between the sets exist, the more efficient the system is.
 * This will have the biggest advantage when items that were added contiguously are commonly
 * used within the same sets, and work fastest if the intersections between different sets are contiguously long.
 *
 * That makes it perfect for representing sets that are inherited across tree-like structures, like for example in C++:
 * - Macros defined in files(Macros are inherited from included files)
 * - Strings contained in files(Strings are inherited from included files)
 * - Set of all included files
 *
 * Measurements(see in kdevelop languages/cpp/cppduchain/tests/duchaintest) show that even in worst case(with totally random sets)
 * these set-repositories are 2 times faster than std::set, and 4 times faster than QSet.
 *
 * The main disadvantages are that a global repository needs to be managed, and needs to be secured from simultaneous write-access
 * during multi-threading. This is done internally if the doLocking flag is set while constructing.
 * */

class QString;

namespace Utils {

class SetNode;
class BasicSetRepository;

/**
 * This object is copyable. It represents a set, and allows iterating through the represented indices.
 * */
class KDEVCPPDUCHAIN_EXPORT Set {
public:
  class Iterator;
  typedef unsigned int Index;
  
  Set();
  //Internal constructor
  Set(uint treeNode, BasicSetRepository* repository);
  ~Set();
  Set(const Set& rhs);
  Set& operator=(const Set& rhs);

  QString dumpDotGraph() const;

  //Returns an itrator that can be used to iterate over the contained indices
  Iterator iterator() const;

  //Returns this set converted to a standard set that contains all indices contained by this set.
  std::set<unsigned int> stdSet() const;

  ///Returns the count of items in the set
  unsigned int count() const;

  bool contains(Index index) const;
  
  ///@warning: The following operations can change the global repository, and thus need to be serialized
  ///          using mutexes in case of multi-threading.

  ///Set union
  Set operator +(const Set& rhs) const;
  Set& operator +=(const Set& rhs);
  
  ///Set intersection
  Set operator &(const Set& rhs) const;
  Set& operator &=(const Set& rhs);

  ///Set subtraction
  Set operator -(const Set& rhs) const;
  Set& operator -=(const Set& rhs);
  
  ///Returns a pointer to the repository this set belongs to. Returns zero when this set is not initialized yet.
  BasicSetRepository* repository() const;
private:
  friend class BasicSetRepository;
  
  uint m_tree;
  mutable BasicSetRepository* m_repository;
};

/**
 * This is a repository that can be used to efficiently manage generic sets
 * that are represented by interweaved binary trees.
 *
 * All strings are based on items that are contained in one master-repository,
 * starting at one.
 *
 * An index of zero is interpreted as invalid.
 * */

class KDEVCPPDUCHAIN_EXPORT BasicSetRepository {
public:
  ///@param name The name must be unique, and is used for loading and storing the data
  ///@param doLocking Whether the repository should secured internally for multi-threading. If this is false, you need to secure it yourself
  ///@param dataSize Size of the internal data repository in bytes
  BasicSetRepository(QString name, bool doLocking, uint dataSize = 5000000u);
  ~BasicSetRepository();
  typedef unsigned int Index;

  /**
   * Takes a sorted list of index-ranges, and creates a set from them.
   * The ranges must be sorted. Each range consists of 2 items in the vector, [start, end)
   * */
  Set createSetFromRanges(const std::vector<Index>& indices);

  /**
   * Takes a simple set of indices, not ranges.
   * */
  Set createSet(const std::set<Index>& indices);

  /**
   * Creates a set that only contains that single index.
   * For better performance, you should create bigger sets than this.
   * */
  Set createSet(Index i);
  
private:
  friend class Set;
  class Private;
  Private* d;
};

/**
 * Use this to iterate over the indices contained in a set
 * */
class KDEVCPPDUCHAIN_EXPORT Set::Iterator {
public:
  Iterator();
  Iterator(const Iterator& rhs);
  Iterator& operator=(const Iterator& rhs);
  
  ~Iterator();
  operator bool() const;
  Iterator& operator++();
  BasicSetRepository::Index operator*() const;
private:
  friend class Set;
  class IteratorPrivate;
  IteratorPrivate* d;
};

}

#endif
