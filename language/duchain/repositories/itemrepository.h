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


class KDEVPLATFORMLANGUAGE_EXPORT AbstractItemRepository {
  public:
    virtual ~AbstractItemRepository();
    ///@param path is supposed to be a shared directory-name that the item-repository is to be loaded from
    ///@param clear will be true if the old repository should be discarded and a new one started
    ///If this returns false, that indicates that loading failed. In that case, all repositories will be discarded.
    virtual bool open(const QString& path, bool clear) = 0;
    virtual void close() = 0;
    virtual void store() = 0;
};

/**
 * Manages a set of item-repositores and allows loading/storing them all at once from/to disk.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ItemRepositoryRegistry {
  public:
    ItemRepositoryRegistry(QString openPath = QString(), KLockFile::Ptr lock = KLockFile::Ptr());
    ~ItemRepositoryRegistry();
    
    ///Path is supposed to be a shared directory-name that the item-repositories are to be loaded from
    ///@param clear Whether a fresh start should be done, and all repositories cleared
    ///If this returns false, loading has failed, and all repositories have been discarded.
    bool open(const QString& path, bool clear = false, KLockFile::Ptr lock = KLockFile::Ptr());
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
    QString m_path;
    QList<AbstractItemRepository*> m_repositories;
    QMap<QString, QAtomicInt*> m_customCounters;
    bool m_cleared;
    KLockFile::Ptr m_lock;
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
class KDEVPLATFORMLANGUAGE_EXPORT Bucket {
  enum {
    AdditionalSpacePerItem = DynamicData::Size + 2,
    NextBucketHashSize = 500 //Affects the average count of bucket-chains that need to be walked in ItemRepository::index
  };
  public:
    enum {
      ObjectMapSize = (ItemRepositoryBucketSize / ItemRequest::AverageSize) + 1,
      ///@todo Change these to use sizes instead of counts
      MaxFreeItemsForHide = 4, //When less then this count of free items in one buckets is reached, the bucket is removed from the global list of buckets with free items
      MinFreeItemsForReuse = 13, //When this count of free items in one bucket is reached, consider re-assigning them to new requests
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
    unsigned short index(const ItemRequest& request, const DynamicData* dynamic) {
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

      m_changed = true;

      unsigned int totalSize = request.itemSize() + AdditionalSpacePerItem;
      
      //If this triggers, your item is too big.
      Q_ASSERT(totalSize < ItemRepositoryBucketSize);
      
      if(totalSize > m_available) {
        //Try finding the smallest freed item that can hold the data
        unsigned short currentIndex = m_largestFreeItem;
        unsigned short previousIndex = 0;
        while(currentIndex && freeSize(currentIndex) >= (totalSize-AdditionalSpacePerItem)) {
          unsigned short follower = followerIndex(currentIndex);
          if(follower && freeSize(follower) >= (totalSize-AdditionalSpacePerItem)) {
            //The item also fits into the smaller follower, so use that one
            previousIndex = currentIndex;
            currentIndex = follower;
          }else{
            //The item fits into currentIndex, but not into the follower. So use currentIndex
            break;
          }
        }
        
        if(!currentIndex || freeSize(currentIndex) < (totalSize-AdditionalSpacePerItem))
          return 0;
        
        if(previousIndex)
          setFollowerIndex(previousIndex, followerIndex(currentIndex));
        else
          m_largestFreeItem = followerIndex(currentIndex);
        
        insertedAt = currentIndex;
        --m_freeItemCount;
        Q_ASSERT((bool)m_freeItemCount == (bool)m_largestFreeItem);
      }else{
        //We have to insert the item
        insertedAt = ItemRepositoryBucketSize - m_available;
        
        insertedAt += AdditionalSpacePerItem; //Room for the prepended follower-index

        m_available -= totalSize;
      }
      
      DynamicData::initialize(m_data + insertedAt - AdditionalSpacePerItem);
      
      if(index)
        setFollowerIndex(index, insertedAt);
      setFollowerIndex(insertedAt, 0);
      
      if(m_objectMap[localHash] == 0)
        m_objectMap[localHash] = insertedAt;
      
      char* borderBehind = m_data + insertedAt + (totalSize-AdditionalSpacePerItem);
      
      quint64 oldValueBehind = 0;
      if(m_available >= 8) {
        oldValueBehind = *(quint64*)borderBehind;
        *((quint64*)borderBehind) = 0xfafafafafafafafaLLU;
      }
      
      //Last thing we do, because createItem may recursively do even more transformation of the repository
      request.createItem((Item*)(m_data + insertedAt));
      
      if(m_available >= 8) {
        //If this assertion triggers, then the item writes a bigger range than it advertised in 
        Q_ASSERT(*((quint64*)borderBehind) == 0xfafafafafafafafaLLU);
        *((quint64*)borderBehind) = oldValueBehind;
      }
      
      Q_ASSERT(itemFromIndex(insertedAt)->hash() == request.hash());
      Q_ASSERT(itemFromIndex(insertedAt)->itemSize() == request.itemSize());
      
      if(dynamic)
        itemFromIndex(insertedAt, dynamic);
      
      return insertedAt;
    }
    
    ///@param modulo Returns whether this bucket contains an item with (hash % modulo) == (item.hash % modulo)
    ///              The default-parameter is the size of the next-bucket hash that is used by setNextBucketForHash and nextBucketForHash
    bool hasClashingItem(uint hash, uint modulo = NextBucketHashSize) {
      m_lastUsed = 0;
      
      uint hashMod = hash % modulo;
      unsigned short localHash = hash % m_objectMapSize;
      unsigned short currentIndex = m_objectMap[localHash];
      
      if(currentIndex == 0)
        return false;

      while(currentIndex) {
        if(itemFromIndex(currentIndex)->hash() % modulo == hashMod)
          return true; //Clash
        currentIndex = followerIndex(currentIndex);
      }
      return false;
    }
    
    void deleteItem(unsigned short index, unsigned int hash) {
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
      
      if(!previousIndex)
        //The item was directly in the object map
        m_objectMap[localHash] = followerIndex(index);
      else
        setFollowerIndex(previousIndex, followerIndex(index));
      
      setFreeSize(index, size);
      
      //Insert the free item into the chain opened by m_largestFreeItem
      currentIndex = m_largestFreeItem;
      previousIndex = 0;
      
      while(currentIndex && *(unsigned short*)(m_data + currentIndex) > size) {
        previousIndex = currentIndex;
        currentIndex = followerIndex(currentIndex);
      }
      
      if(previousIndex)
        //This item is larger than all already registered free items, or there are none.
        setFollowerIndex(previousIndex, index);
      else
        m_largestFreeItem = index;

      setFollowerIndex(index, currentIndex);
      
      ++m_freeItemCount;
      Q_ASSERT((bool)m_freeItemCount == (bool)m_largestFreeItem);
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

    uint usedMemory() const {
      return ItemRepositoryBucketSize - m_available;
    }
    
    template<class Visitor>
    bool visitAllItems(Visitor& visitor) const {
      m_lastUsed = 0;
      for(int a = 0; a < m_objectMapSize; ++a) {
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
    
    short unsigned int largestFreeItemSize() const {
      if(m_largestFreeItem)
        return freeSize(m_largestFreeItem);
      else
        return 0;
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
    
  private:
    
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
struct Locker<true> : public QMutexLocker {
  Locker(QMutex* mutex) : QMutexLocker(mutex) {
  }
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
template<class Item, class ItemRequest, class DynamicData = NoDynamicData, bool threadSafe = true, unsigned int bucketHashSize = 524288>
class KDEVPLATFORMLANGUAGE_EXPORT ItemRepository : public AbstractItemRepository {

  typedef Locker<threadSafe> ThisLocker;
  
  enum {
    ItemRepositoryVersion = 7,
    BucketStartOffset = sizeof(uint) * 7 + sizeof(short unsigned int) * bucketHashSize //Position in the data where the bucket array starts
  };
  
  public:
  ///@param registry May be zero, then the repository will not be registered at all. Else, the repository will register itself to that registry.
  ItemRepository(QString repositoryName, ItemRepositoryRegistry* registry  = &globalItemRepositoryRegistry(), uint repositoryVersion = 1) : m_mutex(QMutex::Recursive), m_repositoryName(repositoryName), m_registry(registry), m_file(0), m_dynamicFile(0), m_repositoryVersion(repositoryVersion) {
    m_unloadingEnabled = true;
    m_metaDataChanged = true;
    m_buckets.resize(10);
    m_buckets.fill(0);
    m_freeSpaceBucketsSize = 0;
    m_fastBuckets = m_buckets.data();
    m_bucketCount = m_buckets.size();
    
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
    
    short unsigned int* bucketHashPosition = m_firstBucketForHash + ((hash * 1234271) % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;
    short unsigned int previousPreviousBucketNumber = 0;
    
    short unsigned int size = request.itemSize();
    
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
        if(!pickedBucketInChain && bucketPtr->largestFreeItemSize() >= size) {
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
    
    m_metaDataChanged = true; ///@todo Better tracking of whether the data has changed
    
    if(!pickedBucketInChain && useBucket == m_currentBucket) {
      //Try finding an existing bucket with deleted space to store the data into
      for(uint a = 0; a < m_freeSpaceBucketsSize; ++a) {
        Bucket<Item, ItemRequest, DynamicData>* bucketPtr = bucketForIndex(m_freeSpaceBuckets[a]);
        if(size <= bucketPtr->largestFreeItemSize()) {
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
      unsigned short indexInBucket = bucketPtr->index(request, dynamic);
      
      if(indexInBucket) {
        if(!(*bucketHashPosition))
          (*bucketHashPosition) = useBucket;
        
        ++m_statItemCount;

        //Set the follower pointer in the earlier bucket for the hash
        if(!pickedBucketInChain && previousBucketNumber && previousBucketNumber != useBucket) {
          //Should happen rarely
          ++m_statBucketHashClashes;
          
          //If the used bucket already has previousBucketNumber as a follower, insert it instead of previousBucketNumber,
          //so we don't create a loop
          bool replacePreviousWithUsed = false;
          
          {
            uint checkBucket = useBucket;
            while(checkBucket) {
              if(checkBucket == previousBucketNumber) {
                replacePreviousWithUsed = true;
                break;
              }
              checkBucket = bucketForIndex(checkBucket)->nextBucketForHash(request.hash());
            }
          }
          
          if(!replacePreviousWithUsed)
            m_buckets[previousBucketNumber]->setNextBucketForHash(request.hash(), useBucket);
          else if(previousPreviousBucketNumber) {
            bucketForIndex(previousPreviousBucketNumber)->setNextBucketForHash(request.hash(), useBucket);
          } else {
            //useBucket needs to be the first bucket in the global bucket hash for this hash-value
            Q_ASSERT(*bucketHashPosition == previousBucketNumber);
            *bucketHashPosition = useBucket;
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
    
    short unsigned int* bucketHashPosition = m_firstBucketForHash + ((hash * 1234271) % bucketHashSize);
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
  
  ///Deletes the item from the repository. It is crucial that the given hash and size are correct.
  void deleteItem(unsigned int index) {
    ThisLocker lock(&m_mutex);
    
    m_metaDataChanged = true;
    
    unsigned short bucket = (index >> 16);
    Q_ASSERT(bucket); //We don't use zero buckets

    unsigned int hash = itemFromIndex(index)->hash();
    
    short unsigned int* bucketHashPosition = m_firstBucketForHash + ((hash * 1234271) % bucketHashSize);
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
    
    Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[bucket];
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    
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
    if(previousBucketNumber == 0) {
      //The item is directly in the m_firstBucketForHash hash
      //Put the next item in the nextBucketForHash chain into m_firstBucketForHash that has a hash clashing in that array.
      Q_ASSERT(*bucketHashPosition == bucket);
      
      while(!bucketPtr->hasClashingItem(hash, bucketHashSize)) 
      {
        unsigned short next = bucketPtr->nextBucketForHash(hash);
        *bucketHashPosition = next;
        
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
      if(!bucketPtr->hasClashingItem(hash)) {
        previousBucketPtr->setNextBucketForHash(hash, bucketPtr->nextBucketForHash(hash));
        Q_ASSERT(bucketPtr->nextBucketForHash(hash) != previousBucketNumber);
      }
    }
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
  void visitAllItems(Visitor& visitor, bool /*onlyInMemory*/ = false) const {
    ThisLocker lock(&m_mutex);
    for(uint a = 0; a < m_bucketCount; ++a) {
      if(m_buckets[a])
        if(!m_buckets[a]->visitAllItems(visitor))
          return;
    }
  }
  
  ///Synchronizes the state on disk to the one in memory, and does some memory-management.
  ///Should be called on a regular basis. Can be called centrally from the global item repository registry.
  virtual void store() {
    ThisLocker lock(&m_mutex);
    if(m_file) {
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
        uint itemRepositoryVersion  = ItemRepositoryVersion;
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
    
  Bucket<Item, ItemRequest, DynamicData>* bucketForIndex(short unsigned int index) {
    Bucket<Item, ItemRequest, DynamicData>* bucketPtr = m_fastBuckets[index];
    if(!bucketPtr) {
      initializeBucket(index);
      bucketPtr = m_fastBuckets[index];
    }
    return bucketPtr;
  }
    
  virtual bool open(const QString& path, bool clear) {
    close();
    m_currentOpenPath = path;
    kDebug() << "opening repository" << m_repositoryName << "at" << path;
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
    if(clear || m_file->size() == 0) {
      
      m_file->resize(0);
      m_file->write((char*)&m_repositoryVersion, sizeof(uint));
      uint hashSize = bucketHashSize;
      m_file->write((char*)&hashSize, sizeof(uint));
      uint itemRepositoryVersion  = ItemRepositoryVersion;
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
      
      if(storedVersion != m_repositoryVersion || hashSize != bucketHashSize || itemRepositoryVersion != ItemRepositoryVersion) {
        kDebug() << "repository" << m_repositoryName << "version mismatch in" << m_file->fileName() << ", stored: version " << storedVersion << "hashsize" << hashSize << "repository-version" << itemRepositoryVersion << " current: version" << m_repositoryVersion << "hashsize" << bucketHashSize << "repository-version" << ItemRepositoryVersion;
        delete m_file;
        m_file = 0;
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
    
    m_fastBuckets = m_buckets.data();
    m_bucketCount = m_buckets.size();
    return true;
  }
  
  virtual void close() {
    if(!m_currentOpenPath.isEmpty()) {
    }
    m_currentOpenPath = QString();
    
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

  inline void initializeBucket(unsigned int bucketNumber) const {
    Q_ASSERT(bucketNumber);
    if(!m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber] = new Bucket<Item, ItemRequest, DynamicData>();
      if(m_file)
        m_fastBuckets[bucketNumber]->initialize(m_file, BucketStartOffset + (bucketNumber-1) * Bucket<Item, ItemRequest, DynamicData>::DataSize);
      else
        m_fastBuckets[bucketNumber]->initialize();
    }
  }
  
  void storeBucket(unsigned int bucketNumber) const {
    if(m_file && m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber]->store(m_file, BucketStartOffset + (bucketNumber-1) * Bucket<Item, ItemRequest, DynamicData>::DataSize);
    }
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
