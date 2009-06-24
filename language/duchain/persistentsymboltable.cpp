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
#include "duchain.h"
#include "duchainlock.h"
#include <util/embeddedfreetree.h>

//For now, just _always_ use the cache
const uint MinimumCountForCache = 1;

namespace KDevelop {

Utils::BasicSetRepository recursiveImportCacheRepository("Recursive Imports Cache", 0, false);

DEFINE_LIST_MEMBER_HASH(PersistentSymbolTableItem, declarations, IndexedDeclaration)

class PersistentSymbolTableItem {
  public:
  PersistentSymbolTableItem() : centralFreeItem(-1) {
    initializeAppendedLists();
  }
  PersistentSymbolTableItem(const PersistentSymbolTableItem& rhs, bool dynamic = true) : id(rhs.id), centralFreeItem(rhs.centralFreeItem) {
    initializeAppendedLists(dynamic);
    copyListsFrom(rhs);
  }
  
  ~PersistentSymbolTableItem() {
    freeAppendedLists();
  }
  
  inline unsigned int hash() const {
    //We only compare the declaration. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return id.getIndex();
  }
  
  unsigned int itemSize() const {
    return dynamicSize();
  }
  
  uint classSize() const {
    return sizeof(PersistentSymbolTableItem);
  }
  
  IndexedQualifiedIdentifier id;
  int centralFreeItem;
  
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
    new (item) PersistentSymbolTableItem(m_item, false);
  }
  
  static void destroy(PersistentSymbolTableItem* item, KDevelop::AbstractItemRepository&) {
    item->~PersistentSymbolTableItem();
  }

  static bool persistent(const PersistentSymbolTableItem*) {
    return true; //Nothing to do
  }
  
  bool equals(const PersistentSymbolTableItem* item) const {
    return m_item.id == item->id;
  }
  
  const PersistentSymbolTableItem& m_item;
};

DEFINE_LIST_MEMBER_HASH(PersistentContextTableItem, contexts, IndexedDUContext)

class PersistentContextTableItem {
  public:
  PersistentContextTableItem() : centralFreeItem(-1) {
    initializeAppendedLists();
  }
  PersistentContextTableItem(const PersistentContextTableItem& rhs, bool dynamic = true) : id(rhs.id), centralFreeItem(rhs.centralFreeItem) {
    initializeAppendedLists(dynamic);
    copyListsFrom(rhs);
  }
  
  ~PersistentContextTableItem() {
    freeAppendedLists();
  }
  
  inline unsigned int hash() const {
    //We only compare the context. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return id.getIndex();
  }
  
  size_t itemSize() const {
    return dynamicSize();
  }
  
  uint classSize() const {
    return sizeof(PersistentContextTableItem);
  }
  
  IndexedQualifiedIdentifier id;
  int centralFreeItem;
  
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
    new (item) PersistentContextTableItem(m_item, false);
  }
  
  static void destroy(PersistentContextTableItem* item, KDevelop::AbstractItemRepository&) {
    item->~PersistentContextTableItem();
  }
  
  static bool persistent(const PersistentContextTableItem*) {
    return true; //Nothing to do
  }
  
  bool equals(const PersistentContextTableItem* item) const {
    return m_item.id == item->id;
  }
  
  const PersistentContextTableItem& m_item;
};

template<class ValueType>
struct CacheEntry {
  
  typedef KDevVarLengthArray<ValueType> Data;
  typedef QHash<TopDUContext::IndexedRecursiveImports, Data > DataHash;
  
  DataHash m_hash;
};

struct PersistentSymbolTablePrivate {
  PersistentSymbolTablePrivate() : m_declarations("Persistent Declaration Table"), m_contexts("Persistent Context Table") {
    m_contexts.setMutex(m_declarations.mutex());
  }
  //Maps declaration-ids to declarations
  ItemRepository<PersistentSymbolTableItem, PersistentSymbolTableRequestItem, true, false> m_declarations;
  ItemRepository<PersistentContextTableItem, PersistentContextTableRequestItem, true, false> m_contexts;
  
  
  QHash<IndexedQualifiedIdentifier, CacheEntry<IndexedDeclaration> > m_declarationsCache;
  QHash<IndexedQualifiedIdentifier, CacheEntry<IndexedDUContext> > m_contextsCache;
  
  //We cache the imports so the currently used nodes are very close in memory, which leads to much better CPU cache utilization
  QHash<TopDUContext::IndexedRecursiveImports, PersistentSymbolTable::CachedIndexedRecursiveImports> m_importsCache;
};

void PersistentSymbolTable::clearCache()
{
  ENSURE_CHAIN_WRITE_LOCKED
  {
    QMutexLocker lock(d->m_declarations.mutex());
    QMutexLocker lock2(d->m_contexts.mutex());
    d->m_importsCache.clear();
    d->m_contextsCache.clear();
    d->m_declarationsCache.clear();
  }
}

PersistentSymbolTable::PersistentSymbolTable() : d(new PersistentSymbolTablePrivate())
{
}

PersistentSymbolTable::~PersistentSymbolTable()
{
  //Workaround for a strange destruction-order related crash duing shutdown
  //We just let the data leak. This doesn't hurt, as there is no meaningful destructors.
//   delete d;
}

void PersistentSymbolTable::addDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration)
{
  QMutexLocker lock(d->m_declarations.mutex());
  ENSURE_CHAIN_WRITE_LOCKED
  
  d->m_declarationsCache.remove(id);
  
  PersistentSymbolTableItem item;
  item.id = id;
  PersistentSymbolTableRequestItem request(item);
  
  uint index = d->m_declarations.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentSymbolTableItem* oldItem = d->m_declarations.itemFromIndex(index);
    
    EmbeddedTreeAlgorithms<IndexedDeclaration, IndexedDeclarationHandler> alg(oldItem->declarations(), oldItem->declarationsSize(), oldItem->centralFreeItem);
    
    if(alg.indexOf(declaration) != -1)
      return;
    
    DynamicItem<PersistentSymbolTableItem, true> editableItem = d->m_declarations.dynamicItemFromIndex(index);
    
    EmbeddedTreeAddItem<IndexedDeclaration, IndexedDeclarationHandler> add(const_cast<IndexedDeclaration*>(editableItem->declarations()), editableItem->declarationsSize(), editableItem->centralFreeItem, declaration);
    
    uint newSize = add.newItemCount();
    if(newSize != editableItem->declarationsSize()) {
      //We need to resize. Update and fill the new item, and delete the old item.
      item.declarationsList().resize(newSize);
      add.transferData(item.declarationsList().data(), newSize, &item.centralFreeItem);
      
      d->m_declarations.deleteItem(index);
      Q_ASSERT(!d->m_declarations.findIndex(request));
    }else{
      //We're fine, the item could be added to the existing list
      return;
    }
  }else{
    item.declarationsList().append(declaration);
  }

  //This inserts the changed item
  d->m_declarations.index(request);
}

void PersistentSymbolTable::removeDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration)
{
  QMutexLocker lock(d->m_declarations.mutex());
  ENSURE_CHAIN_WRITE_LOCKED
  
  d->m_declarationsCache.remove(id);
  Q_ASSERT(!d->m_declarationsCache.contains(id));
  
  PersistentSymbolTableItem item;
  item.id = id;
  PersistentSymbolTableRequestItem request(item);
  
  uint index = d->m_declarations.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentSymbolTableItem* oldItem = d->m_declarations.itemFromIndex(index);
    
    EmbeddedTreeAlgorithms<IndexedDeclaration, IndexedDeclarationHandler> alg(oldItem->declarations(), oldItem->declarationsSize(), oldItem->centralFreeItem);
    
    if(alg.indexOf(declaration) == -1)
      return;
    
    DynamicItem<PersistentSymbolTableItem, true> editableItem = d->m_declarations.dynamicItemFromIndex(index);
    
    EmbeddedTreeRemoveItem<IndexedDeclaration, IndexedDeclarationHandler> remove(const_cast<IndexedDeclaration*>(editableItem->declarations()), editableItem->declarationsSize(), editableItem->centralFreeItem, declaration);
    
    uint newSize = remove.newItemCount();
    if(newSize != editableItem->declarationsSize()) {
      //We need to resize. Update and fill the new item, and delete the old item.
      item.declarationsList().resize(newSize);
      remove.transferData(item.declarationsList().data(), newSize, &item.centralFreeItem);
      
      d->m_declarations.deleteItem(index);
      Q_ASSERT(!d->m_declarations.findIndex(request));
    }else{
      //We're fine, the item could be added to the existing list
      return;
    }
  }

  //This inserts the changed item
  if(item.declarationsSize())
    d->m_declarations.index(request);
}

struct DeclarationCacheVisitor {
  DeclarationCacheVisitor(KDevVarLengthArray<IndexedDeclaration>& _cache) : cache(_cache) {
  }
  
  bool operator()(const IndexedDeclaration& decl) const {
    cache.append(decl);
    return true;
  }
  
  KDevVarLengthArray<IndexedDeclaration>& cache;
};

PersistentSymbolTable::FilteredDeclarationIterator PersistentSymbolTable::getFilteredDeclarations(const IndexedQualifiedIdentifier& id, const TopDUContext::IndexedRecursiveImports& visibility) const {
  
  QMutexLocker lock(d->m_declarations.mutex());
  ENSURE_CHAIN_READ_LOCKED
  
  Declarations decls = getDeclarations(id).iterator();
  
  CachedIndexedRecursiveImports cachedImports;
  
  QHash<TopDUContext::IndexedRecursiveImports, CachedIndexedRecursiveImports>::const_iterator it = d->m_importsCache.constFind(visibility);
  if(it != d->m_importsCache.constEnd()) {
    cachedImports = *it;
  }else{
    cachedImports = CachedIndexedRecursiveImports(visibility.set().stdSet());
    d->m_importsCache.insert(visibility, cachedImports);
  }
  
  if(decls.dataSize() > MinimumCountForCache)
  {
    //Do visibility caching
    CacheEntry<IndexedDeclaration>& cached(d->m_declarationsCache[id]);
    CacheEntry<IndexedDeclaration>::DataHash::const_iterator cacheIt = cached.m_hash.constFind(visibility);
    if(cacheIt != cached.m_hash.constEnd())
      return FilteredDeclarationIterator(Declarations::Iterator(cacheIt->constData(), cacheIt->size(), -1), cachedImports);

    CacheEntry<IndexedDeclaration>::DataHash::iterator insertIt = cached.m_hash.insert(visibility, KDevVarLengthArray<IndexedDeclaration>());
    
    KDevVarLengthArray<IndexedDeclaration>& cache(*insertIt);
    
    {
      typedef ConvenientEmbeddedSetTreeFilterVisitor<IndexedDeclaration, IndexedDeclarationHandler, IndexedTopDUContext, CachedIndexedRecursiveImports, DeclarationTopContextExtractor, DeclarationCacheVisitor> FilteredDeclarationCacheVisitor;
    
      //The visitor visits all the declarations from within its constructor
      DeclarationCacheVisitor v(cache);
      FilteredDeclarationCacheVisitor visitor(v, decls.iterator(), cachedImports);
    }
    
    return FilteredDeclarationIterator(Declarations::Iterator(cache.constData(), cache.size(), -1), cachedImports, true);
  }else{
    return FilteredDeclarationIterator(decls.iterator(), cachedImports);
  }
}

struct DUContextCacheVisitor {
  DUContextCacheVisitor(KDevVarLengthArray<IndexedDUContext>& _cache) : cache(_cache) {
  }
  
  bool operator()(const IndexedDUContext& decl) const {
    cache.append(decl);
    return true;
  }
  
  KDevVarLengthArray<IndexedDUContext>& cache;
};

PersistentSymbolTable::FilteredDUContextIterator PersistentSymbolTable::getFilteredContexts(const IndexedQualifiedIdentifier& id, const TopDUContext::IndexedRecursiveImports& visibility) const {
  QMutexLocker lock(d->m_contexts.mutex());
  ENSURE_CHAIN_READ_LOCKED
  
  Contexts contexts = getContexts(id);
  
  CachedIndexedRecursiveImports cachedImports;
  
  QHash<TopDUContext::IndexedRecursiveImports, CachedIndexedRecursiveImports>::const_iterator it = d->m_importsCache.constFind(visibility);
  if(it != d->m_importsCache.constEnd()) {
    cachedImports = *it;
  }else{
    cachedImports = CachedIndexedRecursiveImports(visibility.set().stdSet());
    d->m_importsCache.insert(visibility, cachedImports);
  }
  
  if(contexts.dataSize() > MinimumCountForCache)
  {
    //Do visibility caching
    CacheEntry<IndexedDUContext>& cached(d->m_contextsCache[id]);
    CacheEntry<IndexedDUContext>::DataHash::const_iterator cacheIt = cached.m_hash.constFind(visibility);
    if(cacheIt != cached.m_hash.constEnd())
      return FilteredDUContextIterator(Contexts::Iterator(cacheIt->constData(), cacheIt->size(), -1), cachedImports);

    CacheEntry<IndexedDUContext>::DataHash::iterator insertIt = cached.m_hash.insert(visibility, KDevVarLengthArray<IndexedDUContext>());
    
    KDevVarLengthArray<IndexedDUContext>& cache(*insertIt);
    
    {
      typedef ConvenientEmbeddedSetTreeFilterVisitor<IndexedDUContext, IndexedDUContextHandler, IndexedTopDUContext, CachedIndexedRecursiveImports, DUContextTopContextExtractor, DUContextCacheVisitor> FilteredDUContextCacheVisitor;
    
      //The visitor visits all the declarations from within its constructor
      DUContextCacheVisitor v(cache);
      FilteredDUContextCacheVisitor visitor(v, contexts.iterator(), cachedImports);
    }
    
    return FilteredDUContextIterator(Contexts::Iterator(cache.constData(), cache.size(), -1), cachedImports, true);
  }else{
    return FilteredDUContextIterator(contexts.iterator(), cachedImports);
  }
}

PersistentSymbolTable::Declarations PersistentSymbolTable::getDeclarations(const IndexedQualifiedIdentifier& id) const {
  QMutexLocker lock(d->m_declarations.mutex());
  ENSURE_CHAIN_READ_LOCKED
  
  PersistentSymbolTableItem item;
  item.id = id;
  PersistentSymbolTableRequestItem request(item);
  
  uint index = d->m_declarations.findIndex(item);
  
  if(index) {
    const PersistentSymbolTableItem* repositoryItem = d->m_declarations.itemFromIndex(index);
    return PersistentSymbolTable::Declarations(repositoryItem->declarations(), repositoryItem->declarationsSize(), repositoryItem->centralFreeItem);
  }else{
    return PersistentSymbolTable::Declarations();
  }
}

void PersistentSymbolTable::declarations(const IndexedQualifiedIdentifier& id, uint& countTarget, const IndexedDeclaration*& declarationsTarget) const
{
  QMutexLocker lock(d->m_declarations.mutex());
  ENSURE_CHAIN_READ_LOCKED
  
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

void PersistentSymbolTable::addContext(const IndexedQualifiedIdentifier& id, const IndexedDUContext& context)
{
  QMutexLocker lock(d->m_contexts.mutex());
  ENSURE_CHAIN_WRITE_LOCKED
  
  d->m_contextsCache.remove(id);
  
  PersistentContextTableItem item;
  item.id = id;
  PersistentContextTableRequestItem request(item);
  
  uint index = d->m_contexts.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentContextTableItem* oldItem = d->m_contexts.itemFromIndex(index);
    
    EmbeddedTreeAlgorithms<IndexedDUContext, IndexedDUContextHandler> alg(oldItem->contexts(), oldItem->contextsSize(), oldItem->centralFreeItem);
    
    if(alg.indexOf(context) != -1)
      return;
    
    QMutexLocker lock(d->m_contexts.mutex());
    
    DynamicItem<PersistentContextTableItem, true> editableItem = d->m_contexts.dynamicItemFromIndex(index);
    
    EmbeddedTreeAddItem<IndexedDUContext, IndexedDUContextHandler> add(const_cast<IndexedDUContext*>(editableItem->contexts()), editableItem->contextsSize(), editableItem->centralFreeItem, context);
    
    uint newSize = add.newItemCount();
    if(newSize != editableItem->contextsSize()) {
      //We need to resize. Update and fill the new item, and delete the old item.
      item.contextsList().resize(newSize);
      add.transferData(item.contextsList().data(), newSize, &item.centralFreeItem);
      
      d->m_contexts.deleteItem(index);
      Q_ASSERT(!d->m_contexts.findIndex(request));
    }else{
      //We're fine, the item could be added to the existing list
      return;
    }
  }else{
    item.contextsList().append(context);
  }

  //This inserts the changed item
  d->m_contexts.index(request);
}

void PersistentSymbolTable::removeContext(const IndexedQualifiedIdentifier& id, const IndexedDUContext& context)
{
  QMutexLocker lock(d->m_contexts.mutex());
  ENSURE_CHAIN_WRITE_LOCKED
  
  d->m_contextsCache.remove(id);
  
  PersistentContextTableItem item;
  item.id = id;
  PersistentContextTableRequestItem request(item);
  
  uint index = d->m_contexts.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const PersistentContextTableItem* oldItem = d->m_contexts.itemFromIndex(index);
    
    EmbeddedTreeAlgorithms<IndexedDUContext, IndexedDUContextHandler> alg(oldItem->contexts(), oldItem->contextsSize(), oldItem->centralFreeItem);
    
    if(alg.indexOf(context) == -1)
      return;
    
    QMutexLocker lock(d->m_contexts.mutex());
    
    DynamicItem<PersistentContextTableItem, true> editableItem = d->m_contexts.dynamicItemFromIndex(index);
    
    EmbeddedTreeRemoveItem<IndexedDUContext, IndexedDUContextHandler> remove(const_cast<IndexedDUContext*>(editableItem->contexts()), editableItem->contextsSize(), editableItem->centralFreeItem, context);
    
    uint newSize = remove.newItemCount();
    if(newSize != editableItem->contextsSize()) {
      //We need to resize. Update and fill the new item, and delete the old item.
      item.contextsList().resize(newSize);
      remove.transferData(item.contextsList().data(), newSize, &item.centralFreeItem);
      
      d->m_contexts.deleteItem(index);
      Q_ASSERT(!d->m_contexts.findIndex(request));
    }else{
      //We're fine, the item could be added to the existing list
      return;
    }
  }

  //This inserts the changed item
  if(item.contextsSize())
    d->m_contexts.index(request);
}

PersistentSymbolTable::Contexts PersistentSymbolTable::getContexts(const IndexedQualifiedIdentifier& id) const {
  QMutexLocker lock(d->m_contexts.mutex());
  ENSURE_CHAIN_READ_LOCKED

  PersistentContextTableItem item;
  item.id = id;
  PersistentContextTableRequestItem request(item);
  
  uint index = d->m_contexts.findIndex(item);
  
  if(index) {
    const PersistentContextTableItem* repositoryItem = d->m_contexts.itemFromIndex(index);
    return PersistentSymbolTable::Contexts(repositoryItem->contexts(), repositoryItem->contextsSize(), repositoryItem->centralFreeItem);
  }else{
    return PersistentSymbolTable::Contexts();
  }
}

void PersistentSymbolTable::contexts(const IndexedQualifiedIdentifier& id, uint& countTarget, const IndexedDUContext*& contextsTarget) const {
  QMutexLocker lock(d->m_contexts.mutex());
  ENSURE_CHAIN_READ_LOCKED

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
    QualifiedIdentifier id(item->id.identifier());
    if(identifiers.contains(id))
      kDebug() << "identifier" << id.toString() << "appears for" << identifiers[id] << "th time";
    
    ++identifiers[id];
    
    for(uint a = 0; a < item->declarationsSize(); ++a) {
      IndexedDeclaration decl(item->declarations()[a]);
      if(!decl.isDummy()) {
        if(declarations.contains(decl)) {
          kDebug() << "declaration found for multiple identifiers. Previous identifier:" << declarations[decl].toString() << "current identifier:" << id.toString();
        }else{
          declarations.insert(decl, id);
        }
      }
      if(decl.data() && decl.data()->qualifiedIdentifier() != item->id.identifier())
        kDebug() << decl.data()->url().str() << "declaration" << decl.data()->qualifiedIdentifier() << "is registered as" << item->id.identifier();
      
      if(!decl.data() && !decl.isDummy()) {
        kDebug() << "Item in symbol-table is invalid:" << id.toString() << item->declarations()[a].localIndex() << IndexedTopDUContext(item->declarations()[a].topContextIndex()).url().str();
      }
    }
    return true;
  }
  QHash<QualifiedIdentifier, uint> identifiers;
  QHash<IndexedDeclaration, QualifiedIdentifier> declarations;
};

struct ContextVisitor {
  bool operator() (const PersistentContextTableItem* item) {
    QualifiedIdentifier id(item->id.identifier());
    if(identifiers.contains(id))
      kDebug() << "identifier" << id.toString() << "appears for" << identifiers[id] << "th time";
    
    ++identifiers[id];
    
    for(uint a = 0; a < item->contextsSize(); ++a) {
      if(!item->contexts()[a].data() && !!item->contexts()[a].isDummy()) {
        kDebug() << "Item in Context-table is invalid:" << id.toString() << item->contexts()[a].localIndex() << IndexedTopDUContext(item->contexts()[a].topContextIndex()).url().str();
      }else if(item->contexts()[a].data() && item->contexts()[a].data()->scopeIdentifier(true) != id) {
        kDebug() << item->contexts()[a].data()->url().str() << "context" << item->contexts()[a].data()->scopeIdentifier(true) << "is registered as" << id;
      }
    }
    return true;
  }
  QHash<QualifiedIdentifier, uint> identifiers;
};

void PersistentSymbolTable::selfAnalysis() {
  {
    QMutexLocker lock(d->m_declarations.mutex());
    
    Visitor v;
    kDebug() << d->m_declarations.statistics();
    d->m_declarations.visitAllItems(v);
    kDebug() << "visited" << v.identifiers.size() << "identifiers";
  }

  {
    QMutexLocker lock(d->m_contexts.mutex());
    
    ContextVisitor v;
    kDebug() << d->m_contexts.statistics();
    d->m_contexts.visitAllItems(v);
    kDebug() << "visited" << v.identifiers.size() << "identifiers";
  }
}

PersistentSymbolTable& PersistentSymbolTable::self() {
  static PersistentSymbolTable ret;
  return ret;
}

}

