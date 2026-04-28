/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

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

#include "duchainprivate.h"

namespace KDevelop {
/// Set to true as soon as the duchain is deleted
bool DUChain::m_deleted = false;

///Must be locked through KDevelop::SpinLock before using chainsByIndex
///This lock should be locked only for very short times
QMutex DUChain::chainsByIndexLock;
std::vector<TopDUContext*> DUChain::chainsByIndex;

Q_GLOBAL_STATIC(DUChainPrivate, sdDUChainPrivate)

DUChain::DUChain()
{
    Q_ASSERT(ICore::self());

    connect(
        ICore::self()->documentController(), &IDocumentController::documentLoadedPrepare, this,
        &DUChain::documentLoadedPrepare);
    connect(
        ICore::self()->documentController(), &IDocumentController::documentUrlChanged, this,
        &DUChain::documentRenamed);
    connect(
        ICore::self()->documentController(), &IDocumentController::documentActivated, this,
        &DUChain::documentActivated);
    connect(ICore::self()->documentController(), &IDocumentController::documentClosed, this, &DUChain::documentClosed);
}

DUChain::~DUChain()
{
    DUChain::m_deleted = true;
}

DUChain* DUChain::self()
{
    return sdDUChainPrivate->instance;
}

extern void initModificationRevisionSetRepository();
extern void initDeclarationRepositories();
extern void initIdentifierRepository();
extern void initTypeRepository();
extern void initInstantiationInformationRepository();

QString DUChain::repositoryPathForSession(const KDevelop::ISessionLock::Ptr& session)
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    cacheDir += QLatin1String("/kdevduchain");
    QString baseDir = QProcessEnvironment::systemEnvironment().value(QStringLiteral("KDEV_DUCHAIN_DIR"), cacheDir);
    baseDir += QStringLiteral("/%1-%2").arg(QCoreApplication::applicationName(), session->id());
    return baseDir;
}

void DUChain::initialize()
{
    // Initialize the global item repository as first thing after loading the session
    Q_ASSERT(ICore::self());
    Q_ASSERT(ICore::self()->activeSession());

    ItemRepositoryRegistry::initialize(repositoryPathForSession(ICore::self()->activeSessionLock()));

    initReferenceCounting();

    // This needs to be initialized here too as the function is not threadsafe, but can
    // sometimes be called from different threads. This results in the underlying QFile
    // being 0 and hence crashes at some point later when accessing the contents via
    // read. See https://bugs.kde.org/show_bug.cgi?id=250779
    RecursiveImportRepository::repository();

    LockedItemRepository::initialize<EnvironmentInformationList>();
    LockedItemRepository::initialize<EnvironmentInformation>();

    // similar to above, see https://bugs.kde.org/show_bug.cgi?id=255323
    initDeclarationRepositories();

    initModificationRevisionSetRepository();
    initIdentifierRepository();
    initTypeRepository();
    initInstantiationInformationRepository();

    PersistentSymbolTable::self();
    Importers::self();
    CodeModel::self();

    globalImportIdentifier();
    globalIndexedImportIdentifier();
    globalAliasIdentifier();
    globalIndexedAliasIdentifier();
}

DUChainLock* DUChain::lock()
{
    return &sdDUChainPrivate->lock;
}

QList<TopDUContext*> DUChain::allChains() const
{
    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
    return sdDUChainPrivate->m_chainsByUrl.values();
}

void DUChain::updateContextEnvironment(TopDUContext* context, ParsingEnvironmentFile* file)
{
    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

    removeFromEnvironmentManager(context);

    context->setParsingEnvironmentFile(file);

    addToEnvironmentManager(context);
}

void DUChain::removeDocumentChain(TopDUContext* context)
{
    ENSURE_CHAIN_WRITE_LOCKED;
    IndexedTopDUContext indexed(context->indexed());
    Q_ASSERT(indexed.data() == context); ///This assertion fails if you call removeDocumentChain(..) on a document that has not been added to the du-chain
    context->m_dynamicData->deleteOnDisk();
    Q_ASSERT(indexed.data() == context);
    sdDUChainPrivate->removeDocumentChainFromMemory(context);
    Q_ASSERT(!indexed.data());
    Q_ASSERT(!environmentFileForDocument(indexed));

    QMutexLocker lock(&sdDUChainPrivate->m_chainsMutex);
    sdDUChainPrivate->m_availableTopContextIndices.push_back(indexed.index());
}

void DUChain::addDocumentChain(TopDUContext* chain)
{
    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

//   qCDebug(LANGUAGE) << "duchain: adding document" << chain->url().str() << " " << chain;
    Q_ASSERT(chain);

    Q_ASSERT(!sdDUChainPrivate->hasChainForIndex(chain->ownIndex()));

    {
        QMutexLocker lock(&DUChain::chainsByIndexLock);
        if (DUChain::chainsByIndex.size() <= chain->ownIndex())
            DUChain::chainsByIndex.resize(chain->ownIndex() + 100, nullptr);

        DUChain::chainsByIndex[chain->ownIndex()] = chain;
    }
    {
        Q_ASSERT(DUChain::chainsByIndex[chain->ownIndex()]);
    }
    Q_ASSERT(sdDUChainPrivate->hasChainForIndex(chain->ownIndex()));

    sdDUChainPrivate->m_chainsByUrl.insert(chain->url(), chain);

    Q_ASSERT(sdDUChainPrivate->hasChainForIndex(chain->ownIndex()));

    chain->setInDuChain(true);

    l.unlock();

    addToEnvironmentManager(chain);

    // This function might be called during shutdown by stale parse jobs
    // Make sure we don't access null-pointers here
    if (ICore::self() && ICore::self()->languageController() &&
        ICore::self()->languageController()->backgroundParser()->trackerForUrl(chain->url())) {
        //Make sure the context stays alive at least as long as the context is open
        ReferencedTopDUContext ctx(chain);
        sdDUChainPrivate->m_openDocumentContexts.insert(ctx);
    }
}

void DUChain::addToEnvironmentManager(TopDUContext* chain)
{
    ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
    if (!file)
        return; //We don't need to manage

    Q_ASSERT(file->indexedTopContext().index() == chain->ownIndex());

    if (ParsingEnvironmentFile* alreadyHave = sdDUChainPrivate->findInformation(file->indexedTopContext().index())) {
        ///If this triggers, there has already been another environment-information registered for this top-context.
        ///removeFromEnvironmentManager should have been called before to remove the old environment-information.
        Q_ASSERT(alreadyHave == file.data());
        Q_UNUSED(alreadyHave);
        return;
    }

    sdDUChainPrivate->addEnvironmentInformation(file);
}

void DUChain::removeFromEnvironmentManager(TopDUContext* chain)
{
    ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
    if (!file)
        return; //We don't need to manage

    sdDUChainPrivate->removeEnvironmentInformation(file);
}

TopDUContext* DUChain::chainForDocument(const QUrl& document, bool proxyContext) const
{
    return chainForDocument(IndexedString(document), proxyContext);
}

bool DUChain::isInMemory(uint topContextIndex) const
{
    return DUChainPrivate::hasChainForIndex(topContextIndex);
}

IndexedString DUChain::urlForIndex(uint index) const
{
    {
        TopDUContext* chain = DUChainPrivate::readChainForIndex(index);
        if (chain)
            return chain->url();
    }

    return TopDUContextDynamicData::loadUrl(index);
}

TopDUContext* DUChain::loadChain(uint index)
{
    QSet<uint> loaded;
    sdDUChainPrivate->loadChain(index, loaded);

    {
        QMutexLocker lock(&chainsByIndexLock);

        if (chainsByIndex.size() > index) {
            TopDUContext* top = chainsByIndex[index];
            if (top)
                return top;
        }
    }

    return nullptr;
}

TopDUContext* DUChain::chainForDocument(const KDevelop::IndexedString& document, bool proxyContext) const
{
    ENSURE_CHAIN_READ_LOCKED;

    if (sdDUChainPrivate->m_destroyed)
        return nullptr;

    const QList<ParsingEnvironmentFilePointer> list = sdDUChainPrivate->getEnvironmentInformation(document);

    for (const ParsingEnvironmentFilePointer& file : list) {
        if (isInMemory(file->indexedTopContext().index()) && file->isProxyContext() == proxyContext) {
            return file->topContext();
        }
    }

    for (const ParsingEnvironmentFilePointer& file : list) {
        if (proxyContext == file->isProxyContext()) {
            return file->topContext();
        }
    }

    //Allow selecting a top-context even if there is no ParsingEnvironmentFile
    const QList<TopDUContext*> ret = chainsForDocument(document);
    for (TopDUContext* ctx : ret) {
        if (!ctx->parsingEnvironmentFile() || (ctx->parsingEnvironmentFile()->isProxyContext() == proxyContext))
            return ctx;
    }

    return nullptr;
}

QList<TopDUContext*> DUChain::chainsForDocument(const QUrl& document) const
{
    return chainsForDocument(IndexedString(document));
}

QList<TopDUContext*> DUChain::chainsForDocument(const IndexedString& document) const
{
    QList<TopDUContext*> chains;

    if (sdDUChainPrivate->m_destroyed)
        return chains;

    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

    // Match all parsed versions of this document
    for (auto it = sdDUChainPrivate->m_chainsByUrl.lowerBound(document); it != sdDUChainPrivate->m_chainsByUrl.end();
         ++it) {
        if (it.key() == document)
            chains << it.value();
        else
            break;
    }

    return chains;
}

TopDUContext* DUChain::chainForDocument(const QUrl& document, const KDevelop::ParsingEnvironment* environment,
                                        bool proxyContext) const
{
    return chainForDocument(IndexedString(document), environment, proxyContext);
}

ParsingEnvironmentFilePointer DUChain::environmentFileForDocument(const IndexedString& document,
                                                                  const ParsingEnvironment* environment,
                                                                  bool proxyContext) const
{
    ENSURE_CHAIN_READ_LOCKED;

    if (sdDUChainPrivate->m_destroyed)
        return ParsingEnvironmentFilePointer();
    const QList<ParsingEnvironmentFilePointer> list = sdDUChainPrivate->getEnvironmentInformation(document);

//    qCDebug(LANGUAGE) << document.str() << ": matching" << list.size() << (onlyProxyContexts ? "proxy-contexts" : (noProxyContexts ? "content-contexts" : "contexts"));

    for (auto& envFilePtr : list) {
        if (envFilePtr && (envFilePtr->isProxyContext() == proxyContext) && envFilePtr->matchEnvironment(environment) &&
            // Verify that the environment-file and its top-context are "good": The top-context must exist,
            // and there must be a content-context associated to the proxy-context.
            envFilePtr->topContext() &&
            (!proxyContext || DUChainUtils::contentContextFromProxyContext(envFilePtr->topContext()))) {
            return envFilePtr;
        }
    }
    return ParsingEnvironmentFilePointer();
}

QList<ParsingEnvironmentFilePointer> DUChain::allEnvironmentFiles(const IndexedString& document)
{
    return sdDUChainPrivate->getEnvironmentInformation(document);
}

ParsingEnvironmentFilePointer DUChain::environmentFileForDocument(IndexedTopDUContext topContext) const
{
    if (topContext.index() == 0)
        return ParsingEnvironmentFilePointer();

    return ParsingEnvironmentFilePointer(sdDUChainPrivate->loadInformation(topContext.index()));
}

TopDUContext* DUChain::chainForDocument(const IndexedString& document, const ParsingEnvironment* environment,
                                        bool proxyContext) const
{
    if (sdDUChainPrivate->m_destroyed)
        return nullptr;
    ParsingEnvironmentFilePointer envFile = environmentFileForDocument(document, environment, proxyContext);
    if (envFile) {
        return envFile->topContext();
    } else {
        return nullptr;
    }
}

QList<QUrl> DUChain::documents() const
{
    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

    QList<QUrl> ret;
    ret.reserve(sdDUChainPrivate->m_chainsByUrl.count());
    for (TopDUContext* top : std::as_const(sdDUChainPrivate->m_chainsByUrl)) {
        ret << top->url().toUrl();
    }

    return ret;
}

QList<IndexedString> DUChain::indexedDocuments() const
{
    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

    QList<IndexedString> ret;
    ret.reserve(sdDUChainPrivate->m_chainsByUrl.count());
    for (TopDUContext* top : std::as_const(sdDUChainPrivate->m_chainsByUrl)) {
        ret << top->url();
    }

    return ret;
}

void DUChain::documentActivated(KDevelop::IDocument* doc)
{
    if (sdDUChainPrivate->m_destroyed)
        return;

    DUChainReadLocker lock(DUChain::lock());
    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

    auto backgroundParser = ICore::self()->languageController()->backgroundParser();
    auto addWithHighPriority = [backgroundParser, doc]() {
                                   backgroundParser->addDocument(IndexedString(doc->url()),
                                                                 TopDUContext::VisibleDeclarationsAndContexts,
                                                                 BackgroundParser::BestPriority);
                               };

    TopDUContext* ctx = DUChainUtils::standardContextForUrl(doc->url(), true);
    //Check whether the document has an attached environment-manager, and whether that one thinks the document needs to be updated.
    //If yes, update it.
    if (ctx && ctx->parsingEnvironmentFile() && ctx->parsingEnvironmentFile()->needsUpdate()) {
        qCDebug(LANGUAGE) << "Document needs update, using best priority since it just got activated:" << doc->url();
        addWithHighPriority();
    } else if (backgroundParser->managedDocuments().contains(IndexedString(doc->url()))) {
        // increase priority if there's already parse job of this document in the queue
        qCDebug(LANGUAGE) << "Prioritizing activated document:" << doc->url();
        addWithHighPriority();
    }
}

void DUChain::documentClosed(IDocument* document)
{
    if (sdDUChainPrivate->m_destroyed)
        return;

    IndexedString url(document->url());

    const auto currentDocumentContexts = sdDUChainPrivate->m_openDocumentContexts;
    for (const ReferencedTopDUContext& top : currentDocumentContexts) {
        if (top->url() == url)
            sdDUChainPrivate->m_openDocumentContexts.remove(top);
    }
}

void DUChain::documentLoadedPrepare(KDevelop::IDocument* doc)
{
    if (sdDUChainPrivate->m_destroyed)
        return;

    const IndexedString url(doc->url());
    DUChainWriteLocker lock(DUChain::lock());
    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

    TopDUContext* standardContext = DUChainUtils::standardContextForUrl(doc->url());
    QList<TopDUContext*> chains = chainsForDocument(url);

    const auto languages = ICore::self()->languageController()->languagesForUrl(doc->url());

    if (standardContext) {
        Q_ASSERT(chains.contains(standardContext)); //We have just loaded it
        Q_ASSERT((standardContext->url() == url));

        sdDUChainPrivate->m_openDocumentContexts.insert(standardContext);

        bool needsUpdate = standardContext->parsingEnvironmentFile() &&
                           standardContext->parsingEnvironmentFile()->needsUpdate();
        if (!needsUpdate) {
            //Only apply the highlighting if we don't need to update, else we might highlight total crap
            //Do instant highlighting only if all imports are loaded, to make sure that we don't block the user-interface too long
            //Else the highlighting will be done in the background-thread
            //This is not exactly right, as the direct imports don't necessarily equal the real imports used by uses
            //but it approximates the correct behavior.
            bool allImportsLoaded = true;
            const auto importedParentContexts = standardContext->importedParentContexts();
            for (const DUContext::Import& import : importedParentContexts) {
                if (!import.indexedContext().indexedTopContext().isLoaded())
                    allImportsLoaded = false;
            }

            if (allImportsLoaded) {
                l.unlock();
                lock.unlock();
                for (const auto language : languages) {
                    if (language->codeHighlighting()) {
                        language->codeHighlighting()->highlightDUChain(standardContext);
                    }
                }

                qCDebug(LANGUAGE) << "highlighted" << doc->url() << "in foreground";
                return;
            }
        } else {
            qCDebug(LANGUAGE) << "not highlighting the duchain because the documents needs an update";
        }

        if (needsUpdate || !(standardContext->features() & TopDUContext::AllDeclarationsContextsAndUses)) {
            ICore::self()->languageController()->backgroundParser()->addDocument(IndexedString(doc->url()),
                                                                                 TopDUContext::AllDeclarationsContextsAndUses
                                                                                 | TopDUContext::ForceUpdate);
            return;
        }
    }

    //Add for highlighting etc.
    ICore::self()->languageController()->backgroundParser()->addDocument(IndexedString(
                                                                             doc->url()),
                                                                         TopDUContext::AllDeclarationsContextsAndUses);
}

void DUChain::documentRenamed(KDevelop::IDocument* doc)
{
    if (sdDUChainPrivate->m_destroyed)
        return;

    const auto url = doc->url();
    // url is invalid when a file open in KDevelop is deleted externally, then the user
    // closes the file by clicking the Close File button on KTextEditor's prompt.
    if (url.isValid()) {
        ICore::self()->languageController()->backgroundParser()->addDocument(IndexedString{url},
                                                                             TopDUContext::AllDeclarationsContextsAndUses
                                                                             | TopDUContext::ForceUpdate);
    }
}

Uses* DUChain::uses()
{
    return &sdDUChainPrivate->m_uses;
}

Definitions* DUChain::definitions()
{
    return &sdDUChainPrivate->m_definitions;
}

static void finalCleanup()
{
    DUChainWriteLocker writeLock(DUChain::lock());
    qCDebug(LANGUAGE) << "doing final cleanup";

    int cleaned = 0;
    while ((cleaned = globalItemRepositoryRegistry().finalCleanup())) {
        qCDebug(LANGUAGE) << "cleaned" << cleaned << "B";
        if (cleaned < 1000) {
            qCDebug(LANGUAGE) << "cleaned enough";
            break;
        }
    }
    qCDebug(LANGUAGE) << "final cleanup ready";
}

void DUChain::shutdown()
{
    // if core is not shutting down, we can end up in deadlocks or crashes
    // since language plugins might still try to access static duchain stuff
    Q_ASSERT(!ICore::self() || ICore::self()->shuttingDown());

    qCDebug(LANGUAGE) << "Cleaning up and shutting down DUChain";

    QMutexLocker lock(&sdDUChainPrivate->cleanupMutex());

    {
        //Acquire write-lock of the repository, so when kdevelop crashes in that process, the repository is discarded
        //Crashes here may happen in an inconsistent state, thus this makes sense, to protect the user from more crashes
        globalItemRepositoryRegistry().lockForWriting();
        sdDUChainPrivate->cleanupTopContexts();
        globalItemRepositoryRegistry().unlockForWriting();
    }

    sdDUChainPrivate->doMoreCleanup(); //Must be done _before_ finalCleanup, else we may be deleting yet needed data

    sdDUChainPrivate->m_openDocumentContexts.clear();
    sdDUChainPrivate->m_destroyed = true;
    sdDUChainPrivate->clear();

    {
        //Acquire write-lock of the repository, so when kdevelop crashes in that process, the repository is discarded
        //Crashes here may happen in an inconsistent state, thus this makes sense, to protect the user from more crashes
        globalItemRepositoryRegistry().lockForWriting();
        finalCleanup();
        globalItemRepositoryRegistry().unlockForWriting();
    }

    globalItemRepositoryRegistry().shutdown();
}

uint DUChain::newTopContextIndex()
{
    {
        QMutexLocker lock(&sdDUChainPrivate->m_chainsMutex);
        if (!sdDUChainPrivate->m_availableTopContextIndices.isEmpty()) {
            uint ret = sdDUChainPrivate->m_availableTopContextIndices.back();
            sdDUChainPrivate->m_availableTopContextIndices.pop_back();
            if (TopDUContextDynamicData::fileExists(ret)) {
                qCWarning(LANGUAGE) << "Problem in the management of available top-context indices";
                return newTopContextIndex();
            }
            return ret;
        }
    }
    static QAtomicInt& currentId(globalItemRepositoryRegistry().customCounter(QStringLiteral("Top-Context Counter"),
                                                                              1));
    return currentId.fetchAndAddRelaxed(1);
}

void DUChain::refCountUp(TopDUContext* top)
{
    QMutexLocker l(&sdDUChainPrivate->m_referenceCountsMutex);
    // note: value is default-constructed to zero if it does not exist
    ++sdDUChainPrivate->m_referenceCounts[top];
}

bool DUChain::deleted()
{
    return m_deleted;
}

void DUChain::refCountDown(TopDUContext* top)
{
    QMutexLocker l(&sdDUChainPrivate->m_referenceCountsMutex);
    auto it = sdDUChainPrivate->m_referenceCounts.find(top);
    if (it == sdDUChainPrivate->m_referenceCounts.end()) {
        //qCWarning(LANGUAGE) << "tried to decrease reference-count for" << top->url().str() << "but this top-context is not referenced";
        return;
    }
    auto& refCount = *it;
    --refCount;
    if (!refCount) {
        sdDUChainPrivate->m_referenceCounts.erase(it);
    }
}

void DUChain::emitDeclarationSelected(const DeclarationPointer& decl)
{
    if (sdDUChainPrivate->m_destroyed)
        return;

    emit declarationSelected(decl);
}

void DUChain::emitUpdateReady(const IndexedString& url, const ReferencedTopDUContext& topContext)
{
    if (sdDUChainPrivate->m_destroyed)
        return;

    emit updateReady(url, topContext);
}

KDevelop::ReferencedTopDUContext DUChain::waitForUpdate(const KDevelop::IndexedString& document,
                                                        KDevelop::TopDUContext::Features minFeatures, bool proxyContext)
{
    Q_ASSERT(!lock()->currentThreadHasReadLock() && !lock()->currentThreadHasWriteLock());

    WaitForUpdate waiter;
    updateContextForUrl(document, minFeatures, &waiter);

    while (!waiter.m_ready) {
        // we might have been shut down in the meanwhile
        if (!ICore::self()) {
            return nullptr;
        }

        QMetaObject::invokeMethod(ICore::self()->languageController()->backgroundParser(), "parseDocuments");
        QCoreApplication::processEvents();
        QThread::usleep(1000);
    }

    if (!proxyContext) {
        DUChainReadLocker readLock(DUChain::lock());
        return DUChainUtils::contentContextFromProxyContext(waiter.m_topContext);
    }

    return waiter.m_topContext;
}

void DUChain::updateContextForUrl(const IndexedString& document, TopDUContext::Features minFeatures,
                                  QObject* notifyReady, int priority) const
{
    // the call to DUChainUtils::standardContextForUrl(...) takes surprisingly long
    // however if a document is already scheduled for parsing, we know it needs parsing and can
    // skip the expansive check.
    auto* backgroundParser = ICore::self()->languageController()->backgroundParser();
    // atomically check if the document is already scheduled and if so add a new listener:
    bool listenerAdded = backgroundParser->addListenerToDocumentIfExist(document, minFeatures, priority, notifyReady);
    // if the document isn't already scheduled and the listener was added:
    if (!listenerAdded)
    {
        DUChainReadLocker lock(DUChain::lock());
        // check if the document needs parsing
        TopDUContext* standardContext = DUChainUtils::standardContextForUrl(document.toUrl());
        if (standardContext && standardContext->parsingEnvironmentFile() &&
            !standardContext->parsingEnvironmentFile()->needsUpdate() &&
            standardContext->parsingEnvironmentFile()->featuresSatisfied(minFeatures)) {
            // if the document doesn't need parsing we can immedistely return the already parsed context to the listener
            lock.unlock();
            if (notifyReady) {
                // do not remove qualification KDevelop:: or invokeMethod will not find the proper method
                QMetaObject::invokeMethod(notifyReady, "updateReady", Qt::DirectConnection,
                                Q_ARG(KDevelop::IndexedString, document),
                                Q_ARG(KDevelop::ReferencedTopDUContext, ReferencedTopDUContext(standardContext)));
            }
        } else {
            ///Start a parse-job for the given document
            lock.unlock();
            backgroundParser->addDocument(document, minFeatures, priority, notifyReady);
        }
    }
}

void DUChain::disablePersistentStorage(bool disable)
{
    sdDUChainPrivate->m_cleanupDisabled = disable;
}

void DUChain::storeToDisk()
{
    bool wasDisabled = sdDUChainPrivate->m_cleanupDisabled;
    sdDUChainPrivate->m_cleanupDisabled = false;

    sdDUChainPrivate->doMoreCleanup();

    sdDUChainPrivate->m_cleanupDisabled = wasDisabled;
}

bool DUChain::compareToDisk()
{
    DUChainWriteLocker writeLock(DUChain::lock());

    ///Step 1: Compare the repositories
    return true;
}
}

#include "moc_duchain.cpp"
