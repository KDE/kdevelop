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

#include "codemodel.h"

#include <QHash>
#include <QVector>

#include "appendedlist.h"
#include <serialization/itemrepository.h>
#include "identifier.h"
#include <serialization/indexedstring.h>
#include <serialization/referencecounting.h>
#include <util/embeddedfreetree.h>

#define ifDebug(x)

namespace KDevelop {

class CodeModelItemHandler {
    public:
    static int leftChild(const CodeModelItem& m_data) {
        return (int)m_data.referenceCount;
    }
    static void setLeftChild(CodeModelItem& m_data, int child) {
        m_data.referenceCount = (uint)child;
    }
    static int rightChild(const CodeModelItem& m_data) {
        return (int)m_data.uKind;
    }
    static void setRightChild(CodeModelItem& m_data, int child) {
        m_data.uKind = (uint)child;
    }
    //Copies this item into the given one
    static void copyTo(const CodeModelItem& m_data, CodeModelItem& data) {
      data = m_data;
    }
    
    static void createFreeItem(CodeModelItem& data) {
        data = CodeModelItem();
        data.referenceCount = (uint)-1;
        data.uKind = (uint)-1;
    }
    
    static bool isFree(const CodeModelItem& m_data) {
        return !m_data.id.isValid();
    }

    static const CodeModelItem& data(const CodeModelItem& m_data) {
      return m_data;
    }
    
    static bool equals(const CodeModelItem& m_data, const CodeModelItem& rhs) {
      return m_data.id == rhs.id;
    }
};


DEFINE_LIST_MEMBER_HASH(CodeModelRepositoryItem, items, CodeModelItem)

class CodeModelRepositoryItem {
  public:
  CodeModelRepositoryItem() : centralFreeItem(-1) {
    initializeAppendedLists();
  }
  CodeModelRepositoryItem(const CodeModelRepositoryItem& rhs, bool dynamic = true) : file(rhs.file), centralFreeItem(rhs.centralFreeItem) {
    initializeAppendedLists(dynamic);
    copyListsFrom(rhs);
  }
  
  ~CodeModelRepositoryItem() {
    freeAppendedLists();
  }
  
  unsigned int hash() const {
    //We only compare the declaration. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return file.index();
  }
  
  uint itemSize() const {
    return dynamicSize();
  }
  
  uint classSize() const {
    return sizeof(CodeModelRepositoryItem);
  }
  
  IndexedString file;
  int centralFreeItem;
  
  START_APPENDED_LISTS(CodeModelRepositoryItem);
  APPENDED_LIST_FIRST(CodeModelRepositoryItem, CodeModelItem, items);
  END_APPENDED_LISTS(CodeModelRepositoryItem, items);
};

class CodeModelRequestItem {
  public:
  
  CodeModelRequestItem(const CodeModelRepositoryItem& item) : m_item(item) {
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

  void createItem(CodeModelRepositoryItem* item) const {
    Q_ASSERT(shouldDoDUChainReferenceCounting(item));
    Q_ASSERT(shouldDoDUChainReferenceCounting(((char*)item) + (itemSize()-1)));
    new (item) CodeModelRepositoryItem(m_item, false);
  }
  
  static void destroy(CodeModelRepositoryItem* item, KDevelop::AbstractItemRepository&) {
    Q_ASSERT(shouldDoDUChainReferenceCounting(item));
//     Q_ASSERT(shouldDoDUChainReferenceCounting(((char*)item) + (itemSize()-1)));
    item->~CodeModelRepositoryItem();
  }
  
  static bool persistent(const CodeModelRepositoryItem* item) {
    Q_UNUSED(item);
    return true;
  }
  
  bool equals(const CodeModelRepositoryItem* item) const {
    return m_item.file == item->file;
  }
  
  const CodeModelRepositoryItem& m_item;
};


class CodeModelPrivate {
public:

  CodeModelPrivate() : m_repository("Code Model") {
  }
  //Maps declaration-ids to items
  ItemRepository<CodeModelRepositoryItem, CodeModelRequestItem> m_repository;
};

CodeModel::CodeModel() : d(new CodeModelPrivate())
{
}

CodeModel::~CodeModel()
{
  delete d;
}

void CodeModel::addItem(const IndexedString& file, const IndexedQualifiedIdentifier& id, CodeModelItem::Kind kind)
{
  ifDebug( kDebug() << "addItem" << file.str() << id.identifier().toString() << id.index; )
    
  if(!id.isValid())
    return;
  CodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);
  
  uint index = d->m_repository.findIndex(item);
  
  CodeModelItem newItem;
  newItem.id = id;
  newItem.kind = kind;
  newItem.referenceCount = 1;
  
  #ifdef TEST_REFERENCE_COUNTING_2
  {
  uint count = 0;
  const CodeModelItem* i;
  this->items(file, count, i);
  for(int a = 0; a < count; ++a)
    Q_ASSERT(i[a].id.hasReferenceCount());
  }
  #endif
  
  if(index) {
    const CodeModelRepositoryItem* oldItem = d->m_repository.itemFromIndex(index);
    EmbeddedTreeAlgorithms<CodeModelItem, CodeModelItemHandler> alg(oldItem->items(), oldItem->itemsSize(), oldItem->centralFreeItem);
    
    int listIndex = alg.indexOf(newItem);
    
    QMutexLocker lock(d->m_repository.mutex());
    
    DynamicItem<CodeModelRepositoryItem, true> editableItem = d->m_repository.dynamicItemFromIndex(index);
    CodeModelItem* items = const_cast<CodeModelItem*>(editableItem->items());
    
    if(listIndex != -1) {
      //Only update the reference-count
        ++items[listIndex].referenceCount;
        items[listIndex].kind = kind;
        return;
    }else{
      //Add the item to the list
      EmbeddedTreeAddItem<CodeModelItem, CodeModelItemHandler> add(items, editableItem->itemsSize(), editableItem->centralFreeItem, newItem);
      
      if(add.newItemCount() != editableItem->itemsSize()) {
        //The data needs to be transferred into a bigger list. That list is within "item".
        
        item.itemsList().resize(add.newItemCount());
        add.transferData(item.itemsList().data(), item.itemsList().size(), &item.centralFreeItem);
        
        d->m_repository.deleteItem(index);
      }else{
        //We're fine: The item fits into the existing list.
#ifdef TEST_REFERENCE_COUNTING_2
        {
        uint count = 0;
        const CodeModelItem* i;
        this->items(file, count, i);
        for(int a = 0; a < count; ++a)
          Q_ASSERT(i[a].id.hasReferenceCount());
        }
#endif
        return;
      }
    }
  }else{
    //We're creating a new index
    item.itemsList().append(newItem);
  }

#ifdef TEST_REFERENCE_COUNTING_2
  {
  uint count = 0;
  const CodeModelItem* i;
  this->items(file, count, i);
  for(int a = 0; a < count; ++a)
    Q_ASSERT(i[a].id.hasReferenceCount());
  }
#endif

  Q_ASSERT(!d->m_repository.findIndex(request));

  //This inserts the changed item
  volatile uint newIndex = d->m_repository.index(request);
  Q_UNUSED(newIndex);
  ifDebug( kDebug() << "new index" << newIndex; )
  
#ifdef TEST_REFERENCE_COUNTING_2
  {
  uint count = 0;
  const CodeModelItem* i;
  this->items(file, count, i);
  for(int a = 0; a < count; ++a)
    Q_ASSERT(i[a].id.hasReferenceCount());
  }
#endif
  
  Q_ASSERT(d->m_repository.findIndex(request));
}

void CodeModel::updateItem(const IndexedString& file, const IndexedQualifiedIdentifier& id, CodeModelItem::Kind kind)
{
  ifDebug( kDebug() << file.str() << id.identifier().toString() << kind; )
    
  if(!id.isValid())
    return;

#ifdef TEST_REFERENCE_COUNTING_2
  {
  uint count = 0;
  const CodeModelItem* i;
  this->items(file, count, i);
  for(int a = 0; a < count; ++a)
    Q_ASSERT(i[a].id.hasReferenceCount());
  }
#endif
  
  CodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);
  
  CodeModelItem newItem;
  newItem.id = id;
  newItem.kind = kind;
  newItem.referenceCount = 1;
  
  uint index = d->m_repository.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    QMutexLocker lock(d->m_repository.mutex());
    DynamicItem<CodeModelRepositoryItem, true> oldItem = d->m_repository.dynamicItemFromIndex(index);
    
    EmbeddedTreeAlgorithms<CodeModelItem, CodeModelItemHandler> alg(oldItem->items(), oldItem->itemsSize(), oldItem->centralFreeItem);
    int listIndex = alg.indexOf(newItem);
    Q_ASSERT(listIndex != -1);
    
    CodeModelItem* items = const_cast<CodeModelItem*>(oldItem->items());
    
    Q_ASSERT(items[listIndex].id == id);
    items[listIndex].kind = kind;
    
#ifdef TEST_REFERENCE_COUNTING_2
    {
    uint count = 0;
    const CodeModelItem* i;
    this->items(file, count, i);
    for(int a = 0; a < count; ++a)
      Q_ASSERT(i[a].id.hasReferenceCount());
    }
#endif
    
    return;
  }
  
  Q_ASSERT(0); //The updated item as not in the symbol table!
}

void CodeModel::removeItem(const IndexedString& file, const IndexedQualifiedIdentifier& id)
//void CodeModel::removeDeclaration(const QualifiedIdentifier& id, const IndexedDeclaration& declaration)
{
  if(!id.isValid())
    return;
#ifdef TEST_REFERENCE_COUNTING_2
  {
  uint count = 0;
  const CodeModelItem* i;
  this->items(file, count, i);
  for(int a = 0; a < count; ++a)
    Q_ASSERT(i[a].id.hasReferenceCount());
  }
#endif  
  ifDebug( kDebug() << "removeItem" << file.str() << id.identifier().toString(); )
  CodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);
  
  uint index = d->m_repository.findIndex(item);
  
  if(index) {
    
    CodeModelItem searchItem;
    searchItem.id = id;
    
    QMutexLocker lock(d->m_repository.mutex());
    DynamicItem<CodeModelRepositoryItem, true> oldItem = d->m_repository.dynamicItemFromIndex(index);
    
    EmbeddedTreeAlgorithms<CodeModelItem, CodeModelItemHandler> alg(oldItem->items(), oldItem->itemsSize(), oldItem->centralFreeItem);
    
    int listIndex = alg.indexOf(searchItem);
    if(listIndex == -1)
      return;
    
    CodeModelItem* items = const_cast<CodeModelItem*>(oldItem->items());
    
    --items[listIndex].referenceCount;
    
    if(oldItem->items()[listIndex].referenceCount)
      return; //Nothing to remove, there's still a reference-count left
    
    //We have reduced the reference-count to zero, so remove the item from the list
    
    EmbeddedTreeRemoveItem<CodeModelItem, CodeModelItemHandler> remove(items, oldItem->itemsSize(), oldItem->centralFreeItem, searchItem);
    
    uint newItemCount = remove.newItemCount();
    if(newItemCount != oldItem->itemsSize()) {
      if(newItemCount == 0) {
        //Has become empty, delete the item
        d->m_repository.deleteItem(index);
#ifdef TEST_REFERENCE_COUNTING_2
  {
  uint count = 0;
  const CodeModelItem* i;
  this->items(file, count, i);
  for(int a = 0; a < count; ++a)
    Q_ASSERT(i[a].id.hasReferenceCount());
  }
#endif
        
        return;
      }else{
        //Make smaller
        item.itemsList().resize(newItemCount);
        remove.transferData(item.itemsList().data(), item.itemsSize(), &item.centralFreeItem);
        
        //Delete the old list
        d->m_repository.deleteItem(index);
        //Add the new list
        d->m_repository.index(request);
#ifdef TEST_REFERENCE_COUNTING_2
  {
  uint count = 0;
  const CodeModelItem* i;
  this->items(file, count, i);
  for(int a = 0; a < count; ++a)
    Q_ASSERT(i[a].id.hasReferenceCount());
  }
#endif
        return;
      }
    }
  }
#ifdef TEST_REFERENCE_COUNTING_2
  {
  uint count = 0;
  const CodeModelItem* i;
  this->items(file, count, i);
  for(int a = 0; a < count; ++a)
    Q_ASSERT(i[a].id.hasReferenceCount());
  }
#endif
}

void CodeModel::items(const IndexedString& file, uint& count, const CodeModelItem*& items) const
{
  ifDebug( kDebug() << "items" << file.str(); )

  CodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);
  
  uint index = d->m_repository.findIndex(item);
  
  if(index) {
    const CodeModelRepositoryItem* repositoryItem = d->m_repository.itemFromIndex(index);
    ifDebug( kDebug() << "found index" << index << repositoryItem->itemsSize(); )
    count = repositoryItem->itemsSize();
    items = repositoryItem->items();
  }else{
    ifDebug( kDebug() << "found no index"; )
    count = 0;
    items = 0;
  }
}

CodeModel& CodeModel::self() {
  static CodeModel ret;
  return ret;
}

}
