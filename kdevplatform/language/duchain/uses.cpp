/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "uses.h"

#include "declarationid.h"
#include "duchainpointer.h"
#include "serialization/itemrepository.h"
#include "topducontext.h"

namespace KDevelop {
DEFINE_LIST_MEMBER_HASH(UsesItem, uses, IndexedTopDUContext)

class UsesItem
{
public:
    UsesItem()
    {
        initializeAppendedLists();
    }
    UsesItem(const UsesItem& rhs, bool dynamic = true) : declaration(rhs.declaration)
    {
        initializeAppendedLists(dynamic);
        copyListsFrom(rhs);
    }

    ~UsesItem()
    {
        freeAppendedLists();
    }

    UsesItem& operator=(const UsesItem& rhs) = delete;

    size_t hash() const
    {
        //We only compare the declaration. This allows us implementing a map, although the item-repository
        //originally represents a set.
        return declaration.hash();
    }

    unsigned int itemSize() const
    {
        return dynamicSize();
    }

    uint classSize() const
    {
        return sizeof(UsesItem);
    }

    DeclarationId declaration;

    START_APPENDED_LISTS(UsesItem);
    APPENDED_LIST_FIRST(UsesItem, IndexedTopDUContext, uses);
    END_APPENDED_LISTS(UsesItem, uses);
};

class UsesRequestItem
{
public:

    UsesRequestItem(const UsesItem& item) : m_item(item)
    {
    }
    enum {
        AverageSize = 30 //This should be the approximate average size of an Item
    };

    size_t hash() const
    {
        return m_item.hash();
    }

    uint itemSize() const
    {
        return m_item.itemSize();
    }

    void createItem(UsesItem* item) const
    {
        new (item) UsesItem(m_item, false);
    }

    static void destroy(UsesItem* item, KDevelop::AbstractItemRepository&)
    {
        item->~UsesItem();
    }

    static bool persistent(const UsesItem* /*item*/)
    {
        return true;
    }

    bool equals(const UsesItem* item) const
    {
        return m_item.declaration == item->declaration;
    }

    const UsesItem& m_item;
};

// Maps declaration-ids to Uses
using UsesRepo = ItemRepository<UsesItem, UsesRequestItem>;
template<>
class ItemRepositoryFor<Uses>
{
    friend struct LockedItemRepository;
    static UsesRepo& repo()
    {
        static QMutex mutex;
        static UsesRepo repo { QStringLiteral("Use Map"), &mutex };
        return repo;
    }
};

Uses::Uses()
{
    LockedItemRepository::initialize<Uses>();
}

void Uses::addUse(const DeclarationId& id, const IndexedTopDUContext& use)
{
    UsesItem item;
    item.declaration = id;
    item.usesList().append(use);
    UsesRequestItem request(item);

    LockedItemRepository::write<Uses>([&](UsesRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            // Check whether the item is already in the mapped list, else copy the list into the new created item
            const UsesItem* oldItem = repo.itemFromIndex(index);
            for (unsigned int a = 0; a < oldItem->usesSize(); ++a) {
                if (oldItem->uses()[a] == use)
                    return; // Already there
                item.usesList().append(oldItem->uses()[a]);
            }

            repo.deleteItem(index);
        }

        // This inserts the changed item
        repo.index(request);
    });
}

void Uses::removeUse(const DeclarationId& id, const IndexedTopDUContext& use)
{
    UsesItem item;
    item.declaration = id;
    UsesRequestItem request(item);

    LockedItemRepository::write<Uses>([&](UsesRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            // Check whether the item is already in the mapped list, else copy the list into the new created item
            const UsesItem* oldItem = repo.itemFromIndex(index);
            for (unsigned int a = 0; a < oldItem->usesSize(); ++a)
                if (!(oldItem->uses()[a] == use))
                    item.usesList().append(oldItem->uses()[a]);

            repo.deleteItem(index);
            Q_ASSERT(repo.findIndex(item) == 0);

            // This inserts the changed item
            if (item.usesSize() != 0)
                repo.index(request);
        }
    });
}

bool Uses::hasUses(const DeclarationId& id) const
{
    UsesItem item;
    item.declaration = id;

    return LockedItemRepository::read<Uses>([&item](const UsesRepo& repo) {
        return static_cast<bool>(repo.findIndex(item));
    });
}

KDevVarLengthArray<IndexedTopDUContext> Uses::uses(const DeclarationId& id) const
{
    KDevVarLengthArray<IndexedTopDUContext> ret;

    UsesItem item;
    item.declaration = id;
    UsesRequestItem request(item);

    LockedItemRepository::read<Uses>([&](const UsesRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            const UsesItem* repositoryItem = repo.itemFromIndex(index);
            FOREACH_FUNCTION(const IndexedTopDUContext& decl, repositoryItem->uses)
            ret.append(decl);
        }
    });

    return ret;
}
}
