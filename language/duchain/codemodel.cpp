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
#include "repositories/itemrepository.h"
#include "identifier.h"
#include "indexedstring.h"

#define ifDebug(x)

namespace KDevelop {

DEFINE_LIST_MEMBER_HASH(CodeModelRepositoryItem, items, CodeModelItem)
  
class CodeModelRepositoryItem {
  public:
  CodeModelRepositoryItem() {
    initializeAppendedLists();
  }
  CodeModelRepositoryItem(const CodeModelRepositoryItem& rhs) : file(rhs.file) {
    initializeAppendedLists();
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
  
  unsigned short int itemSize() const {
    return dynamicSize();
  }
  
  uint classSize() const {
    return sizeof(CodeModelRepositoryItem);
  }
  
  IndexedString file;
  
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
  
  size_t itemSize() const {
      return m_item.itemSize();
  }

  void createItem(CodeModelRepositoryItem* item) const {
    item->initializeAppendedLists(false);
    item->file = m_item.file;
    item->copyListsFrom(m_item);
  }
  
  bool equals(const CodeModelRepositoryItem* item) const {
    return m_item.file == item->file;
  }
  
  const CodeModelRepositoryItem& m_item;
};


struct CodeModelPrivate {
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

///@todo Sorting in the items list, so searches can be faster
void CodeModel::addItem(const IndexedString& file, const IndexedQualifiedIdentifier& id, CodeModelItem::Kind kind)
{
  ifDebug( kDebug() << "addItem" << file.str() << id.identifier().toString(); )
    
  if(!id.isValid())
    return;
  CodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);
  
  uint index = d->m_repository.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const CodeModelRepositoryItem* oldItem = d->m_repository.itemFromIndex(index);
      ifDebug( kDebug() << "found index" << index << "count:" << oldItem->itemsSize(); )
    int freePlace = -1;
    for(uint a = 0; a < oldItem->itemsSize(); ++a) {
      ifDebug( kDebug() << "id at" << a << oldItem->items()[a].id.identifier().toString(); )
      if(oldItem->items()[a].id == id) {
        CodeModelRepositoryItem* editableItem = d->m_repository.dynamicItemFromIndex(index);
        ++const_cast<CodeModelItem*>(editableItem->items())[a].referenceCount;
        const_cast<CodeModelItem*>(editableItem->items())[a].kind = kind;
        return; //Already there
      }else if(freePlace == -1 && !oldItem->items()[a].id.isValid()) {
        freePlace = (int)a; //Remember an unused position where we can insert the item
        ifDebug( kDebug() << "found free place at" << freePlace; )
      }
    }
    
    CodeModelItem newItem;
    newItem.referenceCount = 1;
    newItem.id = id;
    newItem.kind = kind;

    if(freePlace != -1) {
      //We can insert the item at a currently unused position
      CodeModelRepositoryItem* editableItem = d->m_repository.dynamicItemFromIndex(index);
      const_cast<CodeModelItem*>(editableItem->items())[freePlace] = newItem;
      ifDebug( kDebug() << "using freePlace" << freePlace; )
      return;
    }
    
    item.copyListsFrom(*oldItem);
    item.itemsList().append(newItem);
    
    //Append a few additional items, so we don't need to re-create that often
    item.itemsList().append(CodeModelItem());
    item.itemsList().append(CodeModelItem());
    item.itemsList().append(CodeModelItem());
    item.itemsList().append(CodeModelItem());
    
    ifDebug( kDebug() << "deleting list, and creating new with" << item.itemsSize() << "items"; )
    d->m_repository.deleteItem(index);
  }else{
    //We're creating a new index
    CodeModelItem newItem;
    newItem.referenceCount = 1;
    newItem.id = id;
    newItem.kind = kind;
    item.itemsList().append(newItem);
  }
  Q_ASSERT(!d->m_repository.findIndex(request));

  //This inserts the changed item
  uint newIndex = d->m_repository.index(request);
  Q_UNUSED(newIndex);
  ifDebug( kDebug() << "new index" << newIndex; )
  
  Q_ASSERT(d->m_repository.findIndex(request));
}

void CodeModel::removeItem(const IndexedString& file, const IndexedQualifiedIdentifier& id)
//void CodeModel::removeDeclaration(const QualifiedIdentifier& id, const IndexedDeclaration& declaration)
{
  if(!id.isValid())
    return;
  ifDebug( kDebug() << "removeItem" << file.str() << id.identifier().toString(); )
  CodeModelRepositoryItem item;
  item.file = file;
  CodeModelRequestItem request(item);
  
  uint index = d->m_repository.findIndex(item);
  
  if(index) {
    
    uint freeItemCount = 0;
    
    CodeModelRepositoryItem* oldItem = d->m_repository.dynamicItemFromIndex(index);
    for(uint a = 0; a < oldItem->itemsSize(); ++a) {
      if(oldItem->items()[a].id == id) {
        
        --const_cast<CodeModelItem*>(oldItem->items())[a].referenceCount;
        ifDebug( kDebug() << "reduced reference-count for" << id.identifier().toString() << "to" << oldItem->items()[a].referenceCount; )
        
        if(!oldItem->items()[a].referenceCount) {
          const_cast<CodeModelItem*>(oldItem->items())[a].id = IndexedQualifiedIdentifier();
          ifDebug( kDebug() << "marking index" << a << "as free"; )
        }
      }
      if(!oldItem->items()[a].id.isValid()) {
        ++freeItemCount;
      }
    }
    if(freeItemCount == oldItem->itemsSize()) {
      ifDebug( kDebug() << "no items left, deleting"; )
      d->m_repository.deleteItem(index);
    }else if(freeItemCount > 10) {
        ifDebug( kDebug() << "resizing to make smaller"; )
        
        for(uint a = 0; a < oldItem->itemsSize(); ++a)
          if(oldItem->items()[a].id.isValid())
            item.itemsList().append(oldItem->items()[a]);
        
        d->m_repository.deleteItem(index);
        ifDebug( kDebug() << "creating new entry with" << item.itemsSize() << "entries"; )
        d->m_repository.index(request);
    }
  }
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

