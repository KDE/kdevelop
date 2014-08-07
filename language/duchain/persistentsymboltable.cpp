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

Utils::BasicSetRepository* RecursiveImportCacheRepository::repository() {
  static Utils::BasicSetRepository recursiveImportCacheRepositoryObject("Recursive Imports Cache", 0, false);
  return &recursiveImportCacheRepositoryObject;
}

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
  
  uint itemSize() const {
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

template<class ValueType>
struct CacheEntry {
  
  typedef KDevVarLengthArray<ValueType> Data;
  typedef QHash<TopDUContext::IndexedRecursiveImports, Data > DataHash;
  
  DataHash m_hash;
};

class PersistentSymbolTablePrivate
{
public:

  PersistentSymbolTablePrivate() : m_declarations("Persistent Declaration Table") {
  }
  //Maps declaration-ids to declarations
  ItemRepository<PersistentSymbolTableItem, PersistentSymbolTableRequestItem, true, false> m_declarations;
  
  
  QHash<IndexedQualifiedIdentifier, CacheEntry<IndexedDeclaration> > m_declarationsCache;
  
  //We cache the imports so the currently used nodes are very close in memory, which leads to much better CPU cache utilization
  QHash<TopDUContext::IndexedRecursiveImports, PersistentSymbolTable::CachedIndexedRecursiveImports> m_importsCache;
};

void PersistentSymbolTable::clearCache()
{
  ENSURE_CHAIN_WRITE_LOCKED
  {
    QMutexLocker lock(d->m_declarations.mutex());
    d->m_importsCache.clear();
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

PersistentSymbolTable::Declarations PersistentSymbolTable::getDeclarations(const IndexedQualifiedIdentifier& id) const {
  QMutexLocker lock(d->m_declarations.mutex());
  ENSURE_CHAIN_READ_LOCKED
  
  PersistentSymbolTableItem item;
  item.id = id;
  
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

struct DebugVisitor
{
  DebugVisitor(const QTextStream& _out)
    : out(_out)
  {
  }

  bool operator() (const PersistentSymbolTableItem* item) {
    QDebug qout(out.device());
    QualifiedIdentifier id(item->id.identifier());
    if(identifiers.contains(id)) {
      qout << "identifier" << id.toString() << "appears for" << identifiers[id] << "th time";
    }
    
    ++identifiers[id];
    
    for(uint a = 0; a < item->declarationsSize(); ++a) {
      IndexedDeclaration decl(item->declarations()[a]);
      if(!decl.isDummy()) {
        if(declarations.contains(decl)) {
          qout << "declaration found for multiple identifiers. Previous identifier:" << declarations[decl].toString() << "current identifier:" << id.toString() << endl;
        }else{
          declarations.insert(decl, id);
        }
      }
      if(decl.data() && decl.data()->qualifiedIdentifier() != item->id.identifier()) {
        qout << decl.data()->url().str() << "declaration" << decl.data()->qualifiedIdentifier() << "is registered as" << item->id.identifier() << endl;
      }
      
      const QString url = IndexedTopDUContext(decl.topContextIndex()).url().str();
      if(!decl.data() && !decl.isDummy()) {
        qout << "Item in symbol-table is invalid:" << id.toString() << "- localIndex:" << decl.localIndex() << "- url:" << url << endl;
      } else {
        qout << "Item in symbol-table:" << id.toString() << "- localIndex:" << decl.localIndex() << "- url:" << url << "- range:" << decl.data()->range() << endl;
      }
    }
    return true;
  }

  const QTextStream& out;
  QHash<QualifiedIdentifier, uint> identifiers;
  QHash<IndexedDeclaration, QualifiedIdentifier> declarations;
};

void PersistentSymbolTable::dump(const QTextStream& out)
{
  {
    QMutexLocker lock(d->m_declarations.mutex());
    
    QDebug qout(out.device());
    DebugVisitor v(out);
    d->m_declarations.visitAllItems(v);

    qout << "Statistics:" << endl;
    qout << d->m_declarations.statistics() << endl;
  }
}

PersistentSymbolTable& PersistentSymbolTable::self() {
  static PersistentSymbolTable ret;
  return ret;
}

}
