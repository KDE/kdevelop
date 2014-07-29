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

#include "importers.h"

#include <QtCore/QHash>
#include <QtCore/QVector>

#include "declarationid.h"
#include "duchainpointer.h"
#include "repositories/itemrepository.h"
#include "topducontext.h"

namespace KDevelop {

DEFINE_LIST_MEMBER_HASH(ImportersItem, importers, IndexedDUContext)
  
class ImportersItem {
  public:
  ImportersItem() {
    initializeAppendedLists();
  }
  ImportersItem(const ImportersItem& rhs, bool dynamic = true) : declaration(rhs.declaration) {
    initializeAppendedLists(dynamic);
    copyListsFrom(rhs);
  }
  
  ~ImportersItem() {
    freeAppendedLists();
  }
  
  unsigned int hash() const {
    //We only compare the declaration. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return declaration.hash();
  }
  
  unsigned int itemSize() const {
    return dynamicSize();
  }
  
  uint classSize() const {
    return sizeof(ImportersItem);
  }
  
  DeclarationId declaration;
  
  START_APPENDED_LISTS(ImportersItem);
  APPENDED_LIST_FIRST(ImportersItem, IndexedDUContext, importers);
  END_APPENDED_LISTS(ImportersItem, importers);
};

class ImportersRequestItem {
  public:
  
  ImportersRequestItem(const ImportersItem& item) : m_item(item) {
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

  void createItem(ImportersItem* item) const {
    new (item) ImportersItem(m_item, false);
  }
  
  static void destroy(ImportersItem* item, KDevelop::AbstractItemRepository&) {
    item->~ImportersItem();
  }
  
  static bool persistent(const ImportersItem* /*item*/) {
    return true;
  }
  
  bool equals(const ImportersItem* item) const {
    return m_item.declaration == item->declaration;
  }
  
  const ImportersItem& m_item;
};


class ImportersPrivate
{
public:

  ImportersPrivate() : m_importers("Importer Map") {
  }
  //Maps declaration-ids to Importers
  ItemRepository<ImportersItem, ImportersRequestItem> m_importers;
};

Importers::Importers() : d(new ImportersPrivate())
{
}

Importers::~Importers()
{
  delete d;
}

void Importers::addImporter(const DeclarationId& id, const IndexedDUContext& use)
{
  ImportersItem item;
  item.declaration = id;
  item.importersList().append(use);
  ImportersRequestItem request(item);
  
  uint index = d->m_importers.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const ImportersItem* oldItem = d->m_importers.itemFromIndex(index);
    for(unsigned int a = 0; a < oldItem->importersSize(); ++a) {
      if(oldItem->importers()[a] == use)
        return; //Already there
      item.importersList().append(oldItem->importers()[a]);
    }
    
    d->m_importers.deleteItem(index);
  }

  //This inserts the changed item
  d->m_importers.index(request);
}

void Importers::removeImporter(const DeclarationId& id, const IndexedDUContext& use)
{
  ImportersItem item;
  item.declaration = id;
  ImportersRequestItem request(item);
  
  uint index = d->m_importers.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const ImportersItem* oldItem = d->m_importers.itemFromIndex(index);
    for(unsigned int a = 0; a < oldItem->importersSize(); ++a)
      if(!(oldItem->importers()[a] == use))
        item.importersList().append(oldItem->importers()[a]);
    
    d->m_importers.deleteItem(index);
    Q_ASSERT(d->m_importers.findIndex(item) == 0);
    
    //This inserts the changed item
    if(item.importersSize() != 0)
      d->m_importers.index(request);
  }
}

KDevVarLengthArray<IndexedDUContext> Importers::importers(const DeclarationId& id) const
{
  KDevVarLengthArray<IndexedDUContext> ret;

  ImportersItem item;
  item.declaration = id;
  ImportersRequestItem request(item);
  
  uint index = d->m_importers.findIndex(item);
  
  if(index) {
    const ImportersItem* repositoryItem = d->m_importers.itemFromIndex(index);
    FOREACH_FUNCTION(const IndexedDUContext& decl, repositoryItem->importers)
      ret.append(decl);
  }
  
  return ret;
}

Importers& Importers::self() {
  static Importers globalImporters;
  return globalImporters;
}

}
