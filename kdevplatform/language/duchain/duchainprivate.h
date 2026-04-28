/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KDEVPLATFORM_DUCHAINPRIVATE_H
#define KDEVPLATFORM_DUCHAINPRIVATE_H

#include "duchain.h"
#include "duchainlock.h"

#include <QCoreApplication>
#include <QHash>
#include <QMultiMap>
#include <QProcessEnvironment>
#include <QReadWriteLock>
#include <QAtomicInt>
#include <QThread>
#include <QStandardPaths>
#include <QMutex>
#include <QMutexLocker>
#include <QRecursiveMutex>
#include <QTimer>
#include <QRandomGenerator>

#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/isession.h>
#include <util/algorithm.h>

#include "../interfaces/ilanguagesupport.h"
#include "../interfaces/icodehighlighting.h"
#include "../backgroundparser/backgroundparser.h"
#include <debug.h>

#include "language-features.h"
#include "topducontext.h"
#include "topducontextdata.h"
#include "topducontextdynamicdata.h"
#include "parsingenvironment.h"
#include "declaration.h"
#include "definitions.h"
#include "duchainutils.h"
#include "use.h"
#include "uses.h"
#include "abstractfunctiondeclaration.h"
#include "duchainregister.h"
#include "persistentsymboltable.h"
#include "serialization/itemrepository.h"
#include "waitforupdate.h"
#include "importers.h"
#include "codemodel.h"

#if HAVE_MALLOC_TRIM
#include "malloc.h"
#endif

namespace {
//Additional "soft" cleanup steps that are done before the actual cleanup.
//During "soft" cleanup, the consistency is not guaranteed. The repository is
//marked to be updating during soft cleanup, so if kdevelop crashes, it will be cleared.
//The big advantage of the soft cleanup steps is, that the duchain is always only locked for
//short times, which leads to no lockup in the UI.
const int SOFT_CLEANUP_STEPS = 1;

// seconds to wait before trying to cleanup the DUChain
const uint cleanupEverySeconds = 200;

///Approximate maximum count of top-contexts that are checked during final cleanup
const uint maxFinalCleanupCheckContexts = 2000;
const uint minimumFinalCleanupCheckContextsPercentage = 10; //Check at least n% of all top-contexts during cleanup
}

namespace KDevelop {

//An entry for the item-repository that holds some meta-data. Behind this entry, the actual ParsingEnvironmentFileData is stored.
class EnvironmentInformationItem
{
public:
    EnvironmentInformationItem(uint topContext, uint size) : m_topContext(topContext)
        , m_size(size)
    {
    }

    ~EnvironmentInformationItem()
    {
    }

    Q_DISABLE_COPY_MOVE(EnvironmentInformationItem)

    unsigned int hash() const
    {
        return m_topContext;
    }

    unsigned int itemSize() const
    {
        return sizeof(*this) + m_size;
    }

    uint m_topContext;
    uint m_size;//Size of the data behind, that holds the actual item
};

struct ItemRepositoryIndexHash
{
    uint
    operator()(unsigned int __x) const
    { return 173 * (__x >> 2) + 11 * (__x >> 16); }
};

class EnvironmentInformationRequest
{
public:

    ///This constructor should only be used for lookup
    EnvironmentInformationRequest(uint topContextIndex) : m_file(nullptr)
        , m_index(topContextIndex)
    {
    }

    EnvironmentInformationRequest(const ParsingEnvironmentFile* file) : m_file(file)
        , m_index(file->indexedTopContext().index())
    {
    }

    enum {
        AverageSize = 32 //This should be the approximate average size of an Item
    };

    unsigned int hash() const
    {
        return m_index;
    }

    uint itemSize() const
    {
        return sizeof(EnvironmentInformationItem) + DUChainItemSystem::self().dynamicSize(*m_file->d_func());
    }

    void createItem(EnvironmentInformationItem* item) const
    {
        new (item) EnvironmentInformationItem(m_index, DUChainItemSystem::self().dynamicSize(*m_file->d_func()));
        Q_ASSERT(m_file->d_func()->m_dynamic);
        auto* data =
            reinterpret_cast<DUChainBaseData*>(reinterpret_cast<char*>(item) + sizeof(EnvironmentInformationItem));
        DUChainItemSystem::self().copy(*m_file->d_func(), *data, true);
        Q_ASSERT(data->m_range == m_file->d_func()->m_range);
        Q_ASSERT(data->classId == m_file->d_func()->classId);
        Q_ASSERT(data->m_dynamic == false);
    }

    static void destroy(EnvironmentInformationItem* item, KDevelop::AbstractItemRepository&)
    {
        item->~EnvironmentInformationItem();
        //We don't need to call the destructor, because that's done in DUChainBase::makeDynamic()
        //We just need to make sure that every environment-file is dynamic when it's deleted
//     DUChainItemSystem::self().callDestructor((DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem)));
    }

    static bool persistent(const EnvironmentInformationItem*)
    {
        //Cleanup done separately
        return true;
    }

    bool equals(const EnvironmentInformationItem* item) const
    {
        return m_index == item->m_topContext;
    }

    const ParsingEnvironmentFile* m_file;
    uint m_index;
};

//This thing is not actually used, but it's needed for compiling
DECLARE_LIST_MEMBER_HASH(EnvironmentInformationListItem, items, uint)

///A list of environment-information/top-contexts mapped to a file-name
class EnvironmentInformationListItem
{
public:
    explicit EnvironmentInformationListItem(const IndexedString& file)
        : m_file(file)
    {
        initializeAppendedLists(true);
    }

    explicit EnvironmentInformationListItem(const EnvironmentInformationListItem& rhs, bool dynamic)
        : m_file(rhs.m_file)
    {
        initializeAppendedLists(dynamic);
        copyListsFrom(rhs);
    }

    ~EnvironmentInformationListItem()
    {
        freeAppendedLists();
    }

    Q_DISABLE_COPY_MOVE(EnvironmentInformationListItem)

    unsigned int hash() const
    {
        //We only compare the declaration. This allows us implementing a map, although the item-repository
        //originally represents a set.
        // FIXME: This should return a hash value.
        return m_file.index();
    }

    unsigned short int itemSize() const
    {
        return dynamicSize();
    }

    IndexedString m_file;

    uint classSize() const
    {
        return sizeof(*this);
    }

    START_APPENDED_LISTS(EnvironmentInformationListItem);
    ///Contains the index of each contained environment-item
    APPENDED_LIST_FIRST(EnvironmentInformationListItem, uint, items);
    END_APPENDED_LISTS(EnvironmentInformationListItem, items);
};

class EnvironmentInformationListRequest
{
public:

    ///This constructor should only be used for lookup
    explicit EnvironmentInformationListRequest(const IndexedString& file)
        : m_file(file)
        , m_item(nullptr)
    {
    }
    ///This is used to actually construct the information in the repository
    EnvironmentInformationListRequest(const IndexedString& file, const EnvironmentInformationListItem& item)
        : m_file(file)
        , m_item(&item)
    {
    }

    enum {
        AverageSize = 160 //This should be the approximate average size of an Item
    };

    unsigned int hash() const
    {
        // FIXME: This should return a hash value.
        return m_item ? m_item->hash() : m_file.index();
    }

    uint itemSize() const
    {
        return m_item->itemSize();
    }

    void createItem(EnvironmentInformationListItem* item) const
    {
        Q_ASSERT(m_item->m_file == m_file);
        new (item) EnvironmentInformationListItem(*m_item, false);
    }

    static void destroy(EnvironmentInformationListItem* item, KDevelop::AbstractItemRepository&)
    {
        item->~EnvironmentInformationListItem();
    }

    static bool persistent(const EnvironmentInformationListItem*)
    {
        //Cleanup is done separately
        return true;
    }

    bool equals(const EnvironmentInformationListItem* item) const
    {
        return m_file == item->m_file;
    }

    IndexedString m_file;
    const EnvironmentInformationListItem* m_item;
};

/// NOTE: The following two repositories are thread-safe, and DUChainPrivate::m_chainsMutex should not be locked when
/// using them, because they may trigger I/O.

/// Maps filenames to a list of top-contexts/environment-information.
using EnvironmentInformationListRepo
    = ItemRepository<EnvironmentInformationListItem, EnvironmentInformationListRequest>;

struct EnvironmentInformationList {
};
template<>
class ItemRepositoryFor<EnvironmentInformationList>
{
    friend struct LockedItemRepository;
    static EnvironmentInformationListRepo& repo()
    {
        static QMutex mutex;
        static EnvironmentInformationListRepo repo(QStringLiteral("Environment Lists"), &mutex);
        return repo;
    }
};

/// Maps top-context-indices to environment-information item.
using EnvironmentInformationRepo = ItemRepository<EnvironmentInformationItem, EnvironmentInformationRequest>;

struct EnvironmentInformation {
};
template<>
class ItemRepositoryFor<EnvironmentInformation>
{
    friend struct LockedItemRepository;
    static EnvironmentInformationRepo& repo()
    {
        static QMutex mutex;
        static EnvironmentInformationRepo repo(QStringLiteral("Environment Information"), &mutex);
        return repo;
    }
};

class DUChainPrivate
{
    class CleanupThread
        : public QThread
    {
public:
        explicit CleanupThread(DUChainPrivate* data)
            : m_data(data)
        {
        }

        void stopThread()
        {
            quit();
            wait();
        }

private:
        void run() override
        {
            QTimer timer;
            connect(&timer, &QTimer::timeout, &timer, [this]() {
                    Q_ASSERT(QThread::currentThread() == this);
                    //Just to make sure the cache is cleared periodically
                    ModificationRevisionSet::clearCache();

                    m_data->doMoreCleanup(SOFT_CLEANUP_STEPS, TryLock);
                });
            timer.start(cleanupEverySeconds * 1000);
            exec();
        }
        DUChainPrivate* m_data;
    };

public:
    DUChainPrivate();
    ~DUChainPrivate();

    void clear();

    ///DUChain must be write-locked
    ///Also removes from the environment-manager if the top-context is not on disk
    void removeDocumentChainFromMemory(TopDUContext* context);

    ///Must be locked before accessing content of this class.
    ///Should be released during expensive disk-operations and such.
    QRecursiveMutex m_chainsMutex;

    QRecursiveMutex m_cleanupMutex;

    CleanupThread* m_cleanup;

    DUChain* instance;
    DUChainLock lock;
    QMultiMap<IndexedString, TopDUContext*> m_chainsByUrl;

    //Must be locked before accessing m_referenceCounts
    QMutex m_referenceCountsMutex;
    QHash<TopDUContext*, uint> m_referenceCounts;

    Definitions m_definitions;
    Uses m_uses;
    QSet<uint> m_loading;
    bool m_cleanupDisabled;

    //List of available top-context indices, protected by m_chainsMutex
    QVector<uint> m_availableTopContextIndices;

    ///Used to keep alive the top-context that belong to documents loaded in the editor
    QSet<ReferencedTopDUContext> m_openDocumentContexts;

    bool m_destroyed;

    ///The item must not be stored yet
    ///m_chainsMutex should not be locked, since this can trigger I/O
    void addEnvironmentInformation(ParsingEnvironmentFilePointer info);

    ///The item must be managed currently
    ///m_chainsMutex does not need to be locked
    void removeEnvironmentInformation(ParsingEnvironmentFilePointer info);

    ///m_chainsMutex should _not_ be locked, because this may trigger I/O
    QList<ParsingEnvironmentFilePointer> getEnvironmentInformation(const IndexedString& url);

    ///Must be called _without_ the chainsByIndex spin-lock locked
    static bool hasChainForIndex(uint index);

    ///Must be called _without_ the chainsByIndex spin-lock locked. Returns the top-context if it is loaded.
    static TopDUContext* readChainForIndex(uint index);

    ///Makes sure that the chain with the given index is loaded
    ///@warning m_chainsMutex must NOT be locked when this is called
    void loadChain(uint index, QSet<uint>& loaded);

    ///Stores all environment-information
    ///Also makes sure that all information that stays is referenced, so it stays alive.
    ///@param atomic If this is false, the write-lock will be released time by time
    void storeAllInformation(bool atomic, DUChainWriteLocker& locker);

    QRecursiveMutex& cleanupMutex() { return m_cleanupMutex; }

    /// defines how we interact with the ongoing language parse jobs
    enum LockFlag {
        /// no locking required, only used when we locked previously
        NoLock = 0,
        /// lock all parse jobs and block until we succeeded. required at shutdown
        BlockingLock = 1,
        /// only try to lock and abort on failure, good for the intermittent cleanups
        TryLock = 2,
    };
    ///@param retries When this is nonzero, then doMoreCleanup will do the specified amount of cycles
    ///doing the cleanup without permanently locking the du-chain. During these steps the consistency
    ///of the disk-storage is not guaranteed, but only few changes will be done during these steps,
    ///so the final step where the duchain is permanently locked is much faster.
    void doMoreCleanup(int retries = 0, LockFlag lockFlag = BlockingLock);

    ///Checks whether the information is already loaded.
    ParsingEnvironmentFile* findInformation(uint topContextIndex);

    ///Loads/gets the environment-information for the given top-context index, or returns zero if none exists
    ///@warning m_chainsMutex should NOT be locked when this is called, because it triggers I/O
    ///@warning no other mutexes should be locked, as that may lead to a dedalock
    ParsingEnvironmentFile* loadInformation(uint topContextIndex);

    ///Will check a selection of all top-contexts for up-to-date ness, and remove them if out of date
    void cleanupTopContexts();

private:

    void addContextsForRemoval(QSet<uint>& topContexts, IndexedTopDUContext top);

    ///Stores the environment-information for the given url
    void storeInformationList(const IndexedString& url);

    //Loaded environment information. Protected by m_chainsMutex
    QMultiMap<IndexedString, ParsingEnvironmentFilePointer> m_fileEnvironmentInformations;
    QHash<uint, ParsingEnvironmentFilePointer> m_indexEnvironmentInformations;
};

}

#endif

