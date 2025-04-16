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

#include <type_traits>

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

/// Maps declaration-ids to definitions
using DefinitionsRepo = ItemRepository<DefinitionsItem, DefinitionsRequestItem>;

template<>
class ItemRepositoryFor<Definitions>
{
    friend struct LockedItemRepository;
    static DefinitionsRepo& repo()
    {
        static QMutex mutex;
        static DefinitionsRepo repo{QStringLiteral("Definition Map"), &mutex};
        return repo;
    }
};

namespace {
template<typename Repo>
[[nodiscard]] KDevVarLengthArray<IndexedDeclaration> definitionsInRepo(const DeclarationId& id, const Repo& repo)
{
    KDevVarLengthArray<IndexedDeclaration> ret;

    DefinitionsItem item;
    item.declaration = id;
    const auto collectDefinitions = [&item, &ret](const DefinitionsRepo& repo) {
        if (const auto index = repo.findIndex(item)) {
            const auto* const repositoryItem = repo.itemFromIndex(index);
            FOREACH_FUNCTION (const IndexedDeclaration& decl, repositoryItem->definitions)
                ret.append(decl);
        }
    };

    if constexpr (std::is_same_v<Repo, std::nullptr_t>) {
        LockedItemRepository::read<Definitions>(collectDefinitions);
    } else {
        static_assert(std::is_same_v<Repo, DefinitionsRepo>);
        collectDefinitions(repo);
    }

    return ret;
}

} // unnamed namespace

class DefinitionsVisitor
{
public:
    explicit DefinitionsVisitor(const QTextStream& _out)
        : out(_out)
    {
    }

    void setRepo(const DefinitionsRepo& repo)
    {
        Q_ASSERT(!m_repo);
        m_repo = &repo;
    }

    bool operator()(const DefinitionsItem* item)
    {
        Q_ASSERT(m_repo);

        QDebug qout(out.device());
        auto id = item->declaration;

        // DefinitionsRepo is locked during the visiting. The mutex of DefinitionsRepo is nonrecursive.
        // Pass the already locked repository to definitionsInRepo() in order to prevent a deadlock.
        const auto allDefinitions = definitionsInRepo(id, *m_repo);

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
    const QTextStream& out;
    const DefinitionsRepo* m_repo = nullptr;
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
    return definitionsInRepo(id, nullptr);
}

void Definitions::dump(const QTextStream& out)
{
    DefinitionsVisitor v(out);
    LockedItemRepository::read<Definitions>([&](const DefinitionsRepo& repo) {
        v.setRepo(repo);
        repo.visitAllItems(v);
    });
}
}
