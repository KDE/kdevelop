/* This file is part of KDevelop
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

#ifndef KDEVPLATFORM_PERSISTENTSETMAP_H
#define KDEVPLATFORM_PERSISTENTSETMAP_H

#include "appendedlist.h"
#include "../../util/embeddedfreetree.h"
#include "serialization/itemrepository.h"

namespace KDevelop {

template<class T>
struct IndexHasher {
  static uint hash(const T& t) {
    return t.index();
  }
};

template<class T>
struct HashHasher {
  static uint hash(const T& t) {
    return t.hash();
  }
};

// #define DEFINE_PERSISTENT_SET_MAP(key, data, hasher, ) \
//     typedef TemporaryDataManager<KDevVarLengthArray<type, 10> > temporaryHash ## container ## member ## Type; \
//     K_GLOBAL_STATIC_WITH_ARGS(temporaryHash ## container ## member ## Type, temporaryHash ## container ## member ## Static, ( #container "::" #member )) \
//     temporaryHash ## container ## member ## Type& temporaryHash ## container ## member() { \
//         return *temporaryHash ## container ## member ## Static; \
//     }


template<class Key, class Data, class Hasher>
class PersistentSetMapItem {
  public:
  static KDevelop::TemporaryDataManager<KDevVarLengthArray<Data, 10> >& temporaryHashPersistentSetMapItemdata() {
    static KDevelop::TemporaryDataManager<KDevVarLengthArray<Data, 10> > manager;
    return manager;
  }
    
  PersistentSetMapItem() : centralFreeItem(-1) {
    initializeAppendedLists();
  }
  PersistentSetMapItem(const PersistentSetMapItem& rhs) : key(rhs.key), centralFreeItem(rhs.centralFreeItem) {
    initializeAppendedLists();
    copyListsFrom(rhs);
  }
  
  ~PersistentSetMapItem() {
    freeAppendedLists();
  }
  
  unsigned int hash() const {
    //We only compare the data. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return Hasher::hash(key);
  }
  
  unsigned int itemSize() const {
    return dynamicSize();
  }
  
  uint classSize() const {
    return sizeof(*this);
  }
  
  Key key;
  int centralFreeItem;
  
  START_APPENDED_LISTS(PersistentSetMapItem);
  APPENDED_LIST_FIRST(PersistentSetMapItem, Data, data);
  END_APPENDED_LISTS(PersistentSetMapItem, data);
};

template<class Key, class Data, class Hasher>
class PersistentSetMapItemRequest {
  public:
  
  PersistentSetMapItemRequest(const PersistentSetMapItem<Key, Data, Hasher>& item) : m_item(item) {
  }
  enum {
    AverageSize = 30 //This should be the approximate average size of an Item
  };

  unsigned int hash() const {
    return m_item.hash();
  }
  
  uint itemSize() const {
      return m_item.itemSize();
  }

  void createItem(PersistentSetMapItem<Key, Data, Hasher>* item) const {
    item->initializeAppendedLists(false);
    item->key = m_item.key;
    item->centralFreeItem = m_item.centralFreeItem;
    item->copyListsFrom(m_item);
  }
  
  bool equals(const PersistentSetMapItem<Key, Data, Hasher>* item) const {
    return m_item.key == item->key;
  }
  
  const PersistentSetMapItem<Key, Data, Hasher>& m_item;
};

/**
 * This class allows easily implement a very efficient persistent map from a key, to a set of data items.
 * @param Key The key class, from which a set of Data items is mapped. It must be safely memory-copyable((no virtual functions etc),
 * @param Data The data class, of which a set will be stored. The data must be safely memory-copyable(no virtual functions etc),
 *             and it must be compatbiel with EmbeddedFreeTree
 * @param Handler Must be a handler that allows storing additional information into invalid items, @see util/embeddedfreetree.h
 * @param Hasher A hasher that extracts a hash-value from the key.
 * */
template<class Key, class Data, class Handler, class Hasher = HashHasher<Key> >
class PersistentSetMap {
  public:
    PersistentSetMap(QString name) : m_repository(name) {
    }
    void addItem(const Key& key, const Data& data);
    void removeItem(const Key& key, const Data& data);
    ///The returned list may contain "invalid" items, those have to be filtered out by the user.
    void items(const Key& key, uint& countTarget, const Data*& datasTarget) const;
  private:
    ItemRepository<PersistentSetMapItem<Key, Data, Hasher>, PersistentSetMapItemRequest<Key, Data, Hasher> > m_repository;
};

template<class Key, class Data, class Handler, class Hasher>
void PersistentSetMap<Key, Data, Handler, Hasher>::addItem(const Key& key, const Data& data)
{
  PersistentSetMapItem<Key, Data, Hasher> item;
  item.key = key;
  PersistentSetMapItemRequest<Key, Data, Hasher> request(item);
  
  uint index = m_repository.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentSetMapItem<Key, Data, Hasher>* oldItem = m_repository.itemFromIndex(index);
    
    EmbeddedTreeAlgorithms<Data, Handler> alg(oldItem->data(), oldItem->dataSize(), oldItem->centralFreeItem);
    
    if(alg.indexOf(data) != -1)
      return;
    
    QMutexLocker lock(m_repository.mutex());
    
    PersistentSetMapItem<Key, Data, Hasher>* editableItem = m_repository.dynamicItemFromIndex(index);
    
    EmbeddedTreeAddItem<Data, Handler> add(const_cast<Data*>(editableItem->data()), editableItem->dataSize(), editableItem->centralFreeItem, data);
    
    uint newSize = add.newItemCount();
    if(newSize != editableItem->dataSize()) {
      //We need to resize. Update and fill the new item, and delete the old item.
      item.datasList().resize(newSize);
      add.transferData(item.datasList().data(), newSize, &item.centralFreeItem);
      
      m_repository.deleteItem(index);
      Q_ASSERT(!m_repository.findIndex(request));
    }else{
      //We're fine, the item could be added to the existing list
      return;
    }
  }else{
    item.datasList().append(data);
  }

  //This inserts the changed item
  m_repository.index(request);
}

template<class Key, class Data, class Handler, class Hasher>
void PersistentSetMap<Key, Data, Handler, Hasher>::removeItem(const Key& key, const Data& data)
{
  PersistentSetMapItem<Key, Data, Hasher> item;
  item.key = key;
  PersistentSetMapItemRequest<Key, Data, Hasher> request(item);
  
  uint index = m_repository.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentSetMapItem<Key, Data, Hasher>* oldItem = m_repository.itemFromIndex(index);
    
    EmbeddedTreeAlgorithms<Data, Handler> alg(oldItem->data(), oldItem->dataSize(), oldItem->centralFreeItem);
    
    if(alg.indexOf(data) == -1)
      return;
    
    QMutexLocker lock(m_repository.mutex());
    
    PersistentSetMapItem<Key, Data, Hasher>* editableItem = m_repository.dynamicItemFromIndex(index);
    
    EmbeddedTreeRemoveItem<Data, Handler> remove(const_cast<Data*>(editableItem->data()), editableItem->dataSize(), editableItem->centralFreeItem, data);
    
    uint newSize = remove.newItemCount();
    if(newSize != editableItem->dataSize()) {
      //We need to resize. Update and fill the new item, and delete the old item.
      item.datasList().resize(newSize);
      remove.transferData(item.datasList().data(), newSize, &item.centralFreeItem);
      
      m_repository.deleteItem(index);
      Q_ASSERT(!m_repository.findIndex(request));
    }else{
      //We're fine, the item could be added to the existing list
      return;
    }
  }

  //This inserts the changed item
  if(item.dataSize())
    m_repository.index(request);
}

template<class Key, class Data, class Handler, class Hasher>
void PersistentSetMap<Key, Data, Handler, Hasher>::items(const Key& key, uint& countTarget, const Data*& datasTarget) const
{
  PersistentSetMapItem<Key, Data, Hasher> item;
  item.key = key;
  PersistentSetMapItemRequest<Key, Data, Hasher> request(item);
  
  uint index = m_repository.findIndex(item);
  
  if(index) {
    const PersistentSetMapItem<Key, Data, Hasher>* repositoryItem = m_repository.itemFromIndex(index);
    countTarget = repositoryItem->dataSize();
    datasTarget = repositoryItem->data();
  }else{
    countTarget = 0;
    datasTarget = 0;
  }
}

}

#endif
