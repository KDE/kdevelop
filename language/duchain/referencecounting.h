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
#ifndef KDEVPLATFORM_REFERENCECOUNTING_H
#define KDEVPLATFORM_REFERENCECOUNTING_H

#include "../languageexport.h"
#include <QMap>
#include <QPair>

//When this is enabled, the duchain unloading is disabled as well, and you should start
//with a cleared ~/.kdevduchain
// #define TEST_REFERENCE_COUNTING
#include <util/spinlock.h>

namespace KDevelop {
  
  ///Since shouldDoDUChainReferenceCounting is called extremely often, we export some internals into the header here,
  ///so the reference-counting code can be inlined.
  
  KDEVPLATFORMLANGUAGE_EXPORT extern bool doReferenceCounting;
  KDEVPLATFORMLANGUAGE_EXPORT  extern SpinLockData refCountingLock;
  KDEVPLATFORMLANGUAGE_EXPORT  extern QMap<void*, QPair<uint, uint> >* refCountingRanges;
  KDEVPLATFORMLANGUAGE_EXPORT  extern bool refCountingHasAdditionalRanges;
  KDEVPLATFORMLANGUAGE_EXPORT  extern void* refCountingFirstRangeStart;
  KDEVPLATFORMLANGUAGE_EXPORT  extern QPair<uint, uint> refCountingFirstRangeExtent;

  ///@internal The spin-lock ,must already be locked
  inline bool shouldDoDUChainReferenceCountingInternal(void* item)
  {
    QMap< void*, QPair<uint, uint> >::iterator it = refCountingRanges->upperBound(item);
    if(it != refCountingRanges->begin()) {
      --it;
      return ((char*)it.key()) <= (char*)item && (char*)item < ((char*)it.key()) + it.value().first;
    }
    
    return false;
  }
  
  ///This is used by indexed items to decide whether they should do reference-counting
  inline bool shouldDoDUChainReferenceCounting(void* item) 
  {
    if(!doReferenceCounting) //Fast path, no place has been marked for reference counting, 99% of cases
      return false;

    SpinLock<> lock(refCountingLock);

    if(refCountingFirstRangeStart &&
       (((char*)refCountingFirstRangeStart) <= (char*)item) &&
       ((char*)item < ((char*)refCountingFirstRangeStart) + refCountingFirstRangeExtent.first))
        return true;

    if(refCountingHasAdditionalRanges)
      return shouldDoDUChainReferenceCountingInternal(item);
    else
      return false;
  }
  
  ///Enable reference-counting for the given range
  ///You should only enable the reference-counting for the time it's really needed,
  ///and it always has to be enabled too when the items are deleted again, else
  ///it will lead to inconsistencies in the repository.
  ///@warning If you are not working on the duchain internal storage mechanism, you should
  ///not care about this stuff at all.
  ///@param start Position where to start the reference-counting
  ///@param size Size of the area in bytes
  KDEVPLATFORMLANGUAGE_EXPORT void enableDUChainReferenceCounting(void* start, unsigned int size);
  ///Must be called as often as enableDUChainReferenceCounting, with the same ranges
  ///Must never be called for the same range twice, and not for overlapping ranges
  ///@param start Position where to start the reference-counting
  ///@param size Size of the area in bytes
  KDEVPLATFORMLANGUAGE_EXPORT void disableDUChainReferenceCounting(void* start);
  
  ///Use this as local variable within the object that maintains the reference-count,
  ///and use
  struct ReferenceCountManager {
    #ifndef TEST_REFERENCE_COUNTING
    inline void increase(uint& ref, uint /*targetId*/) {
      ++ref;
    }
    inline void decrease(uint& ref, uint /*targetId*/) {
      Q_ASSERT(ref);
      --ref;
    }
    
    #else
    
    ReferenceCountManager();
    ~ReferenceCountManager();

    ReferenceCountManager(const ReferenceCountManager& rhs);
    ReferenceCountManager& operator=(const ReferenceCountManager& rhs);
    
    void increase(uint& ref, uint targetId);
    void decrease(uint& ref, uint targetId);

//     bool hasReferenceCount() const;
    
    uint m_id;
    #endif
  };
}

#endif
