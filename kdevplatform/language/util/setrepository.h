/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SETREPOSITORY_H
#define KDEVPLATFORM_SETREPOSITORY_H

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
 * @tparam T The type the indices will be converted to
 * @tparam Conversion Should be a class that has a toIndex member function that takes an object of type T as parameter, and returns an index,
 *                   and a toItem member function that takes an index, and returns an item of type T.
 * */
template <class T, class Conversion>
class ConvenientIterator
    : public Conversion
{
public:
    explicit ConvenientIterator(const Set::Iterator& it = Set::Iterator()) : m_it(it)
    {
    }
    explicit ConvenientIterator(const Set& set) : m_it(set.iterator())
    {
    }

    operator bool() const {
        return m_it;
    }

    ConvenientIterator& operator++()
    {
        ++m_it;
        return *this;
    }

    T operator*() const
    {
        return Conversion::toItem(*m_it);
    }

    uint index() const
    {
        return *m_it;
    }

private:
    Set::Iterator m_it;
};

struct DummyLocker
{
};

template <class T>
struct IdentityConversion
{
    static T toIndex(const T& t)
    {
        return t;
    }
    static T toItem(const T& t)
    {
        return t;
    }
};

///This is a virtual set-node that allows conveniently iterating through the tree-structure,
///accessing the contained items directly, and accessing the ranges.
template <class T, class Conversion, class StaticRepository>
class VirtualSetNode
{
private:
    using ClassType = VirtualSetNode<T, Conversion, StaticRepository>;

public:
    inline explicit VirtualSetNode(const SetNodeData* data = nullptr) : m_data(data)
    {
    }

    inline bool isValid() const
    {
        return ( bool )m_data;
    }

    ///If this returns false, a left and a right node are available.
    ///If this returns true, this node represents a single item, that can be retrieved by calling item() or operator*.
    inline bool isFinalNode() const
    {
        return m_data->leftNode() == 0;
    }

    inline T firstItem() const
    {
        return Conversion::toItem(start());
    }

    inline T lastItem() const
    {
        return Conversion::toItem(end() - 1);
    }

    inline T operator*() const
    {
        return Conversion::toItem(start());
    }

    inline ClassType leftChild() const
    {
        if (m_data->leftNode())
            return ClassType(StaticRepository::repository()->nodeFromIndex(m_data->leftNode()));
        else
            return ClassType(nullptr);
    }

    inline ClassType rightChild() const
    {
        if (m_data->rightNode())
            return ClassType(StaticRepository::repository()->nodeFromIndex(m_data->rightNode()));
        else
            return ClassType(nullptr);
    }

    ///Returns the start of this node's range. If this is a final node, the length of the range is 1.
    inline uint start() const
    {
        return m_data->start();
    }

    ///Returns the end of this node's range.
    inline uint end() const
    {
        return m_data->end();
    }

private:

    const SetNodeData* m_data;
};

template <class T, class Conversion, class StaticRepository, bool doReferenceCounting = false,
    class StaticAccessLocker = DummyLocker>
class StorableSet
    : public Conversion
{
public:

    using Node = VirtualSetNode<T, Conversion, StaticRepository>;

    StorableSet(const StorableSet& rhs) : m_setIndex(rhs.m_setIndex)
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        if (doReferenceCounting)
            set().staticRef();
    }

    explicit StorableSet(const std::set<uint>& indices)
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        m_setIndex = StaticRepository::repository()->createSet(indices).setIndex();
        if (doReferenceCounting)
            set().staticRef();
    }

    StorableSet()
    {
    }

    ~StorableSet()
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        if (doReferenceCounting)
            set().staticUnref();
    }

    void insert(const T& t)
    {
        insertIndex(Conversion::toIndex(t));
    }

    bool isEmpty() const
    {
        return m_setIndex == 0;
    }

    uint count() const
    {
        return set().count();
    }

    void insertIndex(uint index)
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        Set set(m_setIndex, StaticRepository::repository());
        Set oldSet(set);
        Set addedSet = StaticRepository::repository()->createSet(index);
        if (doReferenceCounting)
            addedSet.staticRef();
        set += addedSet;
        m_setIndex = set.setIndex();

        if (doReferenceCounting) {
            set.staticRef();
            oldSet.staticUnref();
            addedSet.staticUnref();
        }
    }

    void remove(const T& t)
    {
        removeIndex(Conversion::toIndex(t));
    }

    void removeIndex(uint index)
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        Set set(m_setIndex, StaticRepository::repository());
        Set oldSet(set);
        Set removedSet = StaticRepository::repository()->createSet(index);
        if (doReferenceCounting) {
            removedSet.staticRef();
        }
        set -= removedSet;
        m_setIndex = set.setIndex();

        if (doReferenceCounting) {
            set.staticRef();
            oldSet.staticUnref();
            removedSet.staticUnref();
        }
    }

    Set set() const
    {
        return Set(m_setIndex, StaticRepository::repository());
    }

    bool contains(const T& item) const
    {
        return containsIndex(Conversion::toIndex(item));
    }

    bool containsIndex(uint index) const
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        Set set(m_setIndex, StaticRepository::repository());
        return set.contains(index);
    }

    StorableSet& operator +=(const StorableSet& rhs)
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        Set set(m_setIndex, StaticRepository::repository());
        Set oldSet(set);
        Set otherSet(rhs.m_setIndex, StaticRepository::repository());
        set += otherSet;
        m_setIndex = set.setIndex();

        if (doReferenceCounting) {
            set.staticRef();
            oldSet.staticUnref();
        }
        return *this;
    }

    StorableSet& operator -=(const StorableSet& rhs)
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        Set set(m_setIndex, StaticRepository::repository());
        Set oldSet(set);
        Set otherSet(rhs.m_setIndex, StaticRepository::repository());
        set -= otherSet;
        m_setIndex = set.setIndex();

        if (doReferenceCounting) {
            set.staticRef();
            oldSet.staticUnref();
        }
        return *this;
    }

    StorableSet& operator &=(const StorableSet& rhs)
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        Set set(m_setIndex, StaticRepository::repository());
        Set oldSet(set);
        Set otherSet(rhs.m_setIndex, StaticRepository::repository());
        set &= otherSet;
        m_setIndex = set.setIndex();

        if (doReferenceCounting) {
            set.staticRef();
            oldSet.staticUnref();
        }
        return *this;
    }

    StorableSet& operator=(const StorableSet& rhs)
    {
        StaticAccessLocker lock;
        Q_UNUSED(lock);
        if (doReferenceCounting)
            set().staticUnref();
        m_setIndex = rhs.m_setIndex;
        if (doReferenceCounting)
            set().staticRef();
        return *this;
    }

    StorableSet operator +(const StorableSet& rhs) const
    {
        StorableSet ret(*this);
        ret += rhs;
        return ret;
    }

    StorableSet operator -(const StorableSet& rhs) const
    {
        StorableSet ret(*this);
        ret -= rhs;
        return ret;
    }

    StorableSet operator &(const StorableSet& rhs) const
    {
        StorableSet ret(*this);
        ret &= rhs;
        return ret;
    }

    bool operator==(const StorableSet& rhs) const
    {
        return m_setIndex == rhs.m_setIndex;
    }

    using Iterator = ConvenientIterator<T, Conversion>;

    Iterator iterator() const
    {
        return ConvenientIterator<T, Conversion>(set());
    }

    Node node() const
    {
        return Node(StaticRepository::repository()->nodeFromIndex(m_setIndex));
    }

    uint setIndex() const
    {
        return m_setIndex;
    }

private:

    uint m_setIndex = 0;
};

template<class T, class Conversion, class StaticRepository, bool doReferenceCounting, class StaticAccessLocker>
size_t qHash(const StorableSet<T, Conversion, StaticRepository, doReferenceCounting, StaticAccessLocker>& set)
{
    return set.setIndex();
}
/** This is a helper-class that helps inserting a bunch of items into a set without caring about grouping them together.
 *
 * It creates a much better tree-structure if many items are inserted at one time, and this class helps doing that in
 * cases where there is no better choice then storing a temporary list of items and inserting them all at once.
 *
 * This set will then care about really inserting them into the repository once the real set is requested.
 *
 * @todo eventually make this unnecessary
 *
 * @tparam T Should be the type that should be dealt
 * @tparam Conversion Should be a class that has a toIndex member function that takes an object of type T as parameter, and returns an index,
 *                   and a toItem member function that takes an index, and returns an item of type T.
 **/
template <class T, class Conversion>
class LazySet
    : public Conversion
{
public:
    /** @param rep The repository the set should belong/belongs to
     *  @param lockBeforeAccess If this is nonzero, the given mutex will be locked before each modification to the repository.
     *  @param basicSet If this is explicitly given, the given set will be used as base. However it will not be changed.
     *
     * @warning Watch for deadlocks, never use this class while the mutex given through lockBeforeAccess is locked
     */
    explicit LazySet(BasicSetRepository* rep, QMutex* lockBeforeAccess = nullptr, const Set& basicSet = Set()) : m_rep(
            rep)
        , m_set(basicSet)
        , m_lockBeforeAccess(lockBeforeAccess)
    {
    }

    void insert(const T& t)
    {
        if (!m_temporaryRemoveIndices.empty())
            apply();
        m_temporaryIndices.insert(Conversion::toIndex(t));
    }

    void insertIndex(uint index)
    {
        if (!m_temporaryRemoveIndices.empty())
            apply();
        m_temporaryIndices.insert(index);
    }

    void remove(const T& t)
    {
        if (!m_temporaryIndices.empty())
            apply();
        m_temporaryRemoveIndices.insert(Conversion::toIndex(t));
    }

    ///Returns the set this LazySet represents. When this is called, the set is constructed in the repository.
    Set set() const
    {
        apply();
        return m_set;
    }

    ///@warning this is expensive, because the set is constructed
    bool contains(const T& item) const
    {
        QMutexLocker l(m_lockBeforeAccess);
        uint index = Conversion::toIndex(item);

        if (m_temporaryRemoveIndices.empty()) {
            //Simplification without creating the set
            if (m_temporaryIndices.find(index) != m_temporaryIndices.end())
                return true;

            return m_set.contains(index);
        }

        return set().contains(index);
    }

    LazySet& operator +=(const Set& set)
    {
        if (!m_temporaryRemoveIndices.empty())
            apply();
        QMutexLocker l(m_lockBeforeAccess);
        m_set += set;
        return *this;
    }

    LazySet& operator -=(const Set& set)
    {
        if (!m_temporaryIndices.empty())
            apply();
        QMutexLocker l(m_lockBeforeAccess);
        m_set -= set;
        return *this;
    }

    LazySet operator +(const Set& set) const
    {
        apply();
        QMutexLocker l(m_lockBeforeAccess);
        Set ret = m_set + set;
        return LazySet(m_rep, m_lockBeforeAccess, ret);
    }

    LazySet operator -(const Set& set) const
    {
        apply();
        QMutexLocker l(m_lockBeforeAccess);
        Set ret = m_set - set;
        return LazySet(m_rep, m_lockBeforeAccess, ret);
    }

    void clear()
    {
        QMutexLocker l(m_lockBeforeAccess);
        m_set = Set();
        m_temporaryIndices.clear();
        m_temporaryRemoveIndices.clear();
    }

    ConvenientIterator<T, Conversion> iterator() const
    {
        apply();
        return ConvenientIterator<T, Conversion>(set());
    }

private:
    void apply() const
    {
        if (!m_temporaryIndices.empty()) {
            QMutexLocker l(m_lockBeforeAccess);
            Set tempSet = m_rep->createSet(m_temporaryIndices);
            m_temporaryIndices.clear();
            m_set += tempSet;
        }
        if (!m_temporaryRemoveIndices.empty()) {
            QMutexLocker l(m_lockBeforeAccess);
            Set tempSet = m_rep->createSet(m_temporaryRemoveIndices);
            m_temporaryRemoveIndices.clear();
            m_set -= tempSet;
        }
    }
    BasicSetRepository* m_rep;
    mutable Set m_set;
    QMutex* m_lockBeforeAccess;
    using IndexList = std::set<Utils::BasicSetRepository::Index>;
    mutable IndexList m_temporaryIndices;
    mutable IndexList m_temporaryRemoveIndices;
};

///Persistent repository that manages string-sets, also correctly increasing the string reference-counts as needed
struct KDEVPLATFORMLANGUAGE_EXPORT StringSetRepository
    : public Utils::BasicSetRepository
{
    explicit StringSetRepository(const QString& name, QRecursiveMutex* mutex);
    void itemRemovedFromSets(uint index) override;
    void itemAddedToSets(uint index) override;
};
}

#endif
