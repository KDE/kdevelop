/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef APPENDEDLIST_H
#define APPENDEDLIST_H

#include <QMutex>
#include <QVector>
#include <QStack>
#include <kglobal.h>
#include <util/kdevvarlengtharray.h>

namespace KDevelop {

/**
 * This file contains macros and classes that can be used to conveniently implement classes that store the data of an arbitrary count
 * of additional lists within the same memory block directly behind the class data, in a way that one the whole data can be stored by one copy-operation
 * to another place, like needed in ItemRepository. These macros simplify having two versions of a class: One that has its lists attached in memory,
 * and one version that has them contained as a directly accessible KDevVarLengthArray. Both versions have their lists accessible through access-functions,
 * have a completeSize() function that computes the size of the one-block version, and a copyListsFrom(..) function which can copy the lists from one
 * version to the other.
 *
 * @warning Always follow these rules:
 * You must call initalizeAppendedLists(bool) on construction, also in any copy-constructor, but before calling copyFrom(..).
 * The parameter to that function should be whether the lists in the items should be dynamic, and thus most times "true".
 * You must call freeAppendedLists() on destruction, our you will be leaking memory(only when dynamic)
 *
 * For each embedded list, you must use macros to define a global hash that will be used to allocate the temporary lists, example fir identifier.cpp:
 * DEFINE_LIST_MEMBER_HASH(IdentifierPrivate, templateIdentifiers, uint);
 *
 * See identifier.cpp for an example how to use these classes. @todo Document this a bit more
 * */


enum {
  DynamicAppendedListMask = 1 << 31
};

/**
 * Manages a repository of items for temporary usage. The items will be allocated with an index on alloc(),
 * and freed on free(index). When freed, the same index will be re-used for a later allocation, thus no real allocations
 * will be happening in most cases.
 * The returned indices will always be ored with DynamicAppendedListMask.
 *
 */
template<class T, bool threadSafe = true>
class TemporaryDataManager {
  enum {
    DynamicAppendedListRevertMask = 0xffffffff - DynamicAppendedListMask
  };
  public:
    TemporaryDataManager() {
    }
    ~TemporaryDataManager() {
      foreach(T* item, m_items)
        delete item;
    }

    T& getItem(uint index) {
      Q_ASSERT(index & DynamicAppendedListMask);
      index &= DynamicAppendedListRevertMask;

      if(threadSafe)
        m_mutex.lock();

      T& ret(*m_items[index]);

      if(threadSafe)
        m_mutex.unlock();

      return ret;
    }

    ///Allocates an item index, which from now on you can get using getItem, until you call free(..) on the index.
    ///The returned item is not initialized and may contain random older content, so you should clear it after getting it for the first time
    uint alloc() {
      if(threadSafe)
        m_mutex.lock();

      uint ret;

      if(!m_freeIndices.isEmpty()) {
        ret = m_freeIndices.pop();
      }else{
        ret = m_items.size();
        m_items.append(new T);
      }

      if(threadSafe)
        m_mutex.unlock();

      Q_ASSERT(!(ret & DynamicAppendedListMask));

      return ret | DynamicAppendedListMask;
    }

    void free(uint index) {
      Q_ASSERT(index & DynamicAppendedListMask);
      index &= DynamicAppendedListRevertMask;

      if(threadSafe)
        m_mutex.lock();

      m_freeIndices.push(index);

      if(threadSafe)
        m_mutex.unlock();
    }

  private:
    QVector<T*> m_items;
    QStack<uint> m_freeIndices;
    QMutex m_mutex;
};

///Foreach macro that takes a container and a function-name, and will iterate through the vector returned by that function, using the length returned by the function-name with "Size" appended.
//This might be a little slow
#define FOREACH_FUNCTION(item, container) for(uint a = 0, mustDo = 1; a < container ## Size(); ++a) if((mustDo == 0 || mustDo == 1) && (mustDo = 2)) for(item(container()[a]); mustDo; mustDo = 0)

#define DEFINE_LIST_MEMBER_HASH(container, member, type) \
    typedef TemporaryDataManager<KDevVarLengthArray<type, 10> > temporaryHash ## container ## member ## Type; \
    K_GLOBAL_STATIC(temporaryHash ## container ## member ## Type, temporaryHash ## container ## member ## Static) \
    temporaryHash ## container ## member ## Type& temporaryHash ## container ## member() { \
        return *temporaryHash ## container ## member ## Static; \
    }

#define DECLARE_LIST_MEMBER_HASH(container, member, type) KDevelop::TemporaryDataManager<KDevVarLengthArray<type, 10> >& temporaryHash ## container ## member();

///This implements the interfaces so this container can be used as a predecessor for classes with appended lists.
///You should do this within the abstract base class that opens a tree of classes that can have appended lists,
///so each class that uses them, can also give its predecessor to START_APPENDE_LISTS, to increase flexibility.
///This  creates a boolean entry that is initialized when initializeAppendedLists is called.
///You can call appendedListsDynamic() to find out whether the item is marked as dynamic.
///When this item is used, the same rules have to be followed as for a class with appended lists: You have to call
///initializeAppendedLists(...) and freeAppendedLists(..)
///Also, when you use this, you have to implement a size_t classSize() function, that returns the size of the class including derived classes,
///but not including the dynamic data. Optionally you can implement a static bool appendedListDynamicDefault() function, that returns the default-value for the "dynamic" parameter.
///to initializeAppendedLists.
#define APPENDED_LISTS_STUB(container) \
bool m_dynamic : 1;                          \
unsigned int offsetBehindLastList() const { return 0; } \
size_t dynamicSize() const { return classSize(); } \
template<class T> bool listsEqual(const T& /*rhs*/) const { return true; } \
template<class T> void copyAllFrom(const T& /*rhs*/) const { } \
void initializeAppendedLists(bool dynamic = appendedListDynamicDefault()) { m_dynamic = dynamic; }  \
void freeAppendedLists() { } \
bool appendedListsDynamic() const { return m_dynamic; }


///use this if the class does not have a base class that also uses appended lists
#define START_APPENDED_LISTS(container) \
unsigned int offsetBehindBase() const { return 0; }

///Use this if one of the base-classes of the container also has the appended lists interfaces implemented.
///To reduce the probability of future problems, you should give the direct base class this one inherits from.
///@note: Multiple inheritance is not supported, however it will work ok if only one of the base-classes uses appended lists.
#define START_APPENDED_LISTS_BASE(container, base) \
unsigned int offsetBehindBase() const { return base :: offsetBehindLastList(); }


#define APPENDED_LIST_COMMON(container, type, name) \
      uint name ## Data; \
      unsigned int name ## Size() const { if(!appendedListsDynamic()) return name ## Data; else return temporaryHash ## container ## name().getItem(name ## Data).size(); } \
      KDevVarLengthArray<type, 10>& name ## List() { return temporaryHash ## container ## name().getItem(name ## Data); }\
      template<class T> bool name ## Equals(const T& rhs) const { unsigned int size = name ## Size(); return size == rhs.name ## Size() && memcmp( name(), rhs.name(), size * sizeof(type) ) == 0; } \
      template<class T> void name ## CopyFrom( const T& rhs ) { \
        if(appendedListsDynamic()) {  \
          KDevVarLengthArray<type, 10>& item( temporaryHash ## container ## name().getItem(name ## Data) ); \
          item.resize(rhs.name ## Size()); \
        }else{ \
          Q_ASSERT(name ## Data == 0); /*If this triggers, then the list wasn't initialized properly with initializeAppendedLists(..)*/ \
          name ## Data = rhs.name ## Size(); \
        }\
        memcpy(const_cast<type*>(name()), rhs.name(), name ## Size() * sizeof(type)); \
      } \
      void name ## Initialize(bool dynamic) { if(dynamic) {name ## Data = temporaryHash ## container ## name().alloc(); temporaryHash ## container ## name().getItem(name ## Data).clear(); } else {name ## Data = 0;} }  \
      void name ## Free() { if(appendedListsDynamic()) temporaryHash ## container ## name().free(name ## Data); }  \


///@todo Make these things a bit faster(less recursion)

#define APPENDED_LIST_FIRST(container, type, name)        APPENDED_LIST_COMMON(container, type, name) \
                                               const type* name() const { if(!appendedListsDynamic()) return (type*)(((char*)this) + classSize() + offsetBehindBase()); else return temporaryHash ## container ## name().getItem(name ## Data).data(); } \
                                               unsigned int name ## OffsetBehind() const { return name ## Size() * sizeof(type) + offsetBehindBase(); } \
                                               template<class T> bool name ## ListChainEquals( const T& rhs ) const { return name ## Equals(rhs); } \
                                               template<class T> void name ## CopyAllFrom( const T& rhs ) { name ## CopyFrom(rhs); } \
                                               void name ## InitializeChain(bool dynamic) { name ## Initialize(dynamic); }  \
                                               void name ## FreeChain() { name ## Free(); }

#define APPENDED_LIST(container, type, name, predecessor) APPENDED_LIST_COMMON(container, type, name) \
                                               const type* name() const { if(!appendedListsDynamic()) return (type*)(((char*)this) + classSize() + predecessor ## OffsetBehind()); else return temporaryHash ## container ## name().getItem(name ## Data).data();  } \
                                               unsigned int name ## OffsetBehind() const { return name ## Size() * sizeof(type) + predecessor ## OffsetBehind(); } \
                                               template<class T> bool name ## ListChainEquals( const T& rhs ) const { return name ## Equals(rhs) && predecessor ## ListChainEquals(rhs); } \
                                               template<class T> void name ## CopyAllFrom( const T& rhs ) { predecessor ## CopyAllFrom(rhs); name ## CopyFrom(rhs); } \
                                               void name ## InitializeChain(bool dynamic) { name ## Initialize(dynamic); predecessor ## InitializeChain(dynamic);  }  \
                                               void name ## FreeChain() { name ## Free(); predecessor ## Free(); }

#define END_APPENDED_LISTS(container, predecessor) /* Returns the size of the object containing the appended lists, including them */ \
                                      unsigned int completeSize() const { return classSize() + predecessor ## OffsetBehind(); } \
                                     /* Compares all local appended lists(not from base classes) and returns true if they are equal */                \
                                      template<class T> bool listsEqual(const T& rhs) const { return predecessor ## ListChainEquals(rhs); } \
                                     /* Copies all the local appended lists(not from base classes) from the given item.*/   \
                                      template<class T> void copyListsFrom(const T& rhs) { return predecessor ## CopyAllFrom(rhs); } \
                                      void initializeAppendedLists(bool dynamic = appendedListDynamicDefault()) { predecessor ## Data = (dynamic ? KDevelop::DynamicAppendedListMask : 0); predecessor ## InitializeChain(dynamic); } \
                                      void freeAppendedLists() { if(appendedListsDynamic()) predecessor ## FreeChain(); } \
                                      bool appendedListsDynamic() const { return predecessor ## Data & KDevelop::DynamicAppendedListMask; } \
                                      unsigned int offsetBehindLastList() const { return predecessor ## OffsetBehind(); } \
                                      size_t dynamicSize() const { return offsetBehindLastList() + classSize(); }
/**
 * This is a class that allows you easily putting instances of your class into an ItemRepository as seen in itemrepository.h.
 * All your class needs to do is:
 * - Be implemented using the APPENDED_LIST macros.
 * - Except for these appended lists, only contain directly copyable data like indices(no pointers, no virtual functions)
 * - Implement operator==(..) which should compare everything, including the lists. @warning The default operator will not work!
 * - Implement an operator=(..) function that also copies the lists using copyListsFrom(..)
 * - Implement a hash() function. The hash should equal for two instances when operator==(..) returns true.
 * - Should be completely functional without a constructor called, only the data copied
 * If those conditions are fulfilled, the data can easily be put into a repository using this request class.
 * */

template<class Type, uint averageAppendedBytes = 8>
class AppendedListItemRequest {
  public:
  AppendedListItemRequest(const Type& item) : m_item(item) {
  }

  enum {
    AverageSize = sizeof(Type) + averageAppendedBytes
  };

  unsigned int hash() const {
    return m_item.hash();
  }

  size_t itemSize() const {
      return m_item.dynamicSize();
  }

  void createItem(Type* item) const {
    item->initializeAppendedLists(false);
    *item = m_item;
  }

  bool equals(const Type* item) const {
    return m_item == *item;
  }

  const Type& m_item;
};
}

///This function is outside of the namespace, so it can always be found. It's used as default-parameter to initializeAppendedLists(..),
///and you can for example implement a function called like this in your local class hierarchy to override this default.
inline bool appendedListDynamicDefault() {
  return true;
}

#endif
