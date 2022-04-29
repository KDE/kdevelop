/*
    SPDX-FileCopyrightText: 2022 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ITEMREPOSITORYREFERENCECOUNTING_H
#define KDEVPLATFORM_ITEMREPOSITORYREFERENCECOUNTING_H

#include "itemrepository.h"

namespace KDevelop {

struct ItemRepositoryReferenceCounting {
    template <typename Item>
    static inline bool inc(Item* item)
    {
        Q_ASSERT(item);
        const auto index = item->index();
        Q_ASSERT(index);

        if (!shouldDoDUChainReferenceCounting(item)) {
            return false;
        }

        LockedItemRepository::write<Item>(
            [&](auto& repo) { item->increase(repo.dynamicItemFromIndexSimple(index)->m_refCount, index); });
        return true;
    }

    template <typename Item>
    static inline bool dec(Item* item)
    {
        Q_ASSERT(item);
        const auto index = item->index();
        Q_ASSERT(index);

        if (!shouldDoDUChainReferenceCounting(item)) {
            return false;
        }

        LockedItemRepository::write<Item>(
            [&](auto& repo) { item->decrease(repo.dynamicItemFromIndexSimple(index)->m_refCount, index); });
        return true;
    }

    struct AssumeValidIndex {
        bool operator()(unsigned int index) const
        {
            Q_ASSERT(index);
            return true;
        }
    };

    template <typename Item, typename CheckIndex = AssumeValidIndex>
    static inline void setIndex(Item* item, unsigned int& m_index, unsigned int index,
                                CheckIndex checkIndex = AssumeValidIndex{})
    {
        Q_ASSERT(item);
        if (m_index == index) {
            return;
        }

        if (shouldDoDUChainReferenceCounting(item)) {
            LockedItemRepository::write<Item>([&](auto& repo) {
                if (checkIndex(m_index)) {
                    item->decrease(repo.dynamicItemFromIndexSimple(m_index)->m_refCount, m_index);
                }

                m_index = index;

                if (checkIndex(m_index)) {
                    item->increase(repo.dynamicItemFromIndexSimple(m_index)->m_refCount, m_index);
                }
            });
        } else {
            m_index = index;
        }
    }
};

}

#endif
