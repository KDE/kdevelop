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

#include "persistentsymboltable.h"

#include <QHash>
#include <QVector>

#include "declarationid.h"
#include "appendedlist.h"
#include "repositories/itemrepository.h"
#include "identifier.h"
#include "ducontext.h"
#include "topducontext.h"

namespace KDevelop {

DEFINE_LIST_MEMBER_HASH(PersistentSymbolTableItem, declarations, IndexedDeclaration)
  
class PersistentSymbolTableItem {
  public:
  PersistentSymbolTableItem() {
    initializeAppendedLists();
  }
  PersistentSymbolTableItem(const PersistentSymbolTableItem& rhs) : id(rhs.id) {
    initializeAppendedLists();
    copyListsFrom(rhs);
  }
  
  ~PersistentSymbolTableItem() {
    freeAppendedLists();
  }
  
  unsigned int hash() const {
    //We only compare the declaration. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return id.index;
  }
  
  unsigned short int itemSize() const {
    return dynamicSize();
  }
  
  uint classSize() const {
    return sizeof(PersistentSymbolTableItem);
  }
  
  IndexedQualifiedIdentifier id;
  
  START_APPENDED_LISTS(PersistentSymbolTableItem);
  APPENDED_LIST_FIRST(PersistentSymbolTableItem, IndexedDeclaration, declarations);
  END_APPENDED_LISTS(PersistentSymbolTableItem, declarations);
};

class PersistentSymbolTableRequestItem {
  public:
  
  PersistentSymbolTableRequestItem(const PersistentSymbolTableItem& item) : m_item(item) {
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

  void createItem(PersistentSymbolTableItem* item) const {
    item->initializeAppendedLists(false);
    item->id = m_item.id;
    item->copyListsFrom(m_item);
  }
  
  bool equals(const PersistentSymbolTableItem* item) const {
    return m_item.id == item->id;
  }
  
  const PersistentSymbolTableItem& m_item;
};

DEFINE_LIST_MEMBER_HASH(PersistentContextTableItem, contexts, IndexedDUContext)

class PersistentContextTableItem {
  public:
  PersistentContextTableItem() {
    initializeAppendedLists();
  }
  PersistentContextTableItem(const PersistentContextTableItem& rhs) : id(rhs.id) {
    initializeAppendedLists();
    copyListsFrom(rhs);
  }
  
  ~PersistentContextTableItem() {
    freeAppendedLists();
  }
  
  unsigned int hash() const {
    //We only compare the context. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return id.index;
  }
  
  unsigned short int itemSize() const {
    return dynamicSize();
  }
  
  uint classSize() const {
    return sizeof(PersistentContextTableItem);
  }
  
  IndexedQualifiedIdentifier id;
  
  START_APPENDED_LISTS(PersistentContextTableItem);
  APPENDED_LIST_FIRST(PersistentContextTableItem, IndexedDUContext, contexts);
  END_APPENDED_LISTS(PersistentContextTableItem, contexts);
};

class PersistentContextTableRequestItem {
  public:
  
  PersistentContextTableRequestItem(const PersistentContextTableItem& item) : m_item(item) {
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

  void createItem(PersistentContextTableItem* item) const {
    item->initializeAppendedLists(false);
    item->id = m_item.id;
    item->copyListsFrom(m_item);
  }
  
  bool equals(const PersistentContextTableItem* item) const {
    return m_item.id == item->id;
  }
  
  const PersistentContextTableItem& m_item;
};


struct PersistentSymbolTablePrivate {
  PersistentSymbolTablePrivate() : m_declarations("Persistent Declaration Table"), m_contexts("Persistent Context Table") {
  }
  //Maps declaration-ids to declarations
  ItemRepository<PersistentSymbolTableItem, PersistentSymbolTableRequestItem> m_declarations;
  ItemRepository<PersistentContextTableItem, PersistentContextTableRequestItem> m_contexts;
};

PersistentSymbolTable::PersistentSymbolTable() : d(new PersistentSymbolTablePrivate())
{
}

PersistentSymbolTable::~PersistentSymbolTable()
{
  delete d;
}

void PersistentSymbolTable::addDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration)
{
  PersistentSymbolTableItem item;
  item.id = id;
  item.declarationsList().append(declaration);
  PersistentSymbolTableRequestItem request(item);
  
  uint index = d->m_declarations.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentSymbolTableItem* oldItem = d->m_declarations.itemFromIndex(index);
    for(uint a = 0; a < oldItem->declarationsSize(); ++a) {
      if(oldItem->declarations()[a] == declaration)
        return; //Already there
      item.declarationsList().append(oldItem->declarations()[a]);
    }
    
    d->m_declarations.deleteItem(index);
  }

  //This inserts the changed item
  d->m_declarations.index(request);
}

void PersistentSymbolTable::removeDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration)
{
  PersistentSymbolTableItem item;
  item.id = id;
  PersistentSymbolTableRequestItem request(item);
  
  uint index = d->m_declarations.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentSymbolTableItem* oldItem = d->m_declarations.itemFromIndex(index);
    uint oldSize = oldItem->declarationsSize();
    
    bool found = false;
    
    for(uint a = 0; a < oldSize; ++a)
      if(!(oldItem->declarations()[a] == declaration))
        item.declarationsList().append(oldItem->declarations()[a]);
      else
        found = true;
    
    if(found) {
      d->m_declarations.deleteItem(index);
      Q_ASSERT(d->m_declarations.findIndex(item) == 0);
      
      //This inserts the changed item
      if(item.declarationsSize() != 0) {
        uint newIndex = d->m_declarations.index(request);
        Q_ASSERT(oldSize == d->m_declarations.itemFromIndex(newIndex)->declarationsSize()+1);
      }
    }else{
      kDebug() << "tried to remove a declaration from the symbol-table that isn't there" << id.identifier().toString();
    }
  }
}

void PersistentSymbolTable::declarations(const IndexedQualifiedIdentifier& id, uint& countTarget, const IndexedDeclaration*& declarationsTarget ) const
{
  PersistentSymbolTableItem item;
  item.id = id;
  PersistentSymbolTableRequestItem request(item);
  
  uint index = d->m_declarations.findIndex(item);
  
  if(index) {
    const PersistentSymbolTableItem* repositoryItem = d->m_declarations.itemFromIndex(index);
    countTarget = repositoryItem->declarationsSize();
    declarationsTarget = repositoryItem->declarations();
  }else{
    countTarget = 0;
    declarationsTarget = 0;
  }
}

void PersistentSymbolTable::addContext(const IndexedQualifiedIdentifier& id, const IndexedDUContext& context) {
  PersistentContextTableItem item;
  item.id = id;
  item.contextsList().append(context);
  PersistentContextTableRequestItem request(item);
  
  uint index = d->m_contexts.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentContextTableItem* oldItem = d->m_contexts.itemFromIndex(index);
    for(uint a = 0; a < oldItem->contextsSize(); ++a) {
      if(oldItem->contexts()[a] == context)
        return; //Already there
      item.contextsList().append(oldItem->contexts()[a]);
    }
    
    d->m_contexts.deleteItem(index);
  }

  //This inserts the changed item
  d->m_contexts.index(request);
}

void PersistentSymbolTable::removeContext(const IndexedQualifiedIdentifier& id, const IndexedDUContext& context) {
  PersistentContextTableItem item;
  item.id = id;
  PersistentContextTableRequestItem request(item);
  
  uint index = d->m_contexts.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentContextTableItem* oldItem = d->m_contexts.itemFromIndex(index);
    for(uint a = 0; a < oldItem->contextsSize(); ++a)
      if(!(oldItem->contexts()[a] == context))
        item.contextsList().append(oldItem->contexts()[a]);
    
    d->m_contexts.deleteItem(index);
    Q_ASSERT(d->m_contexts.findIndex(item) == 0);
    
    //This inserts the changed item
    if(item.contextsSize() != 0)
      d->m_contexts.index(request);
  }
}

void PersistentSymbolTable::contexts(const IndexedQualifiedIdentifier& id, uint& countTarget, const IndexedDUContext*& contextsTarget) const {
  PersistentContextTableItem item;
  item.id = id;
  PersistentContextTableRequestItem request(item);
  
  uint index = d->m_contexts.findIndex(item);
  
  if(index) {
    const PersistentContextTableItem* repositoryItem = d->m_contexts.itemFromIndex(index);
    countTarget = repositoryItem->contextsSize();
    contextsTarget = repositoryItem->contexts();
  }else{
    countTarget = 0;
    contextsTarget = 0;
  }
}

struct Visitor {
  bool operator() (const PersistentSymbolTableItem* item) {
    for(int a = 0; a < item->declarationsSize(); ++a) {
      if(!item->declarations()[a].data()) {
        kDebug() << "Item in symbol-table is invalid:" << item->id.identifier().toString() << item->declarations()[a].localIndex() << IndexedTopDUContext(item->declarations()[a].topContextIndex()).url().str();
      }
    }
    return true;
  }
};

void PersistentSymbolTable::selfAnalysis() {
  Visitor v;
  kDebug() << d->m_declarations.statistics();
  d->m_declarations.visitAllItems(v);
}

PersistentSymbolTable& PersistentSymbolTable::self() {
  static PersistentSymbolTable ret;
  return ret;
}

}

