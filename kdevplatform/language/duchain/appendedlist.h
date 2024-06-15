/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_APPENDEDLIST_H
#define KDEVPLATFORM_APPENDEDLIST_H

#include <QList>
#include <QMutex>
#include <QVector>

#include <util/kdevvarlengtharray.h>
#include <util/stack.h>

#include <ctime>
#include <iostream>

namespace KDevelop {
class AbstractItemRepository;
/**
 * This file contains macros and classes that can be used to conveniently implement classes that store the data of an arbitrary count
 * of additional lists within the same memory block directly behind the class data, in a way that one the whole data can be stored by one copy-operation
 * to another place, like needed in ItemRepository. These macros simplify having two versions of a class: One that has its lists attached in memory,
 * and one version that has them contained as a directly accessible KDevVarLengthArray. Both versions have their lists accessible through access-functions,
 * have a completeSize() function that computes the size of the one-block version, and a copyListsFrom(..) function which can copy the lists from one
 * version to the other.
 *
 * @warning Always follow these rules:
 * \li You must call initializeAppendedLists(bool) on construction, also in any copy-constructor, but before calling copyFrom(..).
 * \li The parameter to that function should be whether the lists in the items should be dynamic, and thus most times "true".
 * \li You must call freeAppendedLists() on destruction, our you will be leaking memory(only when dynamic)
 *
 * For each embedded list, you must use macros to define a global hash that will be used to allocate the temporary lists.
 * For example in @c identifier.cpp we have:
 *
 * @code
 * DEFINE_LIST_MEMBER_HASH(IdentifierPrivate, templateIdentifiers, uint);
 * @endcode
 *
 * In general, see @c identifier.cpp for an example on how to use these macros.
 *
 * @todo Document this a bit more
 * */

enum {
    DynamicAppendedListMask = 1 << 31
};
enum {
    DynamicAppendedListRevertMask = ~DynamicAppendedListMask
};
/**
 * Manages a repository of items for temporary usage. The items will be allocated with an index on alloc(),
 * and freed on free(index). When freed, the same index will be re-used for a later allocation, thus no real allocations
 * will be happening in most cases.
 * The returned indices will always be ored with DynamicAppendedListMask.
 *
 */
template <class T, bool threadSafe = true>
class TemporaryDataManager
{
public:
    explicit TemporaryDataManager(const QByteArray& id = {})
        : m_id(id)
    {
        int first = alloc(); //Allocate the zero item, just to reserve that index
        Q_ASSERT(first == ( int )DynamicAppendedListMask);
        Q_UNUSED(first);
    }
    ~TemporaryDataManager()
    {
        free(DynamicAppendedListMask); //Free the zero index, so we don't get wrong warnings
        int cnt = usedItemCount();
        if (cnt) //Don't use qDebug, because that may not work during destruction
            std::cout << m_id.constData() << " There were items left on destruction: " << usedItemCount() << "\n";

        qDeleteAll(m_items);
    }

    inline T& item(int index)
    {
        //For performance reasons this function does not lock the mutex, it's called too often and must be
        //extremely fast. There is special measures in alloc() to make this safe.
        Q_ASSERT(index & DynamicAppendedListMask);

        return *m_items.at(index & KDevelop::DynamicAppendedListRevertMask);
    }

    ///Allocates an item index, which from now on you can get using item(), until you call free(..) on the index.
    ///The returned item is not initialized and may contain random older content, so you should clear it after getting it for the first time
    int alloc()
    {
        if (threadSafe)
            m_mutex.lock();

        int ret;
        if (!m_freeIndicesWithData.isEmpty()) {
            ret = m_freeIndicesWithData.pop();
        } else if (!m_freeIndices.isEmpty()) {
            ret = m_freeIndices.pop();
            Q_ASSERT(!m_items.at(ret));
            m_items[ret] = new T;
        } else {
            if (m_items.size() >= m_items.capacity()) {
                //We need to re-allocate
                const int newItemsSize = m_items.capacity() + 20 + (m_items.capacity() / 3);
                const QVector<T*> oldItems = m_items; // backup
                m_items.reserve(newItemsSize); // detach, grow container

                const auto now = time(nullptr);

                // We do this in this place so it isn't called too often. The result is that we will always have some additional data around.
                // However the index itself should anyway not consume too much data.
                while (!m_deleteLater.isEmpty()) {
                    // We delete only after 5 seconds
                    if (now - m_deleteLater.first().first <= 5) {
                        break;
                    }
                    m_deleteLater.removeFirst();
                }

                //The only function that does not lock the mutex is item(..), because that function must be very efficient.
                //Since it's only a few instructions from the moment m_items is read to the moment it's used,
                //deleting the old data after a few seconds should be safe.
                m_deleteLater.append(qMakePair(now, oldItems));
            }

            ret = m_items.size();
            m_items.append(new T);
            Q_ASSERT(m_items.size() <= m_items.capacity());
        }

        if (threadSafe)
            m_mutex.unlock();

        Q_ASSERT(!(ret & DynamicAppendedListMask));

        return ret | DynamicAppendedListMask;
    }

    void free(int index)
    {
        Q_ASSERT(index & DynamicAppendedListMask);
        index &= KDevelop::DynamicAppendedListRevertMask;

        if (threadSafe)
            m_mutex.lock();

        freeItem(m_items.at(index));

        m_freeIndicesWithData.push(index);

        //Hold the amount of free indices with data between 100 and 200
        if (m_freeIndicesWithData.size() > 200) {
            for (int a = 0; a < 100; ++a) {
                int deleteIndexData = m_freeIndicesWithData.pop();
                auto& item = m_items[deleteIndexData];
                delete item;
                item = nullptr;
                m_freeIndices.push(deleteIndexData);
            }
        }

        if (threadSafe)
            m_mutex.unlock();
    }

    int usedItemCount() const
    {
        int ret = 0;
        for (auto* item : m_items) {
            if (item) {
                ++ret;
            }
        }

        return ret - m_freeIndicesWithData.size();
    }

private:
    //To save some memory, clear the lists
    void freeItem(T* item)
    {
        item->clear(); ///@todo make this a template specialization that only does this for containers
    }

    Q_DISABLE_COPY(TemporaryDataManager)

private:
    QVector<T*> m_items; /// note: non-shared, ref count of 1 when accessed with non-const methods => no detach
    Stack<int> m_freeIndicesWithData;
    Stack<int> m_freeIndices;
    QMutex m_mutex;
    QByteArray m_id;
    QList<QPair<time_t, QVector<T*>>> m_deleteLater;
};

///Foreach macro that takes a container and a function-name, and will iterate through the vector returned by that function, using the length returned by the function-name with "Size" appended.
//This might be a little slow
#define FOREACH_FUNCTION(item, container) \
    for (uint a__ = 0, mustDo__ = 1, containerSize = container ## Size(); a__ < containerSize; ++a__) \
        if ((mustDo__ == 0 || mustDo__ == 1) && (mustDo__ = 2)) \
            for (item(container()[a__]); mustDo__; mustDo__ = 0)

#define DEFINE_LIST_MEMBER_HASH(container, member, type) \
    using temporaryHash ## container ## member ## Type = KDevelop::TemporaryDataManager<KDevVarLengthArray<type, 10>>; \
    Q_GLOBAL_STATIC_WITH_ARGS(temporaryHash ## container ## member ## Type, \
                              temporaryHash ## container ## member ## Static, ( # container "::" # member)) \
    temporaryHash ## container ## member ## Type & temporaryHash ## container ## member() { \
        return *temporaryHash ## container ## member ## Static; \
    }

#define DECLARE_LIST_MEMBER_HASH(container, member, type) \
    KDevelop::TemporaryDataManager<KDevVarLengthArray<type, 10>> &temporaryHash ## container ## member();

///This implements the interfaces so this container can be used as a predecessor for classes with appended lists.
///You should do this within the abstract base class that opens a tree of classes that can have appended lists,
///so each class that uses them, can also give its predecessor to START_APPENDE_LISTS, to increase flexibility.
///This  creates a boolean entry that is initialized when initializeAppendedLists is called.
///You can call appendedListsDynamic() to find out whether the item is marked as dynamic.
///When this item is used, the same rules have to be followed as for a class with appended lists: You have to call
///initializeAppendedLists(...) and freeAppendedLists(..)
///Also, when you use this, you have to implement a uint classSize() function, that returns the size of the class including derived classes,
///but not including the dynamic data. Optionally you can implement a static bool appendedListDynamicDefault() function, that returns the default-value for the "dynamic" parameter.
///to initializeAppendedLists.
#define APPENDED_LISTS_STUB(container) \
    bool m_dynamic : 1;                          \
    unsigned int offsetBehindLastList() const { return 0; } \
    uint dynamicSize() const { return classSize(); } \
    template <class T> bool listsEqual(const T& /*rhs*/) const { return true; } \
    template <class T> void copyAllFrom(const T& /*rhs*/) const { } \
    void initializeAppendedLists(bool dynamic = appendedListDynamicDefault()) { m_dynamic = dynamic; }  \
    void freeAppendedLists() { } \
    bool appendedListsDynamic() const { return m_dynamic; }

///use this if the class does not have a base class that also uses appended lists
#define START_APPENDED_LISTS(container) \
    unsigned int offsetBehindBase() const { return 0; } \
    void freeDynamicData() { freeAppendedLists(); }

///Use this if one of the base-classes of the container also has the appended lists interfaces implemented.
///To reduce the probability of future problems, you should give the direct base class this one inherits from.
///@note: Multiple inheritance is not supported, however it will work ok if only one of the base-classes uses appended lists.
#define START_APPENDED_LISTS_BASE(container, base) \
    unsigned int offsetBehindBase() const { return base :: offsetBehindLastList(); } \
    void freeDynamicData() { freeAppendedLists(); base::freeDynamicData(); }

#define APPENDED_LIST_COMMON(container, type, name) \
    uint name ## Data; \
    unsigned int name ## Size() const { \
        if ((name ## Data & KDevelop::DynamicAppendedListRevertMask) == 0) \
            return 0; \
        if (!appendedListsDynamic()) \
            return name ## Data; \
        return temporaryHash ## container ## name().item(name ## Data).size(); \
    } \
    KDevVarLengthArray<type, 10>& name ## List() { \
        name ## NeedDynamicList(); \
        return temporaryHash ## container ## name().item(name ## Data); \
    } \
    template <class T> bool name ## Equals(const T &rhs) const { \
        unsigned int size = name ## Size(); \
        if (size != rhs.name ## Size()) \
            return false; \
        for (uint a = 0; a < size; ++a) { \
            if (!(name()[a] == rhs.name()[a])) \
                return false; \
        } \
        return true; \
    } \
    template <class T> void name ## CopyFrom(const T &rhs) { \
        if (rhs.name ## Size() == 0 && (name ## Data & KDevelop::DynamicAppendedListRevertMask) == 0) \
            return; \
        if (appendedListsDynamic()) {  \
            name ## NeedDynamicList(); \
            KDevVarLengthArray<type, 10>& item(temporaryHash ## container ## name().item(name ## Data)); \
            item.clear();                    \
            const type* otherCurr = rhs.name(); \
            const type* otherEnd = otherCurr + rhs.name ## Size(); \
            for (; otherCurr < otherEnd; ++otherCurr) \
                item.append(*otherCurr); \
        } else { \
            Q_ASSERT(name ## Data == 0); /* It is dangerous to overwrite the contents of non-dynamic lists(Most probably a mistake) */ \
            name ## Data = rhs.name ## Size(); \
            auto* curr = const_cast<type*>(name()); \
            type* end = curr + name ## Size(); \
            const type* otherCurr = rhs.name(); \
            for (; curr < end; ++curr, ++otherCurr) \
                new (curr) type(*otherCurr); /* Call the copy constructors */ \
        } \
    } \
    void name ## NeedDynamicList() { \
        Q_ASSERT(appendedListsDynamic()); \
        if ((name ## Data & KDevelop::DynamicAppendedListRevertMask) == 0) { \
            name ## Data = temporaryHash ## container ## name().alloc(); \
            Q_ASSERT(temporaryHash ## container ## name().item(name ## Data).isEmpty()); \
        } \
    } \
    void name ## Initialize(bool dynamic) { name ## Data = (dynamic ? KDevelop::DynamicAppendedListMask : 0); }  \
    void name ## Free() { \
        if (appendedListsDynamic()) { \
            if (name ## Data & KDevelop::DynamicAppendedListRevertMask) \
                temporaryHash ## container ## name().free(name ## Data); \
        } else { \
            auto* curr = const_cast<type*>(name()); \
            type* end = curr + name ## Size(); \
            for (; curr < end; ++curr) \
                curr->~type();                     /*call destructors*/ \
        } \
    }  \


///@todo Make these things a bit faster(less recursion)

#define APPENDED_LIST_FIRST(container, type, name) \
    APPENDED_LIST_COMMON(container, type, name) \
    const type * name() const { \
        if ((name ## Data & KDevelop::DynamicAppendedListRevertMask) == 0) \
            return nullptr; \
        if (!appendedListsDynamic()) \
            return reinterpret_cast<const type*>(reinterpret_cast<const char*>(this) + classSize() + \
                                                 offsetBehindBase()); \
        else \
            return temporaryHash ## container ## name().item(name ## Data).data(); \
    } \
    unsigned int name ## OffsetBehind() const { return name ## Size() * sizeof(type) + offsetBehindBase(); } \
    template <class T> bool name ## ListChainEquals(const T &rhs) const { return name ## Equals(rhs); } \
    template <class T> void name ## CopyAllFrom(const T &rhs) { name ## CopyFrom(rhs); } \
    void name ## InitializeChain(bool dynamic) { name ## Initialize(dynamic); }  \
    void name ## FreeChain() { name ## Free(); }

#define APPENDED_LIST(container, type, name, predecessor) \
    APPENDED_LIST_COMMON(container, type, name) \
    const type * name() const { \
        if ((name ## Data & KDevelop::DynamicAppendedListRevertMask) == 0) \
            return nullptr; \
        if (!appendedListsDynamic()) \
            return reinterpret_cast<const type*>(reinterpret_cast<const char*>(this) + classSize() + \
                                                 predecessor ## OffsetBehind()); \
        else \
            return temporaryHash ## container ## name().item(name ## Data).data(); \
    } \
    unsigned int name ## OffsetBehind() const { return name ## Size() * sizeof(type) + predecessor ## OffsetBehind(); } \
    template <class T> bool name ## ListChainEquals(const T &rhs) const { return name ## Equals(rhs) && \
                                                                                 predecessor ## ListChainEquals(rhs); } \
    template <class T> void name ## CopyAllFrom(const T &rhs) { predecessor ## CopyAllFrom(rhs); name ## CopyFrom(rhs); \
    } \
    void name ## InitializeChain(bool dynamic) { name ## Initialize(dynamic); predecessor ## InitializeChain(dynamic); \
    }  \
    void name ## FreeChain() { name ## Free(); predecessor ## FreeChain(); }

#define END_APPENDED_LISTS(container, predecessor) \
    /* Returns the size of the object containing the appended lists, including them */ \
    unsigned int completeSize() const { return classSize() + predecessor ## OffsetBehind(); } \
    /* Compares all local appended lists(not from base classes) and returns true if they are equal */                \
    template <class T> bool listsEqual(const T &rhs) const { return predecessor ## ListChainEquals(rhs); } \
    /* Copies all the local appended lists(not from base classes) from the given item.*/   \
    template <class T> void copyListsFrom(const T &rhs) { return predecessor ## CopyAllFrom(rhs); } \
    void initializeAppendedLists(bool dynamic = appendedListDynamicDefault()) { \
        predecessor ## Data = (dynamic ? KDevelop::DynamicAppendedListMask : 0); \
        predecessor ## InitializeChain(dynamic); \
    } \
    void freeAppendedLists() { predecessor ## FreeChain(); } \
    bool appendedListsDynamic() const { return predecessor ## Data & KDevelop::DynamicAppendedListMask; } \
    unsigned int offsetBehindLastList() const { return predecessor ## OffsetBehind(); } \
    uint dynamicSize() const { return offsetBehindLastList() + classSize(); }

/**
 * This is a class that allows you easily putting instances of your class into an ItemRepository as seen in itemrepository.h.
 * All your class needs to do is:
 * - Be implemented using the APPENDED_LIST macros.
 * - Have a real copy-constructor that additionally takes a "bool dynamic = true" parameter, which should be given to initializeAppendedLists
 * - Except for these appended lists, only contain directly copyable data like indices(no pointers, no virtual functions)
 * - Implement operator==(..) which should compare everything, including the lists. @warning The default operator will not work!
 * - Implement a hash() function. The hash should equal for two instances when operator==(..) returns true.
 * - Should be completely functional without a constructor called, only the data copied
 * - Implement a "bool persistent() const" function, that should check the reference-count or other information to decide whether the item should stay in the repository
 * If those conditions are fulfilled, the data can easily be put into a repository using this request class.
 * */

template <class Type, uint averageAppendedBytes = 8>
class AppendedListItemRequest
{
public:
    AppendedListItemRequest(const Type& item) : m_item(item)
    {
    }

    enum {
        AverageSize = sizeof(Type) + averageAppendedBytes
    };

    auto hash() const
    {
        return m_item.hash();
    }

    uint itemSize() const
    {
        return m_item.dynamicSize();
    }

    void createItem(Type* item) const
    {
        new (item) Type(m_item, false);
    }

    static void destroy(Type* item, KDevelop::AbstractItemRepository&)
    {
        item->~Type();
    }

    static bool persistent(const Type* item)
    {
        return item->persistent();
    }

    bool equals(const Type* item) const
    {
        return m_item == *item;
    }

    const Type& m_item;
};
}

///This function is outside of the namespace, so it can always be found. It's used as default-parameter to initializeAppendedLists(..),
///and you can for example implement a function called like this in your local class hierarchy to override this default.
inline bool appendedListDynamicDefault()
{
    return true;
}

#endif
