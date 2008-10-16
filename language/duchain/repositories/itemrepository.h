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

#ifndef ITEMREPOSITORY_H
#define ITEMREPOSITORY_H

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QByteArray>
#include <QtCore/QMutex>
#include <QtCore/QList>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QAtomicInt>
#include <klockfile.h>
#include <kdebug.h>
#include "../../languageexport.h"

//#define DEBUG_ITEMREPOSITORY_LOADING
// #define ifDebugInfiniteRecursion(x) x
#define ifDebugInfiniteRecursion(x)

// #define ifDebugLostSpace(x) x
#define ifDebugLostSpace(x)
#define DEBUG_INCORRECT_DELETE

//Makes sure that all items stay reachable through the basic hash
// #define DEBUG_ITEM_REACHABILITY

#ifdef DEBUG_ITEM_REACHABILITY
#define ENSURE_REACHABLE(bucket) Q_ASSERT(allItemsReachable(bucket));
#else
#define ENSURE_REACHABLE(bucket)
#endif

///When this is uncommented, a 64-bit test-value is written behind the area an item is allowed to write into before
///createItem(..) is called, and an assertion triggers when it was changed during createItem(), which means createItem wrote too long.
///The problem: This temporarily overwrites valid data in the following item, so it will cause serious problems if that data is accessed
///during the call to createItem().
//#define DEBUG_WRITING_EXTENTS

namespace KDevelop {

  /**
   * This file implements a generic bucket-based indexing repository, that can be used for example to index strings.
   * 
   * All you need to do is define your item type that you want to store into the repository, and create a request item
   * similar to ExampleRequestItem that compares and fills the defined item type.
   * 
   * For example the string repository uses "unsigned short" as item-type, uses that actual value to store the length of the string,
   * and uses the space behind to store the actual string content.
   *
   * @see ItemRepository
   * @see stringrepository.h
   * @see indexedstring.h
   *
   * Items must not be larger than the bucket size!
   * */

///Returns a version-number that is used to reset the item-repository after incompatible layout changes
KDEVPLATFORMLANGUAGE_EXPORT uint staticItemRepositoryVersion();

class KDEVPLATFORMLANGUAGE_EXPORT AbstractItemRepository {
  public:
    virtual ~AbstractItemRepository();
    ///@param path is supposed to be a shared directory-name that the item-repository is to be loaded from
    ///@param clear will be true if the old repository should be discarded and a new one started
    ///If this returns false, that indicates that opening failed.
    virtual bool open(const QString& path) = 0;
    virtual void close(bool doStore = false) = 0;
    virtual void store() = 0;
};

/**
 * Manages a set of item-repositores and allows loading/storing them all at once from/to disk.
 * Does not automatically store contained repositories on destruction.
 * For the global standard registry, the storing is triggered from within duchain, so you don't need to care about it.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ItemRepositoryRegistry {
  public:
    ItemRepositoryRegistry(QString openPath = QString(), KLockFile::Ptr lock = KLockFile::Ptr());
    ~ItemRepositoryRegistry();
    
    ///Path is supposed to be a shared directory-name that the item-repositories are to be loaded from
    ///@param clear Whether a fresh start should be done, and all repositories cleared
    ///If this returns false, loading has failed, and all repositories have been discarded.
    ///@note Currently the given path must reference a hidden directory, just to make sure we're
    ///      not accidentally deleting something important
    bool open(const QString& path, bool clear = false, KLockFile::Ptr lock = KLockFile::Ptr());
    ///@warning The current state is not stored to disk.
    void close();
    ///The registered repository will automatically be opened with the current path, if one is set.
    void registerRepository(AbstractItemRepository* repository);
    ///The registered repository will automatically be closed if it was open.
    void unRegisterRepository(AbstractItemRepository* repository);
    ///Returns the path currently set
    QString path() const;
    ///Should be called on a regular basis: Stores all repositories to disk, and eventually unloads unneeded data to save memory
    void store();
    
    ///Call this to lock the directory for writing. When KDevelop crashes while the directory is locked for writing,
    ///it will know that the directory content is inconsistent, and discard it while next startup.
    void lockForWriting();
    ///Call this when you're ready writing, after lockForWriting has been called
    void unlockForWriting();
    
    ///Returns a custom counter, identified by the given identity, that is persistently stored in the repository directory.
    ///If the counter didn't exist before, it will be initialized with initialValue
    QAtomicInt& getCustomCounter(const QString& identity, int initialValue);
  private:
    void deleteDataDirectory();
    QString m_path;
    QList<AbstractItemRepository*> m_repositories;
    QMap<QString, QAtomicInt*> m_customCounters;
    KLockFile::Ptr m_lock;
    mutable QMutex m_mutex;
};

///The global item-repository registry that is used by default
KDEVPLATFORMLANGUAGE_EXPORT ItemRepositoryRegistry& globalItemRepositoryRegistry();

  ///This is the actual data that is stored in the repository. All the data that is not directly in the class-body,
  ///like the text of a string, can be stored behind the item in the same memory region. The only important thing is
  ///that the Request item(@see ExampleItemRequest) correctly advertises the space needed by this item.
class ExampleItem {
  //Every item has to implement this function, and return a valid hash.
  //Must be exactly the same hash value as ExampleItemRequest::hash() has returned while creating the item.
  unsigned int hash() const {
    return 0;
  }
  
  //Every item has to implement this function, and return the complete size this item takes in memory.
  //Must be exactly the same value as ExampleItemRequest::itemSize() has returned while creating the item.
  unsigned short int itemSize() const {
    return 0;
  }
};

/**
 * A request represents the information that is searched in the repository.
 * It must be able to compare itself to items stored in the repository, and it must be able to
 * create items in the. The item-types can also be basic data-types, with additional information stored behind.
 * */

enum {
  ItemRepositoryBucketSize = 1<<16
};

class ExampleRequestItem {

  enum {
    AverageSize = 10 //This should be the approximate average size of an Item
  };

  typedef unsigned int HashType;
  
  //Should return the hash-value associated with this request(For example the hash of a string)
  HashType hash() const {
    return 0;
  }
  
  //Should return the size of an item created with createItem
  size_t itemSize() const {
      return 0;
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  ///@warning Never call non-constant functions on the repository from within this function!
  void createItem(ExampleItem* /*item*/) const {
  }
  
  //Should return whether the here requested item equals the given item
  bool equals(const ExampleItem* /*item*/) const {
    return false;
  }
};

template<class Item, class ItemRequest, class DynamicData>
class Bucket {
  enum {
    AdditionalSpacePerItem = DynamicData::Size + 2
  };
  public:
    enum {
      ObjectMapSize = (ItemRepositoryBucketSize / ItemRequest::AverageSize) + 1,
      MaxFreeItemsForHide = 0, //When less than this count of free items in one buckets is reached, the bucket is removed from the global list of buckets with free items
      MinFreeItemsForReuse = 1//When this count of free items in one bucket is reached, consider re-assigning them to new requests
    };
    enum {
      NextBucketHashSize = ObjectMapSize, //Affects the average count of bucket-chains that need to be walked in ItemRepository::index. Must be a multiple of ObjectMapSize
      DataSize = sizeof(unsigned int) * 4 + ItemRepositoryBucketSize + sizeof(short unsigned int) * (ObjectMapSize + NextBucketHashSize + 1)
    };
    enum {
      CheckStart = 0xff00ff1,
      CheckEnd = 0xfafcfb
    };
    Bucket() : m_available(0), m_data(0), m_objectMap(0), m_objectMapSize(0), m_largestFreeItem(0), m_freeItemCount(0), m_nextBucketHash(0) {
    }
    ~Bucket() {
      delete[] m_data;
      delete[] m_nextBucketHash;
      delete[] m_objectMap;
    }

    void initialize() {
      if(!m_data) {
        m_available = ItemRepositoryBucketSize;
        m_data = new char[ItemRepositoryBucketSize];
        //The bigger we make the map, the lower the probability of a clash(and thus bad performance). However it increases memory usage.
        m_objectMapSize = ObjectMapSize;
        m_objectMap = new short unsigned int[m_objectMapSize];
        memset(m_objectMap, 0, m_objectMapSize * sizeof(short unsigned int));
        m_nextBucketHash = new short unsigned int[NextBucketHashSize];
        memset(m_nextBucketHash, 0, NextBucketHashSize * sizeof(short unsigned int));
        m_changed = true;
        m_lastUsed = 0;
      }
    }
    void initialize(QFile* file, size_t offset) {
      if(!m_data) {
        initialize();
        if(file->size() >= offset + DataSize) {
          file->seek(offset);
          Q_ASSERT(file->size() >= offset + DataSize);
          
          uint checkStart, checkEnd;
          file->read((char*)&checkStart, sizeof(unsigned int));
          file->read((char*)&m_available, sizeof(unsigned int));
          file->read(m_data, ItemRepositoryBucketSize);
          file->read((char*)m_objectMap, sizeof(short unsigned int) * m_objectMapSize);
          file->read((char*)m_nextBucketHash, sizeof(short unsigned int) * NextBucketHashSize);
          file->read((char*)&m_largestFreeItem, sizeof(short unsigned int));
          file->read((char*)&m_freeItemCount, sizeof(unsigned int));
          file->read((char*)&checkEnd, sizeof(unsigned int));
          Q_ASSERT(checkStart == CheckStart);
          Q_ASSERT(checkEnd == CheckEnd);
          Q_ASSERT(file->pos() == offset + DataSize);
          m_changed = false;
          m_lastUsed = 0;
        }
      }
    }

    void store(QFile* file, size_t offset) {
      if(!m_data)
        return;
      
      if(file->size() < offset + DataSize)
        file->resize(offset + DataSize);

      file->seek(offset);
      
      uint checkStart = CheckStart, checkEnd = CheckEnd;
      file->write((char*)&checkStart, sizeof(unsigned int));
      file->write((char*)&m_available, sizeof(unsigned int));
      file->write(m_data, ItemRepositoryBucketSize);
      file->write((char*)m_objectMap, sizeof(short unsigned int) * m_objectMapSize);
      file->write((char*)m_nextBucketHash, sizeof(short unsigned int) * NextBucketHashSize);
      file->write((char*)&m_largestFreeItem, sizeof(short unsigned int));
      file->write((char*)&m_freeItemCount, sizeof(unsigned int));
      file->write((char*)&checkEnd, sizeof(unsigned int));

      Q_ASSERT(file->pos() == offset + DataSize);
      m_changed = false;
#ifdef DEBUG_ITEMREPOSITORY_LOADING
      {
        file->flush();
        file->seek(offset);
        
        uint available, freeItemCount;
        short unsigned int largestFree;
        
        char* d = new char[ItemRepositoryBucketSize];
        short unsigned int* m = new short unsigned int[m_objectMapSize];
        short unsigned int* h = new short unsigned int[NextBucketHashSize];
        
        file->read((char*)&checkStart, sizeof(unsigned int));
        file->read((char*)&available, sizeof(unsigned int));
        file->read(d, ItemRepositoryBucketSize);
        file->read((char*)m, sizeof(short unsigned int) * m_objectMapSize);
        file->read((char*)h, sizeof(short unsigned int) * NextBucketHashSize);
        file->read((char*)&largestFree, sizeof(short unsigned int));
        file->read((char*)&freeItemCount, sizeof(unsigned int));
        file->read((char*)&checkEnd, sizeof(unsigned int));

        Q_ASSERT(checkStart == CheckStart);
        Q_ASSERT(checkEnd == CheckEnd);
        
        Q_ASSERT(available == m_available);
        Q_ASSERT(memcmp(d, m_data, ItemRepositoryBucketSize) == 0);
        Q_ASSERT(memcmp(m, m_objectMap, sizeof(short unsigned int) * m_objectMapSize) == 0);
        Q_ASSERT(memcmp(h, m_nextBucketHash, sizeof(short unsigned int) * NextBucketHashSize) == 0);
        Q_ASSERT(m_largestFreeItem == largestFree);
        Q_ASSERT(m_freeItemCount == freeItemCount);
        
        Q_ASSERT(file->pos() == offset + DataSize);
        
        delete[] m;
        delete[] h;
      }
#endif
    }

    //Tries to find the index this item has in this bucket, or returns zero if the item isn't there yet.
    unsigned short findIndex(const ItemRequest& request, const DynamicData* dynamic) const {
      m_lastUsed = 0;
      
      unsigned short localHash = request.hash() % m_objectMapSize;
      unsigned short index = m_objectMap[localHash];

      unsigned short follower = 0;
      //Walk the chain of items with the same local hash
      while(index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
        index = follower;

      if(index && request.equals(itemFromIndex(index))) {
        if(dynamic)
          itemFromIndex(index, dynamic);
        return index; //We have found the item
      }

      return 0;
    }
    
    //Tries to get the index within this bucket, or returns zero. Will put the item into the bucket if there is room.
    //Created indices will never begin with 0xffff____, so you can use that index-range for own purposes.
    unsigned short index(const ItemRequest& request, const DynamicData* dynamic, unsigned int itemSize) {
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

      m_changed = true;

      unsigned int totalSize = itemSize + AdditionalSpacePerItem;
      //If this triggers, your item is too big.
      Q_ASSERT(totalSize < ItemRepositoryBucketSize);
      
      //The second condition is needed, else we can get problems with zero-length items and an overflow in insertedAt to zero
      if(totalSize > m_available || (!itemSize && totalSize == m_available)) {
        //Try finding the smallest freed item that can hold the data
        unsigned short currentIndex = m_largestFreeItem;
        unsigned short previousIndex = 0;
        
        unsigned short freeChunkSize = 0;
        
        while(currentIndex && freeSize(currentIndex) >= (totalSize-AdditionalSpacePerItem)) {
          unsigned short follower = followerIndex(currentIndex);
          if(follower && freeSize(follower) >= (totalSize-AdditionalSpacePerItem)) {
            //The item also fits into the smaller follower, so use that one
            previousIndex = currentIndex;
            currentIndex = follower;
          }else{
            //The item fits into currentIndex, but not into the follower. So use currentIndex
            freeChunkSize = freeSize(currentIndex) - (totalSize - AdditionalSpacePerItem);
            
            //We need 2 bytes to store the free size
            if(freeChunkSize != 0 && freeChunkSize < AdditionalSpacePerItem+2) {
              //we can not manage the resulting free chunk as a separate item, so we cannot use this position.
              //Just pick the biggest free item, because there we can be sure that
              //either we can manage the split, or we cannot do anything at all in this bucket.
              
              freeChunkSize = freeSize(m_largestFreeItem) - (totalSize - AdditionalSpacePerItem);
              
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
            freeItemPosition = currentIndex + totalSize;
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
      
      DynamicData::initialize(m_data + insertedAt - AdditionalSpacePerItem);
      
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
      request.createItem((Item*)(m_data + insertedAt));
      
#ifdef DEBUG_CREATEITEM_EXTENTS
      if(m_available >= 8) {
        //If this assertion triggers, then the item writes a bigger range than it advertised in 
        Q_ASSERT(*((quint64*)borderBehind) == 0xfafafafafafafafaLLU);
        *((quint64*)borderBehind) = oldValueBehind;
      }
#endif
      
      Q_ASSERT(itemFromIndex(insertedAt)->hash() == request.hash());
      Q_ASSERT(itemFromIndex(insertedAt)->itemSize() == itemSize);
      
      if(dynamic)
        itemFromIndex(insertedAt, dynamic);

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
    
    
    void deleteItem(unsigned short index, unsigned int hash) {
      ifDebugLostSpace( Q_ASSERT(!lostSpace()); )

      m_lastUsed = 0;
      m_changed = true;

      unsigned short size = itemFromIndex(index)->itemSize();
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
      
      memset(const_cast<Item*>(itemFromIndex(index)), 0, size); //For debugging, so we notice the data is wrong   
      
      setFreeSize(index, size);
      
      //Try merging the created free item to other free items around, else add it into the free list
      insertFreeItem(index);
      
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
    }
    
    inline const Item* itemFromIndex(unsigned short index) const {
      m_lastUsed = 0;
      return (Item*)(m_data+index);
    }

    ///When dynamic is nonzero, and apply(..) returns false, the item is freed, and the returned item is invalid.
    inline const Item* itemFromIndex(unsigned short index, const DynamicData* dynamic) const {
      m_lastUsed = 0;
      Item* ret((Item*)(m_data+index));
      if(dynamic)
        dynamic->apply(m_data + index - AdditionalSpacePerItem);
      return ret;
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
          if(!visitor((const Item*)(m_data+currentIndex)))
            return false;
          
          currentIndex = followerIndex(currentIndex);
        }
      }
      return true;
    }
    
    unsigned short nextBucketForHash(uint hash) const {
      m_lastUsed = 0;
      return m_nextBucketHash[hash % NextBucketHashSize];
    }
    
    void setNextBucketForHash(unsigned int hash, unsigned short bucket) {
      m_lastUsed = 0;
      m_changed = true;
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
    
    short unsigned int largestFreeItemSize() const {
      if(m_largestFreeItem)
        return freeSize(m_largestFreeItem);
      else
        return 0;
    }
    
    bool canAllocateItem(short unsigned int size) const {
      short unsigned int currentIndex = m_largestFreeItem;
      while(currentIndex) {
        short unsigned int currentFree = freeSize(currentIndex);
        if(currentFree < size)
          return false;
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
    
    void setChanged() {
      m_changed = true;
    }
    
    //Counts together the space that is neither accessible through m_objectMap nor through the free items
    uint lostSpace() {
      uint need = ItemRepositoryBucketSize - m_available;
      uint found = 0;
      
      for(uint a = 0; a < m_objectMapSize; ++a) {
        uint currentIndex = m_objectMap[a];
        while(currentIndex) {
          found += ((const Item*)(m_data+currentIndex))->itemSize() + AdditionalSpacePerItem;
          
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
    
  private:
    
    ///Merges the given index item, which must have a freeSize() set, to surrounding free items, and inserts the result.
    ///The given index itself should not be in the free items chain yet.
    ///Returns whether the item was inserted somewhere.
    void insertFreeItem(unsigned short index) {
      unsigned short currentIndex = m_largestFreeItem;
      unsigned short previousIndex = 0;
      
      while(currentIndex) {
        
        //Merge behind index
        if(currentIndex == index + freeSize(index) + AdditionalSpacePerItem) {
          
          //Remove currentIndex from the free chain, since it's merged backwards into index
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
      }
      
      insertToFreeChain(index);
    }
    
    ///Only inserts the item in the correct position into the free chain. index must not be in the chain yet.
    void insertToFreeChain(unsigned short index) {
      //Insert the free item into the chain opened by m_largestFreeItem
      unsigned short currentIndex = m_largestFreeItem;
      unsigned short previousIndex = 0;
      
      unsigned short size = freeSize(index);
      
      while(currentIndex && *(unsigned short*)(m_data + currentIndex) > size) {
        Q_ASSERT(currentIndex != index); //must not be in the chain yet
        previousIndex = currentIndex;
        currentIndex = followerIndex(currentIndex);
      }
      
      setFollowerIndex(index, currentIndex);
      
      if(previousIndex)
        setFollowerIndex(previousIndex, index);
      else
        //This item is larger than all already registered free items, or there are none.
        m_largestFreeItem = index;

      ++m_freeItemCount;
    }
    
    ///Returns true if the given index is right behind free space, and thus can be merged to the free space.
    bool isBehindFreeSpace(unsigned short index) const {
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
      return *((unsigned short*)(m_data+(index-2)));
    }

    void setFollowerIndex(unsigned short index, unsigned short follower) {
      Q_ASSERT(index >= 2);
      *((unsigned short*)(m_data+(index-2))) = follower;
    }
    //Only returns the corrent value if the item is actually free
    inline unsigned short freeSize(unsigned short index) const {
      return *((unsigned short*)(m_data+index));
    }

    //Convenience function to set the free-size, only for freed items
    void setFreeSize(unsigned short index, unsigned short size) {
      *((unsigned short*)(m_data+index)) = size;
    }

    unsigned int m_available;
    char* m_data; //Structure of the data: <Position of next item with same hash modulo ItemRepositoryBucketSize>(2 byte), <Item>(item.size() byte)
    short unsigned int* m_objectMap; //Points to the first object in m_data with (hash % m_objectMapSize) == index. Points to the item itself, so substract 1 to get the pointer to the next item with same local hash.
    uint m_objectMapSize;
    short unsigned int m_largestFreeItem; //Points to the largest item that is currently marked as free, or zero. That one points to the next largest one through followerIndex
    unsigned int m_freeItemCount;
    
    unsigned short* m_nextBucketHash;
    
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
    m_mutex->lock();
  }
  ~Locker() {
    m_mutex->unlock();
  }
  QMutex* m_mutex;
};

struct NoDynamicData {
  
  enum {
    Size = 0
  };

  static void initialize(char* /*data*/) {
  }
  
  ///When this returns false, the item is deleted
  bool apply(const char*) const {
    return true;
  }
};

struct ReferenceCounting {
  
  enum {
    Size = sizeof(unsigned int)
  };
  
  ReferenceCounting(bool increment) : m_increment(increment) {
  }

  static void initialize(char* data) {
    *((unsigned int*)data) = 0;
  }
  
  ///When this returns false, the item is deleted
  bool apply(char* data) const {
    if(m_increment)
      ++*((unsigned int*)data);
    else
      --*((unsigned int*)data);
    
    return *((unsigned int*)data);
  }
  
  bool m_increment;
};

///@param Item @see ExampleItem
///@param ItemRequest @see ExampleReqestItem
///@param DynamicData Can be used to attach additional metadata of constant size to each item. 
///                   That meta-data can be manipulated by giving manipulators to the ItemRepository Member functions. 
///                   This can be used to implement reference-counting, @see ReferenceCounting
///@param threadSafe Whether class access should be thread-safe
template<class Item, class ItemRequest, class DynamicData = NoDynamicData, bool threadSafe = true, unsigned int targetBucketHashSize = 524288>
class ItemRepository : public AbstractItemRepository {

  typedef Locker<threadSafe> ThisLocker;
  
  enum {
    //Must be a multiple of Bucket::ObjectMapSize, so Bucket::hasClashingItem can be computed
    //Must also be a multiple of Bucket::NextBucketHashSize, for the same reason.(Currently those are same)
    bucketHashSize = (targetBucketHashSize / Bucket<Item, ItemRequest, DynamicData>::ObjectMapSize) * Bucket<Item, ItemRequest, DynamicData>::ObjectMapSize
  };
  
  enum {
    BucketStartOffset = sizeof(uint) * 7 + sizeof(short unsigned int) * bucketHashSize //Position in the data where the bucket array starts
  };
  
  public:
  ///@param registry May be zero, then the repository will not be registered at all. Else, the repository will register itself to that registry.
  ///                If this is zero, you have to care about storing the data using store() and/or close() by yourself. It does not happen automatically.
  ///                For the global standard registry, the storing/loading is triggered from within duchain, so you don't need to care about it.
  ItemRepository(QString repositoryName, ItemRepositoryRegistry* registry  = &globalItemRepositoryRegistry(), uint repositoryVersion = 1) : m_mutex(QMutex::Recursive), m_repositoryName(repositoryName), m_registry(registry), m_file(0), m_dynamicFile(0), m_repositoryVersion(repositoryVersion) {
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
      m_registry->registerRepository(this);
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
  unsigned int index(const ItemRequest& request, const DynamicData* dynamic = 0) {
    
    ThisLocker lock(&m_mutex);
    
    unsigned int hash = request.hash();
    unsigned int size = request.itemSize();
    
    short unsigned int* bucketHashPosition = m_firstBucketForHash + (hash % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;
    short unsigned int previousPreviousBucketNumber = 0;
    
    uint useBucket = m_currentBucket;
    bool pickedBucketInChain = false; //Whether a bucket was picked for re-use that already is in the hash chain
    int reOrderFreeSpaceBucketIndex = -1;
    
    while(previousBucketNumber) {
      //We have a bucket that contains an item with the given hash % bucketHashSize, so check if the item is already there
      Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[previousBucketNumber];
      if(!bucketPtr) {
        initializeBucket(previousBucketNumber);
        bucketPtr = m_fastBuckets[previousBucketNumber];
      }
      
      unsigned short indexInBucket = bucketPtr->findIndex(request, dynamic);
      if(indexInBucket) {
        //We've found the item, it's already there
        return (previousBucketNumber << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
      } else {
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
          previousPreviousBucketNumber = previousBucketNumber;
          previousBucketNumber = next;
        } else
          break;
      }
    }
    
    m_metaDataChanged = true;

    if(!pickedBucketInChain && useBucket == m_currentBucket) {
      //Try finding an existing bucket with deleted space to store the data into
      for(uint a = 0; a < m_freeSpaceBucketsSize; ++a) {
        Bucket<Item, ItemRequest, DynamicData>* bucketPtr = bucketForIndex(m_freeSpaceBuckets[a]);

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
      Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[useBucket];
      if(!bucketPtr) {
        initializeBucket(useBucket);
        bucketPtr = m_fastBuckets[useBucket];
      }
      
      ENSURE_REACHABLE(useBucket);
      Q_ASSERT(!bucketPtr->findIndex(request, 0));
      
      unsigned short indexInBucket = bucketPtr->index(request, dynamic, size);
      
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
        
        return (useBucket << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
      }else{
        //This should never happen when we picked a bucket for re-use
        Q_ASSERT(!pickedBucketInChain);
        Q_ASSERT(reOrderFreeSpaceBucketIndex == -1);
        Q_ASSERT(useBucket == m_currentBucket);

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
    
    ThisLocker lock(&m_mutex);
    
    unsigned int hash = request.hash();
    
    short unsigned int* bucketHashPosition = m_firstBucketForHash + (hash % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;
    
    while(previousBucketNumber) {
      //We have a bucket that contains an item with the given hash % bucketHashSize, so check if the item is already there
      
      Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[previousBucketNumber];
      if(!bucketPtr) {
        initializeBucket(previousBucketNumber);
        bucketPtr = m_fastBuckets[previousBucketNumber];
      }
      
      unsigned short indexInBucket = bucketPtr->findIndex(request, 0);
      if(indexInBucket) {
        //We've found the item, it's already there
        return (previousBucketNumber << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
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
    ThisLocker lock(&m_mutex); 
    
    m_metaDataChanged = true;
    
    unsigned short bucket = (index >> 16);
    Q_ASSERT(bucket); //We don't use zero buckets

    unsigned int hash = itemFromIndex(index)->hash();
    
    short unsigned int* bucketHashPosition = m_firstBucketForHash + (hash % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;

    Q_ASSERT(previousBucketNumber);
    
    if(previousBucketNumber == bucket)
      previousBucketNumber = 0;
    
    Bucket<Item, ItemRequest, DynamicData>* previousBucketPtr = 0;
    
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
    
    Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[bucket];
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    
    --m_statItemCount;
    
    bucketPtr->deleteItem(index, hash);
    
    int indexInFree = m_freeSpaceBuckets.indexOf(bucket);
    
    if(indexInFree == -1 && bucketPtr->freeItemCount() > Bucket<Item, ItemRequest, DynamicData>::MinFreeItemsForReuse) {
      //Add the bucket to the list of buckets from where to re-assign free space
      //We only do it when a specific threshold of empty items is reached, because that way items can stay "somewhat" semantically ordered.
      Q_ASSERT(bucketPtr->largestFreeItemSize());
      uint insertPos;
      for(insertPos = 0; insertPos < m_freeSpaceBucketsSize; ++insertPos) {
        if(bucketForIndex(m_freeSpaceBuckets[insertPos])->largestFreeItemSize() > bucketPtr->largestFreeItemSize())
          break;
      }
      
      m_freeSpaceBuckets.insert(insertPos, bucket);
      ++m_freeSpaceBucketsSize;
    }else if(indexInFree != -1) {
      ///Re-order so the order in m_freeSpaceBuckets is correct(sorted by largest free item size)
      updateFreeSpaceOrder(indexInFree);
    }
    
    /**
     * Now check whether the link previousBucketNumber -> bucket is still needed.
     */
    //return; ///@todo Find out what this problem is about. If we don't return here, some items become undiscoverable through hashes after some time
    if(previousBucketNumber == 0) {
      //The item is directly in the m_firstBucketForHash hash
      //Put the next item in the nextBucketForHash chain into m_firstBucketForHash that has a hash clashing in that array.
      Q_ASSERT(*bucketHashPosition == bucket);
      unsigned short previous = bucket;
      while(!bucketPtr->hasClashingItem(hash, bucketHashSize))
      {
//         Q_ASSERT(!bucketPtr->hasClashingItemReal(hash, bucketHashSize));
        
        unsigned short next = bucketPtr->nextBucketForHash(hash);
        ENSURE_REACHABLE(next);
        ENSURE_REACHABLE(previous);
        
        *bucketHashPosition = next;
        
        ENSURE_REACHABLE(previous);
        ENSURE_REACHABLE(next);
        
        previous = next;
        
        if(next) {
          bucketPtr = m_fastBuckets[next];
          
          if(!bucketPtr) 
          {
            initializeBucket(next);
            bucketPtr = m_fastBuckets[next];
          }
        }else{
          return;
        }
      }
    }else{
      if(!bucketPtr->hasClashingItem(hash, Bucket<Item, ItemRequest, DynamicData>::NextBucketHashSize)) {
//         Q_ASSERT(!bucketPtr->hasClashingItemReal(hash, Bucket<Item, ItemRequest, DynamicData>::NextBucketHashSize));
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
  }
  
  ///This returns an editable version of the item. @warning: Never change an entry that affects the hash,
  ///or the equals(..) function. That would completely destroy the consistency.
  ///@param index The index. It must be valid(match an existing item), and nonzero.
  ///@param dynamic will be applied to the item.
  Item* dynamicItemFromIndex(unsigned int index, const DynamicData* dynamic = 0) {
    Q_ASSERT(index);
    
    ThisLocker lock(&m_mutex);
    
    unsigned short bucket = (index >> 16);
    Q_ASSERT(bucket); //We don't use zero buckets
    
    Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[bucket];
    Q_ASSERT(bucket < m_bucketCount);
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    bucketPtr->setChanged();
    unsigned short indexInBucket = index & 0xffff;
    return const_cast<Item*>(bucketPtr->itemFromIndex(indexInBucket, dynamic));
  }
  
  ///@param index The index. It must be valid(match an existing item), and nonzero.
  ///@param dynamic will be applied to the item.
  const Item* itemFromIndex(unsigned int index, const DynamicData* dynamic = 0) const {
    Q_ASSERT(index);
    
    ThisLocker lock(&m_mutex);
    
    unsigned short bucket = (index >> 16);
    Q_ASSERT(bucket); //We don't use zero buckets
    
    const Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[bucket];
    Q_ASSERT(bucket < m_bucketCount);
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    unsigned short indexInBucket = index & 0xffff;
    return bucketPtr->itemFromIndex(indexInBucket, dynamic);
  }
  
  QString statistics() const {
    QString ret;
    uint loadedBuckets = 0;
    for(uint a = 0; a < m_bucketCount; ++a)
      if(m_fastBuckets[a])
        ++loadedBuckets;
    
    ret += QString("loaded buckets: %1 current bucket: %2 used memory: %3").arg(loadedBuckets).arg(m_currentBucket).arg(usedMemory());
    ret += QString("\nbucket hash clashed items: %1 total items: %2").arg(m_statBucketHashClashes).arg(m_statItemCount);
    uint usedBucketSpace = Bucket<Item, ItemRequest, DynamicData>::DataSize * m_currentBucket;
    uint freeBucketSpace = 0;
    uint lostSpace = 0; //Should be zero, else something is wrong
    for(uint a = 1; a < m_currentBucket+1; ++a) {
      Bucket<Item, ItemRequest, DynamicData>* bucket = bucketForIndex(a);
      if(bucket) {
        freeBucketSpace += bucket->totalFreeItemsSize() + bucket->available();
        lostSpace += bucket->lostSpace();
      }
    }
    ret += QString("\nused space for buckets: %1 free space in buckets: %2 lost space: %3").arg(usedBucketSpace).arg(freeBucketSpace).arg(lostSpace);
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
    ThisLocker lock(&m_mutex);
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
    ThisLocker lock(&m_mutex);
    
    short unsigned int bucket = *(m_firstBucketForHash + (hash % bucketHashSize));
    
    while(bucket) {
      
      Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[bucket];
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
    ThisLocker lock(&m_mutex);
    if(m_file) {
      if(!m_file->open( QFile::ReadWrite ) || !m_dynamicFile->open( QFile::ReadWrite )) {
        kWarning() << "cannot re-open repository file for storing";
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
      //To protect us from inconsistency due to crashes. flush() is not enough.
      m_file->close();
      m_dynamicFile->close();
    }
  }

  private:
  
  ///Makes sure the order within m_freeSpaceBuckets is correct, after largestFreeItemSize has been changed for m_freeSpaceBuckets[index].
  ///If too few space is free within the given bucket, it is removed from m_freeSpaceBuckets.
  void updateFreeSpaceOrder(int index) {
    m_metaDataChanged = true;

    Q_ASSERT(index >= 0 && index < m_freeSpaceBuckets.size());
    Bucket<Item, ItemRequest, DynamicData>* bucketPtr = bucketForIndex(m_freeSpaceBuckets[index]);
    
    unsigned short largestFreeSize = bucketPtr->largestFreeItemSize();
    
    if(largestFreeSize == 0 || bucketPtr->freeItemCount() <= Bucket<Item, ItemRequest, DynamicData>::MaxFreeItemsForHide) {
      //Remove the item from m_freeSpaceBuckets
      m_freeSpaceBuckets.remove(index);
      m_freeSpaceBucketsSize = m_freeSpaceBuckets.size();
    }else{
      while(1) {
        int prev = index-1;
        int next = index+1;
        if(prev >= 0 && bucketForIndex(m_freeSpaceBuckets[prev])->largestFreeItemSize() > largestFreeSize) {
          //The predecessor has a bigger largestFreeItemSize, but it should be smallest first. So flip.
          uint oldPrevValue = m_freeSpaceBuckets[prev];
          m_freeSpaceBuckets[prev] = m_freeSpaceBuckets[index];
          m_freeSpaceBuckets[index] = oldPrevValue;
          index = prev;
        }else if(next < m_freeSpaceBuckets.size() && bucketForIndex(m_freeSpaceBuckets[next])->largestFreeItemSize() < largestFreeSize) {
          //The successor has a smaller largestFreeItemSize, but the order is smallest-first, so flip.
          uint oldNextValue = m_freeSpaceBuckets[next];
          m_freeSpaceBuckets[next] = m_freeSpaceBuckets[index];
          m_freeSpaceBuckets[index] = oldNextValue;
          index = next;
        }else{
          break;
        }
      }
    }
  }
    
  Bucket<Item, ItemRequest, DynamicData>* bucketForIndex(short unsigned int index) const {
    Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[index];
    if(!bucketPtr) {
      initializeBucket(index);
      bucketPtr = m_fastBuckets[index];
    }
    return bucketPtr;
  }
    
  virtual bool open(const QString& path) {
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
      Q_ASSERT(m_file->pos() == BucketStartOffset);
    
      m_freeSpaceBucketsSize = 0;
      m_dynamicFile->write((char*)&m_freeSpaceBucketsSize, sizeof(uint));
      m_freeSpaceBuckets.clear();
    }else{
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
    
    //To protect us from inconsistency due to crashes. flush() is not enough.
    m_file->close();
    m_dynamicFile->close();
    
    m_fastBuckets = m_buckets.data();
    m_bucketCount = m_buckets.size();
    return true;
  }

  ///@warning by default, this does not store the current state to disk.
  virtual void close(bool doStore = false) {
    if(!m_currentOpenPath.isEmpty()) {
    }
    m_currentOpenPath = QString();
    
    if(doStore)
      store();
    
    if(m_file)
      m_file->close();
    delete m_file;
    m_file = 0;
    
    if(m_dynamicFile)
      m_dynamicFile->close();
    delete m_dynamicFile;
    m_dynamicFile = 0;
    
    delete[] m_firstBucketForHash;
    
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
      
      Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[bucket];
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
    
    Bucket<Item, ItemRequest, DynamicData>* bucketPtr = bucketForIndex(bucket);
    
    AllItemsReachableVisitor visitor(this);
    return bucketPtr->visitAllItems(visitor);
  }
  
  inline void initializeBucket(unsigned int bucketNumber) const {
    Q_ASSERT(bucketNumber);
    if(!m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber] = new Bucket<Item, ItemRequest, DynamicData>();
      if(m_file) {
        if(m_file->open( QFile::ReadOnly )) {
          m_fastBuckets[bucketNumber]->initialize(m_file, BucketStartOffset + (bucketNumber-1) * Bucket<Item, ItemRequest, DynamicData>::DataSize);
          m_file->close();
        }else{
          kWarning() << "cannot open repository-file for reading";
        }
      } else
        m_fastBuckets[bucketNumber]->initialize();
    }
  }
  //m_file must be opened
  void storeBucket(unsigned int bucketNumber) const {
    if(m_file && m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber]->store(m_file, BucketStartOffset + (bucketNumber-1) * Bucket<Item, ItemRequest, DynamicData>::DataSize);
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
  ///with hash @param hash @meat each other.
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

  bool m_metaDataChanged;
  mutable QMutex m_mutex;
  QString m_repositoryName;
  unsigned int m_size;
  mutable uint m_currentBucket;
  //List of buckets that have free space available that can be assigned. Sorted by size: Smallest space first
  QVector<uint> m_freeSpaceBuckets;
  uint m_freeSpaceBucketsSize; //for speedup
  mutable QVector<Bucket<Item, ItemRequest, DynamicData>* > m_buckets;
  mutable Bucket<Item, ItemRequest, DynamicData>** m_fastBuckets; //For faster access
  mutable uint m_bucketCount;
  uint m_statBucketHashClashes, m_statItemCount;
  unsigned int m_bucketHashSize;
  //Maps hash-values modulo 1<<bucketHashSizeBits to the first bucket such a hash-value appears in
  short unsigned int* m_firstBucketForHash;
  
  QString m_currentOpenPath;
  ItemRepositoryRegistry* m_registry;
  //File that contains the buckets
  QFile* m_file;
  //File that contains more dynamic data, like the list of buckets with deleted items
  QFile* m_dynamicFile;
  uint m_repositoryVersion;
  bool m_unloadingEnabled;
};

}

#endif
