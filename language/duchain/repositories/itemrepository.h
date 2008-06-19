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

#include <QString>
#include <QVector>
#include <QByteArray>
#include <QMutex>
#include <kdebug.h>
#include <hash_map>
#include "../languageexport.h"

namespace std {
  using namespace __gnu_cxx;
}

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

  ///This is the actual data that is stored in the repository. All the data that is not directly in the class-body,
  ///like the text of a string, can be stored behind the item in the same memory region. The only important thing is
  ///that the Request item(@see ExampleItemRequest) correctly advertises the space needed by this item.
class ExampleItem {
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
  void createItem(ExampleItem* /*item*/) const {
  }
  
  //Should return whether the here requested item equals the given item
  bool equals(const ExampleItem* /*item*/) const {
    return false;
  }
};

template<class Item, class ItemRequest>
class KDEVPLATFORMLANGUAGE_EXPORT Bucket {
  public:
    Bucket() : m_available(0), m_data(0), m_objectMap(0), m_objectMapSize(0) {
    }
    ~Bucket() {
      delete[] m_data;
      delete[] m_objectMap;
    }

    void initialize() {
      if(!m_data) {
        m_available = ItemRepositoryBucketSize;
        m_data = new char[ItemRepositoryBucketSize];
        //The bigger we make the map, the lower the probability of a clash(and thus bad performance). However it increases memory usage.
        m_objectMapSize = (ItemRepositoryBucketSize / ItemRequest::AverageSize) + 1;
        m_objectMap = new short unsigned int[m_objectMapSize];
        memset(m_objectMap, 0, m_objectMapSize * sizeof(short unsigned int));
      }
    }

    //Tries to find the index this item has in this bucket, or returns zero if the item isn't there yet.
    unsigned short findIndex(const ItemRequest& request) const {
      unsigned short localHash = request.hash() % m_objectMapSize;
      unsigned short index = m_objectMap[localHash];

      unsigned short follower = 0;
      //Walk the chain of items with the same local hash
      while(index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
        index = follower;

      if(index && request.equals(itemFromIndex(index)))
        return index; //We have found the item

      return 0;
    }
    
    //Tries to get the index within this bucket, or returns zero. Will put the item into the bucket if there is room.
    //Created indices will never begin with 0xffff____, so you can use that index-range for own purposes.
    unsigned short index(const ItemRequest& request) {
      unsigned short localHash = request.hash() % m_objectMapSize;
      unsigned short index = m_objectMap[localHash];
      unsigned short insertedAt = 0;

      unsigned short follower = 0;
      //Walk the chain of items with the same local hash
      while(index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
        index = follower;

      if(index && request.equals(itemFromIndex(index)))
        return index; //We have found the item

      unsigned int totalSize = request.itemSize() + 2;
      if(totalSize > m_available)
        return 0; //Not enough space for the item

      //We have to insert the item
      insertedAt = ItemRepositoryBucketSize - m_available;
      
      insertedAt += 2; //Room for the prepended follower-index
      if(index)
        setFollowerIndex(index, insertedAt);
      setFollowerIndex(insertedAt, 0);
      
      m_available -= totalSize;

      if(m_objectMap[localHash] == 0)
        m_objectMap[localHash] = insertedAt;
      
      //Last thing we do, because createItem may recursively do even more transformation of the repository
      request.createItem((Item*)(m_data + insertedAt));
      
      return insertedAt;
    }
    
    inline const Item* itemFromIndex(unsigned short index) const {
      return (Item*)(m_data+index);
    }
    
    uint usedMemory() const {
      return ItemRepositoryBucketSize - m_available;
    }
    
    template<class Visitor>
    bool visitAllItems(Visitor& visitor) const {
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
    
    unsigned short nextBucketForHash(uint hash) {
      std::hash_map<uint, unsigned short>::const_iterator it = m_nextBucketForHash.find(hash);
      if(it != m_nextBucketForHash.end())
        return (*it).second;
      else
        return 0;
    }
    
    void setNextBucketForHash(unsigned int hash, unsigned short bucket) {
      m_nextBucketForHash.insert(std::make_pair(hash, bucket));
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
    unsigned int m_available;
    char* m_data; //Structure of the data: <Position of next item with same hash modulo ItemRepositoryBucketSize>(2 byte), <Item>(item.size() byte)
    short unsigned int* m_objectMap; //Points to the first object in m_data with (hash % m_objectMapSize) == index. Points to the item itself, so substract 1 to get the pointer to the next item with same local hash.
    uint m_objectMapSize;
    
    ///@todo eventually replace this with an efficient structure that can also easily be stored to disk
    std::hash_map<uint, unsigned short> m_nextBucketForHash;
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

///@param Item @see ExampleItem
///@param threadSafe Whether class access should be thread-safe
template<class Item, class ItemRequest, bool threadSafe = true, unsigned int bucketHashSize = 524288>
class KDEVPLATFORMLANGUAGE_EXPORT ItemRepository {

  typedef Locker<threadSafe> ThisLocker;
    
  public:
  ItemRepository(QString repositoryName) : m_mutex(QMutex::Recursive), m_repositoryName(repositoryName) {
    m_buckets.resize(10);
    m_buckets.fill(0);
    m_fastBuckets = m_buckets.data();
    m_bucketCount = m_buckets.size();
    
    m_firstBucketForHash = new short unsigned int[bucketHashSize];
    memset(m_firstBucketForHash, 0, bucketHashSize * sizeof(short unsigned int));
    
    m_statBucketHashClashes = m_statItemCount = 0;
    m_currentBucket = 1; //Skip the first bucket, we won't use it so we have the zero indices for special purposes
  }
  
  ~ItemRepository() {
    //kDebug() << m_repositoryName << ":" << statistics();

    typedef Bucket<Item, ItemRequest> B;
    foreach(B* bucket, m_buckets)
      delete bucket;
    
    delete[] m_firstBucketForHash;
  }

  ///Returns the index for the given item. If the item is not in the repository yet, it is inserted.
  ///The index can never be zero. Zero is reserved for your own usage as invalid
  unsigned int index(const ItemRequest& request) {
    
    ThisLocker lock(&m_mutex);
    
    unsigned int hash = request.hash();
    
    short unsigned int* bucketHashPosition = m_firstBucketForHash + (hash % bucketHashSize);
    short unsigned int previousBucketNumber = *bucketHashPosition;
    
    while(previousBucketNumber) {
      //We have a bucket that contains an item with the given hash % bucketHashSize, so check if the item is already there
      
      Bucket<Item, ItemRequest>* bucketPtr = m_fastBuckets[previousBucketNumber];
      if(!bucketPtr) {
        initializeBucket(previousBucketNumber);
        bucketPtr = m_fastBuckets[previousBucketNumber];
      }
      
      unsigned short indexInBucket = bucketPtr->findIndex(request);
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
    
    //The item isn't in the repository set, find a new bucket for it
    
    while(1) {
      if(m_currentBucket >= m_bucketCount) {
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
      Bucket<Item, ItemRequest>* bucketPtr = m_fastBuckets[m_currentBucket];
      if(!bucketPtr) {
        initializeBucket(m_currentBucket);
        bucketPtr = m_fastBuckets[m_currentBucket];
      }
      unsigned short indexInBucket = bucketPtr->index(request);
      
      if(indexInBucket) {
        if(!(*bucketHashPosition))
          (*bucketHashPosition) = m_currentBucket;
        
        ++m_statItemCount;

        //Set the follower pointer in the earlier bucket for the hash
        if(previousBucketNumber && previousBucketNumber != m_currentBucket) {
          //Should happen rarely
          ++m_statBucketHashClashes;
          m_buckets[previousBucketNumber]->setNextBucketForHash(request.hash(), m_currentBucket);
        }
        
        return (m_currentBucket << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
      }else{
        ++m_currentBucket;
      }
    }
    //We haven't found a bucket that already contains the item, so append it to the current bucket
    
    kWarning() << "Found no bucket for an item in" << m_repositoryName;
    return 0;
  }

  ///@param index The index. It must be valid(match an existing item), and nonzero.
  const Item* itemFromIndex(unsigned int index) const {
    Q_ASSERT(index);
    
    ThisLocker lock(&m_mutex);
    
    unsigned short bucket = (index >> 16);
    
    const Bucket<Item, ItemRequest>* bucketPtr = m_fastBuckets[bucket];
    if(!bucketPtr) {
      initializeBucket(bucket);
      bucketPtr = m_fastBuckets[bucket];
    }
    unsigned short indexInBucket = index & 0xffff;
    return bucketPtr->itemFromIndex(indexInBucket);
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

  private:

  inline void initializeBucket(unsigned int bucketNumber) const {
    if(!m_fastBuckets[bucketNumber]) {
      m_fastBuckets[bucketNumber] = new Bucket<Item, ItemRequest>();
      m_fastBuckets[bucketNumber]->initialize();
    }
  }

  mutable QMutex m_mutex;
  QString m_repositoryName;
  unsigned int m_size;
  mutable uint m_currentBucket;
  mutable QVector<Bucket<Item, ItemRequest>* > m_buckets;
  mutable Bucket<Item, ItemRequest>** m_fastBuckets; //For faster access
  mutable uint m_bucketCount;
  uint m_statBucketHashClashes, m_statItemCount;
  //Maps hash-values modulo 1<<bucketHashSizeBits to the first bucket such a hash-value appears in
  short unsigned int* m_firstBucketForHash;
};

}

#endif
