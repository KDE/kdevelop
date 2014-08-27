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

#ifndef KDEVPLATFORM_APPENDEDLIST_STATIC_H
#define KDEVPLATFORM_APPENDEDLIST_STATIC_H

#include <util/kdevvarlengtharray.h>
#include <serialization/referencecounting.h>

namespace KDevelop {

/**
 * This file contains macros and classes that can be used to conveniently
 * implement classes that store the data of an arbitrary count
 * of additional lists within the same memory block directly behind the
 * class data, in a way that one the whole data can be stored by one copy-operation
 * to another place, like needed in ItemRepository. These macros simplify
 * having two versions of a class: One that has its lists attached in memory,
 * and one version that has them contained as a directly accessible
 * KDevVarLengthArray. Both versions have their lists accessible through access-functions,
 * have a completeSize() function that computes the size of the one-block
 * version, and a copyListsFrom(..) function which can copy the lists from one
 * version to the other. The class that contains these lists must have
 * a boolean template parameter called "dynamic".
 * 
 * See identifier.cpp for an example how to use these classes. @todo Document this a bit more
 * */

// Foreach macro that takes a container and a function-name, and will iterate through the vector returned by that function, using the length returned by the function-name with "Size" appended.
//This might be a little slow
#define FOREACH_FUNCTION_STATIC(item, container) \
      for(uint a__ = 0, mustDo__ = 1; a__ < container ## Size(); ++a__) \
        if((mustDo__ == 0 || mustDo__ == 1) && (mustDo__ = 2)) \
          for(item(container()[a__]); mustDo__; mustDo__ = 0)

#define START_APPENDED_LISTS_STATIC(selftype) typedef selftype SelfType;

#define APPENDED_LIST_COMMON_STATIC(type, name) \
      KDevelop::AppendedList<dynamic, type> name ## List; \
      unsigned int name ## Size() const { return name ## List.size(); } \
      template<class T> bool name ## Equals(const T& rhs) const { \
        unsigned int size = name ## Size(); \
        if(size != rhs.name ## Size()) return false; \
        for(uint a = 0; a < size; ++a) {if(!(name()[a] == rhs.name()[a])) return false;} \
        return true; \
      }

///@todo Make these things a bit faster(less recursion)

#define APPENDED_LIST_FIRST_STATIC(type, name) \
      APPENDED_LIST_COMMON_STATIC(type, name) \
      const type* name() const { return name ## List.data( reinterpret_cast<const char*>(this) + sizeof(SelfType) ); } \
      unsigned int name ## OffsetBehind() const { return name ## List.dynamicDataSize(); } \
      template<class T> bool name ## ListChainEquals( const T& rhs ) const { return name ## Equals(rhs); } \
      template<class T> void name ## CopyAllFrom( const T& rhs ) { name ## List.copy(const_cast<type*>(name()), rhs.name(), rhs.name ## Size()); }

#define APPENDED_LIST_STATIC(type, name, predecessor) \
      APPENDED_LIST_COMMON_STATIC(type, name) \
      const type* name() const { return name ## List.data( reinterpret_cast<const char*>(this) + sizeof(SelfType) + predecessor ## OffsetBehind() ); } \
      unsigned int name ## OffsetBehind() const { return name ## List.dynamicDataSize() + predecessor ## OffsetBehind(); } \
      template<class T> bool name ## ListChainEquals( const T& rhs ) const { return name ## Equals(rhs) && predecessor ## ListChainEquals(rhs); } \
      template<class T> void name ## CopyAllFrom( const T& rhs ) { name ## List.copy(const_cast<type*>(name()), rhs.name(), rhs.name ## Size()); predecessor ## CopyAllFrom(); }

#define END_APPENDED_LISTS_STATIC(predecessor) \
      /* Returns the size of the object containing the appended lists, including them */ \
      unsigned int completeSize() const { return sizeof(SelfType) + predecessor ## OffsetBehind(); } \
      unsigned int lastOffsetBehind() const { return predecessor ## OffsetBehind(); } \
      /* Compares all appended lists and returns true if they are equal */                \
      template<class T> bool listsEqual(const T& rhs) const { return predecessor ## ListChainEquals(rhs); } \
      template<class T> void copyListsFrom(const T& rhs) { return predecessor ## CopyAllFrom(rhs); }

template<bool dynamic, class T>
class AppendedList : public KDevVarLengthArray<T, 10> {
  public: 
    unsigned int dynamicDataSize() const {
      return this->size() * sizeof(T);
    }
    const T* data(const char* /*position*/) const {
      return KDevVarLengthArray<T, 10>::data();
    }
    void copy(T* /*target*/, const T* data, uint size) {
      Q_ASSERT(!shouldDoDUChainReferenceCounting(KDevVarLengthArray<T, 10>::data()));
      bool empty = KDevVarLengthArray<T, 10>::isEmpty();
      Q_ASSERT(empty);
      Q_UNUSED(empty);
      for(uint a = 0; a < size; ++a)
        this->append(data[a]);
    }
    
    void free(T*) {
      Q_ASSERT(!shouldDoDUChainReferenceCounting(KDevVarLengthArray<T, 10>::data()));
    }
};

template<class T> 
class AppendedList<false, T> {
  public:
    AppendedList() : listSize(0) {
    }
    unsigned int listSize;
    unsigned int size() const {
      return listSize;
    }
    
    void free(T* position) {
      if(listSize)
        Q_ASSERT(shouldDoDUChainReferenceCounting(position)); //Since it's constant, it must be in the repository
      for(unsigned int a = 0; a < listSize; ++a)
        (position+a)->~T();
    }

    //currentOffset should point to the position where the data of this item should be saved
    const T* data(const char* position) const {
      return reinterpret_cast<const T*>(position);
    }
    //Count of bytes that were appended
    unsigned int dynamicDataSize() const {
      return listSize * sizeof(T);
    }
    void copy(T* target, const T* data, uint size) {
      if(size)
        Q_ASSERT(shouldDoDUChainReferenceCounting(target)); //Since it's constant, it must be in the repository
      for(uint a = 0; a < size; ++a)
        new (target+a) T(data[a]); //Properly call all the copy constructors
      listSize = size;
    }
};
}

#endif
