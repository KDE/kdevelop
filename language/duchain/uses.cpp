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

#include "uses.h"

#include <QtCore/QHash>
#include <QtCore/QVector>

#include "declarationid.h"
#include "duchainpointer.h"
#include "repositories/itemrepository.h"
#include "topducontext.h"

namespace KDevelop {

DEFINE_LIST_MEMBER_HASH(UsesItem, uses, IndexedTopDUContext)
  
class UsesItem {
  public:
  UsesItem() {
    initializeAppendedLists();
  }
  UsesItem(const UsesItem& rhs, bool dynamic = true) : declaration(rhs.declaration) {
    initializeAppendedLists(dynamic);
    copyListsFrom(rhs);
  }
  
  ~UsesItem() {
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
    return sizeof(UsesItem);
  }
  
  DeclarationId declaration;
  
  START_APPENDED_LISTS(UsesItem);
  APPENDED_LIST_FIRST(UsesItem, IndexedTopDUContext, uses);
  END_APPENDED_LISTS(UsesItem, uses);
};

class UsesRequestItem {
  public:
  
  UsesRequestItem(const UsesItem& item) : m_item(item) {
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

  void createItem(UsesItem* item) const {
    new (item) UsesItem(m_item, false);
  }
  
  static void destroy(UsesItem* item, KDevelop::AbstractItemRepository&) {
    item->~UsesItem();
  }
  
  static bool persistent(const UsesItem* /*item*/) {
    return true;
  }
  
  bool equals(const UsesItem* item) const {
    return m_item.declaration == item->declaration;
  }
  
  const UsesItem& m_item;
};


class UsesPrivate
{
public:

  UsesPrivate() : m_uses("Use Map") {
  }
  //Maps declaration-ids to Uses
  ItemRepository<UsesItem, UsesRequestItem> m_uses;
};

Uses::Uses() : d(new UsesPrivate())
{
}

Uses::~Uses()
{
  delete d;
}

void Uses::addUse(const DeclarationId& id, const IndexedTopDUContext& use)
{
  UsesItem item;
  item.declaration = id;
  item.usesList().append(use);
  UsesRequestItem request(item);
  
  uint index = d->m_uses.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const UsesItem* oldItem = d->m_uses.itemFromIndex(index);
    for(unsigned int a = 0; a < oldItem->usesSize(); ++a) {
      if(oldItem->uses()[a] == use)
        return; //Already there
      item.usesList().append(oldItem->uses()[a]);
    }
    
    d->m_uses.deleteItem(index);
  }

  //This inserts the changed item
  d->m_uses.index(request);
}

void Uses::removeUse(const DeclarationId& id, const IndexedTopDUContext& use)
{
  UsesItem item;
  item.declaration = id;
  UsesRequestItem request(item);
  
  uint index = d->m_uses.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const UsesItem* oldItem = d->m_uses.itemFromIndex(index);
    for(unsigned int a = 0; a < oldItem->usesSize(); ++a)
      if(!(oldItem->uses()[a] == use))
        item.usesList().append(oldItem->uses()[a]);
    
    d->m_uses.deleteItem(index);
    Q_ASSERT(d->m_uses.findIndex(item) == 0);
    
    //This inserts the changed item
    if(item.usesSize() != 0)
      d->m_uses.index(request);
  }
}

bool Uses::hasUses(const DeclarationId& id) const
{
  UsesItem item;
  item.declaration = id;
  return (bool) d->m_uses.findIndex(item);
}

KDevVarLengthArray<IndexedTopDUContext> Uses::uses(const DeclarationId& id) const
{
  KDevVarLengthArray<IndexedTopDUContext> ret;

  UsesItem item;
  item.declaration = id;
  UsesRequestItem request(item);
  
  uint index = d->m_uses.findIndex(item);
  
  if(index) {
    const UsesItem* repositoryItem = d->m_uses.itemFromIndex(index);
    FOREACH_FUNCTION(const IndexedTopDUContext& decl, repositoryItem->uses)
      ret.append(decl);
  }
  
  return ret;
}


}
