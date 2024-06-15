/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "importers.h"

#include "declarationid.h"
#include "duchainpointer.h"
#include "serialization/itemrepository.h"
#include "topducontext.h"

namespace KDevelop {
DEFINE_LIST_MEMBER_HASH(ImportersItem, importers, IndexedDUContext)

class ImportersItem
{
public:
    ImportersItem()
    {
        initializeAppendedLists();
    }
    ImportersItem(const ImportersItem& rhs, bool dynamic = true) : declaration(rhs.declaration)
    {
        initializeAppendedLists(dynamic);
        copyListsFrom(rhs);
    }

    ~ImportersItem()
    {
        freeAppendedLists();
    }

    ImportersItem& operator=(const ImportersItem& rhs) = delete;

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
        return sizeof(ImportersItem);
    }

    DeclarationId declaration;

    START_APPENDED_LISTS(ImportersItem);
    APPENDED_LIST_FIRST(ImportersItem, IndexedDUContext, importers);
    END_APPENDED_LISTS(ImportersItem, importers);
};

class ImportersRequestItem
{
public:

    ImportersRequestItem(const ImportersItem& item) : m_item(item)
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

    void createItem(ImportersItem* item) const
    {
        new (item) ImportersItem(m_item, false);
    }

    static void destroy(ImportersItem* item, KDevelop::AbstractItemRepository&)
    {
        item->~ImportersItem();
    }

    static bool persistent(const ImportersItem* /*item*/)
    {
        return true;
    }

    bool equals(const ImportersItem* item) const
    {
        return m_item.declaration == item->declaration;
    }

    const ImportersItem& m_item;
};

// Maps declaration-ids to Importers
using ImportersRepo = ItemRepository<ImportersItem, ImportersRequestItem>;

template<>
class ItemRepositoryFor<Importers>
{
    friend struct LockedItemRepository;
    static ImportersRepo& repo()
    {
        static QMutex mutex;
        static ImportersRepo repo { QStringLiteral("Importer Map"), &mutex };
        return repo;
    }
};

Importers::Importers()
{
    LockedItemRepository::initialize<Importers>();
}

void Importers::addImporter(const DeclarationId& id, const IndexedDUContext& use)
{
    ImportersItem item;
    item.declaration = id;
    item.importersList().append(use);
    ImportersRequestItem request(item);

    LockedItemRepository::write<Importers>([&](ImportersRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            // Check whether the item is already in the mapped list, else copy the list into the new created item
            const ImportersItem* oldItem = repo.itemFromIndex(index);
            for (unsigned int a = 0; a < oldItem->importersSize(); ++a) {
                if (oldItem->importers()[a] == use)
                    return; // Already there
                item.importersList().append(oldItem->importers()[a]);
            }

            repo.deleteItem(index);
        }

        // This inserts the changed item
        repo.index(request);
    });
}

void Importers::removeImporter(const DeclarationId& id, const IndexedDUContext& use)
{
    ImportersItem item;
    item.declaration = id;
    ImportersRequestItem request(item);

    LockedItemRepository::write<Importers>([&](ImportersRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            // Check whether the item is already in the mapped list, else copy the list into the new created item
            const ImportersItem* oldItem = repo.itemFromIndex(index);
            for (unsigned int a = 0; a < oldItem->importersSize(); ++a)
                if (!(oldItem->importers()[a] == use))
                    item.importersList().append(oldItem->importers()[a]);

            repo.deleteItem(index);
            Q_ASSERT(repo.findIndex(item) == 0);

            // This inserts the changed item
            if (item.importersSize() != 0)
                repo.index(request);
        }
    });
}

KDevVarLengthArray<IndexedDUContext> Importers::importers(const DeclarationId& id) const
{
    KDevVarLengthArray<IndexedDUContext> ret;

    ImportersItem item;
    item.declaration = id;
    ImportersRequestItem request(item);

    LockedItemRepository::read<Importers>([&](const ImportersRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            const ImportersItem* repositoryItem = repo.itemFromIndex(index);
            FOREACH_FUNCTION(const IndexedDUContext& decl, repositoryItem->importers)
            ret.append(decl);
        }
    });

    return ret;
}

Importers& Importers::self()
{
    static Importers globalImporters;
    return globalImporters;
}
}
