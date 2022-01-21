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

    unsigned int hash() const
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

    unsigned int hash() const
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

class UsesPrivate
{
public:
    mutable QMutex m_mutex;
    //Maps declaration-ids to Uses
    using Repo = ItemRepository<UsesItem, UsesRequestItem, true, false>;
    // mutable as things like findIndex are not const
    mutable Repo m_uses{QStringLiteral("Use Map"), &m_mutex};
};

Uses::Uses()
    : d_ptr(new UsesPrivate())
{
}

Uses::~Uses() = default;

void Uses::addUse(const DeclarationId& id, const IndexedTopDUContext& use)
{
    Q_D(Uses);

    UsesItem item;
    item.declaration = id;
    item.usesList().append(use);
    UsesRequestItem request(item);

    QMutexLocker lock(&d->m_mutex);
    uint index = d->m_uses.findIndex(item);

    if (index) {
        //Check whether the item is already in the mapped list, else copy the list into the new created item
        const UsesItem* oldItem = d->m_uses.itemFromIndex(index);
        for (unsigned int a = 0; a < oldItem->usesSize(); ++a) {
            if (oldItem->uses()[a] == use)
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
    Q_D(Uses);

    UsesItem item;
    item.declaration = id;
    UsesRequestItem request(item);

    QMutexLocker lock(&d->m_mutex);
    uint index = d->m_uses.findIndex(item);

    if (index) {
        //Check whether the item is already in the mapped list, else copy the list into the new created item
        const UsesItem* oldItem = d->m_uses.itemFromIndex(index);
        for (unsigned int a = 0; a < oldItem->usesSize(); ++a)
            if (!(oldItem->uses()[a] == use))
                item.usesList().append(oldItem->uses()[a]);

        d->m_uses.deleteItem(index);
        Q_ASSERT(d->m_uses.findIndex(item) == 0);

        //This inserts the changed item
        if (item.usesSize() != 0)
            d->m_uses.index(request);
    }
}

bool Uses::hasUses(const DeclarationId& id) const
{
    Q_D(const Uses);

    UsesItem item;
    item.declaration = id;

    QMutexLocker lock(&d->m_mutex);
    return ( bool ) d->m_uses.findIndex(item);
}

KDevVarLengthArray<IndexedTopDUContext> Uses::uses(const DeclarationId& id) const
{
    Q_D(const Uses);

    KDevVarLengthArray<IndexedTopDUContext> ret;

    UsesItem item;
    item.declaration = id;
    UsesRequestItem request(item);

    QMutexLocker lock(&d->m_mutex);
    uint index = d->m_uses.findIndex(item);

    if (index) {
        const UsesItem* repositoryItem = d->m_uses.itemFromIndex(index);
        FOREACH_FUNCTION(const IndexedTopDUContext &decl, repositoryItem->uses)
        ret.append(decl);
    }

    return ret;
}
}
