/*
 * This file is part of KDevelop
 *
 * Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "referencecounting.h"
#include <QMutex>
#include <QMap>
#include <QAtomicInt>
#include <kdebug.h>
#include "serialization/itemrepository.h"
#include <util/spinlock.h>

namespace KDevelop {

  bool doReferenceCounting = false;

  //Protects the reference-counting data through a spin-lock
  SpinLockData refCountingLock;
  
  QMap<void*, QPair<uint, uint> >* refCountingRanges = new QMap<void*, QPair<uint, uint> >(); //ptr, <size, count>, leaked intentionally!
  bool refCountingHasAdditionalRanges = false; //Whether 'refCountingRanges' is non-empty
  
  //Speedup: In most cases there is only exactly one reference-counted range active,
  //so the first reference-counting range can be marked here.
  void* refCountingFirstRangeStart = 0;
  QPair<uint, uint> refCountingFirstRangeExtent = qMakePair(0u, 0u);
}

void KDevelop::disableDUChainReferenceCounting(void* start)
{
  SpinLock<> lock(refCountingLock);

  if(refCountingFirstRangeStart && ((char*)refCountingFirstRangeStart) <= (char*)start && (char*)start < ((char*)refCountingFirstRangeStart) + refCountingFirstRangeExtent.first)
  {
    Q_ASSERT(refCountingFirstRangeExtent.second > 0);
    --refCountingFirstRangeExtent.second;
    if(refCountingFirstRangeExtent.second == 0) {
      refCountingFirstRangeExtent = qMakePair<uint, uint>(0, 0);
      refCountingFirstRangeStart = 0;
    }
  }
  else if(refCountingHasAdditionalRanges)
  {
    QMap< void*, QPair<uint, uint> >::iterator it = refCountingRanges->upperBound(start);
    if(it != refCountingRanges->begin()) {
      --it;
      if(((char*)it.key()) <= (char*)start && (char*)start < ((char*)it.key()) + it.value().first)
      {
        //Contained
      }else{
        Q_ASSERT(0);
      }
    }
    Q_ASSERT(it.value().second > 0);
    --it.value().second;
    if(it.value().second == 0)
      refCountingRanges->erase(it);
    refCountingHasAdditionalRanges = !refCountingRanges->isEmpty();
  }else{
    Q_ASSERT(0);
  }
  
  if(!refCountingFirstRangeStart && !refCountingHasAdditionalRanges)
    doReferenceCounting = false;
}

void KDevelop::enableDUChainReferenceCounting(void* start, unsigned int size)
{
  SpinLock<> lock(refCountingLock);
  
  doReferenceCounting = true;
  
  if(refCountingFirstRangeStart && ((char*)refCountingFirstRangeStart) <= (char*)start && (char*)start < ((char*)refCountingFirstRangeStart) + refCountingFirstRangeExtent.first)
  {
    //Increase the count for the first range
    ++refCountingFirstRangeExtent.second;
  }else if(refCountingHasAdditionalRanges || refCountingFirstRangeStart)
  {
    //There is additional ranges in the ranges-structure. Add any new ranges there as well.
    QMap< void*, QPair<uint, uint> >::iterator it = refCountingRanges->upperBound(start);
    if(it != refCountingRanges->begin()) {
      --it;
      if(((char*)it.key()) <= (char*)start && (char*)start < ((char*)it.key()) + it.value().first)
      {
        //Contained, count up
      }else{
        it = refCountingRanges->end(); //Insert own item
      }
    }else if(it != refCountingRanges->end() && it.key() > start) {
      //The item is behind
      it = refCountingRanges->end();
    }
    
    if(it == refCountingRanges->end()) {
      QMap< void*, QPair<uint, uint> >::iterator inserted = refCountingRanges->insert(start, qMakePair(size, 1u));
      //Merge following ranges
      QMap< void*, QPair<uint, uint> >::iterator it = inserted;
      ++it;
      while(it != refCountingRanges->end() && it.key() < ((char*)start) + size) {
        
        inserted.value().second += it.value().second; //Accumulate count
        if(((char*)start) + size < ((char*)inserted.key()) + it.value().first) {
          //Update end position
          inserted.value().first = (((char*)inserted.key()) + it.value().first) - ((char*)start);
        }
        
        it = refCountingRanges->erase(it);
      }
    }else{
      ++it.value().second;
      if(it.value().first < size)
        it.value().first = size;
    }
    
    refCountingHasAdditionalRanges = true;
  }else{
    refCountingFirstRangeStart = start;
    refCountingFirstRangeExtent.first = size;
    refCountingFirstRangeExtent.second = 1;
  }
  
  Q_ASSERT(refCountingHasAdditionalRanges == (refCountingRanges && !refCountingRanges->isEmpty()));
#ifdef TEST_REFERENCE_COUNTING
  Q_ASSERT(shouldDoDUChainReferenceCounting(start));
  Q_ASSERT(shouldDoDUChainReferenceCounting(((char*)start + (size-1))));
#endif
}

#ifdef TEST_REFERENCE_COUNTING

QAtomicInt& id() {
  static QAtomicInt& ret(KDevelop::globalItemRepositoryRegistry().getCustomCounter("referencer ids", 1));
  return ret;
}



namespace KDevelop {
ReferenceCountManager::ReferenceCountManager() : m_id(id().fetchAndAddRelaxed(1)) {
}

struct ReferenceCountItem {
  ///Item entries:
  ReferenceCountItem(uint id, uint target) : m_id(id), m_targetId(target) {
  }
  
  //Every item has to implement this function, and return a valid hash.
  //Must be exactly the same hash value as ReferenceCountItemRequest::hash() has returned while creating the item.
  unsigned int hash() const {
    return KDevHash() << m_id << m_targetId;
  }

  //Every item has to implement this function, and return the complete size this item takes in memory.
  //Must be exactly the same value as ReferenceCountItemRequest::itemSize() has returned while creating the item.
  unsigned short int itemSize() const {
    return sizeof(ReferenceCountItem);
  }
  
  uint m_id;
  uint m_targetId;
  
  ///Request entries:
  enum {
    AverageSize = 8
  };

  void createItem(ReferenceCountItem* item) const {
    *item = *this;
  }
  static void destroy(ReferenceCountItem* /*item*/, AbstractItemRepository&) {
  }
  
  static bool persistent(const ReferenceCountItem*) {
    return true;
  }

  bool equals(const ReferenceCountItem* item) const {
    return m_id == item->m_id && m_targetId == item->m_targetId;
  }
};

static RepositoryManager< ItemRepository<ReferenceCountItem, ReferenceCountItem, false, true, sizeof(ReferenceCountItem)>, false>& references() {
  static RepositoryManager< ItemRepository<ReferenceCountItem, ReferenceCountItem, false, true, sizeof(ReferenceCountItem)>, false> referencesObject("Reference Count Debugging");
  return referencesObject;
}
static RepositoryManager< ItemRepository<ReferenceCountItem, ReferenceCountItem, false, true, sizeof(ReferenceCountItem)>, false>& oldReferences() {
  static RepositoryManager< ItemRepository<ReferenceCountItem, ReferenceCountItem, false, true, sizeof(ReferenceCountItem)>, false> oldReferencesObject("Old Reference Count Debugging");
  return oldReferencesObject;
}

void KDevelop::initReferenceCounting() {
  references();
  oldReferences();
}

ReferenceCountManager::~ReferenceCountManager() {
  //Make sure everything is cleaned up when the object is destroyed
//   Q_ASSERT(!references().contains(m_id));
}

ReferenceCountManager::ReferenceCountManager(const ReferenceCountManager& rhs) : m_id(id().fetchAndAddRelaxed(1)) {
  //New id
}

ReferenceCountManager& ReferenceCountManager::ReferenceCountManager::operator=(const ReferenceCountManager& rhs) {
  //Keep id
  return *this;
}

// bool ReferenceCountManager::hasReferenceCount() const {
//   return references->findIndex(ReferenceCountItem);
// }

void ReferenceCountManager::increase(uint& ref, uint targetId) {
  Q_ASSERT(shouldDoDUChainReferenceCounting(this));
  Q_ASSERT(!references->findIndex(ReferenceCountItem(m_id, targetId)));
  ++ref;
  
  {
    int oldIndex = oldReferences->findIndex(ReferenceCountItem(m_id, targetId));
    if(oldIndex)
      oldReferences->deleteItem(oldIndex);
  }
  
  Q_ASSERT(references->index(ReferenceCountItem(m_id, targetId)));
}

void ReferenceCountManager::decrease(uint& ref, uint targetId) {
  Q_ASSERT(ref > 0);
  Q_ASSERT(shouldDoDUChainReferenceCounting(this));
  Q_ASSERT(!oldReferences->findIndex(ReferenceCountItem(m_id, targetId)));
  uint refIndex = references->findIndex(ReferenceCountItem(m_id, targetId));
  Q_ASSERT(refIndex);
  --ref;
  references->deleteItem(refIndex);
  oldReferences->index(ReferenceCountItem(m_id, targetId));
}
}

#else
namespace KDevelop {
  void initReferenceCounting() {
  }
}
#endif
