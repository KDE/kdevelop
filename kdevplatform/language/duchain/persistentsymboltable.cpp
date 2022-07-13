/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "persistentsymboltable.h"

#include <QHash>

#include "declaration.h"
#include "declarationid.h"
#include "appendedlist.h"
#include "serialization/itemrepository.h"
#include "identifier.h"
#include "ducontext.h"
#include "topducontext.h"
#include "duchain.h"
#include "duchainlock.h"

#include <util/convenientfreelist.h>
#include <util/embeddedfreetree.h>

#include <language/util/setrepository.h>

namespace KDevelop {

namespace {
// For now, just _always_ use the cache
const uint MinimumCountForCache = 1;

using TextStreamFunction = QTextStream& (*)(QTextStream&);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
constexpr TextStreamFunction endl = Qt::endl;
#else
constexpr TextStreamFunction endl = ::endl;
#endif

QDebug fromTextStream(const QTextStream& out)
{
    if (out.device())
        return {out.device()};
    return {out.string()};
}

struct IndexedDeclarationHandler {
    inline static int leftChild(const IndexedDeclaration& m_data) { return ((int)(m_data.dummyData().first)) - 1; }
    inline static void setLeftChild(IndexedDeclaration& m_data, int child)
    {
        m_data.setDummyData(qMakePair((uint)(child + 1), m_data.dummyData().second));
    }
    inline static int rightChild(const IndexedDeclaration& m_data) { return ((int)m_data.dummyData().second) - 1; }
    inline static void setRightChild(IndexedDeclaration& m_data, int child)
    {
        m_data.setDummyData(qMakePair(m_data.dummyData().first, (uint)(child + 1)));
    }
    inline static void createFreeItem(IndexedDeclaration& data)
    {
        data = IndexedDeclaration();
        data.setIsDummy(true);
        data.setDummyData(qMakePair(0u, 0u)); // Since we subtract 1, this equals children -1, -1
    }
    // Copies this item into the given one
    inline static void copyTo(const IndexedDeclaration& m_data, IndexedDeclaration& data) { data = m_data; }

    inline static bool isFree(const IndexedDeclaration& m_data) { return m_data.isDummy(); }

    inline static bool equals(const IndexedDeclaration& m_data, const IndexedDeclaration& rhs) { return m_data == rhs; }
};

struct DeclarationTopContextExtractor {
    inline static IndexedTopDUContext extract(const IndexedDeclaration& decl) { return decl.indexedTopContext(); }
};

struct DUContextTopContextExtractor {
    inline static IndexedTopDUContext extract(const IndexedDUContext& ctx) { return ctx.indexedTopContext(); }
};

struct RecursiveImportCacheRepository {
    static Utils::BasicSetRepository* repository()
    {
        static QRecursiveMutex mutex;
        static Utils::BasicSetRepository recursiveImportCacheRepositoryObject(QStringLiteral("Recursive Imports Cache"),
                                                                              &mutex, nullptr, false);
        return &recursiveImportCacheRepositoryObject;
    }
};

DEFINE_LIST_MEMBER_HASH(PersistentSymbolTableItem, declarations, IndexedDeclaration)

class PersistentSymbolTableItem
{
public:
    PersistentSymbolTableItem() : centralFreeItem(-1)
    {
        initializeAppendedLists();
    }
    PersistentSymbolTableItem(const PersistentSymbolTableItem& rhs, bool dynamic = true) : id(rhs.id)
        , centralFreeItem(rhs.centralFreeItem)
    {
        initializeAppendedLists(dynamic);
        copyListsFrom(rhs);
    }

    ~PersistentSymbolTableItem()
    {
        freeAppendedLists();
    }

    PersistentSymbolTableItem& operator=(const PersistentSymbolTableItem& rhs) = delete;

    inline unsigned int hash() const
    {
        //We only compare the declaration. This allows us implementing a map, although the item-repository
        //originally represents a set.
        return id.index();
    }

    unsigned int itemSize() const
    {
        return dynamicSize();
    }

    uint classSize() const
    {
        return sizeof(PersistentSymbolTableItem);
    }

    IndexedQualifiedIdentifier id;
    int centralFreeItem;

    START_APPENDED_LISTS(PersistentSymbolTableItem);
    APPENDED_LIST_FIRST(PersistentSymbolTableItem, IndexedDeclaration, declarations);
    END_APPENDED_LISTS(PersistentSymbolTableItem, declarations);
};

class PersistentSymbolTableRequestItem
{
public:

    PersistentSymbolTableRequestItem(const PersistentSymbolTableItem& item) : m_item(item)
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

    void createItem(PersistentSymbolTableItem* item) const
    {
        new (item) PersistentSymbolTableItem(m_item, false);
    }

    static void destroy(PersistentSymbolTableItem* item, KDevelop::AbstractItemRepository&)
    {
        item->~PersistentSymbolTableItem();
    }

    static bool persistent(const PersistentSymbolTableItem*)
    {
        return true; //Nothing to do
    }

    bool equals(const PersistentSymbolTableItem* item) const
    {
        return m_item.id == item->id;
    }

    const PersistentSymbolTableItem& m_item;
};

using CachedDeclarations = KDevVarLengthArray<IndexedDeclaration>;
using CachedDeclarationsByImports = QHash<TopDUContext::IndexedRecursiveImports, CachedDeclarations>;
using Declarations = ConstantConvenientEmbeddedSet<IndexedDeclaration, IndexedDeclarationHandler>;
using CachedIndexedRecursiveImports =
    Utils::StorableSet<IndexedTopDUContext, IndexedTopDUContextIndexConversion, RecursiveImportCacheRepository, true>;
using FilteredDeclarationIterator =
    ConvenientEmbeddedSetTreeFilterIterator<IndexedDeclaration, IndexedDeclarationHandler, IndexedTopDUContext,
                                            CachedIndexedRecursiveImports, DeclarationTopContextExtractor>;

// Maps declaration-ids to declarations, together with some caches
class PersistentSymbolTableRepo : public ItemRepository<PersistentSymbolTableItem, PersistentSymbolTableRequestItem>
{
    using ItemRepository::ItemRepository;

public:
    QHash<IndexedQualifiedIdentifier, CachedDeclarationsByImports> declarationsCache;

    // We cache the imports so the currently used nodes are very close in memory, which leads to much better CPU cache
    // utilization
    QHash<TopDUContext::IndexedRecursiveImports, CachedIndexedRecursiveImports> importsCache;
};
}

template<>
class ItemRepositoryFor<PersistentSymbolTable>
{
    friend struct LockedItemRepository;
    static PersistentSymbolTableRepo& repo()
    {
        static QMutex mutex;
        static PersistentSymbolTableRepo repo { QStringLiteral("Persistent Declaration Table"), &mutex };
        return repo;
    }

public:
    static void init() { repo(); }
};

void PersistentSymbolTable::clearCache()
{
    LockedItemRepository::write<PersistentSymbolTable>([](PersistentSymbolTableRepo& repo) {
        repo.importsCache.clear();
        repo.declarationsCache.clear();
    });
}

PersistentSymbolTable::PersistentSymbolTable()
{
    ItemRepositoryFor<PersistentSymbolTable>::init();
    RecursiveImportCacheRepository::repository();
}

PersistentSymbolTable::~PersistentSymbolTable() = default;

void PersistentSymbolTable::addDeclaration(const IndexedQualifiedIdentifier& id, const IndexedDeclaration& declaration)
{
    ENSURE_CHAIN_WRITE_LOCKED

    PersistentSymbolTableItem item;
    item.id = id;

    LockedItemRepository::write<PersistentSymbolTable>([&item, &declaration](PersistentSymbolTableRepo& repo) {
        repo.declarationsCache.remove(item.id);

        uint index = repo.findIndex(item);

        if (index) {
            // Check whether the item is already in the mapped list, else copy the list into the new created item
            const PersistentSymbolTableItem* oldItem = repo.itemFromIndex(index);

            EmbeddedTreeAlgorithms<IndexedDeclaration, IndexedDeclarationHandler> alg(
                oldItem->declarations(), oldItem->declarationsSize(), oldItem->centralFreeItem);

            if (alg.indexOf(declaration) != -1)
                return;

            DynamicItem<PersistentSymbolTableItem, true> editableItem = repo.dynamicItemFromIndex(index);

            EmbeddedTreeAddItem<IndexedDeclaration, IndexedDeclarationHandler> add(
                const_cast<IndexedDeclaration*>(editableItem->declarations()), editableItem->declarationsSize(),
                editableItem->centralFreeItem, declaration);

            uint newSize = add.newItemCount();
            if (newSize != editableItem->declarationsSize()) {
                // We need to resize. Update and fill the new item, and delete the old item.
                item.declarationsList().resize(newSize);
                add.transferData(item.declarationsList().data(), newSize, &item.centralFreeItem);

                repo.deleteItem(index);
                Q_ASSERT(!repo.findIndex(item));
            } else {
                // We're fine, the item could be added to the existing list
                return;
            }
        } else {
            item.declarationsList().append(declaration);
        }

        // This inserts the changed item
        repo.index(item);
    });
}

void PersistentSymbolTable::removeDeclaration(const IndexedQualifiedIdentifier& id,
                                              const IndexedDeclaration& declaration)
{
    ENSURE_CHAIN_WRITE_LOCKED

    PersistentSymbolTableItem item;
    item.id = id;

    LockedItemRepository::write<PersistentSymbolTable>([&item, &declaration](PersistentSymbolTableRepo& repo) {
        repo.declarationsCache.remove(item.id);

        uint index = repo.findIndex(item);

        if (index) {
            // Check whether the item is already in the mapped list, else copy the list into the new created item
            const PersistentSymbolTableItem* oldItem = repo.itemFromIndex(index);

            EmbeddedTreeAlgorithms<IndexedDeclaration, IndexedDeclarationHandler> alg(
                oldItem->declarations(), oldItem->declarationsSize(), oldItem->centralFreeItem);

            if (alg.indexOf(declaration) == -1)
                return;

            DynamicItem<PersistentSymbolTableItem, true> editableItem = repo.dynamicItemFromIndex(index);

            EmbeddedTreeRemoveItem<IndexedDeclaration, IndexedDeclarationHandler> remove(
                const_cast<IndexedDeclaration*>(editableItem->declarations()), editableItem->declarationsSize(),
                editableItem->centralFreeItem, declaration);

            uint newSize = remove.newItemCount();
            if (newSize != editableItem->declarationsSize()) {
                // We need to resize. Update and fill the new item, and delete the old item.
                item.declarationsList().resize(newSize);
                remove.transferData(item.declarationsList().data(), newSize, &item.centralFreeItem);

                repo.deleteItem(index);
                Q_ASSERT(!repo.findIndex(item));
            } else {
                // We're fine, the item could be added to the existing list
                return;
            }
        }

        // This inserts the changed item
        if (item.declarationsSize())
            repo.index(item);
    });
}

void PersistentSymbolTable::visitDeclarations(const IndexedQualifiedIdentifier& id,
                                              const DeclarationVisitor& visitor) const
{
    ENSURE_CHAIN_READ_LOCKED

    PersistentSymbolTableItem item;
    item.id = id;

    LockedItemRepository::read<PersistentSymbolTable>([&item, &visitor](const PersistentSymbolTableRepo& repo) {
        uint index = repo.findIndex(item);

        if (!index) {
            return;
        }

        const PersistentSymbolTableItem* repositoryItem = repo.itemFromIndex(index);

        const auto numDeclarations = repositoryItem->declarationsSize();
        const auto declarations = repositoryItem->declarations();
        for (uint i = 0; i < numDeclarations; ++i) {
            if (visitor(declarations[i]) == VisitorState::Break) {
                break;
            }
        }
    });
}

void PersistentSymbolTable::visitFilteredDeclarations(const IndexedQualifiedIdentifier& id,
                                                      const TopDUContext::IndexedRecursiveImports& visibility,
                                                      const DeclarationVisitor& visitor) const
{
    ENSURE_CHAIN_READ_LOCKED

    PersistentSymbolTableItem item;
    item.id = id;

    LockedItemRepository::write<PersistentSymbolTable>([&](PersistentSymbolTableRepo& repo) {
        uint index = repo.findIndex(item);
        if (!index) {
            return;
        }

        const PersistentSymbolTableItem* repositoryItem = repo.itemFromIndex(index);
        const auto declarations = Declarations(repositoryItem->declarations(), repositoryItem->declarationsSize(),
                                               repositoryItem->centralFreeItem);

        CachedIndexedRecursiveImports cachedImports;
        auto it = repo.importsCache.constFind(visibility);
        if (it != repo.importsCache.constEnd()) {
            cachedImports = *it;
        } else {
            cachedImports = CachedIndexedRecursiveImports(visibility.set().stdSet());
            repo.importsCache.insert(visibility, cachedImports);
        }

        auto filterIterator = [&]() {
            if (declarations.dataSize() > MinimumCountForCache) {
                // Do visibility caching
                auto& cached = repo.declarationsCache[id];
                auto cacheIt = cached.constFind(visibility);
                if (cacheIt != cached.constEnd()) {
                    return FilteredDeclarationIterator(
                        Declarations::Iterator(cacheIt->constData(), cacheIt->size(), -1), cachedImports);
                }

                auto insertIt = cached.insert(visibility, {});
                auto& cache = *insertIt;
                {
                    auto cacheVisitor = [&cache](const IndexedDeclaration& decl) {
                        cache.append(decl);
                    };

                    using FilteredDeclarationCacheVisitor =
                        ConvenientEmbeddedSetTreeFilterVisitor<IndexedDeclaration, IndexedDeclarationHandler,
                                                               IndexedTopDUContext, CachedIndexedRecursiveImports,
                                                               DeclarationTopContextExtractor, decltype(cacheVisitor)>;

                    // The visitor visits all the declarations from within its constructor
                    FilteredDeclarationCacheVisitor visitor(cacheVisitor, declarations.iterator(), cachedImports);
                }

                return FilteredDeclarationIterator(Declarations::Iterator(cache.constData(), cache.size(), -1),
                                                   cachedImports, true);
            } else {
                return FilteredDeclarationIterator(declarations.iterator(), cachedImports);
            }
        }();

        for (; filterIterator; ++filterIterator) {
            if (visitor(*filterIterator) == VisitorState::Break) {
                break;
            }
        }
    });
}

struct DebugVisitor
{
    explicit DebugVisitor(const QTextStream& _out)
        : out(_out)
    {
    }

    bool operator()(const PersistentSymbolTableItem* item)
    {
        QDebug qout = fromTextStream(out);
        QualifiedIdentifier id(item->id.identifier());
        if (identifiers.contains(id)) {
            qout << "identifier" << id.toString() << "appears for" << identifiers[id] << "th time";
        }

        ++identifiers[id];

        for (uint a = 0; a < item->declarationsSize(); ++a) {
            IndexedDeclaration decl(item->declarations()[a]);
            if (!decl.isDummy()) {
                if (declarations.contains(decl)) {
                    qout << "declaration found for multiple identifiers. Previous identifier:" <<
                        declarations[decl].toString() << "current identifier:" << id.toString() << endl;
                } else {
                    declarations.insert(decl, id);
                }
            }
            if (decl.data() && decl.data()->qualifiedIdentifier() != item->id.identifier()) {
                qout << decl.data()->url().str() << "declaration" << decl.data()->qualifiedIdentifier() <<
                    "is registered as" << item->id.identifier() << endl;
            }

            const QString url = IndexedTopDUContext(decl.topContextIndex()).url().str();
            if (!decl.data() && !decl.isDummy()) {
                qout << "Item in symbol-table is invalid:" << id.toString() << "- localIndex:" << decl.localIndex() <<
                    "- url:" << url << endl;
            } else {
                qout << "Item in symbol-table:" << id.toString() << "- localIndex:" << decl.localIndex() << "- url:" <<
                    url;
                if (auto d = decl.data()) {
                    qout << "- range:" << d->range();
                } else {
                    qout << "- null declaration";
                }
                qout << endl;
            }
        }

        return true;
    }

    const QTextStream& out;
    QHash<QualifiedIdentifier, uint> identifiers;
    QHash<IndexedDeclaration, QualifiedIdentifier> declarations;
};

void PersistentSymbolTable::dump(const QTextStream& out)
{
    QDebug qout = fromTextStream(out);
    DebugVisitor v(out);

    LockedItemRepository::read<PersistentSymbolTable>([&](const PersistentSymbolTableRepo& repo) {
        repo.visitAllItems(v);

        qout << "Statistics:" << endl;
        qout << repo.statistics() << endl;
    });
}

PersistentSymbolTable& PersistentSymbolTable::self()
{
    static PersistentSymbolTable ret;
    return ret;
}
}
