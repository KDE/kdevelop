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

#ifndef KDEVPLATFORM_ITEMREPOSITORY_H
#define KDEVPLATFORM_ITEMREPOSITORY_H

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <KMessageBox>
#include <KLocalizedString>
#include <KDebug>

#include <language/duchain/referencecounting.h>

#include "abstractitemrepository.h"
#include "repositorymanager.h"
#include "itemrepositoryregistry.h"

//#define DEBUG_MONSTERBUCKETS

// #define DEBUG_ITEMREPOSITORY_LOADING
// #define ifDebugInfiniteRecursion(x) x
#define ifDebugInfiniteRecursion(x)

// #define ifDebugLostSpace(x) x
#define ifDebugLostSpace(x)
// #define DEBUG_INCORRECT_DELETE

//Makes sure that all items stay reachable through the basic hash
// #define DEBUG_ITEM_REACHABILITY

///@todo Dynamic bucket hash size

#ifdef DEBUG_ITEM_REACHABILITY
#define ENSURE_REACHABLE(bucket) Q_ASSERT(allItemsReachable(bucket));
#define IF_ENSURE_REACHABLE(x) x
#else
#define ENSURE_REACHABLE(bucket)
#define IF_ENSURE_REACHABLE(x)
#endif

///Do not enable this #define, the issue it catches is non-critical and happens on a regular basis
// #define DEBUG_HASH_SEQUENCES

#define ITEMREPOSITORY_USE_MMAP_LOADING

//Assertion macro that prevents warnings if debugging is disabled
//Only use it to verify values, it should not call any functions, since else the function will even be called in release mode
#ifdef QT_NO_DEBUG
#define VERIFY(X) if(!(X)) {kWarning() << "Failed to verify expression" << #X;}
#else
#define VERIFY(X) Q_ASSERT(X)
#endif

///When this is uncommented, a 64-bit test-value is written behind the area an item is allowed to write into before
///createItem(..) is called, and an assertion triggers when it was changed during createItem(), which means createItem wrote too long.
///The problem: This temporarily overwrites valid data in the following item, so it will cause serious problems if that data is accessed
///during the call to createItem().
// #define DEBUG_WRITING_EXTENTS

namespace KDevelop {

/**
 * This file implements a generic bucket-based indexing repository, that can be used for example to index strings.
 *
 * All you need to do is define your item type that you want to store into the repository, and create a request item
 * similar to ExampleItemRequest that compares and fills the defined item type.
 *
 * For example the string repository uses "unsigned short" as item-type, uses that actual value to store the length of the string,
 * and uses the space behind to store the actual string content.
 *
 * @see AbstractItemRepository
 * @see ItemRepository
 *
 * @see ExampleItem
 * @see ExampleItemRequest
 *
 * @see typerepository.h
 * @see stringrepository.h
 * @see indexedstring.h
 */

enum {
  ItemRepositoryBucketSize = 1<<16
};

/**
 * Buckets are the memory-units that are used to store the data in an ItemRepository.
 *
 * About monster buckets: Normally a bucket has a size of 64kb, but when an item is
 * allocated that is larger than that, a "monster bucket" is allocated, which spans the
 * space of multiple buckets.
 */
template<class Item, class ItemRequest, bool markForReferenceCounting, uint fixedItemSize>
class Bucket {
  public:
    enum {
      AdditionalSpacePerItem = 2
    };
    enum {
      ObjectMapSize = ((ItemRepositoryBucketSize / ItemRequest::AverageSize) * 3) / 2 + 1,
      MaxFreeItemsForHide = 0, //When less than this count of free items in one buckets is reached, the bucket is removed from the global list of buckets with free items
      MaxFreeSizeForHide = fixedItemSize ? fixedItemSize : 0, //Only when the largest free size is smaller then this, the bucket is taken from the free list
      MinFreeItemsForReuse = 10,//When this count of free items in one bucket is reached, consider re-assigning them to new requests
      MinFreeSizeForReuse = ItemRepositoryBucketSize/20 //When the largest free item is bigger then this, the bucket is automatically added to the free list
    };
    enum {
      NextBucketHashSize = ObjectMapSize, //Affects the average count of bucket-chains that need to be walked in ItemRepository::index. Must be a multiple of ObjectMapSize
      DataSize = sizeof(char) + sizeof(unsigned int) * 3 + ItemRepositoryBucketSize + sizeof(short unsigned int) * (ObjectMapSize + NextBucketHashSize + 1)
    };
    enum {
      CheckStart = 0xff00ff1,
      CheckEnd = 0xfafcfb
    };
    Bucket() : m_monsterBucketExtent(0), m_available(0), m_data(0), m_mappedData(0), m_objectMap(0), m_objectMapSize(0), m_largestFreeItem(0), m_freeItemCount(0), m_nextBucketHash(0), m_dirty(false) {
    }
    ~Bucket() {
      if(m_data != m_mappedData) {
        delete[] m_data;
        delete[] m_nextBucketHash;
        delete[] m_objectMap;
      }
    }

    void initialize(uint monsterBucketExtent) {
      if(!m_data) {
        m_monsterBucketExtent = monsterBucketExtent;
        m_available = ItemRepositoryBucketSize;
        m_data = new char[ItemRepositoryBucketSize + monsterBucketExtent * DataSize];
        memset(m_data, 0, (ItemRepositoryBucketSize + monsterBucketExtent * DataSize) * sizeof(char));
        //The bigger we make the map, the lower the probability of a clash(and thus bad performance). However it increases memory usage.
        m_objectMapSize = ObjectMapSize;
        m_objectMap = new short unsigned int[m_objectMapSize];
        memset(m_objectMap, 0, m_objectMapSize * sizeof(short unsigned int));
        m_nextBucketHash = new short unsigned int[NextBucketHashSize];
        memset(m_nextBucketHash, 0, NextBucketHashSize * sizeof(short unsigned int));
        m_changed = true;
        m_dirty = false;
        m_lastUsed = 0;
      }
    }

    template<class T>
    void readValue(char*& from, T& to) {
      to = *reinterpret_cast<T*>(from);
      from += sizeof(T);
    }

    void initializeFromMap(char* current) {
      if(!m_data) {
          char* start = current;
          readValue(current, m_monsterBucketExtent);
          Q_ASSERT(current - start == 4);
          readValue(current, m_available);
          m_objectMapSize = ObjectMapSize;
          m_objectMap = reinterpret_cast<short unsigned int*>(current);
          current += sizeof(short unsigned int) * m_objectMapSize;
          m_nextBucketHash = reinterpret_cast<short unsigned int*>(current);
          current += sizeof(short unsigned int) * NextBucketHashSize;
          readValue(current, m_largestFreeItem);
          readValue(current, m_freeItemCount);
          readValue(current, m_dirty);
          m_data = current;
          m_mappedData = current;

          m_changed = false;
          m_lastUsed = 0;
          VERIFY(current - start == (DataSize - ItemRepositoryBucketSize));
      }
    }

    void store(QFile* file, size_t offset) {
      if(!m_data)
        return;

      if(static_cast<size_t>(file->size()) < offset + (1+m_monsterBucketExtent)*DataSize)
        file->resize(offset + (1+m_monsterBucketExtent)*DataSize);

      file->seek(offset);

      file->write((char*)&m_monsterBucketExtent, sizeof(unsigned int));
      file->write((char*)&m_available, sizeof(unsigned int));
      file->write((char*)m_objectMap, sizeof(short unsigned int) * m_objectMapSize);
      file->write((char*)m_nextBucketHash, sizeof(short unsigned int) * NextBucketHashSize);
      file->write((char*)&m_largestFreeItem, sizeof(short unsigned int));
      file->write((char*)&m_freeItemCount, sizeof(unsigned int));
      file->write((char*)&m_dirty, sizeof(bool));
      file->write(m_data, ItemRepositoryBucketSize + m_monsterBucketExtent * DataSize);

      if(static_cast<size_t>(file->pos()) != offset + (1+m_monsterBucketExtent)*DataSize)
      {
        KMessageBox::error(0, i18n("Failed writing to %1, probably the disk is full", file->fileName()));
        abort();
      }
      
      m_changed = false;
#ifdef DEBUG_ITEMREPOSITORY_LOADING
      {
        file->flush();
        file->seek(offset);

        uint available, freeItemCount, monsterBucketExtent;
        short unsigned int largestFree;
        bool dirty;

        short unsigned int* m = new short unsigned int[m_objectMapSize];
        short unsigned int* h = new short unsigned int[NextBucketHashSize];


        file->read((char*)&monsterBucketExtent, sizeof(unsigned int));
        char* d = new char[ItemRepositoryBucketSize + monsterBucketExtent * DataSize];

        file->read((char*)&available, sizeof(unsigned int));
        file->read((char*)m, sizeof(short unsigned int) * m_objectMapSize);
        file->read((char*)h, sizeof(short unsigned int) * NextBucketHashSize);
        file->read((char*)&largestFree, sizeof(short unsigned int));
        file->read((char*)&freeItemCount, sizeof(unsigned int));
        file->read((char*)&dirty, sizeof(bool));
        file->read(d, ItemRepositoryBucketSize);

        Q_ASSERT(monsterBucketExtent == m_monsterBucketExtent);
        Q_ASSERT(available == m_available);
        Q_ASSERT(memcmp(d, m_data, ItemRepositoryBucketSize + monsterBucketExtent * DataSize) == 0);
        Q_ASSERT(memcmp(m, m_objectMap, sizeof(short unsigned int) * m_objectMapSize) == 0);
        Q_ASSERT(memcmp(h, m_nextBucketHash, sizeof(short unsigned int) * NextBucketHashSize) == 0);
        Q_ASSERT(m_largestFreeItem == largestFree);
        Q_ASSERT(m_freeItemCount == freeItemCount);
        Q_ASSERT(m_dirty == dirty);

        Q_ASSERT(static_cast<size_t>(file->pos()) == offset + DataSize + m_monsterBucketExtent * DataSize);

        delete[] d;
        delete[] m;
        delete[] h;
      }
#endif
    }

    inline char* data() {
      return m_data;
    }

    inline uint dataSize() const {
      return ItemRepositoryBucketSize + m_monsterBucketExtent * DataSize;
    }

    //Tries to find the index this item has in this bucket, or returns zero if the item isn't there yet.
    unsigned short findIndex(const ItemRequest& request) const {
      m_lastUsed = 0;

      unsigned short localHash = request.hash() % m_objectMapSize;
      unsigned short index = m_objectMap[localHash];

      unsigned short follower = 0;
      //Walk the chain of items with the same local hash
      while(index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
        index = follower;

      if(index && request.equals(itemFromIndex(index))) {
        return index; //We have found the item
      }

      return 0;
    }

    //Tries to get the index within this bucket, or returns zero. Will put the item into the bucket if there is room.
    //Created indices will never begin with 0xffff____, so you can use that index-range for own purposes.
    unsigned short index(const ItemRequest& request, unsigned int itemSize) {
      m_lastUsed = 0;

      unsigned short localHash = request.hash() % m_objectMapSize;
      unsigned short index = m_objectMap[localHash];
      unsigned short insertedAt = 0;

      unsigned short follower = 0;
      //Walk the chain of items with the same local hash
      while(index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
        index = follower;

      if(index && request.equals(itemFromIndex(index)))
        return index; //We have found the item

      ifDebugLostSpace( Q_ASSERT(!lostSpace()); )

      prepareChange();

      unsigned int totalSize = itemSize + AdditionalSpacePerItem;

      if(m_monsterBucketExtent) {
        ///This is a monster-bucket. Other rules are applied here. Only one item can be allocated, and that must be bigger than the bucket data
        Q_ASSERT(totalSize > ItemRepositoryBucketSize);
        Q_ASSERT(m_available);
        m_available = 0;

        insertedAt = AdditionalSpacePerItem;
        setFollowerIndex(insertedAt, 0);
        Q_ASSERT(m_objectMap[localHash] == 0);
        m_objectMap[localHash] = insertedAt;
        
        if(markForReferenceCounting)
          enableDUChainReferenceCounting(m_data, dataSize());
        
        request.createItem(reinterpret_cast<Item*>(m_data + insertedAt));
      
        if(markForReferenceCounting)
          disableDUChainReferenceCounting(m_data);
        
        return insertedAt;
      }

      //The second condition is needed, else we can get problems with zero-length items and an overflow in insertedAt to zero
      if(totalSize > m_available || (!itemSize && totalSize == m_available)) {
        //Try finding the smallest freed item that can hold the data
        unsigned short currentIndex = m_largestFreeItem;
        unsigned short previousIndex = 0;

        unsigned short freeChunkSize = 0;

        ///@todo Achieve this without full iteration
        while(currentIndex && freeSize(currentIndex) > itemSize) {
          unsigned short follower = followerIndex(currentIndex);
          if(follower && freeSize(follower) >= itemSize) {
            //The item also fits into the smaller follower, so use that one
            previousIndex = currentIndex;
            currentIndex = follower;
          }else{
            //The item fits into currentIndex, but not into the follower. So use currentIndex
            freeChunkSize = freeSize(currentIndex) - itemSize;

            //We need 2 bytes to store the free size
            if(freeChunkSize != 0 && freeChunkSize < AdditionalSpacePerItem+2) {
              //we can not manage the resulting free chunk as a separate item, so we cannot use this position.
              //Just pick the biggest free item, because there we can be sure that
              //either we can manage the split, or we cannot do anything at all in this bucket.

              freeChunkSize = freeSize(m_largestFreeItem) - itemSize;

              if(freeChunkSize == 0 || freeChunkSize >= AdditionalSpacePerItem+2) {
                previousIndex = 0;
                currentIndex = m_largestFreeItem;
              }else{
                currentIndex = 0;
              }
            }
            break;
          }
        }

        if(!currentIndex || freeSize(currentIndex) < (totalSize-AdditionalSpacePerItem))
          return 0;

        if(previousIndex)
          setFollowerIndex(previousIndex, followerIndex(currentIndex));
        else
          m_largestFreeItem = followerIndex(currentIndex);

        --m_freeItemCount; //Took one free item out of the chain

        ifDebugLostSpace( Q_ASSERT((uint)lostSpace() == (uint)(freeSize(currentIndex) + AdditionalSpacePerItem)); )

        if(freeChunkSize) {
          Q_ASSERT(freeChunkSize >= AdditionalSpacePerItem+2);
          unsigned short freeItemSize = freeChunkSize - AdditionalSpacePerItem;

          unsigned short freeItemPosition;
          //Insert the resulting free chunk into the list of free items, so we don't lose it
          if(isBehindFreeSpace(currentIndex)) {
            //Create the free item at the beginning of currentIndex, so it can be merged with the free space in front
            freeItemPosition = currentIndex;
            currentIndex += freeItemSize + AdditionalSpacePerItem;
          }else{
            //Create the free item behind currentIndex
            freeItemPosition = currentIndex + itemSize + AdditionalSpacePerItem;
          }
          setFreeSize(freeItemPosition, freeItemSize);
          insertFreeItem(freeItemPosition);
        }

        insertedAt = currentIndex;
        Q_ASSERT((bool)m_freeItemCount == (bool)m_largestFreeItem);
      }else{
        //We have to insert the item
        insertedAt = ItemRepositoryBucketSize - m_available;

        insertedAt += AdditionalSpacePerItem; //Room for the prepended follower-index

        m_available -= totalSize;
      }

      ifDebugLostSpace( Q_ASSERT(lostSpace() == totalSize); )

      Q_ASSERT(!index || !followerIndex(index));

      Q_ASSERT(!m_objectMap[localHash] || index);

      if(index)
        setFollowerIndex(index, insertedAt);
      setFollowerIndex(insertedAt, 0);

      if(m_objectMap[localHash] == 0)
        m_objectMap[localHash] = insertedAt;


#ifdef DEBUG_CREATEITEM_EXTENTS
      char* borderBehind = m_data + insertedAt + (totalSize-AdditionalSpacePerItem);

      quint64 oldValueBehind = 0;
      if(m_available >= 8) {
        oldValueBehind = *(quint64*)borderBehind;
        *((quint64*)borderBehind) = 0xfafafafafafafafaLLU;
      }
#endif

      //Last thing we do, because createItem may recursively do even more transformation of the repository
      if(markForReferenceCounting)
        enableDUChainReferenceCounting(m_data, dataSize());
        
      request.createItem(reinterpret_cast<Item*>(m_data + insertedAt));

      if(markForReferenceCounting)
        disableDUChainReferenceCounting(m_data);
      
#ifdef DEBUG_CREATEITEM_EXTENTS
      if(m_available >= 8) {
        //If this assertion triggers, then the item writes a bigger range than it advertised in
        Q_ASSERT(*((quint64*)borderBehind) == 0xfafafafafafafafaLLU);
        *((quint64*)borderBehind) = oldValueBehind;
      }
#endif

      Q_ASSERT(itemFromIndex(insertedAt)->hash() == request.hash());
      Q_ASSERT(itemFromIndex(insertedAt)->itemSize() == itemSize);

      ifDebugLostSpace( if(lostSpace()) kDebug() << "lost space:" << lostSpace(); Q_ASSERT(!lostSpace()); )
      
      return insertedAt;
    }

    ///@param modulo Returns whether this bucket contains an item with (hash % modulo) == (item.hash % modulo)
    ///              The default-parameter is the size of the next-bucket hash that is used by setNextBucketForHash and nextBucketForHash
    ///              @param modulo MUST be a multiple of ObjectMapSize, because (b-a) | (x * h1) => (b-a) | h2, where a|b means a is a multiple of b.
    ///                            This this allows efficiently computing the clashes using the local object map hash.

    bool hasClashingItem(uint hash, uint modulo) {

      Q_ASSERT(modulo % ObjectMapSize == 0);

      m_lastUsed = 0;

      uint hashMod = hash % modulo;
      unsigned short localHash = hash % m_objectMapSize;
      unsigned short currentIndex = m_objectMap[localHash];

      if(currentIndex == 0)
        return false;

      while(currentIndex) {
        uint currentHash = itemFromIndex(currentIndex)->hash();

        Q_ASSERT(currentHash % m_objectMapSize == localHash);

        if(currentHash % modulo == hashMod)
          return true; //Clash
        currentIndex = followerIndex(currentIndex);
      }
      return false;
    }

    struct ClashVisitor {
      ClashVisitor(int _hash, int _modulo) : result(0), hash(_hash), modulo(_modulo) {
      }
      bool operator()(const Item* item) {
        if((item->hash() % modulo) == (hash % modulo))
          result = item;

        return true;
      }
      const Item* result;
      uint hash, modulo;
    };
    
    void countFollowerIndexLengths(uint& usedSlots, uint& lengths, uint& slotCount, uint& longestInBucketFollowerChain) {
      for(uint a = 0; a < m_objectMapSize; ++a) {
        unsigned short currentIndex = m_objectMap[a];
        ++slotCount;
        uint length = 0;
        
        if(currentIndex) {
          ++usedSlots;
          
          while(currentIndex) {
            ++length;
            ++lengths;
            currentIndex = followerIndex(currentIndex);
          }
          if(length > longestInBucketFollowerChain) {
//             kDebug() << "follower-chain at" << a << ":" << length;
            
            longestInBucketFollowerChain = length;
          }
        }
      }
    }
    
    //A version of hasClashingItem that visits all items naively without using any assumptions.
    //This was used to debug hasClashingItem, but should not be used otherwise.
    bool hasClashingItemReal(uint hash, uint modulo) {

      m_lastUsed = 0;

      ClashVisitor visitor(hash, modulo);
      visitAllItems<ClashVisitor>(visitor);
      return (bool)visitor.result;
    }

    //Returns whether the given item is reachabe within this bucket, through its hash.
    bool itemReachable(const Item* item, uint hash) const {

      unsigned short localHash = hash % m_objectMapSize;
      unsigned short currentIndex = m_objectMap[localHash];

      while(currentIndex) {
        if(itemFromIndex(currentIndex) == item)
          return true;

        currentIndex = followerIndex(currentIndex);
      }
      return false;
    }

    template<class Visitor>
    bool visitItemsWithHash(Visitor& visitor, uint hash, unsigned short bucketIndex) const {
      m_lastUsed = 0;

      unsigned short localHash = hash % m_objectMapSize;
      unsigned short currentIndex = m_objectMap[localHash];

      while(currentIndex) {
        if(!visitor(itemFromIndex(currentIndex), (bucketIndex << 16) + currentIndex))
          return false;

        currentIndex = followerIndex(currentIndex);
      }
      return true;
    }


    template<class Repository>
    void deleteItem(unsigned short index, unsigned int hash, Repository& repository) {
      ifDebugLostSpace( Q_ASSERT(!lostSpace()); )

      m_lastUsed = 0;
      prepareChange();

      unsigned int size = itemFromIndex(index)->itemSize();
      //Step 1: Remove the item from the data-structures that allow finding it: m_objectMap
      unsigned short localHash = hash % m_objectMapSize;
      unsigned short currentIndex = m_objectMap[localHash];
      unsigned short previousIndex = 0;

      //Fix the follower-link by setting the follower of the previous item to the next one, or updating m_objectMap
      while(currentIndex != index) {
        previousIndex = currentIndex;
        currentIndex = followerIndex(currentIndex);
        //If this assertion triggers, the deleted item was not registered under the given hash
        Q_ASSERT(currentIndex);
      }
      Q_ASSERT(currentIndex == index);

      if(!previousIndex)
        //The item was directly in the object map
        m_objectMap[localHash] = followerIndex(index);
      else
        setFollowerIndex(previousIndex, followerIndex(index));

      Item* item = const_cast<Item*>(itemFromIndex(index));
      
      if(markForReferenceCounting)
        enableDUChainReferenceCounting(m_data, dataSize());
        
      ItemRequest::destroy(item, repository);
        
      if(markForReferenceCounting)
        disableDUChainReferenceCounting(m_data);
      
      memset(item, 0, size); //For debugging, so we notice the data is wrong

      if(m_monsterBucketExtent) {
        ///This is a monster-bucket. Make it completely empty again.
        Q_ASSERT(!m_available);
        m_available = ItemRepositoryBucketSize;

        //Items are always inserted into monster-buckets at a fixed position
        Q_ASSERT(currentIndex == AdditionalSpacePerItem);
        Q_ASSERT(m_objectMap[localHash] == 0);
      }else{
        ///Put the space into the free-set
        setFreeSize(index, size);

        //Try merging the created free item to other free items around, else add it into the free list
        insertFreeItem(index);

        if(m_freeItemCount == 1 && freeSize(m_largestFreeItem) + m_available == ItemRepositoryBucketSize) {
          //Everything has been deleted, there is only free space left. Make the bucket empty again,
          //so it can later also be used as a monster-bucket.
          m_available = ItemRepositoryBucketSize;
          m_freeItemCount = 0;
          m_largestFreeItem = 0;
        }
      }

      Q_ASSERT((bool)m_freeItemCount == (bool)m_largestFreeItem);
      ifDebugLostSpace( Q_ASSERT(!lostSpace()); )
#ifdef DEBUG_INCORRECT_DELETE
      //Make sure the item cannot be found any more
      {
        unsigned short localHash = hash % m_objectMapSize;
        unsigned short currentIndex = m_objectMap[localHash];

        while(currentIndex && currentIndex != index) {
          previousIndex = currentIndex;
          currentIndex = followerIndex(currentIndex);
        }
        Q_ASSERT(!currentIndex); //The item must not be found
      }
#endif
//       Q_ASSERT(canAllocateItem(size));
    }

    ///@warning The returned item may be in write-protected memory, so never try doing a const_cast and changing some data
    ///         If you need to change something, use dynamicItemFromIndex
    ///@warning When using multi-threading, mutex() must be locked as long as you use the returned data
    inline const Item* itemFromIndex(unsigned short index) const {
      m_lastUsed = 0;
      return reinterpret_cast<Item*>(m_data+index);
    }

    bool isEmpty() const {
      return m_available == ItemRepositoryBucketSize;
    }

    ///Returns true if this bucket has no nextBucketForHash links
    bool noNextBuckets() const {
      for(int a = 0; a < NextBucketHashSize; ++a)
        if(m_nextBucketHash[a])
          return false;
      return true;
    }

    uint available() const {
      return m_available;
    }

    uint usedMemory() const {
      return ItemRepositoryBucketSize - m_available;
    }

    template<class Visitor>
    bool visitAllItems(Visitor& visitor) const {
      m_lastUsed = 0;
      for(uint a = 0; a < m_objectMapSize; ++a) {
        uint currentIndex = m_objectMap[a];
        while(currentIndex) {
          //Get the follower early, so there is no problems when the current
          //index is removed
          
          if(!visitor(reinterpret_cast<const Item*>(m_data+currentIndex)))
            return false;

          currentIndex = followerIndex(currentIndex);
        }
      }
      return true;
    }
    
    ///Returns whether something was changed
    template<class Repository>
    int finalCleanup(Repository& repository) {
      int changed = 0;
      
      while(m_dirty) {
        m_dirty = false;
        
        for(uint a = 0; a < m_objectMapSize; ++a) {
          uint currentIndex = m_objectMap[a];
          while(currentIndex) {
            //Get the follower early, so there is no problems when the current
            //index is removed
            
            const Item* item = reinterpret_cast<const Item*>(m_data+currentIndex);
            
            if(!ItemRequest::persistent(item)) {
              changed += item->itemSize();
              deleteItem(currentIndex, item->hash(), repository);
              m_dirty = true; //Set to dirty so we re-iterate
              break;
            }

            currentIndex = followerIndex(currentIndex);
          }
        }
      }
      return changed;
    }

    unsigned short nextBucketForHash(uint hash) const {
      m_lastUsed = 0;
      return m_nextBucketHash[hash % NextBucketHashSize];
    }

    void setNextBucketForHash(unsigned int hash, unsigned short bucket) {
      m_lastUsed = 0;
      prepareChange();
      m_nextBucketHash[hash % NextBucketHashSize] = bucket;
    }

    uint freeItemCount() const {
      return m_freeItemCount;
    }

    short unsigned int totalFreeItemsSize() const {
      short unsigned int ret = 0;
      short unsigned int currentIndex = m_largestFreeItem;
      while(currentIndex) {
        ret += freeSize(currentIndex);
        currentIndex = followerIndex(currentIndex);
      }
      return ret;
    }

    //Size of the largest item that could be inserted into this bucket
    short unsigned int largestFreeSize() const {
      short unsigned int ret = 0;
      if(m_largestFreeItem)
        ret = freeSize(m_largestFreeItem);
      if(m_available > (uint)(AdditionalSpacePerItem + (uint)ret)) {
        ret = m_available - AdditionalSpacePerItem;
        Q_ASSERT(ret == (m_available - AdditionalSpacePerItem));
      }
      return ret;
    }

    bool canAllocateItem(unsigned int size) const {
      short unsigned int currentIndex = m_largestFreeItem;
      while(currentIndex) {
        short unsigned int currentFree = freeSize(currentIndex);
        if(currentFree < size)
          return false;
        //Either we need an exact match, or 2 additional bytes to manage the resulting gap
        if(size == currentFree || currentFree - size >= AdditionalSpacePerItem + 2)
          return true;
        currentIndex = followerIndex(currentIndex);
      }

      return false;
    }

    void tick() const {
      ++m_lastUsed;
    }

    //How many ticks ago the item was last used
    int lastUsed() const {
      return m_lastUsed;
    }

    //Whether this bucket was changed since it was last stored
    bool changed() const {
      return m_changed;
    }

    void prepareChange() {
      m_changed = true;
      m_dirty = true;
      makeDataPrivate();
    }
    
    bool dirty() const {
      return m_dirty;
    }
    
    ///Returns the count of following buckets that were merged onto this buckets data array
    uint monsterBucketExtent() const {
      return m_monsterBucketExtent;
    }

    //Counts together the space that is neither accessible through m_objectMap nor through the free items
    uint lostSpace() {
      if(m_monsterBucketExtent)
        return 0;

      uint need = ItemRepositoryBucketSize - m_available;
      uint found = 0;

      for(uint a = 0; a < m_objectMapSize; ++a) {
        uint currentIndex = m_objectMap[a];
        while(currentIndex) {
          found += reinterpret_cast<const Item*>(m_data+currentIndex)->itemSize() + AdditionalSpacePerItem;

          currentIndex = followerIndex(currentIndex);
        }
      }
      uint currentIndex = m_largestFreeItem;
      while(currentIndex) {
        found += freeSize(currentIndex) + AdditionalSpacePerItem;

        currentIndex = followerIndex(currentIndex);
      }
      return need-found;
    }
    ///Slow
    bool isFreeItem(unsigned short index) const {
      unsigned short currentIndex = m_largestFreeItem;
      unsigned short currentSize = 0xffff;
      while(currentIndex) {
        Q_ASSERT(freeSize(currentIndex) <= currentSize);
        currentSize = freeSize(currentIndex);
        if(index == currentIndex)
          return true;

        currentIndex = followerIndex(currentIndex);
      }
      return false;
    }

  private:

    void makeDataPrivate() {
      if(m_mappedData == m_data) {
        short unsigned int* oldObjectMap = m_objectMap;
        short unsigned int* oldNextBucketHash = m_nextBucketHash;
        
        m_data = new char[ItemRepositoryBucketSize + m_monsterBucketExtent * DataSize];
        m_objectMap = new short unsigned int[m_objectMapSize];
        m_nextBucketHash = new short unsigned int[NextBucketHashSize];
        
        memcpy(m_data, m_mappedData, ItemRepositoryBucketSize + m_monsterBucketExtent * DataSize);
        memcpy(m_objectMap, oldObjectMap, m_objectMapSize * sizeof(short unsigned int));
        memcpy(m_nextBucketHash, oldNextBucketHash, NextBucketHashSize * sizeof(short unsigned int));
      }
    }
    
    ///Merges the given index item, which must have a freeSize() set, to surrounding free items, and inserts the result.
    ///The given index itself should not be in the free items chain yet.
    ///Returns whether the item was inserted somewhere.
    void insertFreeItem(unsigned short index) {

      //If the item-size is fixed, we don't need to do any management. Just keep a list of free items. Items of other size will never be requested.
      if(!fixedItemSize) {
        unsigned short currentIndex = m_largestFreeItem;
        unsigned short previousIndex = 0;

        while(currentIndex) {
          Q_ASSERT(currentIndex != index);
          
#ifndef QT_NO_DEBUG
          unsigned short currentFreeSize = freeSize(currentIndex);
#endif
          
          ///@todo Achieve this without iterating through all items in the bucket(This is very slow)
          //Merge behind index
          if(currentIndex == index + freeSize(index) + AdditionalSpacePerItem) {

            //Remove currentIndex from the free chain, since it's merged backwards into index
            if(previousIndex && followerIndex(currentIndex))
              Q_ASSERT(freeSize(previousIndex) >= freeSize(followerIndex(currentIndex)));

            if(previousIndex)
              setFollowerIndex(previousIndex, followerIndex(currentIndex));
            else
              m_largestFreeItem = followerIndex(currentIndex);

            --m_freeItemCount; //One was removed

            //currentIndex is directly behind index, touching its space. Merge them.
            setFreeSize(index, freeSize(index) + AdditionalSpacePerItem + freeSize(currentIndex));

            //Recurse to do even more merging
            insertFreeItem(index);
            return;
          }

          //Merge before index
          if(index == currentIndex + freeSize(currentIndex) + AdditionalSpacePerItem) {

            if(previousIndex && followerIndex(currentIndex))
              Q_ASSERT(freeSize(previousIndex) >= freeSize(followerIndex(currentIndex)));

            //Remove currentIndex from the free chain, since insertFreeItem wants
            //it not to be in the chain, and it will be inserted in another place
            if(previousIndex)
              setFollowerIndex(previousIndex, followerIndex(currentIndex));
            else
              m_largestFreeItem = followerIndex(currentIndex);

            --m_freeItemCount; //One was removed

            //index is directly behind currentIndex, touching its space. Merge them.
            setFreeSize(currentIndex, freeSize(currentIndex) + AdditionalSpacePerItem + freeSize(index));

            //Recurse to do even more merging
            insertFreeItem(currentIndex);
            return;
          }

          previousIndex = currentIndex;
          currentIndex = followerIndex(currentIndex);
          if(currentIndex)
            Q_ASSERT(freeSize(currentIndex) <= currentFreeSize);
        }
      }
      insertToFreeChain(index);
    }

    ///Only inserts the item in the correct position into the free chain. index must not be in the chain yet.
    void insertToFreeChain(unsigned short index) {
  
      if(!fixedItemSize) {
        ///@todo Use some kind of tree to find the correct position in the chain(This is very slow)
        //Insert the free item into the chain opened by m_largestFreeItem
        unsigned short currentIndex = m_largestFreeItem;
        unsigned short previousIndex = 0;

        unsigned short size = freeSize(index);

        while(currentIndex && freeSize(currentIndex) > size) {
          Q_ASSERT(currentIndex != index); //must not be in the chain yet
          previousIndex = currentIndex;
          currentIndex = followerIndex(currentIndex);
        }

        if(currentIndex)
          Q_ASSERT(freeSize(currentIndex) <= size);
        
        setFollowerIndex(index, currentIndex);

        if(previousIndex) {
          Q_ASSERT(freeSize(previousIndex) >= size);
          setFollowerIndex(previousIndex, index);
        } else
          //This item is larger than all already registered free items, or there are none.
          m_largestFreeItem = index;
      }else{
        Q_ASSERT(freeSize(index) == fixedItemSize);
        //When all items have the same size, just prepent to the front.
        setFollowerIndex(index, m_largestFreeItem);
        m_largestFreeItem = index;
      }

      ++m_freeItemCount;
    }

    ///Returns true if the given index is right behind free space, and thus can be merged to the free space.
    bool isBehindFreeSpace(unsigned short index) const {
      ///@todo Without iteration!
      unsigned short currentIndex = m_largestFreeItem;

      while(currentIndex) {
        if(index == currentIndex + freeSize(currentIndex) + AdditionalSpacePerItem)
          return true;

        currentIndex = followerIndex(currentIndex);
      }
      return false;
    }

    ///@param index the index of an item @return The index of the next item in the chain of items with a same local hash, or zero
    inline unsigned short followerIndex(unsigned short index) const {
      Q_ASSERT(index >= 2);
      return *reinterpret_cast<unsigned short*>(m_data+(index-2));
    }

    void setFollowerIndex(unsigned short index, unsigned short follower) {
      Q_ASSERT(index >= 2);
      *reinterpret_cast<unsigned short*>(m_data+(index-2)) = follower;
    }
    // Only returns the current value if the item is actually free
    inline unsigned short freeSize(unsigned short index) const {
      return *reinterpret_cast<unsigned short*>(m_data+index);
    }

    //Convenience function to set the free-size, only for freed items
    void setFreeSize(unsigned short index, unsigned short size) {
      *reinterpret_cast<unsigned short*>(m_data+index) = size;
    }

    uint m_monsterBucketExtent; //If this is a monster-bucket, this contains the count of follower-buckets that belong to this one
    unsigned int m_available;
    char* m_data; //Structure of the data: <Position of next item with same hash modulo ItemRepositoryBucketSize>(2 byte), <Item>(item.size() byte)
    char* m_mappedData; //Read-only memory-mapped data. If this equals m_data, m_data must not be written
    short unsigned int* m_objectMap; //Points to the first object in m_data with (hash % m_objectMapSize) == index. Points to the item itself, so subtract 1 to get the pointer to the next item with same local hash.
    uint m_objectMapSize;
    short unsigned int m_largestFreeItem; //Points to the largest item that is currently marked as free, or zero. That one points to the next largest one through followerIndex
    unsigned int m_freeItemCount;

    unsigned short* m_nextBucketHash;

    bool m_dirty; //Whether the data was changed since the last finalCleanup
    bool m_changed; //Whether this bucket was changed since it was last stored to disk
    mutable int m_lastUsed; //How many ticks ago this bucket was last accessed
};

template<bool lock>
struct Locker { //This is a dummy that does nothing
  template<class T>
  Locker(const T& /*t*/) {
  }
};
template<>
struct Locker<true> {
  Locker(QMutex* mutex) : m_mutex(mutex) {
    m_mutex->lockInline();
  }
  ~Locker() {
    m_mutex->unlockInline();
  }
  QMutex* m_mutex;
};

///This object needs to be kept alive as long as you change the contents of an item
///stored in the repository. It is needed to correctly track the reference counting
///within disk-storage.
///@warning You can not freely copy this around, when you create a copy, the copy source
///         becomes invalid
template<class Item, bool markForReferenceCounting>
class DynamicItem {
  public:
    DynamicItem(Item* i, void* start, uint size) : m_item(i), m_start(start) {
      if(markForReferenceCounting)
        enableDUChainReferenceCounting(m_start, size);
//       kDebug() << "enabling" << i << "to" << (void*)(((char*)i)+size);
    }
    
    ~DynamicItem() {
      if(m_start) {
//         kDebug() << "destructor-disabling" << m_item;
        if(markForReferenceCounting)
          disableDUChainReferenceCounting(m_start);
      }
    }
    
    DynamicItem(const DynamicItem& rhs) : m_item(rhs.m_item), m_start(rhs.m_start) {
//         kDebug() << "stealing" << m_item;
      Q_ASSERT(rhs.m_start);
      rhs.m_start = 0;
    }
    
    Item* operator->() {
      return m_item;
    }

    Item* m_item;
  private:
    mutable void* m_start;
    DynamicItem& operator=(const DynamicItem&);
};

///@param Item @see ExampleItem
///@param ItemRequest @see ExampleReqestItem
///@param fixedItemSize When this is true, all inserted items must have the same size.
///                     This greatly simplifies and speeds up the task of managing free items within the buckets.
///@param markForReferenceCounting Whether the data within the repository should be marked for reference-counting.
///                                This costs a bit of performance, but must be enabled if there may be data in the repository
///                                that does on-disk reference counting, like IndexedString, IndexedIdentifier, etc.
///@param threadSafe Whether class access should be thread-safe. Disabling this is dangerous when you do multi-threading.
///                  You have to make sure that mutex() is locked whenever the repository is accessed.
template<class Item, class ItemRequest, bool markForReferenceCounting = true, bool threadSafe = true, uint fixedItemSize = 0, unsigned int targetBucketHashSize = 524288*2>
class ItemRepository : public AbstractItemRepository {

  typedef Locker<threadSafe> ThisLocker;

  typedef Bucket<Item, ItemRequest, markForReferenceCounting, fixedItemSize> MyBucket;

  enum {
    //Must be a multiple of Bucket::ObjectMapSize, so Bucket::hasClashingItem can be computed
    //Must also be a multiple of Bucket::NextBucketHashSize, for the same reason.(Currently those are same)
    bucketHashSize = (targetBucketHashSize / MyBucket::ObjectMapSize) * MyBucket::ObjectMapSize
  };

  enum {
    BucketStartOffset = sizeof(uint) * 7 + sizeof(short unsigned int) * bucketHashSize //Position in the data where the bucket array starts
  };

  public:
  ///@param registry May be zero, then the repository will not be registered at all. Else, the repository will register itself to that registry.
  ///                If this is zero, you have to care about storing the data using store() and/or close() by yourself. It does not happen automatically.
  ///                For the global standard registry, the storing/loading is triggered from within duchain, so you don't need to care about it.
  ItemRepository(QString repositoryName, ItemRepositoryRegistry* registry  = &globalItemRepositoryRegistry(), uint repositoryVersion = 1, AbstractRepositoryManager* manager = 0) : m_ownMutex(QMutex::Recursive), m_mutex(&m_ownMutex), m_repositoryName(repositoryName), m_registry(registry), m_file(0), m_dynamicFile(0), m_repositoryVersion(repositoryVersion), m_manager(manager) {
    m_unloadingEnabled = true;
    m_metaDataChanged = true;
    m_buckets.resize(10);
    m_buckets.fill(0);
    m_freeSpaceBucketsSize = 0;
    m_fastBuckets = m_buckets.data();
    m_bucketCount = m_buckets.size();
    m_bucketHashSize = bucketHashSize;

    m_firstBucketForHash = new short unsigned int[bucketHashSize];
    memset(m_firstBucketForHash, 0, bucketHashSize * sizeof(short unsigned int));

    m_statBucketHashClashes = m_statItemCount = 0;
    m_currentBucket = 1; //Skip the first bucket, we won't use it so we have the zero indices for special purposes
    if(m_registry)
      m_registry->registerRepository(this, m_manager);
  }

  ~ItemRepository() {
    if(m_registry)
      m_registry->unRegisterRepository(this);
    close();
  }

  ///Unloading of buckets is enabled by default. Use this to disable it. When unloading is enabled, the data
  ///gotten from must only itemFromIndex must not be used for a long time.
  void setUnloadingEnabled(bool enabled) {
      m_unloadingEnabled = enabled;
  }

  ///Returns the index for the given item. If the item is not in the repository yet, it is inserted.
  ///The index can never be zero. Zero is reserved for your own usage as invalid
  ///@param dynamic will be applied to the dynamic data of the found item
  unsigned int index(const ItemRequest& request) {

    ThisLocker lock(m_mutex);

    unsigned int hash = request.hash();
    unsigned int size = request.itemSize();

    short unsigned int* bucketHashPosition = m_firstBucketForHash + (hash % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;

    uint useBucket = m_currentBucket;
    bool pickedBucketInChain = false; //Whether a bucket was picked for re-use that already is in the hash chain
    int reOrderFreeSpaceBucketIndex = -1;

    while(previousBucketNumber) {
      //We have a bucket that contains an item with the given hash % bucketHashSize, so check if the item is already there
      MyBucket* bucketPtr = m_fastBuckets[previousBucketNumber];
      if(!bucketPtr) {
        initializeBucket(previousBucketNumber);
        bucketPtr = m_fastBuckets[previousBucketNumber];
      }

      unsigned short indexInBucket = bucketPtr->findIndex(request);
      if(indexInBucket) {
        //We've found the item, it's already there
        uint index = (previousBucketNumber << 16) + indexInBucket;
        verifyIndex(index);
        return index; //Combine the index in the bucket, and the bucker number into one index
      } else {
#ifdef DEBUG_HASH_SEQUENCES
        if(previousBucketNumber==*bucketHashPosition)
          Q_ASSERT(bucketPtr->hasClashingItem(hash, bucketHashSize));
        else
          Q_ASSERT(bucketPtr->hasClashingItem(hash, MyBucket::NextBucketHashSize));
#endif
        
        if(!pickedBucketInChain && bucketPtr->canAllocateItem(size)) {
          //Remember that this bucket has enough space to store the item, if it isn't already stored.
          //This gives a better structure, and saves us from cyclic follower structures.
          useBucket = previousBucketNumber;
          reOrderFreeSpaceBucketIndex = m_freeSpaceBuckets.indexOf(useBucket);
          pickedBucketInChain = true;
        }
        //The item isn't in bucket previousBucketNumber, but maybe the bucket has a pointer to the next bucket that might contain the item
        //Should happen rarely
        short unsigned int next = bucketPtr->nextBucketForHash(hash);
        if(next) {
          previousBucketNumber = next;
        } else
          break;
      }
    }

    m_metaDataChanged = true;

    if(!pickedBucketInChain && useBucket == m_currentBucket) {
      //Try finding an existing bucket with deleted space to store the data into
      for(uint a = 0; a < m_freeSpaceBucketsSize; ++a) {
        MyBucket* bucketPtr = bucketForIndex(m_freeSpaceBuckets[a]);
        Q_ASSERT(bucketPtr->largestFreeSize());

        if(bucketPtr->canAllocateItem(size)) {
          //The item fits into the bucket.
          useBucket = m_freeSpaceBuckets[a];
          reOrderFreeSpaceBucketIndex = a;
          break;
        }
      }
    }

    //The item isn't in the repository yet, find a new bucket for it
    while(1) {
      if(useBucket >= m_bucketCount) {
          if(m_bucketCount >= 0xfffe) { //We have reserved the last bucket index 0xffff for special purposes
          //the repository has overflown.
          kWarning() << "Found no room for an item in" << m_repositoryName << "size of the item:" << request.itemSize();
          return 0;
        }else{
          //Allocate new buckets
          uint oldBucketCount = m_bucketCount;
          m_bucketCount += 10;
          m_buckets.resize(m_bucketCount);
          m_fastBuckets = m_buckets.data();
          memset(m_fastBuckets + oldBucketCount, 0, (m_bucketCount-oldBucketCount) * sizeof(void*));
        }
      }
      MyBucket* bucketPtr = m_fastBuckets[useBucket];
      if(!bucketPtr) {
        initializeBucket(useBucket);
        bucketPtr = m_fastBuckets[useBucket];
      }

      ENSURE_REACHABLE(useBucket);
      Q_ASSERT_X(!bucketPtr->findIndex(request), Q_FUNC_INFO, "found item in unexpected bucket, ensure your ItemRequest::equals method is correct. Note: For custom AbstractType's e.g. ensure you have a proper equals() override");

      unsigned short indexInBucket = bucketPtr->index(request, size);

      //If we could not allocate the item in an empty bucket, then we need to create a monster-bucket that
      //can hold the data.
      if(bucketPtr->isEmpty() && !indexInBucket) {
        ///@todo Move this compound statement into an own function
        uint totalSize = size + MyBucket::AdditionalSpacePerItem;

        Q_ASSERT((totalSize > ItemRepositoryBucketSize));

        useBucket = 0;
        //The item did not fit in, we need a monster-bucket(Merge consecutive buckets)
        ///Step one: Search whether we can merge multiple empty buckets in the free-list into one monster-bucket
        int rangeStart = -1;
        int rangeEnd = -1;
        for(uint a = 0; a < m_freeSpaceBucketsSize; ++a) {
          MyBucket* bucketPtr = bucketForIndex(m_freeSpaceBuckets[a]);
          if(bucketPtr->isEmpty()) {
            //This bucket is a candidate for monster-bucket merging
            int index = (int)m_freeSpaceBuckets[a];
            if(rangeEnd != index) {
              rangeStart = index;
              rangeEnd = index+1;
            }else{
              ++rangeEnd;
            }
            if(rangeStart != rangeEnd) {
              uint extent = rangeEnd - rangeStart - 1;
              uint totalAvailableSpace = bucketForIndex(rangeStart)->available() + MyBucket::DataSize * (rangeEnd - rangeStart - 1);
              if(totalAvailableSpace > totalSize) {
                Q_ASSERT(extent);
                ///We can merge these buckets into one monster-bucket that can hold the data
                Q_ASSERT((uint)m_freeSpaceBuckets[a-extent] == (uint)rangeStart);
                m_freeSpaceBuckets.remove(a-extent, extent+1);
                m_freeSpaceBucketsSize = m_freeSpaceBuckets.size();
                useBucket = rangeStart;
                convertMonsterBucket(rangeStart, extent);

                break;
              }
            }
          }
        }
        if(!useBucket) {
          //Create a new monster-bucket at the end of the data
          uint needMonsterExtent = (totalSize - ItemRepositoryBucketSize) / MyBucket::DataSize + 1;
          Q_ASSERT(needMonsterExtent);
          if(m_currentBucket + needMonsterExtent + 1 > (uint)m_buckets.size()) {
            uint oldBucketCount = m_bucketCount;
            m_bucketCount += 10 + needMonsterExtent + 1;
            m_buckets.resize(m_bucketCount);
            m_fastBuckets = m_buckets.data();

            memset(m_fastBuckets + oldBucketCount, 0, (m_bucketCount-oldBucketCount) * sizeof(void*));
          }
          useBucket = m_currentBucket;

          convertMonsterBucket(useBucket, needMonsterExtent);
          m_currentBucket += 1 + needMonsterExtent;
          Q_ASSERT(m_fastBuckets[m_currentBucket - 1 - needMonsterExtent] && m_fastBuckets[m_currentBucket - 1 - needMonsterExtent]->monsterBucketExtent() == needMonsterExtent);
        }
        Q_ASSERT(useBucket);
        bucketPtr = bucketForIndex(useBucket);

        indexInBucket = bucketPtr->index(request, size);
        Q_ASSERT(indexInBucket);
      }

      if(indexInBucket) {
        ++m_statItemCount;

        if(!(*bucketHashPosition)) {
          Q_ASSERT(!previousBucketNumber);
          (*bucketHashPosition) = useBucket;
          ENSURE_REACHABLE(useBucket);
        } else if(!pickedBucketInChain && previousBucketNumber && previousBucketNumber != useBucket) {
          //Should happen rarely
          ++m_statBucketHashClashes;

          ///Debug: Detect infinite recursion
          ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, previousBucketNumber));)

          //Find the position where the paths of useBucket and *bucketHashPosition intersect, and insert useBucket
          //there. That way, we don't create loops.
          QPair<unsigned int, unsigned int> intersect = hashChainIntersection(*bucketHashPosition, useBucket, hash);

          Q_ASSERT(m_buckets[previousBucketNumber]->nextBucketForHash(hash) == 0);

          if(!intersect.second) {
            ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(*bucketHashPosition, hash, useBucket));)
            ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(useBucket, hash, previousBucketNumber));)

            Q_ASSERT(m_buckets[previousBucketNumber]->nextBucketForHash(hash) == 0);

            m_buckets[previousBucketNumber]->setNextBucketForHash(hash, useBucket);
            ENSURE_REACHABLE(useBucket);
            ENSURE_REACHABLE(previousBucketNumber);

            ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, useBucket));)
          } else if(intersect.first) {
            ifDebugInfiniteRecursion(Q_ASSERT(bucketForIndex(intersect.first)->nextBucketForHash(hash) == intersect.second);)
            ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(*bucketHashPosition, hash, useBucket));)
            ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, intersect.second));)
            ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, intersect.first));)
            ifDebugInfiniteRecursion(Q_ASSERT(bucketForIndex(intersect.first)->nextBucketForHash(hash) == intersect.second);)

            ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(useBucket, hash, intersect.second));)
            ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(useBucket, hash, intersect.first));)

            bucketForIndex(intersect.first)->setNextBucketForHash(hash, useBucket);

            ENSURE_REACHABLE(useBucket);
            ENSURE_REACHABLE(intersect.second);

            ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, useBucket));)
            ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, intersect.second));)
          } else {
            //State: intersect.first == 0 && intersect.second != 0. This means that whole compleet
            //chain opened by *bucketHashPosition with the hash-value is also following useBucket,
            //so useBucket can just be inserted at the top

            ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(*bucketHashPosition, hash, useBucket));)
            ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(useBucket, hash, *bucketHashPosition));)
            unsigned short oldStart = *bucketHashPosition;

            *bucketHashPosition = useBucket;

            ENSURE_REACHABLE(useBucket);
            ENSURE_REACHABLE(oldStart);
            Q_UNUSED(oldStart);
          }
        }

        if(reOrderFreeSpaceBucketIndex != -1)
          updateFreeSpaceOrder(reOrderFreeSpaceBucketIndex);

        //Combine the index in the bucket, and the bucker number into one index
        uint index = (useBucket << 16) + indexInBucket;
        verifyIndex(index);
        return index;
      }else{
        //This should never happen when we picked a bucket for re-use
        Q_ASSERT(!pickedBucketInChain);
        Q_ASSERT(reOrderFreeSpaceBucketIndex == -1);
        Q_ASSERT(useBucket == m_currentBucket);

        if(!bucketForIndex(useBucket)->isEmpty())
          putIntoFreeList(useBucket, bucketPtr);

        ++m_currentBucket;
        useBucket = m_currentBucket;
      }
    }
    //We haven't found a bucket that already contains the item, so append it to the current bucket

    kWarning() << "Found no bucket for an item in" << m_repositoryName;
    return 0;
  }

  ///Returns zero if the item is not in the repository yet
  unsigned int findIndex(const ItemRequest& request) {

    ThisLocker lock(m_mutex);

    unsigned int hash = request.hash();

    short unsigned int* bucketHashPosition = m_firstBucketForHash + (hash % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;

    while(previousBucketNumber) {
      //We have a bucket that contains an item with the given hash % bucketHashSize, so check if the item is already there

      MyBucket* bucketPtr = m_fastBuckets[previousBucketNumber];
      if(!bucketPtr) {
        initializeBucket(previousBucketNumber);
        bucketPtr = m_fastBuckets[previousBucketNumber];
      }

      unsigned short indexInBucket = bucketPtr->findIndex(request);
      if(indexInBucket) {
        //We've found the item, it's already there
        uint index = (previousBucketNumber << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
        verifyIndex(index);
        return index;
      } else {
        //The item isn't in bucket previousBucketNumber, but maybe the bucket has a pointer to the next bucket that might contain the item
        //Should happen rarely
        short unsigned int next = bucketPtr->nextBucketForHash(hash);
        if(next)
          previousBucketNumber = next;
        else
          break;
      }
    }

    return 0;
  }

  ///Deletes the item from the repository.
  void deleteItem(unsigned int index) {
    verifyIndex(index);
    ThisLocker lock(m_mutex);

    m_metaDataChanged = true;

    unsigned short bucket = (index >> 16);

    unsigned int hash = itemFromIndex(index)->hash();

    short unsigned int* bucketHashPosition = m_firstBucketForHash + (hash % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;

    Q_ASSERT(previousBucketNumber);

    if(previousBucketNumber == bucket)
      previousBucketNumber = 0;

    MyBucket* previousBucketPtr = 0;

    //Apart from removing the item itself, we may have to recreate the nextBucketForHash link, so we need the previous bucket

    while(previousBucketNumber) {
      //We have a bucket that contains an item with the given hash % bucketHashSize, so check if the item is already there

      previousBucketPtr = m_fastBuckets[previousBucketNumber];
      if(!previousBucketPtr) {
        initializeBucket(previousBucketNumber);
        previousBucketPtr = m_fastBuckets[previousBucketNumber];
      }

      short unsigned int nextBucket = previousBucketPtr->nextBucketForHash(hash);
      Q_ASSERT(nextBucket);
      if(nextBucket == bucket)
        break; //Now previousBucketNumber
      else
        previousBucketNumber = nextBucket;
    }

    //Make sure the index was reachable through the hashes
    Q_ASSERT(previousBucketNumber || *bucketHashPosition == bucket);

    MyBucket* bucketPtr = m_fastBuckets[bucket];
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }

    --m_statItemCount;

    bucketPtr->deleteItem(index, hash, *this);

    /**
     * Now check whether the link previousBucketNumber -> bucket is still needed.
     */
    ///@todo Clear the nextBucketForHash links when not needed any more, by doing setNextBucketForHash(hash, 0);
    //return; ///@todo Find out what this problem is about. If we don't return here, some items become undiscoverable through hashes after some time
    if(previousBucketNumber == 0) {
      //The item is directly in the m_firstBucketForHash hash
      //Put the next item in the nextBucketForHash chain into m_firstBucketForHash that has a hash clashing in that array.
      Q_ASSERT(*bucketHashPosition == bucket);
      IF_ENSURE_REACHABLE(unsigned short previous = bucket;)
      auto nextBucket = bucketPtr;
      while(!nextBucket->hasClashingItem(hash, bucketHashSize))
      {
//         Q_ASSERT(!bucketPtr->hasClashingItemReal(hash, bucketHashSize));
        unsigned short next = nextBucket->nextBucketForHash(hash);
        ENSURE_REACHABLE(next);
        ENSURE_REACHABLE(previous);

        *bucketHashPosition = next;

        ENSURE_REACHABLE(previous);
        ENSURE_REACHABLE(next);

        IF_ENSURE_REACHABLE(previous = next;)

        if(next) {
          nextBucket = m_fastBuckets[next];

          if(!nextBucket)
          {
            initializeBucket(next);
            nextBucket = m_fastBuckets[next];
          }
        }else{
          break;
        }
      }
    }else{
      if(!bucketPtr->hasClashingItem(hash, MyBucket::NextBucketHashSize)) {
//         Q_ASSERT(!bucketPtr->hasClashingItemReal(hash, MyBucket::NextBucketHashSize));
        ///Debug: Check for infinite recursion
        walkBucketLinks(*bucketHashPosition, hash);

        Q_ASSERT(previousBucketPtr->nextBucketForHash(hash) == bucket);

        ENSURE_REACHABLE(bucket);

        previousBucketPtr->setNextBucketForHash(hash, bucketPtr->nextBucketForHash(hash));

        ENSURE_REACHABLE(bucket);

        ///Debug: Check for infinite recursion
        Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, bucketPtr->nextBucketForHash(hash)));

        Q_ASSERT(bucketPtr->nextBucketForHash(hash) != previousBucketNumber);
      }
    }

    ENSURE_REACHABLE(bucket);

    if(bucketPtr->monsterBucketExtent()) {
      //Convert the monster-bucket back to multiple normal buckets, and put them into the free list
      uint newBuckets = bucketPtr->monsterBucketExtent()+1;
      Q_ASSERT(bucketPtr->isEmpty());
      if (!previousBucketNumber) {
        // see https://bugs.kde.org/show_bug.cgi?id=272408
        // the monster bucket will be deleted and new smaller ones created
        // the next bucket for this hash is invalid anyways as done above
        // but calling the below unconditionally leads to other issues...
        bucketPtr->setNextBucketForHash(hash, 0);
      }
      convertMonsterBucket(bucket, 0);
      for(uint created = bucket; created < bucket + newBuckets; ++created) {
        putIntoFreeList(created, bucketForIndex(created));
#ifdef DEBUG_MONSTERBUCKETS
        Q_ASSERT(m_freeSpaceBuckets.indexOf(created) != -1);
#endif
      }
    }else{
      putIntoFreeList(bucket, bucketPtr);
    }
    
#ifdef DEBUG_HASH_SEQUENCES
    Q_ASSERT(*bucketHashPosition == 0 || bucketForIndex(*bucketHashPosition)->hasClashingItem(hash, bucketHashSize));
#endif
  }

  ///This returns an editable version of the item. @warning: Never change an entry that affects the hash,
  ///or the equals(..) function. That would completely destroy the consistency.
  ///@param index The index. It must be valid(match an existing item), and nonzero.
  ///@warning If you use this, make sure you lock mutex() before calling,
  ///         and hold it until you're ready using/changing the data..
  
  typedef DynamicItem<Item, markForReferenceCounting> MyDynamicItem;
  
  MyDynamicItem dynamicItemFromIndex(unsigned int index) {
    verifyIndex(index);

    ThisLocker lock(m_mutex);

    unsigned short bucket = (index >> 16);

    MyBucket* bucketPtr = m_fastBuckets[bucket];
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    bucketPtr->prepareChange();
    unsigned short indexInBucket = index & 0xffff;
    return MyDynamicItem(const_cast<Item*>(bucketPtr->itemFromIndex(indexInBucket)), bucketPtr->data(), bucketPtr->dataSize());
  }
  
  ///This returns an editable version of the item. @warning: Never change an entry that affects the hash,
  ///or the equals(..) function. That would completely destroy the consistency.
  ///@param index The index. It must be valid(match an existing item), and nonzero.
  ///@warning If you use this, make sure you lock mutex() before calling,
  ///         and hold it until you're ready using/changing the data..
  ///@warning If you change contained complex items that depend on reference-counting, you
  ///         must use dynamicItemFromIndex(..) instead of dynamicItemFromIndexSimple(..)
  Item* dynamicItemFromIndexSimple(unsigned int index) {
    verifyIndex(index);

    ThisLocker lock(m_mutex);

    unsigned short bucket = (index >> 16);

    MyBucket* bucketPtr = m_fastBuckets[bucket];
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    bucketPtr->prepareChange();
    unsigned short indexInBucket = index & 0xffff;
    return const_cast<Item*>(bucketPtr->itemFromIndex(indexInBucket));
  }
  ///@param Action Must be an object that has an "operator()(Item&)" function.
  ///That function is allowed to do any action on the item, except for anything that
  ///changes its identity/hash-value
  template<class Action>
  void dynamicAction(unsigned int index, Action& action) {
    verifyIndex(index);

    ThisLocker lock(m_mutex);

    unsigned short bucket = (index >> 16);

    MyBucket* bucketPtr = m_fastBuckets[bucket];
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    bucketPtr->prepareChange();
    unsigned short indexInBucket = index & 0xffff;
    
    if(markForReferenceCounting)
      enableDUChainReferenceCounting(bucketPtr->data(), bucketPtr->dataSize());
    
    action(const_cast<Item&>(*bucketPtr->itemFromIndex(indexInBucket, 0)));

    if(markForReferenceCounting)
      disableDUChainReferenceCounting(bucketPtr->data());
  }

  ///@param index The index. It must be valid(match an existing item), and nonzero.
  ///@param dynamic will be applied to the item.
  const Item* itemFromIndex(unsigned int index) const {
    verifyIndex(index);

    ThisLocker lock(m_mutex);

    unsigned short bucket = (index >> 16);

    const MyBucket* bucketPtr = m_fastBuckets[bucket];
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    unsigned short indexInBucket = index & 0xffff;
    return bucketPtr->itemFromIndex(indexInBucket);
  }

  struct Statistics {
    Statistics() : loadedBuckets(-1), currentBucket(-1), usedMemory(-1), loadedMonsterBuckets(-1), usedSpaceForBuckets(-1),
                   freeSpaceInBuckets(-1), lostSpace(-1), freeUnreachableSpace(-1), hashClashedItems(-1), totalItems(-1), hashSize(-1), hashUse(-1), 
                   averageInBucketHashSize(-1), averageInBucketUsedSlotCount(-1), averageInBucketSlotChainLength(-1), longestInBucketChain(-1), longestNextBucketChain(-1), totalBucketFollowerSlots(-1), averageNextBucketForHashSequenceLength(-1) {
    }

    uint loadedBuckets;
    uint currentBucket;
    uint usedMemory;
    uint loadedMonsterBuckets;
    uint usedSpaceForBuckets;
    uint freeSpaceInBuckets;
    uint lostSpace;
    uint freeUnreachableSpace;
    uint hashClashedItems;
    uint totalItems;
    uint emptyBuckets;
    uint hashSize; //How big the hash is
    uint hashUse; //How many slots in the hash are used
    uint averageInBucketHashSize;
    uint averageInBucketUsedSlotCount;
    float averageInBucketSlotChainLength;
    uint longestInBucketChain;
    
    uint longestNextBucketChain;
    uint totalBucketFollowerSlots; //Total count of used slots in the nextBucketForHash structure
    float averageNextBucketForHashSequenceLength; //Average sequence length of a nextBucketForHash sequence(If not empty)

    QString print() const {
      QString ret;
      ret += QString("loaded buckets: %1 current bucket: %2 used memory: %3 loaded monster buckets: %4").arg(loadedBuckets).arg(currentBucket).arg(usedMemory).arg(loadedMonsterBuckets);
      ret += QString("\nbucket hash clashed items: %1 total items: %2").arg(hashClashedItems).arg(totalItems);
      ret += QString("\nused space for buckets: %1 free space in buckets: %2 lost space: %3").arg(usedSpaceForBuckets).arg(freeSpaceInBuckets).arg(lostSpace);
      ret += QString("\nfree unreachable space: %1 empty buckets: %2").arg(freeUnreachableSpace).arg(emptyBuckets);
      ret += QString("\nhash size: %1 hash slots used: %2").arg(hashSize).arg(hashUse);
      ret += QString("\naverage in-bucket hash size: %1 average in-bucket used hash slot count: %2 average in-bucket slot chain length: %3 longest in-bucket follower chain: %4").arg(averageInBucketHashSize).arg(averageInBucketUsedSlotCount).arg(averageInBucketSlotChainLength).arg(longestInBucketChain);
      ret += QString("\ntotal count of used next-bucket-for-hash slots: %1 average next-bucket-for-hash sequence length: %2 longest next-bucket chain: %3").arg(totalBucketFollowerSlots).arg(averageNextBucketForHashSequenceLength).arg(longestNextBucketChain);
      return ret;
    }
    operator QString() const {
      return print();
    }
  };
  
  QString printStatistics() const {
    return statistics().print();
  }

  Statistics statistics() const {
    Statistics ret;
    uint loadedBuckets = 0;
    for(uint a = 0; a < m_bucketCount; ++a)
      if(m_fastBuckets[a])
        ++loadedBuckets;

#ifdef DEBUG_MONSTERBUCKETS
    for(uint a = 0; a < m_freeSpaceBucketsSize; ++a) {
      if(a > 0) {
        uint prev = a-1;
        uint prevLargestFree = bucketForIndex(m_freeSpaceBuckets[prev])->largestFreeSize();
        uint largestFree = bucketForIndex(m_freeSpaceBuckets[a])->largestFreeSize();
        Q_ASSERT( (prevLargestFree < largestFree) || (prevLargestFree == largestFree &&
                         m_freeSpaceBuckets[prev] < m_freeSpaceBuckets[a]) );
      }
    }
#endif
    ret.hashSize = bucketHashSize;
    ret.hashUse = 0;
    for(uint a = 0; a < m_bucketHashSize; ++a)
      if(m_firstBucketForHash[a])
        ++ret.hashUse;

    ret.emptyBuckets = 0;

    uint loadedMonsterBuckets = 0;
    for(uint a = 0; a < m_bucketCount; ++a)
      if(m_fastBuckets[a] && m_fastBuckets[a]->monsterBucketExtent())
        loadedMonsterBuckets += m_fastBuckets[a]->monsterBucketExtent()+1;

    uint usedBucketSpace = MyBucket::DataSize * m_currentBucket;
    uint freeBucketSpace = 0, freeUnreachableSpace = 0;
    uint lostSpace = 0; //Should be zero, else something is wrong
    uint totalInBucketHashSize = 0, totalInBucketUsedSlotCount = 0, totalInBucketChainLengths = 0;
    uint bucketCount = 0;

    ret.totalBucketFollowerSlots = 0;
    ret.averageNextBucketForHashSequenceLength = 0;
    ret.longestNextBucketChain = 0;
    ret.longestInBucketChain = 0;
    
    for(uint a = 1; a < m_currentBucket+1; ++a) {
      MyBucket* bucket = bucketForIndex(a);
      if(bucket) {
        ++bucketCount;
        
        bucket->countFollowerIndexLengths(totalInBucketUsedSlotCount, totalInBucketChainLengths, totalInBucketHashSize, ret.longestInBucketChain);
        
        for(uint aa = 0; aa < MyBucket::NextBucketHashSize; ++aa) {
          uint length = 0;
          uint next = bucket->nextBucketForHash(aa);
          if(next) {
            ++ret.totalBucketFollowerSlots;
            while(next) {
              ++length;
              ++ret.averageNextBucketForHashSequenceLength;
              next = bucketForIndex(next)->nextBucketForHash(aa);
            }
          }
          if(length > ret.longestNextBucketChain) {
//             kDebug() << "next-bucket-chain in" << a << aa << ":" << length;
            ret.longestNextBucketChain = length;
          }
        }
        
        uint bucketFreeSpace = bucket->totalFreeItemsSize() + bucket->available();
        freeBucketSpace += bucketFreeSpace;
        if(m_freeSpaceBuckets.indexOf(a) == -1)
          freeUnreachableSpace += bucketFreeSpace;

        if(bucket->isEmpty()) {
          ++ret.emptyBuckets;
          Q_ASSERT(!bucket->monsterBucketExtent());
#ifdef DEBUG_MONSTERBUCKETS
          Q_ASSERT(m_freeSpaceBuckets.contains(a));
#endif
        }

        lostSpace += bucket->lostSpace();
        a += bucket->monsterBucketExtent();
      }
    }
    
    if(ret.totalBucketFollowerSlots)
      ret.averageNextBucketForHashSequenceLength /= ret.totalBucketFollowerSlots;

    ret.loadedBuckets = loadedBuckets;
    ret.currentBucket = m_currentBucket;
    ret.usedMemory = usedMemory();
    ret.loadedMonsterBuckets = loadedMonsterBuckets;

    ret.hashClashedItems = m_statBucketHashClashes;
    ret.totalItems = m_statItemCount;
    ret.usedSpaceForBuckets = usedBucketSpace;
    ret.freeSpaceInBuckets = freeBucketSpace;
    ret.lostSpace = lostSpace;

    ret.freeUnreachableSpace = freeUnreachableSpace;
    ret.averageInBucketHashSize = totalInBucketHashSize / bucketCount;
    ret.averageInBucketUsedSlotCount = totalInBucketUsedSlotCount / bucketCount;
    ret.averageInBucketSlotChainLength = float(totalInBucketChainLengths) / totalInBucketUsedSlotCount;

    //If m_statBucketHashClashes is high, the bucket-hash needs to be bigger
    return ret;
  }

  uint usedMemory() const {
    uint used = 0;
    for(int a = 0; a < m_buckets.size(); ++a) {
      if(m_buckets[a]) {
        used += m_buckets[a]->usedMemory();
      }
    }
    return used;
  }

  ///This can be used to safely iterate through all items in the repository
  ///@param visitor Should be an instance of a class that has a bool operator()(const Item*) member function,
  ///               that returns whether more items are wanted.
  ///@param onlyInMemory If this is true, only items are visited that are currently in memory.
  template<class Visitor>
  void visitAllItems(Visitor& visitor, bool onlyInMemory = false) const {
    ThisLocker lock(m_mutex);
    for(uint a = 1; a <= m_currentBucket; ++a) {
      if(!onlyInMemory || m_buckets[a]) {
        if(bucketForIndex(a) && !bucketForIndex(a)->visitAllItems(visitor))
          return;
      }
    }
  }

  ///Visits all items that have the given hash-value, plus an arbitrary count of other items with a clashing hash.
  ///@param visitor Should be an instance of a class that has a bool operator()(const Item*, uint index) member function,
  ///               that returns whether more items are wanted.
  template<class Visitor>
  void visitItemsWithHash(Visitor& visitor, uint hash) const {
    ThisLocker lock(m_mutex);

    short unsigned int bucket = *(m_firstBucketForHash + (hash % bucketHashSize));

    while(bucket) {

      MyBucket* bucketPtr = m_fastBuckets[bucket];
      if(!bucketPtr) {
        initializeBucket(bucket);
        bucketPtr = m_fastBuckets[bucket];
      }

      if(!bucketPtr->visitItemsWithHash(visitor, hash, bucket))
        return;

      bucket = bucketPtr->nextBucketForHash(hash);
    }
  }

  ///Synchronizes the state on disk to the one in memory, and does some memory-management.
  ///Should be called on a regular basis. Can be called centrally from the global item repository registry.
  virtual void store() {
    QMutexLocker lock(m_mutex);
    if(m_file) {

      if(!m_file->open( QFile::ReadWrite ) || !m_dynamicFile->open( QFile::ReadWrite )) {
        kFatal() << "cannot re-open repository file for storing";
        return;
      }

      for(uint a = 0; a < m_bucketCount; ++a) {
        if(m_fastBuckets[a]) {
          if(m_fastBuckets[a]->changed()) {
            storeBucket(a);
          }
          if(m_unloadingEnabled) {
            const int unloadAfterTicks = 2;
            if(m_fastBuckets[a]->lastUsed() > unloadAfterTicks) {
                delete m_fastBuckets[a];
                m_fastBuckets[a] = 0;
            }else{
                m_fastBuckets[a]->tick();
            }
          }
        }
      }

      if(m_metaDataChanged) {
        Q_ASSERT(m_dynamicFile);

        m_file->seek(0);
        m_file->write((char*)&m_repositoryVersion, sizeof(uint));
        uint hashSize = bucketHashSize;
        m_file->write((char*)&hashSize, sizeof(uint));
        uint itemRepositoryVersion  = staticItemRepositoryVersion();
        m_file->write((char*)&itemRepositoryVersion, sizeof(uint));
        m_file->write((char*)&m_statBucketHashClashes, sizeof(uint));
        m_file->write((char*)&m_statItemCount, sizeof(uint));

        uint bucketCount = m_buckets.size();
        m_file->write((char*)&bucketCount, sizeof(uint));
        m_file->write((char*)&m_currentBucket, sizeof(uint));
        m_file->write((char*)m_firstBucketForHash, sizeof(short unsigned int) * bucketHashSize);
        Q_ASSERT(m_file->pos() == BucketStartOffset);

        Q_ASSERT(m_freeSpaceBucketsSize == (uint)m_freeSpaceBuckets.size());
        m_dynamicFile->seek(0);
        m_dynamicFile->write((char*)&m_freeSpaceBucketsSize, sizeof(uint));
        m_dynamicFile->write((char*)m_freeSpaceBuckets.data(), sizeof(uint) * m_freeSpaceBucketsSize);

//   #ifdef DEBUG_ITEMREPOSITORY_LOADING
//         {
//           m_file->flush();
//           m_file->seek(0);
//           uint storedVersion, hashSize, itemRepositoryVersion, statBucketHashClashes, statItemCount;
//
//           m_file->read((char*)&storedVersion, sizeof(uint));
//           m_file->read((char*)&hashSize, sizeof(uint));
//           m_file->read((char*)&itemRepositoryVersion, sizeof(uint));
//           m_file->read((char*)&statBucketHashClashes, sizeof(uint));
//           m_file->read((char*)&statItemCount, sizeof(uint));
//           Q_ASSERT(storedVersion == m_repositoryVersion);
//           Q_ASSERT(hashSize == bucketHashSize);
//           Q_ASSERT(itemRepositoryVersion == ItemRepositoryVersion);
//           Q_ASSERT(statBucketHashClashes == m_statBucketHashClashes);
//           Q_ASSERT(statItemCount == m_statItemCount);
//
//           uint bucketCount, currentBucket;
//           m_file->read((char*)&bucketCount, sizeof(uint));
//           Q_ASSERT(bucketCount == (uint)m_buckets.size());
//           m_file->read((char*)&currentBucket, sizeof(uint));
//           Q_ASSERT(currentBucket == m_currentBucket);
//
//           short unsigned int* s = new short unsigned int[bucketHashSize];
//           m_file->read((char*)s, sizeof(short unsigned int) * bucketHashSize);
//           Q_ASSERT(memcmp(s, m_firstBucketForHash, sizeof(short unsigned int) * bucketHashSize) == 0);
//           Q_ASSERT(m_file->pos() == BucketStartOffset);
//           delete[] s;
//         }
//   #endif
      }
      //To protect us from inconsistency due to crashes. flush() is not enough. We need to close.
      m_file->close();
      m_dynamicFile->close();
      Q_ASSERT(!m_file->isOpen());
      Q_ASSERT(!m_dynamicFile->isOpen());
    }
  }

  ///This mutex is used for the thread-safe locking when threadSafe is true. Even if threadSafe is false, it is
  ///always locked before storing to or loading from disk.
  ///@warning If threadSafe is false, and you sometimes call store() from within another thread(As happens in duchain),
  ///         you must always make sure that this mutex is locked before you access this repository.
  ///         Else you will get crashes and inconsistencies.
  ///         In KDevelop This means: Make sure you _always_ lock this mutex before accessing the repository.
  QMutex* mutex() const {
    return m_mutex;
  }

  ///With this, you can replace the internal mutex with another one.
  void setMutex(QMutex* mutex) {
    m_mutex = mutex;
  }
  
  virtual QString repositoryName() const {
    return m_repositoryName;
  }

  private:

  ///Makes sure the order within m_freeSpaceBuckets is correct, after largestFreeSize has been changed for m_freeSpaceBuckets[index].
  ///If too few space is free within the given bucket, it is removed from m_freeSpaceBuckets.
  void updateFreeSpaceOrder(uint index) {
    m_metaDataChanged = true;

    unsigned int* freeSpaceBuckets = m_freeSpaceBuckets.data();

    Q_ASSERT(index < (uint)m_freeSpaceBucketsSize);
    MyBucket* bucketPtr = bucketForIndex(freeSpaceBuckets[index]);

    unsigned short largestFreeSize = bucketPtr->largestFreeSize();

    if(largestFreeSize == 0 || (bucketPtr->freeItemCount() <= MyBucket::MaxFreeItemsForHide && largestFreeSize <= MyBucket::MaxFreeSizeForHide)) {
      //Remove the item from freeSpaceBuckets
      m_freeSpaceBuckets.remove(index);
      m_freeSpaceBucketsSize = m_freeSpaceBuckets.size();
    }else{

      while(1) {
        int prev = index-1;
        int next = index+1;
        if(prev >= 0 && (bucketForIndex(freeSpaceBuckets[prev])->largestFreeSize() > largestFreeSize ||
                         (bucketForIndex(freeSpaceBuckets[prev])->largestFreeSize() == largestFreeSize && freeSpaceBuckets[index] < freeSpaceBuckets[prev]))
          ) {
          //This item should be behind the successor, either because it has a lower largestFreeSize, or because the index is lower
          uint oldPrevValue = freeSpaceBuckets[prev];
          freeSpaceBuckets[prev] = freeSpaceBuckets[index];
          freeSpaceBuckets[index] = oldPrevValue;
          index = prev;
        }else if(next < (int)m_freeSpaceBucketsSize && (bucketForIndex(freeSpaceBuckets[next])->largestFreeSize() < largestFreeSize ||
                                                     (bucketForIndex(freeSpaceBuckets[next])->largestFreeSize() == largestFreeSize && freeSpaceBuckets[index] > freeSpaceBuckets[next]))) {
          //This item should be behind the successor, either because it has a higher largestFreeSize, or because the index is higher
          uint oldNextValue = freeSpaceBuckets[next];
          freeSpaceBuckets[next] = freeSpaceBuckets[index];
          freeSpaceBuckets[index] = oldNextValue;
          index = next;
        }else {
          break;
        }
      }
    }
  }

  ///Does conversion from monster-bucket to normal bucket and from normal bucket to monster-bucket
  ///The bucket @param bucketNumber must already be loaded and empty. the "extent" buckets behind must also be loaded,
  ///and also be empty.
  ///The created buckets are not registered anywhere. When converting from monster-bucket to normal bucket,
  ///oldExtent+1 normal buckets are created, that must be registered somewhere.
  ///@warning During conversion, all the touched buckets are deleted and re-created
  ///@param extent When this is zero, the bucket is converted from monster-bucket to normal bucket.
  ///              When it is nonzero, it is converted to a monster-bucket.
  MyBucket* convertMonsterBucket(short unsigned int bucketNumber, unsigned int extent) {
    Q_ASSERT(bucketNumber);
    MyBucket* bucketPtr = m_fastBuckets[bucketNumber];
    if(!bucketPtr) {
      initializeBucket(bucketNumber);
      bucketPtr = m_fastBuckets[bucketNumber];
    }

    if(extent) {
      //Convert to monster-bucket
#ifdef DEBUG_MONSTERBUCKETS
      for(uint index = bucketNumber; index < bucketNumber + 1 + extent; ++index) {
        Q_ASSERT(bucketPtr->isEmpty());
        Q_ASSERT(!bucketPtr->monsterBucketExtent());
        Q_ASSERT(m_freeSpaceBuckets.indexOf(index) == -1);
      }
#endif
      for(uint index = bucketNumber; index < bucketNumber + 1 + extent; ++index)
        deleteBucket(index);

      m_fastBuckets[bucketNumber] = new MyBucket();

      m_fastBuckets[bucketNumber]->initialize(extent);

#ifdef DEBUG_MONSTERBUCKETS

      for(uint index = bucketNumber+1; index < bucketNumber + 1 + extent; ++index) {
        Q_ASSERT(!m_fastBuckets[index]);
      }
#endif

    }else{
      Q_ASSERT(bucketPtr->monsterBucketExtent());
      Q_ASSERT(bucketPtr->isEmpty());
      uint oldExtent = bucketPtr->monsterBucketExtent();
      deleteBucket(bucketNumber); //Delete the monster-bucket

      for(uint index = bucketNumber; index < bucketNumber + 1 + oldExtent; ++index) {
        Q_ASSERT(!m_fastBuckets[index]);
        m_fastBuckets[index] = new MyBucket();

        m_fastBuckets[index]->initialize(0);
        Q_ASSERT(!m_fastBuckets[index]->monsterBucketExtent());
      }
    }
    return m_fastBuckets[bucketNumber];
  }
  
  MyBucket* bucketForIndex(short unsigned int index) const {
    MyBucket* bucketPtr = m_fastBuckets[index];
    if(!bucketPtr) {
      initializeBucket(index);
      bucketPtr = m_fastBuckets[index];
    }
    return bucketPtr;
  }

  virtual bool open(const QString& path) {
    QMutexLocker lock(m_mutex);

    close();
    m_currentOpenPath = path;
    //kDebug() << "opening repository" << m_repositoryName << "at" << path;
    QDir dir(path);
    m_file = new QFile(dir.absoluteFilePath( m_repositoryName ));
    m_dynamicFile = new QFile(dir.absoluteFilePath( m_repositoryName + "_dynamic" ));
    if(!m_file->open( QFile::ReadWrite ) || !m_dynamicFile->open( QFile::ReadWrite ) ) {
      delete m_file;
      m_file = 0;
      delete m_dynamicFile;
      m_dynamicFile = 0;
      return false;
    }

    m_metaDataChanged = true;
    if(m_file->size() == 0) {

      m_file->resize(0);
      m_file->write((char*)&m_repositoryVersion, sizeof(uint));
      uint hashSize = bucketHashSize;
      m_file->write((char*)&hashSize, sizeof(uint));
      uint itemRepositoryVersion  = staticItemRepositoryVersion();
      m_file->write((char*)&itemRepositoryVersion, sizeof(uint));

      m_statBucketHashClashes = m_statItemCount = 0;

      m_file->write((char*)&m_statBucketHashClashes, sizeof(uint));
      m_file->write((char*)&m_statItemCount, sizeof(uint));

      m_buckets.resize(10);
      m_buckets.fill(0);
      uint bucketCount = m_buckets.size();
      m_file->write((char*)&bucketCount, sizeof(uint));

      m_firstBucketForHash = new short unsigned int[bucketHashSize];
      memset(m_firstBucketForHash, 0, bucketHashSize * sizeof(short unsigned int));

      m_currentBucket = 1; //Skip the first bucket, we won't use it so we have the zero indices for special purposes
      m_file->write((char*)&m_currentBucket, sizeof(uint));
      m_file->write((char*)m_firstBucketForHash, sizeof(short unsigned int) * bucketHashSize);
      //We have completely initialized the file now
      if(m_file->pos() != BucketStartOffset)
      {
        KMessageBox::error(0, i18n("Failed writing to %1, probably the disk is full", m_file->fileName()));
        abort();
      }

      m_freeSpaceBucketsSize = 0;
      m_dynamicFile->write((char*)&m_freeSpaceBucketsSize, sizeof(uint));
      m_freeSpaceBuckets.clear();
    }else{
      m_file->close();
      bool res = m_file->open( QFile::ReadOnly ); //Re-open in read-only mode, so we create a read-only m_fileMap
      VERIFY(res);
      //Check that the version is correct
      uint storedVersion = 0, hashSize = 0, itemRepositoryVersion = 0;

      m_file->read((char*)&storedVersion, sizeof(uint));
      m_file->read((char*)&hashSize, sizeof(uint));
      m_file->read((char*)&itemRepositoryVersion, sizeof(uint));
      m_file->read((char*)&m_statBucketHashClashes, sizeof(uint));
      m_file->read((char*)&m_statItemCount, sizeof(uint));

      if(storedVersion != m_repositoryVersion || hashSize != bucketHashSize || itemRepositoryVersion != staticItemRepositoryVersion()) {
        kDebug() << "repository" << m_repositoryName << "version mismatch in" << m_file->fileName() << ", stored: version " << storedVersion << "hashsize" << hashSize << "repository-version" << itemRepositoryVersion << " current: version" << m_repositoryVersion << "hashsize" << bucketHashSize << "repository-version" << staticItemRepositoryVersion();
        delete m_file;
        m_file = 0;
        delete m_dynamicFile;
        m_dynamicFile = 0;
        return false;
      }
      m_metaDataChanged = false;

      uint bucketCount;
      m_file->read((char*)&bucketCount, sizeof(uint));
      m_buckets.resize(bucketCount);
      m_buckets.fill(0);
      m_file->read((char*)&m_currentBucket, sizeof(uint));

      m_firstBucketForHash = new short unsigned int[bucketHashSize];
      m_file->read((char*)m_firstBucketForHash, sizeof(short unsigned int) * bucketHashSize);

      Q_ASSERT(m_file->pos() == BucketStartOffset);

      m_dynamicFile->read((char*)&m_freeSpaceBucketsSize, sizeof(uint));
      m_freeSpaceBuckets.resize(m_freeSpaceBucketsSize);
      m_dynamicFile->read((char*)m_freeSpaceBuckets.data(), sizeof(uint) * m_freeSpaceBucketsSize);
    }

    m_fileMapSize = 0;
    m_fileMap = 0;

#ifdef ITEMREPOSITORY_USE_MMAP_LOADING
    if(m_file->size() > BucketStartOffset){
      m_fileMap = m_file->map(BucketStartOffset, m_file->size() - BucketStartOffset);
      Q_ASSERT(m_file->isOpen());
      Q_ASSERT(m_file->size() >= BucketStartOffset);
      if(m_fileMap){
        m_fileMapSize = m_file->size() - BucketStartOffset;
      }else{
        kWarning() << "mapping" << m_file->fileName() << "FAILED!";
      }
    }
#endif
    //To protect us from inconsistency due to crashes. flush() is not enough.
    m_file->close();
    m_dynamicFile->close();

    m_fastBuckets = m_buckets.data();
    m_bucketCount = m_buckets.size();
    return true;
  }

  ///@warning by default, this does not store the current state to disk.
  virtual void close(bool doStore = false) {
    m_currentOpenPath.clear();

    if(doStore)
      store();

    if(m_file)
      m_file->close();
    delete m_file;
    m_file = 0;
    m_fileMap = 0;
    m_fileMapSize = 0;

    if(m_dynamicFile)
      m_dynamicFile->close();
    delete m_dynamicFile;
    m_dynamicFile = 0;

    delete[] m_firstBucketForHash;

    ///We intentionally don't delete the buckets here, as their contained memory may be referenced
    ///still from different places.
//     for(int i = 0; i < m_buckets.size(); ++i) {
//       delete m_buckets[i];
//     }

    m_buckets.clear();
    m_firstBucketForHash = 0;
  }

  struct AllItemsReachableVisitor {
    AllItemsReachableVisitor(ItemRepository* rep) : repository(rep) {
    }

    bool operator()(const Item* item) {
      return repository->itemReachable(item);
    }

    ItemRepository* repository;
  };

  //Returns whether the given item is reachable through its hash
  bool itemReachable(const Item* item) const {
    uint hash = item->hash();

    short unsigned int bucket = *(m_firstBucketForHash + (hash % bucketHashSize));

    while(bucket) {

      MyBucket* bucketPtr = m_fastBuckets[bucket];
      if(!bucketPtr) {
        initializeBucket(bucket);
        bucketPtr = m_fastBuckets[bucket];
      }

      if(bucketPtr->itemReachable(item, hash))
        return true;

      bucket = bucketPtr->nextBucketForHash(hash);
    }

    return false;
  }

  //Returns true if all items in the given bucket are reachable through their hashes
  bool allItemsReachable(unsigned short bucket) {
    if(!bucket)
      return true;

    MyBucket* bucketPtr = bucketForIndex(bucket);

    AllItemsReachableVisitor visitor(this);
    return bucketPtr->visitAllItems(visitor);
  }
  
  struct CleanupVisitor {
    bool operator()(const Item* item) {
      if(!ItemRequest::persistent(item)) {
        //Delete the item
      }
      return true;
    }
  };

  virtual int finalCleanup() {
    ThisLocker lock(m_mutex);
    
    int changed = 0;
    for(uint a = 1; a <= m_currentBucket; ++a) {
      MyBucket* bucket = bucketForIndex(a);
      if(bucket && bucket->dirty()) { ///@todo Faster dirty check, without loading bucket
        changed += bucket->finalCleanup(*this);
      }
      a += bucket->monsterBucketExtent(); //Skip buckets that are attached as tail to monster-buckets
    }
    
    return changed;
  }

  inline void initializeBucket(unsigned int bucketNumber) const {
    Q_ASSERT(bucketNumber);
#ifdef DEBUG_MONSTERBUCKETS
    for(uint offset = 1; offset < 5; ++offset) {
      int test = ((int)bucketNumber) - offset;
      if(test >= 0 && m_fastBuckets[test]) {
        Q_ASSERT(m_fastBuckets[test]->monsterBucketExtent() < offset);
      }
    }
#endif

    if(!m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber] = new MyBucket();

      bool doMMapLoading = (bool)m_fileMap;
      
      uint offset = ((bucketNumber-1) * MyBucket::DataSize);
      if(m_file && offset < m_fileMapSize && doMMapLoading && *reinterpret_cast<uint*>(m_fileMap + offset) == 0) {
//         kDebug() << "loading bucket mmap:" << bucketNumber;
        m_fastBuckets[bucketNumber]->initializeFromMap(reinterpret_cast<char*>(m_fileMap + offset));
      } else if(m_file) {
        //Either memory-mapping is disabled, or the item is not in the existing memory-map, 
        //so we have to load it the classical way.
        bool res = m_file->open( QFile::ReadOnly );
        
        if(offset + BucketStartOffset < m_file->size()) {
          VERIFY(res);
          offset += BucketStartOffset;
          m_file->seek(offset);
          uint monsterBucketExtent;
          m_file->read((char*)(&monsterBucketExtent), sizeof(unsigned int));;
          m_file->seek(offset);
          ///FIXME: use the data here instead of copying it again in prepareChange
          QByteArray data = m_file->read((1+monsterBucketExtent) * MyBucket::DataSize);
          m_fastBuckets[bucketNumber]->initializeFromMap(data.data());
          m_fastBuckets[bucketNumber]->prepareChange();
        }else{
          m_fastBuckets[bucketNumber]->initialize(0);
        }
        
        m_file->close();
        
      }else{
        m_fastBuckets[bucketNumber]->initialize(0);
      }
    }else{
      m_fastBuckets[bucketNumber]->initialize(0);
    }
  }

  ///Can only be called on empty buckets
  void deleteBucket(unsigned int bucketNumber) {
    Q_ASSERT(bucketForIndex(bucketNumber)->isEmpty());
    Q_ASSERT(bucketForIndex(bucketNumber)->noNextBuckets());
    delete m_fastBuckets[bucketNumber];
    m_fastBuckets[bucketNumber] = 0;
  }

  //m_file must be opened
  void storeBucket(unsigned int bucketNumber) const {
    if(m_file && m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber]->store(m_file, BucketStartOffset + (bucketNumber-1) * MyBucket::DataSize);
    }
  }

  ///Returns whether @param mustFindBucket was found
  ///If mustFindBucket is zero, the whole chain is just walked. This is good for debugging for infinite recursion.
  bool walkBucketLinks(uint checkBucket, uint hash, uint mustFindBucket = 0) const {
    bool found = false;
    while(checkBucket) {
      if(checkBucket == mustFindBucket)
        found = true;

      checkBucket = bucketForIndex(checkBucket)->nextBucketForHash(hash);
    }
    return found || (mustFindBucket == 0);
  }

  ///Computes the bucket where the chains opened by the buckets @param mainHead and @param intersectorHead
  ///with hash @param hash meet each other.
  ///@return <predecessor of first shared bucket in mainHead, first shared bucket>
  QPair<unsigned int, unsigned int> hashChainIntersection(uint mainHead, uint intersectorHead, uint hash) const {
    uint previous = 0;
    uint current = mainHead;
    while(current) {
      ///@todo Make this more efficient
      if(walkBucketLinks(intersectorHead, hash, current))
        return qMakePair(previous, current);

      previous = current;
      current = bucketForIndex(current)->nextBucketForHash(hash);
    }
    return qMakePair(0u, 0u);
  }

  void putIntoFreeList(unsigned short bucket, MyBucket* bucketPtr) {
    Q_ASSERT(!bucketPtr->monsterBucketExtent());
    int indexInFree = m_freeSpaceBuckets.indexOf(bucket);

    if(indexInFree == -1 && (bucketPtr->freeItemCount() >= MyBucket::MinFreeItemsForReuse || bucketPtr->largestFreeSize() >= MyBucket::MinFreeSizeForReuse)) {

      //Add the bucket to the list of buckets from where to re-assign free space
      //We only do it when a specific threshold of empty items is reached, because that way items can stay "somewhat" semantically ordered.
      Q_ASSERT(bucketPtr->largestFreeSize());
      uint insertPos;
      for(insertPos = 0; insertPos < m_freeSpaceBucketsSize; ++insertPos) {
        if(bucketForIndex(m_freeSpaceBuckets[insertPos])->largestFreeSize() > bucketPtr->largestFreeSize())
          break;
      }

      m_freeSpaceBuckets.insert(insertPos, bucket);
      ++m_freeSpaceBucketsSize;
      updateFreeSpaceOrder(insertPos);
    }else if(indexInFree != -1) {
      ///Re-order so the order in m_freeSpaceBuckets is correct(sorted by largest free item size)
      updateFreeSpaceOrder(indexInFree);
    }
#ifdef DEBUG_MONSTERBUCKETS
    if(bucketPtr->isEmpty()) {
      Q_ASSERT(m_freeSpaceBuckets.contains(bucket));
    }
#endif
  }

  void verifyIndex(uint index) const
  {
    // We don't use zero indices
    Q_ASSERT(index);
    uint bucket = (index >> 16);
    // nor zero buckets
    Q_ASSERT(bucket);
    Q_ASSERT_X(bucket < m_bucketCount, Q_FUNC_INFO,
               qPrintable(QString("index %1 gives invalid bucket number %2, current count is: %3")
                  .arg(index)
                  .arg(bucket)
                  .arg(m_bucketCount)));

    // don't trigger compile warnings in release mode
    Q_UNUSED(bucket);
    Q_UNUSED(index);
  }

  bool m_metaDataChanged;
  mutable QMutex m_ownMutex;
  mutable QMutex* m_mutex;
  QString m_repositoryName;
  unsigned int m_size;
  mutable uint m_currentBucket;
  //List of buckets that have free space available that can be assigned. Sorted by size: Smallest space first. Second order sorting: Bucket index
  QVector<uint> m_freeSpaceBuckets;
  uint m_freeSpaceBucketsSize; //for speedup
  mutable QVector<MyBucket* > m_buckets;
  mutable MyBucket** m_fastBuckets; //For faster access
  mutable uint m_bucketCount;
  uint m_statBucketHashClashes, m_statItemCount;
  unsigned int m_bucketHashSize;
  //Maps hash-values modulo 1<<bucketHashSizeBits to the first bucket such a hash-value appears in
  short unsigned int* m_firstBucketForHash;

  QString m_currentOpenPath;
  ItemRepositoryRegistry* m_registry;
  //File that contains the buckets
  QFile* m_file;
  uchar* m_fileMap;
  uint m_fileMapSize;
  //File that contains more dynamic data, like the list of buckets with deleted items
  QFile* m_dynamicFile;
  uint m_repositoryVersion;
  bool m_unloadingEnabled;
  AbstractRepositoryManager* m_manager;
};

}

#endif
