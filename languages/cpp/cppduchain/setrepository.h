
#ifndef SETREPOSITORY_H
#define SETREPOSITORY_H

#include "basicsetrepository.h"
#include <QMutex>
#include <list>

/**
 * This header defines convenience-class that allow handling set-repositories using the represented higher-level objects instead
 * of indices to them.
 * */


namespace Utils {
/**
 * Use this class to conveniently iterate over the items in a set.
 * @param T The type the indices will be converted to
 * @param Conversion Should be a class that has a toIndex member function that takes an object of type T as parameter, and returns an index,
 *                   and a toItem member function that takes an index, and returns an item of type T.
 * */
template<class T, class Conversion>
class KDEVCPPDUCHAIN_EXPORT ConvenientIterator {
  public:
    ConvenientIterator(Set::Iterator it=Set::Iterator(), const Conversion& conversion = Conversion()) : m_it(it), m_conversion(conversion) {
    }
    ConvenientIterator(const Set& set, const Conversion& conversion = Conversion()) : m_it(set.iterator()), m_conversion(conversion) {
    }
    
    operator bool() const {
      return m_it;
    }

    ConvenientIterator& operator++() {
      ++m_it;
      return *this;
    }

    T operator*() const {
      return m_conversion.toItem(*m_it);
    }

    const T& ref() const {
      return m_conversion.toItem(*m_it);
    }

    private:
    Set::Iterator m_it;
    Conversion m_conversion;
  };

    /** This is a helper-class that helps inserting a bunch of items into a set without caring about grouping them together.
     *
     * It creates a much better tree-structure if many items are inserted at one time, and this class helps doing that in
     * cases where there is no better choice then storing a temporary list of items and inserting them all at once.
     *
     * This set will then care about really inserting them into the repository once the real set is requested.
     * 
     * @todo eventually make this unnecessary
     * 
     * @param T Should be the type that should be dealt
     * @param Conversion Should be a class that has a toIndex member function that takes an object of type T as parameter, and returns an index,
     *                   and a toItem member function that takes an index, and returns an item of type T.
     **/
template<class T, class Conversion>
class KDEVCPPDUCHAIN_EXPORT LazySet {
  public:
    /** @param rep The repository the set should belong/belongs to
     *  @param lockBeforeAccess If this is nonzero, the given mutex will be locked before each modification to the repository.
     *  @param basicSet If this is explicitly given, the given set will be used as base. However it will not be changed.
     *
     * @warning Watch for deadlocks, never use this class while the mutex given through lockBeforeAccess is locked
     */
    LazySet(BasicSetRepository* rep, QMutex* lockBeforeAccess = 0, const Set& basicSet = Set(), const Conversion& conversion = Conversion()) : m_rep(rep), m_set(basicSet), m_lockBeforeAccess(lockBeforeAccess), m_conversion(conversion) {
    }

    void insert(const T& t) {
      if(!m_temporaryRemoveIndices.empty())
        apply();
      m_temporaryIndices.insert(m_conversion.toIndex(t));
    }
    
    void remove(const T& t) {
      if(!m_temporaryIndices.empty())
        apply();
      m_temporaryRemoveIndices.insert(m_conversion.toIndex(t));
    }

    ///Returns the set this LazySet represents. When this is called, the set is constructed in the repository.
    Set set() const {
      apply();
      return m_set;
    }

    ///@warning this is expensive, because the set is constructed
    bool contains(const T& item) const {
      QMutexLocker l(m_lockBeforeAccess);
      uint index = m_conversion.toIndex(item);

      if( m_temporaryRemoveIndices.empty() ) {
        //Simplification without creating the set
        if(m_temporaryIndices.find(index) != m_temporaryIndices.end())
          return true;
        
        return m_set.contains(index);
      }
      
      return set().contains(index);
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

    void clear() {
      QMutexLocker l(m_lockBeforeAccess);
      m_set = Set();
      m_temporaryIndices.clear();
      m_temporaryRemoveIndices.clear();
    }

    ConvenientIterator<T, Conversion> iterator() const {
      apply();
      return ConvenientIterator<T, Conversion>(set(), m_conversion);
    }

  private:
    void apply() const {
      if(!m_temporaryIndices.empty()) {
        QMutexLocker l(m_lockBeforeAccess);
        Set tempSet = m_rep->createSet(m_temporaryIndices);
        m_temporaryIndices.clear();
        m_set += tempSet;
      }
      if(!m_temporaryRemoveIndices.empty()) {
        QMutexLocker l(m_lockBeforeAccess);
        Set tempSet = m_rep->createSet(m_temporaryRemoveIndices);
        m_temporaryRemoveIndices.clear();
        m_set -= tempSet;
      }
    }
    BasicSetRepository* m_rep;
    mutable Set m_set;
    QMutex* m_lockBeforeAccess;
    typedef std::set<Utils::BasicSetRepository::Index> IndexList;
    mutable IndexList m_temporaryIndices;
    mutable IndexList m_temporaryRemoveIndices;
    Conversion m_conversion;
  };
}

#endif

