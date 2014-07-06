/* This file is part of KDevelop
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

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

#include "appendedlist.h"
#include "definitions.h"
#include "declaration.h"
#include "declarationid.h"
#include "duchainpointer.h"
#include "indexedstring.h"
#include "repositories/itemrepository.h"

#include <QHash>
#include <QVector>

namespace KDevelop {

DEFINE_LIST_MEMBER_HASH(DefinitionsItem, definitions, IndexedDeclaration)
  
class DefinitionsItem {
  public:
  DefinitionsItem() {
    initializeAppendedLists();
  }
  DefinitionsItem(const DefinitionsItem& rhs, bool dynamic = true) : declaration(rhs.declaration) {
    initializeAppendedLists(dynamic);
    copyListsFrom(rhs);
  }
  
  ~DefinitionsItem() {
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
    return sizeof(DefinitionsItem);
  }
  
  DeclarationId declaration;
  
  START_APPENDED_LISTS(DefinitionsItem);
  APPENDED_LIST_FIRST(DefinitionsItem, IndexedDeclaration, definitions);
  END_APPENDED_LISTS(DefinitionsItem, definitions);
};

class DefinitionsRequestItem {
  public:
  
  DefinitionsRequestItem(const DefinitionsItem& item) : m_item(item) {
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

  void createItem(DefinitionsItem* item) const {
    new (item) DefinitionsItem(m_item, false);
  }
  
  static void destroy(DefinitionsItem* item, KDevelop::AbstractItemRepository&) {
    item->~DefinitionsItem();
  }
  
  static bool persistent(const DefinitionsItem*) {
    return true;
  }
  
  bool equals(const DefinitionsItem* item) const {
    return m_item.declaration == item->declaration;
  }
  
  const DefinitionsItem& m_item;
};

class DefinitionsVisitor
{
public:
  DefinitionsVisitor(Definitions* _definitions, const QTextStream& _out)
    : definitions(_definitions)
    , out(_out)
  {
  }

  bool operator()(const DefinitionsItem* item)
  {
    QDebug qout(out.device());
    auto id = item->declaration;
    auto allDefinitions = definitions->definitions(id);

    qout << "Definitions for" << id.qualifiedIdentifier() << endl;
    FOREACH_ARRAY(const IndexedDeclaration& decl, allDefinitions) {
      if(decl.data()) {
        qout << " " << decl.data()->qualifiedIdentifier() << "in" << decl.data()->url().byteArray() << "at" << decl.data()->rangeInCurrentRevision() << endl;
      }
    }

    return true;
  }

private:
  const Definitions* definitions;
  const QTextStream& out;
};

class DefinitionsPrivate
{
public:

  DefinitionsPrivate() : m_definitions("Definition Map") {
  }
  //Maps declaration-ids to definitions
  ItemRepository<DefinitionsItem, DefinitionsRequestItem> m_definitions;
};

Definitions::Definitions() : d(new DefinitionsPrivate())
{
}

Definitions::~Definitions()
{
  delete d;
}

void Definitions::addDefinition(const DeclarationId& id, const IndexedDeclaration& definition)
{
  DefinitionsItem item;
  item.declaration = id;
  item.definitionsList().append(definition);
  DefinitionsRequestItem request(item);
  
  uint index = d->m_definitions.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const DefinitionsItem* oldItem = d->m_definitions.itemFromIndex(index);
    for(unsigned int a = 0; a < oldItem->definitionsSize(); ++a) {
      if(oldItem->definitions()[a] == definition)
        return; //Already there
      item.definitionsList().append(oldItem->definitions()[a]);
    }
    
    d->m_definitions.deleteItem(index);
  }

  //This inserts the changed item
  d->m_definitions.index(request);
}

void Definitions::removeDefinition(const DeclarationId& id, const IndexedDeclaration& definition)
{
  DefinitionsItem item;
  item.declaration = id;
  DefinitionsRequestItem request(item);
  
  uint index = d->m_definitions.findIndex(item);
  
  if(index) {
    //Check whether the item is already in the mapped list, else copy the list into the new created item
    const DefinitionsItem* oldItem = d->m_definitions.itemFromIndex(index);
    for(unsigned int a = 0; a < oldItem->definitionsSize(); ++a)
      if(!(oldItem->definitions()[a] == definition))
        item.definitionsList().append(oldItem->definitions()[a]);
    
    d->m_definitions.deleteItem(index);
    Q_ASSERT(d->m_definitions.findIndex(item) == 0);
    
    //This inserts the changed item
    if(item.definitionsSize() != 0)
      d->m_definitions.index(request);
  }
}

KDevVarLengthArray<IndexedDeclaration> Definitions::definitions(const DeclarationId& id) const
{
  KDevVarLengthArray<IndexedDeclaration> ret;

  DefinitionsItem item;
  item.declaration = id;
  DefinitionsRequestItem request(item);
  
  uint index = d->m_definitions.findIndex(item);
  
  if(index) {
    const DefinitionsItem* repositoryItem = d->m_definitions.itemFromIndex(index);
    FOREACH_FUNCTION(const IndexedDeclaration& decl, repositoryItem->definitions)
      ret.append(decl);
  }
  
  return ret;
}

void Definitions::dump(const QTextStream& out)
{
  QMutexLocker lock(d->m_definitions.mutex());
  DefinitionsVisitor v(this, out);
  d->m_definitions.visitAllItems(v);
}

}

