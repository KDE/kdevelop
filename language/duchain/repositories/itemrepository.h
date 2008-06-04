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

namespace Repositories {

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
   * */
  
  /**
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
class Bucket {
  public:
    Bucket() : m_size(0), m_available(0), m_data(0) {
    }
    ~Bucket() {
      delete[] m_data;
    }

    ///Size must be smaller max. 2<<16
    void initialize(unsigned int size) {
      Q_ASSERT(size <= 2<<16);
      if(!m_data) {
        m_size =(unsigned int) size;
        m_available = m_size;
        m_data = new char[size];
        //The bigger we make the map, the lower the probability of a clash(and thus bad performance). However it increases memory usage.
        m_objectMap.resize((m_size / ItemRequest::AverageSize) + 1);
        m_objectMap.fill(0);
      }
    }

    //Tries to get the index within this bucket, or returns zero.
    unsigned short index(const ItemRequest& request) {
      unsigned short localHash = request.hash() % m_objectMap.size();
      unsigned short index = m_objectMap[localHash];
      unsigned short insertedAt = 0;

      //Walk the chain of items with the same local hash
      while(index && followerIndex(index) && !(request.equals(itemFromIndex(index))))
        index = followerIndex(index);

      if(index && request.equals(itemFromIndex(index)))
        return index; //We have found the item

      unsigned int totalSize = request.itemSize() + 2;
      if(totalSize > m_available)
        return 0; //Not enough space for the item

      //We have to insert the item
      insertedAt = m_size - m_available;
      
      insertedAt += 2; //Room for the prepended follower-index
      if(index)
        setFollowerIndex(index, insertedAt);
      setFollowerIndex(insertedAt, 0);
      
      request.createItem((Item*)(m_data + insertedAt));
      m_available -= totalSize;

      if(m_objectMap[localHash] == 0)
        m_objectMap[localHash] = insertedAt;
      return insertedAt;
    }
    
    const Item& itemFromIndex(unsigned short index) const {
      Q_ASSERT(index < m_size);
      return *(Item*)(m_data+index);
    }
    
    const Item* itemFromIndex(unsigned short index) {
      return (Item*)(m_data+index);
    }
    
  private:
    ///@param index the index of an item @return The index of the next item in the chain of items with a same local hash, or zero
    unsigned short followerIndex(unsigned short index) {
      Q_ASSERT(index >= 2);
      return *((unsigned short*)(m_data+(index-2)));
    }

    void setFollowerIndex(unsigned short index, unsigned short follower) {
      Q_ASSERT(index >= 2);
      *((unsigned short*)(m_data+(index-2))) = follower;
    }
    unsigned int m_size, m_available;
    char* m_data; //Structure of the data: <Position of next item with same hash modulo m_size>(2 byte), <Item>(item.size() byte)
    QVector<short unsigned int> m_objectMap; //Points to the first object in m_data with (hash % m_size) == index. Points to the item itself, so substract 1 to get the pointer to the next item with same local hash.
};

///@param Item @see ExampleItem
///This class is thread-safe
template<class Item, class ItemRequest>
class ItemRepository {
  public:
    ///@param size The total size in bytes of this repository
    ///@param bucketSize the size of each bucket that makes up the repository. Must be max. 2<<16(the count of items an unsigned short can hold)
  ItemRepository(QString repositoryName, unsigned int size, unsigned int bucketSize) : m_repositoryName(repositoryName), m_size(size), m_bucketSize(bucketSize) {
    m_buckets.resize(m_size / m_bucketSize );
    m_buckets.fill(0);
    Q_ASSERT(m_buckets.size() <= (2<<16)); //Must fit into unsigned short
  }
  ~ItemRepository() {
    typedef Bucket<Item, ItemRequest> B;
    foreach(B* bucket, m_buckets)
      delete bucket;
  }

  ///Returns the index for the given item. If the item is not in the repository yet, it is inserted.
  ///The index can never be zero. Zero is reserved for your own usage as invalid
  unsigned int index(const ItemRequest& request) {
    QMutexLocker lock(&m_mutex);
    
    unsigned int bucketNumber = request.hash() % m_buckets.size();
    unsigned int bucketCount = m_buckets.size();
    for(unsigned short bucket = 0; bucket < bucketCount; ++bucket) { //Try 
      unsigned int testBucket = (bucketNumber + bucket) % bucketCount;
      initializeBucket(testBucket);
      unsigned short indexInBucket = m_buckets[ testBucket ]->index(request);
      if(indexInBucket)
        return (testBucket << 16) + indexInBucket; //Combine the index in the bucket, and the bucker number into one index
    }
    kWarning() << "Found no bucket for an item in" << m_repositoryName;
    return 0;
  }

  ///@param index The index. It must be valid(match an existing item), and nonzero.
  const Item* itemFromIndex(unsigned int index) const {
    Q_ASSERT(index);
    
    QMutexLocker lock(&m_mutex);
    unsigned short bucket = (index >> 16);
    initializeBucket(bucket);
    unsigned short indexInBucket = index & 0xffff;
    return m_buckets[bucket]->itemFromIndex(indexInBucket);
  }

  private:

  void initializeBucket(unsigned int bucketNumber) const {
    if(!m_buckets[bucketNumber])
      m_buckets[bucketNumber] = new Bucket<Item, ItemRequest>();
    m_buckets[bucketNumber]->initialize(m_bucketSize);
  }

  mutable QMutex m_mutex;
  QString m_repositoryName;
  unsigned int m_size;
  unsigned int m_bucketSize;
  mutable QVector<Bucket<Item, ItemRequest>* > m_buckets;
};

}

#endif
