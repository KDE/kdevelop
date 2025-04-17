/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "definitions.h"

#include "appendedlist.h"
#include "declaration.h"
#include "declarationid.h"
#include "duchainpointer.h"
#include <serialization/indexedstring.h>
#include "serialization/itemrepository.h"

namespace KDevelop {
DEFINE_LIST_MEMBER_HASH(DefinitionsItem, definitions, IndexedDeclaration)

class DefinitionsItem
{
public:
    DefinitionsItem()
    {
        initializeAppendedLists();
    }
    DefinitionsItem(const DefinitionsItem& rhs, bool dynamic = true) : declaration(rhs.declaration)
    {
        initializeAppendedLists(dynamic);
        copyListsFrom(rhs);
    }

    ~DefinitionsItem()
    {
        freeAppendedLists();
    }

    DefinitionsItem& operator=(const DefinitionsItem& rhs) = delete;

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
        return sizeof(DefinitionsItem);
    }

    DeclarationId declaration;

    START_APPENDED_LISTS(DefinitionsItem);
    APPENDED_LIST_FIRST(DefinitionsItem, IndexedDeclaration, definitions);
    END_APPENDED_LISTS(DefinitionsItem, definitions);
};

class DefinitionsRequestItem
{
public:

    DefinitionsRequestItem(const DefinitionsItem& item) : m_item(item)
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

    void createItem(DefinitionsItem* item) const
    {
        new (item) DefinitionsItem(m_item, false);
    }

    static void destroy(DefinitionsItem* item, KDevelop::AbstractItemRepository&)
    {
        item->~DefinitionsItem();
    }

    static bool persistent(const DefinitionsItem*)
    {
        return true;
    }

    bool equals(const DefinitionsItem* item) const
    {
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
        const auto allDefinitions = definitions->definitions(id);

        qout << "Definitions for" << id.qualifiedIdentifier() << Qt::endl;
        for (const IndexedDeclaration& decl : allDefinitions) {
            if (decl.data()) {
                qout << " " << decl.data()->qualifiedIdentifier() << "in" << decl.data()->url().byteArray() << "at"
                     << decl.data()->rangeInCurrentRevision() << Qt::endl;
            }
        }

        return true;
    }

private:
    const Definitions* definitions;
    const QTextStream& out;
};

// Maps declaration-ids to definitions
using DefinitionsRepo = ItemRepository<DefinitionsItem, DefinitionsRequestItem>;

template<>
class ItemRepositoryFor<Definitions>
{
    friend struct LockedItemRepository;
    static DefinitionsRepo& repo()
    {
        static QMutex mutex;
        static DefinitionsRepo repo { QStringLiteral("Definition Map"), &mutex };
        return repo;
    }
};

Definitions::Definitions()
{
    LockedItemRepository::initialize<Definitions>();
}

void Definitions::addDefinition(const DeclarationId& id, const IndexedDeclaration& definition)
{
    DefinitionsItem item;
    item.declaration = id;
    item.definitionsList().append(definition);
    DefinitionsRequestItem request(item);

    LockedItemRepository::write<Definitions>([&](DefinitionsRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            // Check whether the item is already in the mapped list, else copy the list into the new created item
            const DefinitionsItem* oldItem = repo.itemFromIndex(index);
            for (unsigned int a = 0; a < oldItem->definitionsSize(); ++a) {
                if (oldItem->definitions()[a] == definition)
                    return; // Already there
                item.definitionsList().append(oldItem->definitions()[a]);
            }

            repo.deleteItem(index);
        }

        // This inserts the changed item
        repo.index(request);
    });
}

void Definitions::removeDefinition(const DeclarationId& id, const IndexedDeclaration& definition)
{
    DefinitionsItem item;
    item.declaration = id;
    DefinitionsRequestItem request(item);

    LockedItemRepository::write<Definitions>([&](DefinitionsRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            // Check whether the item is already in the mapped list, else copy the list into the new created item
            const DefinitionsItem* oldItem = repo.itemFromIndex(index);
            for (unsigned int a = 0; a < oldItem->definitionsSize(); ++a)
                if (!(oldItem->definitions()[a] == definition))
                    item.definitionsList().append(oldItem->definitions()[a]);

            repo.deleteItem(index);
            Q_ASSERT(repo.findIndex(item) == 0);

            // This inserts the changed item
            if (item.definitionsSize() != 0)
                repo.index(request);
        }
    });
}

KDevVarLengthArray<IndexedDeclaration> Definitions::definitions(const DeclarationId& id) const
{
    KDevVarLengthArray<IndexedDeclaration> ret;

    DefinitionsItem item;
    item.declaration = id;

    LockedItemRepository::read<Definitions>([&](const DefinitionsRepo& repo) {
        uint index = repo.findIndex(item);

        if (index) {
            const DefinitionsItem* repositoryItem = repo.itemFromIndex(index);
            FOREACH_FUNCTION(const IndexedDeclaration& decl, repositoryItem->definitions)
            ret.append(decl);
        }
    });

    return ret;
}

void Definitions::dump(const QTextStream& out)
{
    DefinitionsVisitor v(this, out);
    LockedItemRepository::read<Definitions>([&](const DefinitionsRepo& repo) {
        repo.visitAllItems(v);
    });
}
}
