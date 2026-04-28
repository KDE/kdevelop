/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchainprivate.h"

namespace KDevelop {

//This thing is not actually used, but it's needed for compiling
DEFINE_LIST_MEMBER_HASH(EnvironmentInformationListItem, items, uint)

    DUChainPrivate::DUChainPrivate()
        : instance(nullptr)
        , m_cleanupDisabled(false)
        , m_destroyed(false)
    {
#if defined(TEST_NO_CLEANUP)
        m_cleanupDisabled = true;
#endif

        qRegisterMetaType<DUChainBasePointer>("KDevelop::DUChainBasePointer");
        qRegisterMetaType<DUContextPointer>("KDevelop::DUContextPointer");
        qRegisterMetaType<TopDUContextPointer>("KDevelop::TopDUContextPointer");
        qRegisterMetaType<DeclarationPointer>("KDevelop::DeclarationPointer");
        qRegisterMetaType<FunctionDeclarationPointer>("KDevelop::FunctionDeclarationPointer");
        qRegisterMetaType<KDevelop::IndexedString>("KDevelop::IndexedString");
        qRegisterMetaType<KDevelop::IndexedTopDUContext>("KDevelop::IndexedTopDUContext");
        qRegisterMetaType<KDevelop::ReferencedTopDUContext>("KDevelop::ReferencedTopDUContext");

        instance = new DUChain();
        m_cleanup = new CleanupThread(this);
        m_cleanup->start();

        DUChain::m_deleted = false;

        ///Loading of some static data:
        {
            ///@todo Solve this more duchain-like
            QFile f(globalItemRepositoryRegistry().path() + QLatin1String("/parsing_environment_data"));
            bool opened = f.open(QIODevice::ReadOnly);
            ///FIXME: ugh, so ugly
            ParsingEnvironmentFile::m_staticData =
                reinterpret_cast<StaticParsingEnvironmentData*>(new char[sizeof(StaticParsingEnvironmentData)]);
            if (opened) {
                qCDebug(LANGUAGE) << "reading parsing-environment static data";
                //Read
                f.read(reinterpret_cast<char*>(ParsingEnvironmentFile::m_staticData), sizeof(StaticParsingEnvironmentData));
            } else {
                qCDebug(LANGUAGE) << "creating new parsing-environment static data";
                //Initialize
                new (ParsingEnvironmentFile::m_staticData) StaticParsingEnvironmentData();
            }
        }

        ///Read in the list of available top-context indices
        {
            QFile f(globalItemRepositoryRegistry().path() + QLatin1String("/available_top_context_indices"));
            bool opened = f.open(QIODevice::ReadOnly);
            if (opened) {
                Q_ASSERT((f.size() % sizeof(uint)) == 0);
                m_availableTopContextIndices.resize(f.size() / ( int )sizeof(uint));
                f.read(reinterpret_cast<char*>(m_availableTopContextIndices.data()), f.size());
            }
        }
    }
    DUChainPrivate::~DUChainPrivate()
    {
        qCDebug(LANGUAGE) << "Destroying";
        DUChain::m_deleted = true;
        m_cleanup->stopThread();
        delete m_cleanup;
        delete instance;
    }

    void DUChainPrivate::clear()
    {
        if (!m_cleanupDisabled)
            doMoreCleanup();

        DUChainWriteLocker writeLock(DUChain::lock());

        QMutexLocker l(&m_chainsMutex);

        const auto currentChainsByUrl = m_chainsByUrl;
        for (TopDUContext* top : currentChainsByUrl) {
            removeDocumentChainFromMemory(top);
        }

        m_indexEnvironmentInformations.clear();
        m_fileEnvironmentInformations.clear();

        Q_ASSERT(m_fileEnvironmentInformations.isEmpty());
        Q_ASSERT(m_chainsByUrl.isEmpty());
    }

    ///DUChain must be write-locked
    ///Also removes from the environment-manager if the top-context is not on disk
    void DUChainPrivate::removeDocumentChainFromMemory(TopDUContext* context)
    {
        QMutexLocker l(&m_chainsMutex);

        {
            QMutexLocker l(&m_referenceCountsMutex);

            auto countIt = m_referenceCounts.constFind(context);
            if (countIt != m_referenceCounts.constEnd()) {
                //This happens during shutdown, since everything is unloaded
                qCDebug(LANGUAGE) << "removed a top-context that was reference-counted:" << context->url().str() <<
                    context->ownIndex();
                m_referenceCounts.erase(countIt);
            }
        }

        uint index = context->ownIndex();

        // qCDebug(LANGUAGE) << "duchain: removing document" << context->url().str();
        Q_ASSERT(hasChainForIndex(index));
        Q_ASSERT(m_chainsByUrl.contains(context->url(), context));

        m_chainsByUrl.remove(context->url(), context);

        if (!context->isOnDisk())
            instance->removeFromEnvironmentManager(context);

        l.unlock();
        //DUChain is write-locked, so we can do whatever we want on the top-context, including deleting it
        context->deleteSelf();
        l.relock();

        Q_ASSERT(hasChainForIndex(index));

        QMutexLocker lock(&DUChain::chainsByIndexLock);
        DUChain::chainsByIndex[index] = nullptr;
    }

    ///The item must not be stored yet
    ///m_chainsMutex should not be locked, since this can trigger I/O
    void DUChainPrivate::addEnvironmentInformation(ParsingEnvironmentFilePointer info)
    {
        Q_ASSERT(!findInformation(info->indexedTopContext().index()));
        Q_ASSERT(LockedItemRepository::read<EnvironmentInformation>([&](const EnvironmentInformationRepo& repo) {
            return repo.findIndex(info->indexedTopContext().index()) == 0;
        }));

        QMutexLocker lock(&m_chainsMutex);
        m_fileEnvironmentInformations.insert(info->url(), info);

        m_indexEnvironmentInformations.insert(info->indexedTopContext().index(), info);

        Q_ASSERT(info->d_func()->classId);
    }

    ///The item must be managed currently
    ///m_chainsMutex does not need to be locked
    void DUChainPrivate::removeEnvironmentInformation(ParsingEnvironmentFilePointer info)
    {
        info->makeDynamic(); //By doing this, we make sure the data is actually being destroyed in the destructor

        const auto infoIndex = info->indexedTopContext().index();

        bool removed = false;
        bool removed2 = false;
        {
            QMutexLocker lock(&m_chainsMutex);
            removed = m_fileEnvironmentInformations.remove(info->url(), info);
            removed2 = m_indexEnvironmentInformations.remove(infoIndex);
        }

        LockedItemRepository::write<EnvironmentInformationList>(
            [infoIndex,
             request = EnvironmentInformationListRequest(info->url())](EnvironmentInformationListRepo& repo) mutable {
                // Remove it from the environment information lists if it was there
                const uint index = repo.findIndex(request);

                if (index) {
                    EnvironmentInformationListItem item(*repo.itemFromIndex(index), true);
                    if (item.itemsList().removeOne(infoIndex)) {
                        repo.deleteItem(index);
                        if (!item.itemsList().empty()) {
                            request.m_item = &item;
                            repo.index(request);
                        }
                    }
                }
            });

        LockedItemRepository::write<EnvironmentInformation>(
            [infoIndex, removed, removed2](EnvironmentInformationRepo& repo) {
                const uint index = repo.findIndex(infoIndex);
                if (index) {
                    repo.deleteItem(index);
                }

                Q_UNUSED(removed);
                Q_UNUSED(removed2);
                Q_ASSERT(index || (removed && removed2));
            });

        Q_ASSERT(!findInformation(infoIndex));
    }

    ///m_chainsMutex should _not_ be locked, because this may trigger I/O
    QList<ParsingEnvironmentFilePointer> DUChainPrivate::getEnvironmentInformation(const IndexedString& url)
    {
        QList<ParsingEnvironmentFilePointer> ret;

        {
            KDevVarLengthArray<uint> topContextIndices;
            // First store all the possible indices into the KDevVarLengthArray, so we can process them without holding
            // a mutex locked
            LockedItemRepository::read<EnvironmentInformationList>(
                [&topContextIndices,
                 request = EnvironmentInformationListRequest(url)](const EnvironmentInformationListRepo& repo) {
                    const EnvironmentInformationListItem* item = repo.findItem(request);
                    if (item) {
                        FOREACH_FUNCTION(uint topContextIndex, item->items)
                        topContextIndices << topContextIndex;
                    }
                });

            // Process the indices in a separate step after copying them from the array, so we don't need
            // m_environmentListInfoMutex locked, and can call loadInformation(..) safely, which else might lead to a
            // deadlock.
            for (uint topContextIndex : std::as_const(topContextIndices)) {
                QExplicitlySharedDataPointer<ParsingEnvironmentFile> p =
                    ParsingEnvironmentFilePointer(loadInformation(topContextIndex));
                if (p) {
                    ret << p;
                } else {
                    qCDebug(LANGUAGE) << "Failed to load environment-information for" <<
                        TopDUContextDynamicData::loadUrl(topContextIndex).str();
                }
            }
        }

        QMutexLocker l(&m_chainsMutex);

        //Add those information that have not been added to the stored lists yet
        const auto files = m_fileEnvironmentInformations.values(url);
        for (const ParsingEnvironmentFilePointer& file : files) {
            if (!ret.contains(file))
                ret << file;
        }

        return ret;
    }

    ///Must be called _without_ the chainsByIndex spin-lock locked
    bool DUChainPrivate::hasChainForIndex(uint index)
    {
        QMutexLocker lock(&DUChain::chainsByIndexLock);
        return (DUChain::chainsByIndex.size() > index) && DUChain::chainsByIndex[index];
    }

    ///Must be called _without_ the chainsByIndex spin-lock locked. Returns the top-context if it is loaded.
    TopDUContext* DUChainPrivate::readChainForIndex(uint index)
    {
        QMutexLocker lock(&DUChain::chainsByIndexLock);
        if (DUChain::chainsByIndex.size() > index)
            return DUChain::chainsByIndex[index];
        else
            return nullptr;
    }

    ///Makes sure that the chain with the given index is loaded
    ///@warning m_chainsMutex must NOT be locked when this is called
    void DUChainPrivate::loadChain(uint index, QSet<uint>& loaded)
    {
        QMutexLocker l(&m_chainsMutex);

        if (!hasChainForIndex(index)) {
            if (!Algorithm::insert(m_loading, index).inserted) {
                //It's probably being loaded by another thread. So wait until the load is ready
                while (m_loading.contains(index)) {
                    l.unlock();
                    qCDebug(LANGUAGE) << "waiting for another thread to load index" << index;
                    QThread::usleep(50000);
                    l.relock();
                }
                loaded.insert(index);
                return;
            }
            loaded.insert(index);

            l.unlock();
            qCDebug(LANGUAGE) << "loading top-context" << index;
            TopDUContext* chain = TopDUContextDynamicData::load(index);
            if (chain) {
                chain->setParsingEnvironmentFile(loadInformation(chain->ownIndex()));

                if (!chain->usingImportsCache()) {
                    //Eventually also load all the imported chains, so the import-structure is built
                    const auto importedParentContexts = chain->DUContext::importedParentContexts();
                    for (const DUContext::Import& import : importedParentContexts) {
                        if (!loaded.contains(import.topContextIndex())) {
                            loadChain(import.topContextIndex(), loaded);
                        }
                    }
                }
                chain->rebuildDynamicImportStructure();

                chain->setInDuChain(true);
                instance->addDocumentChain(chain);
            }

            l.relock();
            m_loading.remove(index);
        }
    }

    ///Stores all environment-information
    ///Also makes sure that all information that stays is referenced, so it stays alive.
    ///@param atomic If this is false, the write-lock will be released time by time
    void DUChainPrivate::storeAllInformation(bool atomic, DUChainWriteLocker& locker)
    {
        uint cnt = 0;

        QList<IndexedString> urls;
        {
            QMutexLocker lock(&m_chainsMutex);
            urls += m_fileEnvironmentInformations.keys();
        }

        for (const IndexedString& url : std::as_const(urls)) {
            QList<ParsingEnvironmentFilePointer> check;
            {
                QMutexLocker lock(&m_chainsMutex);
                check = m_fileEnvironmentInformations.values(url);
            }

            for (const ParsingEnvironmentFilePointer& file : std::as_const(check)) {
                EnvironmentInformationRequest req(file.data());

                LockedItemRepository::write<EnvironmentInformation>([&](EnvironmentInformationRepo& repo) {
                    uint index = repo.findIndex(req);

                    if (file->d_func()->isDynamic()) {
                        // This item has been changed, or isn't in the repository yet

                        // Eventually remove an old entry
                        if (index)
                            repo.deleteItem(index);

                        // Add the new entry to the item repository
                        index = repo.index(req);
                        Q_ASSERT(index);

                        auto* item = const_cast<EnvironmentInformationItem*>(repo.itemFromIndex(index));
                        auto* theData = reinterpret_cast<DUChainBaseData*>(reinterpret_cast<char*>(item)
                                                                           + sizeof(EnvironmentInformationItem));

                        Q_ASSERT(theData->m_range == file->d_func()->m_range);
                        Q_ASSERT(theData->m_dynamic == false);
                        Q_ASSERT(theData->classId == file->d_func()->classId);

                        file->setData(theData);

                        ++cnt;
                    } else {
                        repo.itemFromIndex(index); // Prevent unloading of the data, by accessing the item
                    }
                });
            }

            ///We must not release the lock while holding a reference to a ParsingEnvironmentFilePointer, else we may miss the deletion of an
            ///information, and will get crashes.
            if (!atomic && (cnt % 100 == 0)) {
                //Release the lock on a regular basis
                locker.unlock();
                locker.lock();
            }

            storeInformationList(url);

            //Access the data in the repository, so the bucket isn't unloaded
            const auto foundItem = LockedItemRepository::read<EnvironmentInformationList>(
                [request = EnvironmentInformationListRequest(url)](const EnvironmentInformationListRepo& repo) {
                    return static_cast<bool>(repo.findItem(request));
                });
            if (!foundItem) {
                QMutexLocker chainLock(&m_chainsMutex);
                qCDebug(LANGUAGE) << "Did not find stored item for" << url.str()
                                  << "count:" << m_fileEnvironmentInformations.values(url);
            }

            if (!atomic) {
                locker.unlock();
                locker.lock();
            }
        }
    }

    ///@param retries When this is nonzero, then doMoreCleanup will do the specified amount of cycles
    ///doing the cleanup without permanently locking the du-chain. During these steps the consistency
    ///of the disk-storage is not guaranteed, but only few changes will be done during these steps,
    ///so the final step where the duchain is permanently locked is much faster.
    void DUChainPrivate::doMoreCleanup(int retries, LockFlag lockFlag)
    {
        if (m_cleanupDisabled)
            return;

        //This mutex makes sure that there's never 2 threads at he same time trying to clean up
        QMutexLocker lockCleanupMutex(&cleanupMutex());

        if (m_destroyed || m_cleanupDisabled)
            return;

        Q_ASSERT(!instance->lock()->currentThreadHasReadLock() && !instance->lock()->currentThreadHasWriteLock());
        DUChainWriteLocker writeLock(instance->lock());

        //This is used to stop all parsing before starting to do the cleanup. This way less happens during the
        //soft cleanups, and we have a good chance that during the "hard" cleanup only few data has to be written.
        QList<QReadWriteLock*> locked;

        if (lockFlag != NoLock) {
            QList<ILanguageSupport*> languages;
            if (ICore* core = ICore::self())
                if (ILanguageController* lc = core->languageController())
                    languages = lc->loadedLanguages();

            writeLock.unlock();

            //Here we wait for all parsing-threads to stop their processing
            for (const auto language : std::as_const(languages)) {
                if (lockFlag == TryLock) {
                    if (!language->parseLock()->tryLockForWrite()) {
                        qCDebug(LANGUAGE) << "Aborting cleanup because language plugin is still parsing:" <<
                            language->name();
                        // some language is still parsing, don't interfere with the cleanup
                        for (auto* lock : std::as_const(locked)) {
                            lock->unlock();
                        }

                        return;
                    }
                } else {
                    language->parseLock()->lockForWrite();
                }
                locked << language->parseLock();
            }

            writeLock.lock();

            globalItemRepositoryRegistry().lockForWriting();
            qCDebug(LANGUAGE) << "starting cleanup";
        }

        QTime startTime = QTime::currentTime();

        storeAllInformation(!retries, writeLock); //Puts environment-information into a repository

        //We don't need to increase the reference-count, since the cleanup-mutex is locked
        QSet<TopDUContext*> workOnContexts;

        {
            QMutexLocker l(&m_chainsMutex);

            workOnContexts.reserve(m_chainsByUrl.size());
            for (TopDUContext* top : std::as_const(m_chainsByUrl)) {
                workOnContexts << top;
                Q_ASSERT(hasChainForIndex(top->ownIndex()));
            }
        }

        for (TopDUContext* context : std::as_const(workOnContexts)) {
            context->m_dynamicData->store();

            if (retries) {
                //Eventually give other threads a chance to access the duchain
                writeLock.unlock();
                //Sleep to give the other threads a realistic chance to get a read-lock in between
                QThread::usleep(500);
                writeLock.lock();
            }
        }

        //Unload all top-contexts that don't have a reference-count and that are not imported by a referenced one

        bool unloadedOne = true;

        bool unloadAllUnreferenced = !retries;

        //Now unload contexts, but only ones that are not imported by any other currently loaded context
        //The complication: Since during the lock-break new references may be added, we must never keep
        //the du-chain in an invalid state. Thus we can only unload contexts that are not imported by any
        //currently loaded contexts. In case of loops, we have to unload everything at once.
        while (unloadedOne) {
            unloadedOne = false;
            int hadUnloadable = 0;

unloadContexts:

            const auto currentWorkOnContexts = workOnContexts;
            for (TopDUContext * unload : currentWorkOnContexts) {
                bool hasReference = false;

                {
                    QMutexLocker l(&m_referenceCountsMutex);
                    //Test if the context is imported by a referenced one
                    for (auto it = m_referenceCounts.constBegin(), end = m_referenceCounts.constEnd(); it != end;
                         ++it) {
                        auto* context = it.key();
                        if (context == unload || context->imports(unload, CursorInRevision())) {
                            workOnContexts.remove(unload);
                            hasReference = true;
                        }
                    }
                }

                if (!hasReference)
                    ++hadUnloadable; //We have found a context that is not referenced
                else
                    continue; //This context is referenced

                bool isImportedByLoaded = !unload->loadedImporters().isEmpty();

                //If we unload a context that is imported by other contexts, we create a bad loaded state
                if (isImportedByLoaded && !unloadAllUnreferenced)
                    continue;

                //Since we've released the write-lock in between, we've got to call store() again to be sure that none of the data is dynamic
                //If nothing has changed, it is only a low-cost call.
                unload->m_dynamicData->store();
                Q_ASSERT(!unload->d_func()->m_dynamic);
                removeDocumentChainFromMemory(unload);
                workOnContexts.remove(unload);
                unloadedOne = true;

                if (!unloadAllUnreferenced) {
                    //Eventually give other threads a chance to access the duchain
                    writeLock.unlock();
                    //Sleep to give the other threads a realistic chance to get a read-lock in between
                    QThread::usleep(500);
                    writeLock.lock();
                }
            }

            if (hadUnloadable && !unloadedOne) {
                Q_ASSERT(!unloadAllUnreferenced);
                //This can happen in case of loops. We have o unload everything at one time.
                qCDebug(LANGUAGE) << "found" << hadUnloadable <<
                    "unloadable contexts, but could not unload separately. Unloading atomically.";
                unloadAllUnreferenced = true;
                hadUnloadable = 0; //Reset to 0, so we cannot loop forever
                goto unloadContexts;
            }
        }

        if (retries == 0) {
            QMutexLocker lock(&m_chainsMutex);
            //Do this atomically, since we must be sure that _everything_ is already saved
            for (QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator it =
                     m_fileEnvironmentInformations.begin();
                 it != m_fileEnvironmentInformations.end();) {
                ParsingEnvironmentFile* f = it->data();
                Q_ASSERT(f->d_func()->classId);
                if (f->ref.loadRelaxed() == 1) {
                    Q_ASSERT(!f->d_func()->isDynamic()); //It cannot be dynamic, since we have stored before
                    //The ParsingEnvironmentFilePointer is only referenced once. This means that it does not belong to any
                    //loaded top-context, so just remove it to save some memory and processing time.
                    ///@todo use some kind of timeout before removing
                    it = m_fileEnvironmentInformations.erase(it);
                } else {
                    ++it;
                }
            }
        }

        if (retries)
            writeLock.unlock();

        //This must be the last step, due to the on-disk reference counting
        globalItemRepositoryRegistry().store(); //Stores all repositories

        {
            //Store the static parsing-environment file data
            ///@todo Solve this more elegantly, using a general mechanism to store static duchain-like data
            Q_ASSERT(ParsingEnvironmentFile::m_staticData);
            QFile f(globalItemRepositoryRegistry().path() + QLatin1String("/parsing_environment_data"));
            bool opened = f.open(QIODevice::WriteOnly);
            Q_ASSERT(opened);
            Q_UNUSED(opened);
            f.write(reinterpret_cast<const char*>(ParsingEnvironmentFile::m_staticData), sizeof(StaticParsingEnvironmentData));
        }

        ///Write out the list of available top-context indices
        {
            QMutexLocker lock(&m_chainsMutex);

            QFile f(globalItemRepositoryRegistry().path() + QLatin1String("/available_top_context_indices"));
            bool opened = f.open(QIODevice::WriteOnly);
            Q_ASSERT(opened);
            Q_UNUSED(opened);

            f.write(reinterpret_cast<const char*>(m_availableTopContextIndices.data()), m_availableTopContextIndices.size() * sizeof(uint));
        }

        if (retries) {
            doMoreCleanup(retries - 1, NoLock);
            writeLock.lock();
        }

        if (lockFlag != NoLock) {
            globalItemRepositoryRegistry().unlockForWriting();

            const auto elapsedMS = startTime.msecsTo(QTime::currentTime());
            qCDebug(LANGUAGE) << "time spent doing cleanup:" << elapsedMS << "ms - top-contexts still open:" <<
                m_chainsByUrl.size() << "- retries" << retries;
        }

        for (QReadWriteLock* lock : std::as_const(locked)) {
            lock->unlock();
        }

#if HAVE_MALLOC_TRIM
        // trim unused memory but keep a pad buffer of about 50 MB
        // this can greatly decrease the perceived memory consumption of kdevelop
        // see: https://sourceware.org/bugzilla/show_bug.cgi?id=14827
        malloc_trim(50 * 1024 * 1024);
#endif
    }

    ///Checks whether the information is already loaded.
    ParsingEnvironmentFile* DUChainPrivate::findInformation(uint topContextIndex)
    {
        QMutexLocker lock(&m_chainsMutex);
        QHash<uint, ParsingEnvironmentFilePointer>::iterator it = m_indexEnvironmentInformations.find(topContextIndex);
        if (it != m_indexEnvironmentInformations.end())
            return (*it).data();
        return nullptr;
    }

    ///Loads/gets the environment-information for the given top-context index, or returns zero if none exists
    ///@warning m_chainsMutex should NOT be locked when this is called, because it triggers I/O
    ///@warning no other mutexes should be locked, as that may lead to a dedalock
    ParsingEnvironmentFile* DUChainPrivate::loadInformation(uint topContextIndex)
    {
        ParsingEnvironmentFile* alreadyLoaded = findInformation(topContextIndex);
        if (alreadyLoaded)
            return alreadyLoaded;

        // Step two: Check if it is on disk, and if is, load it
        //  TODO: this looks pretty dubious, shouldn't we keep the repo locked while operating on the item?
        const auto item = LockedItemRepository::read<EnvironmentInformation>(
            [req = EnvironmentInformationRequest(topContextIndex)](const EnvironmentInformationRepo& repo) {
                return repo.findItem(req);
            });
        if (!item) {
            //No environment-information stored for this top-context
            return nullptr;
        }

        QMutexLocker lock(&m_chainsMutex);

        //Due to multi-threading, we must do this check after locking the mutex, so we can be sure we don't create the same item twice at the same time
        alreadyLoaded = findInformation(topContextIndex);
        if (alreadyLoaded)
            return alreadyLoaded;

        ///FIXME: ugly, and remove const_cast
        auto* ret = dynamic_cast<ParsingEnvironmentFile*>(
            DUChainItemSystem::self().create(const_cast<DUChainBaseData*>(reinterpret_cast<const DUChainBaseData*>(
                reinterpret_cast<const char*>(item) + sizeof(EnvironmentInformationItem)))));
        if (ret) {
            Q_ASSERT(ret->d_func()->classId);
            Q_ASSERT(ret->indexedTopContext().index() == topContextIndex);
            ParsingEnvironmentFilePointer retPtr(ret);

            m_fileEnvironmentInformations.insert(ret->url(), retPtr);

            Q_ASSERT(!m_indexEnvironmentInformations.contains(ret->indexedTopContext().index()));
            m_indexEnvironmentInformations.insert(ret->indexedTopContext().index(), retPtr);
        }
        return ret;
    }

    struct CleanupListVisitor
    {
        QList<uint> checkContexts;
        bool operator()(const EnvironmentInformationItem* item)
        {
            checkContexts << item->m_topContext;
            return true;
        }
    };

    ///Will check a selection of all top-contexts for up-to-date ness, and remove them if out of date
    void DUChainPrivate::cleanupTopContexts()
    {
        DUChainWriteLocker lock(DUChain::lock());
        qCDebug(LANGUAGE) << "cleaning top-contexts";
        CleanupListVisitor visitor;
        uint startPos = 0;
        LockedItemRepository::write<EnvironmentInformation>([&visitor](EnvironmentInformationRepo& repo) {
            repo.visitAllItems(visitor);
        });

        int checkContextsCount = maxFinalCleanupCheckContexts;
        int percentageOfContexts = (visitor.checkContexts.size() * 100) / minimumFinalCleanupCheckContextsPercentage;

        if (checkContextsCount < percentageOfContexts)
            checkContextsCount = percentageOfContexts;

        if (visitor.checkContexts.size() > (int)checkContextsCount)
            startPos = QRandomGenerator::global()->bounded(visitor.checkContexts.size() - checkContextsCount);

        int endPos = startPos + maxFinalCleanupCheckContexts;
        if (endPos > visitor.checkContexts.size())
            endPos = visitor.checkContexts.size();
        QSet<uint> check;
        for (int a = startPos; a < endPos && check.size() < checkContextsCount; ++a)
            if (check.size() < checkContextsCount)
                addContextsForRemoval(check, IndexedTopDUContext(visitor.checkContexts[a]));

        for (uint topIndex : std::as_const(check)) {
            IndexedTopDUContext top(topIndex);
            if (top.data()) {
                qCDebug(LANGUAGE) << "removing top-context for" << top.data()->url().str() <<
                    "because it is out of date";
                instance->removeDocumentChain(top.data());
            }
        }

        qCDebug(LANGUAGE) << "check ready";
    }

    void DUChainPrivate::addContextsForRemoval(QSet<uint>& topContexts, IndexedTopDUContext top)
    {
        if (topContexts.contains(top.index()))
            return;

        QExplicitlySharedDataPointer<ParsingEnvironmentFile> info(instance->environmentFileForDocument(top));
        ///@todo Also check if the context is "useful"(Not a duplicate context, imported by a useful one, ...)
        if (info && info->needsUpdate()) {
            //This context will be removed
        } else {
            return;
        }

        topContexts.insert(top.index());

        if (info) {
            //Check whether importers need to be removed as well
            const QList<QExplicitlySharedDataPointer<ParsingEnvironmentFile>> importers = info->importers();

            QSet<QExplicitlySharedDataPointer<ParsingEnvironmentFile>> checkNext;

            //Do breadth first search, so less imports/importers have to be loaded, and a lower depth is reached

            for (auto& importer : importers) {
                IndexedTopDUContext c = importer->indexedTopContext();
                // Prevent useless recursion
                if (Algorithm::insert(topContexts, c.index()).inserted) {
                    checkNext.insert(importer);
                }
            }

            for (auto& parsingEnvFile : std::as_const(checkNext)) {
                topContexts.remove(parsingEnvFile->indexedTopContext().index()); // Enable full check again
                addContextsForRemoval(topContexts, parsingEnvFile->indexedTopContext());
            }
        }
    }

    ///Stores the environment-information for the given url
    void DUChainPrivate::storeInformationList(const IndexedString& url)
    {
        EnvironmentInformationListItem newItem(url);

        QSet<uint> newItems;

        {
            QMutexLocker lock(&m_chainsMutex);
            QMultiMap<IndexedString,
                ParsingEnvironmentFilePointer>::iterator start = m_fileEnvironmentInformations.lowerBound(url);
            QMultiMap<IndexedString,
                ParsingEnvironmentFilePointer>::iterator end = m_fileEnvironmentInformations.upperBound(url);

            for (QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator it = start; it != end; ++it) {
                uint topContextIndex = (*it)->indexedTopContext().index();
                newItems.insert(topContextIndex);
                newItem.itemsList().append(topContextIndex);
            }
        }

        LockedItemRepository::write<EnvironmentInformationList>(
            [&, request = EnvironmentInformationListRequest(url)](EnvironmentInformationListRepo& repo) mutable {
                const uint index = repo.findIndex(request);

                if (index) {
                    // We only handle adding items here, since we can never be sure whether everything is loaded
                    // Removal is handled directly in removeEnvironmentInformation

                    const EnvironmentInformationListItem* item = repo.itemFromIndex(index);
                    QSet<uint> oldItems;
                    FOREACH_FUNCTION(uint topContextIndex, item->items)
                    {
                        oldItems.insert(topContextIndex);
                        if (Algorithm::insert(newItems, topContextIndex).inserted) {
                            newItem.itemsList().append(topContextIndex);
                        }
                    }

                    if (oldItems == newItems)
                        return;

                    /// Update/insert a new list
                    repo.deleteItem(index); // Remove the previous item
                }

                Q_ASSERT(repo.findIndex(request) == 0);

                // Insert the new item
                request.m_item = &newItem;
                repo.index(request);

                Q_ASSERT(repo.findIndex(EnvironmentInformationListRequest(url)));
            });
    }

}

