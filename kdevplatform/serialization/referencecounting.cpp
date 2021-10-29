/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "referencecounting.h"
#include "itemrepository.h"

#include <QAtomicInt>

#include <algorithm>

namespace KDevelop {
void DUChainReferenceCounting::Interval::assign(Pointer newStart, unsigned newSize) noexcept
{
    start = newStart;
    size = newSize;
    refCount = 1;
}

auto DUChainReferenceCounting::findInterval(Pointer start, unsigned size) noexcept -> Interval*
{
    return std::find_if(intervals, intervals + count, [start, size](Interval interval) {
        return interval.start == start && interval.size == size;
    });
}

void DUChainReferenceCounting::enable(Pointer start, unsigned size)
{
    auto* const interval = findInterval(start, size);
    if (interval == intervals + count) {
        if (count == maxIntervalCount) {
            qFatal("DUChainReferenceCounting interval count limit of %zu exceeded!", count);
        }
        // "push_back"
        Q_ASSERT(count < maxIntervalCount);
        interval->assign(start, size);
        ++count;
    } else {
        Q_ASSERT(interval < intervals + count);
        ++interval->refCount;
    }

#ifdef TEST_REFERENCE_COUNTING
    Q_ASSERT(shouldDo(start));
    Q_ASSERT(shouldDo(start + size - 1));
#endif
}

void DUChainReferenceCounting::disable(Pointer start, unsigned size)
{
    auto* const interval = findInterval(start, size);
    Q_ASSERT(interval < intervals + count);

    if (interval->refCount == 1) {
        // "erase" interval
        std::move(interval + 1, intervals + count, interval);
        --count;
    } else {
        Q_ASSERT(interval->refCount > 1);
        --interval->refCount;
    }
}

void enableDUChainReferenceCounting(const void* start, unsigned size)
{
    DUChainReferenceCounting::instance().enable(reinterpret_cast<DUChainReferenceCounting::Pointer>(start), size);
}

void disableDUChainReferenceCounting(const void* start, unsigned size)
{
    DUChainReferenceCounting::instance().disable(reinterpret_cast<DUChainReferenceCounting::Pointer>(start), size);
}
}

#ifdef TEST_REFERENCE_COUNTING

QAtomicInt& id()
{
    static QAtomicInt& ret(KDevelop::globalItemRepositoryRegistry().getCustomCounter("referencer ids", 1));
    return ret;
}

namespace KDevelop {
ReferenceCountManager::ReferenceCountManager() : m_id(id().fetchAndAddRelaxed(1))
{
}

struct ReferenceCountItem
{
    ///Item entries:
    ReferenceCountItem(uint id, uint target) : m_id(id)
        , m_targetId(target)
    {
    }

    ReferenceCountItem& operator=(const ReferenceCountItem& rhs) = delete;

    //Every item has to implement this function, and return a valid hash.
    //Must be exactly the same hash value as ReferenceCountItemRequest::hash() has returned while creating the item.
    unsigned int hash() const
    {
        return KDevHash() << m_id << m_targetId;
    }

    //Every item has to implement this function, and return the complete size this item takes in memory.
    //Must be exactly the same value as ReferenceCountItemRequest::itemSize() has returned while creating the item.
    unsigned short int itemSize() const
    {
        return sizeof(ReferenceCountItem);
    }

    uint m_id;
    uint m_targetId;

    ///Request entries:
    enum {
        AverageSize = 8
    };

    void createItem(ReferenceCountItem* item) const
    {
        * item = *this;
    }
    static void destroy(ReferenceCountItem* /*item*/, AbstractItemRepository&)
    {
    }

    static bool persistent(const ReferenceCountItem*)
    {
        return true;
    }

    bool equals(const ReferenceCountItem* item) const
    {
        return m_id == item->m_id && m_targetId == item->m_targetId;
    }
};

static RepositoryManager<ItemRepository<ReferenceCountItem, ReferenceCountItem, false, true,
        sizeof(ReferenceCountItem)>, false>& references()
{
    static RepositoryManager<ItemRepository<ReferenceCountItem, ReferenceCountItem, false, true,
            sizeof(ReferenceCountItem)>, false> referencesObject("Reference Count Debugging");
    return referencesObject;
}
static RepositoryManager<ItemRepository<ReferenceCountItem, ReferenceCountItem, false, true,
        sizeof(ReferenceCountItem)>, false>& oldReferences()
{
    static RepositoryManager<ItemRepository<ReferenceCountItem, ReferenceCountItem, false, true,
            sizeof(ReferenceCountItem)>, false> oldReferencesObject("Old Reference Count Debugging");
    return oldReferencesObject;
}

void KDevelop::initReferenceCounting()
{
    references();
    oldReferences();
}

ReferenceCountManager::~ReferenceCountManager()
{
    //Make sure everything is cleaned up when the object is destroyed
//   Q_ASSERT(!references().contains(m_id));
}

ReferenceCountManager::ReferenceCountManager(const ReferenceCountManager& rhs) : m_id(id().fetchAndAddRelaxed(1))
{
    //New id
}

ReferenceCountManager& ReferenceCountManager::ReferenceCountManager::operator=(const ReferenceCountManager& rhs)
{
    //Keep id
    return *this;
}

// bool ReferenceCountManager::hasReferenceCount() const {
//   return references->findIndex(ReferenceCountItem);
// }

void ReferenceCountManager::increase(uint& ref, uint targetId)
{
    Q_ASSERT(shouldDoDUChainReferenceCounting(this));
    Q_ASSERT(!references->findIndex(ReferenceCountItem(m_id, targetId)));
    ++ref;

    {
        int oldIndex = oldReferences->findIndex(ReferenceCountItem(m_id, targetId));
        if (oldIndex)
            oldReferences->deleteItem(oldIndex);
    }

    Q_ASSERT(references->index(ReferenceCountItem(m_id, targetId)));
}

void ReferenceCountManager::decrease(uint& ref, uint targetId)
{
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
void initReferenceCounting()
{
}
}
#endif
