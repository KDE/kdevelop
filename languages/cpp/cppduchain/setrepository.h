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

#include <QMutex>
#include <ksharedptr.h>
#ifdef Q_CC_MSVC    
#include <hash_map>
#else
#include <ext/hash_map>
#endif
#include <list>
#include <set>
#include <vector>
#include "cppduchainexport.h"

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
class KDEVCPPDUCHAIN_EXPORT Set {
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

  ///Returns the count of items in the set
  unsigned int count() const;
  
  ///Set union
  Set operator +(const Set& rhs) const;
  Set& operator +=(const Set& rhs);
  
  ///Set intersection
  Set operator &(const Set& rhs) const;
  Set& operator &=(const Set& rhs);

  ///Set subtraction
  Set operator -(const Set& rhs) const;
  Set& operator -=(const Set& rhs);
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

class KDEVCPPDUCHAIN_EXPORT BasicSetRepository {
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

  /**
   * Creates a set that only contains that single index.
   * For better performance, you should create bigger sets than this.
   * */
  Set createSet(Index i);
  
  QString dumpDotGraph() const;
  
private:
  class Private;
  Private* d;
};

/**
 * Use this to iterate over the indices contained in a set
 * */
class KDEVCPPDUCHAIN_EXPORT Set::Iterator {
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

/**
 * Manage a set-repository, where each item in the set is of type T. The elemnt-type should be implicitly shared, because it is stored twice.
 * @param Hash Should be a functor-object with an operator() that returns the hash for a given T
 * */
template<class T, class Hash>
  class KDEVCPPDUCHAIN_EXPORT SetRepository : public BasicSetRepository {
  #ifdef Q_CC_MSVC
    typedef stdext::hash_map<T, Index, Hash> ElementHash; ///@todo use a pool allocator, @see rxx_allocator
  #else
    typedef __gnu_cxx::hash_map<T, Index, Hash> ElementHash; ///@todo use a pool allocator, @see rxx_allocator
  #endif
public:
  SetRepository() {
    m_elements.push_back(T()); //Index zero will not be used
  }
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
    typename ElementHash::const_iterator it = m_elementHash.find(item);
    if(it != m_elementHash.end()) {
      
      if(repositoryItem)
        *repositoryItem = (*it).first;
      
      if(repositoryIndex)
        *repositoryIndex = (*it).second;
    }else{
      Index ret = appendIndices(1);
      Q_ASSERT(ret);
      m_elementHash[item] = ret;
      Q_ASSERT(m_elements.size() == ret);
      m_elements.push_back(item);
      
      if(repositoryItem)
        *repositoryItem = item;
      
      if(repositoryIndex)
        *repositoryIndex = ret;
    }
  }

  /** Use this to conveniently iterate over the real items contained in a set(not indices) */
  class Iterator {
  public:
    Iterator(const SetRepository<T, Hash>* rep=0, Set::Iterator it=Set::Iterator()) : m_rep(rep), m_it(it) {
    }
    Iterator(const Iterator& rhs) : m_rep(rhs.m_rep), m_it(rhs.m_it) {
    }
    Iterator& operator=(const Iterator& rhs) {
      m_rep = rhs.m_rep;
      m_it = rhs.m_it;
    }
    
    operator bool() const {
      return m_it && m_rep;
    }

    Iterator& operator++() {
      ++m_it;
      return *this;
    }

    const T& operator*() const {
      Q_ASSERT(m_rep);
      Q_ASSERT(*m_it >= 0 && *m_it < m_rep->elements().size());
      return m_rep->elements()[*m_it];
    }
  private:
    const SetRepository<T, Hash>* m_rep;
    Set::Iterator m_it;
  };
  /**
   * In the returned vector, each element is accessible by its index.
   * */
  const std::vector<T>& elements() const {
    return m_elements;
  }

    /** This is a helper-class that helps inserting a bunch of items into a set without caring about grouping them together.
     *
     * It creates a much better tree-structure if many items are inserted at one time, and this class helps doing that in
     * cases where there is no better choice then storing a temporary list of items and inserting them all at once.
     *
     * This set will then care about really inserting them into the repository once the real set is requested.
     **/
  class LazySet {
  public:
    /** @param rep The repository the set should belong/belongs to
     *  @param lockBeforeAccess If this is nonzero, the given mutex will be locked before each modification to the repository.
     *  @param basicSet If this is explicitly given, the given set will be used as base. However it will not be changed.
     *
     * @warning Watch for deadlocks, never use this class while the mutex given through lockBeforeAccess is locked
     */
    LazySet(SetRepository<T, Hash>* rep, QMutex* lockBeforeAccess, const Set& basicSet = Set()) : m_rep(rep), m_set(basicSet), m_lockBeforeAccess(lockBeforeAccess) {
    }

    void insert(const T& t) {
      if(!m_temporaryRemoveIndices.empty())
        apply();
      m_temporaryIndices.push_back(t);
    }
    
    void remove(const T& t) {
      if(!m_temporaryIndices.empty())
        apply();
      m_temporaryRemoveIndices.push_back(t);
    }

    ///Returns the set this LazySet represents. When this is called, the set is constructed in the repository.
    Set set() const {
      apply();
      return m_set;
    }

    ///@warning this is expensive, because the set is constructed
    bool contains(const T& item) const {
      QMutexLocker l(m_lockBeforeAccess);
      BasicSetRepository::Index i;
      m_rep->getItem(item, &i);

      if( m_temporaryRemoveIndices.empty() && m_temporaryIndices.size() < 10 ) {
        //Simplification without creating the set
        for( typename std::list<T>::const_iterator it = m_temporaryIndices.begin(); it != m_temporaryIndices.end(); ++it )
          if( *it == item )
            return true;
        return (bool)(m_set & m_rep->createSet(i)).iterator();
      }
      
      return (bool)(set() & m_rep->createSet(i)).iterator();
      
    }

    LazySet& operator +=(const Set& set) {
      if(!m_temporaryRemoveIndices.empty())
        apply();
      QMutexLocker l(m_lockBeforeAccess);
      m_set += set;
      return *this;
    }
    
    LazySet& operator -=(const Set& set) {
      if(!m_temporaryIndices.empty())
        apply();
      QMutexLocker l(m_lockBeforeAccess);
      m_set -= set;
      return *this;
    }

    LazySet operator +(const Set& set) const {
      apply();
      QMutexLocker l(m_lockBeforeAccess);
      Set ret = m_set + set;
      return LazySet(m_rep, m_lockBeforeAccess, ret);
    }
    
    LazySet operator -(const Set& set) const {
      apply();
      QMutexLocker l(m_lockBeforeAccess);
      Set ret = m_set - set;
      return LazySet(m_rep, m_lockBeforeAccess, ret);
    }
    
    ///Returns an iterator that can conveniently be used to iterate over the items contained in this set
//     Iterator iterator() const {
//       return Iterator(m_rep, set().iterator());
//     }

  private:
    void apply() const {
      if(!m_temporaryIndices.empty()) {
        QMutexLocker l(m_lockBeforeAccess);
        std::set<Utils::BasicSetRepository::Index> indices;
        typename std::list<T>::const_iterator end = m_temporaryIndices.end();
        for ( typename std::list<T>::const_iterator rit = m_temporaryIndices.begin(); rit != end; ++rit ) {
          Utils::BasicSetRepository::Index idx;
          m_rep->getItem(*rit, &idx);
          indices.insert(idx);
        }
        Set tempSet = m_rep->createSet(indices);
        m_temporaryIndices.clear();
        m_set += tempSet;
      }
      if(!m_temporaryRemoveIndices.empty()) {
        QMutexLocker l(m_lockBeforeAccess);
        std::set<Utils::BasicSetRepository::Index> indices;
        typename std::list<T>::const_iterator end = m_temporaryRemoveIndices.end();
        for ( typename std::list<T>::const_iterator rit = m_temporaryRemoveIndices.begin(); rit != end; ++rit ) {
          Utils::BasicSetRepository::Index idx;
          m_rep->getItem(*rit, &idx);
          indices.insert(idx);
        }
        Set tempSet = m_rep->createSet(indices);
        m_temporaryRemoveIndices.clear();
        m_set -= tempSet;
      }
    }
    SetRepository<T, Hash>* m_rep;
    mutable Set m_set;
    QMutex* m_lockBeforeAccess;
    mutable std::list<T> m_temporaryIndices;
    mutable std::list<T> m_temporaryRemoveIndices;
  };
private:
  // Tracks what elements have been assigned to what indices
  ElementHash m_elementHash;
  std::vector<T> m_elements;
};

}

#endif

