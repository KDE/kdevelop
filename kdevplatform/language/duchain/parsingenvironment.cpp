/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "parsingenvironment.h"
#include "topducontext.h"
#include "duchainregister.h"
#include "topducontextdynamicdata.h"
#include "duchain.h"
#include "duchainlock.h"
#include "topducontextdata.h"
#include <debug.h>
#include <language/backgroundparser/parsejob.h>

#define ENSURE_READ_LOCKED   if (indexedTopContext().isValid()) { ENSURE_CHAIN_READ_LOCKED }
#define ENSURE_WRITE_LOCKED   if (indexedTopContext().isValid()) { ENSURE_CHAIN_READ_LOCKED }

namespace KDevelop {
StaticParsingEnvironmentData* ParsingEnvironmentFile::m_staticData = nullptr;

 #if 0
///Wrapper class around objects that are managed through the DUChain, and may contain arbitrary objects
///that support duchain-like store (IndexedString, StorableSet, and the likes). The object must not contain pointers
///or other non-persistent data.
///
///The object is stored during the normal duchain storage/cleanup cycles.

template <class T>
struct PersistentDUChainObject
{
    ///@param fileName File-name that will be used to store the data of the object in the duchain directory
    PersistentDUChainObject(QString fileName)
    {
        object = ( T* ) new char[sizeof(T)];
        if (!DUChain::self()->addPersistentObject(object, fileName, sizeof(T))) {
            //The constructor is called only if the object did not exist yet
            new (object) T();
        }
    }
    ~PersistentDUChainObject()
    {
        DUChain::self()->unregisterPersistentObject(object);
        delete[] object;
    }

    T* object;
};
#endif

REGISTER_DUCHAIN_ITEM(ParsingEnvironmentFile);

TopDUContext::Features ParsingEnvironmentFile::features() const
{
    ENSURE_READ_LOCKED

    return d_func()->m_features;
}

ParsingEnvironment::ParsingEnvironment()
{
}

ParsingEnvironment::~ParsingEnvironment()
{
}

IndexedString ParsingEnvironmentFile::url() const
{
    ENSURE_READ_LOCKED
    return d_func()->m_url;
}

bool ParsingEnvironmentFile::needsUpdate(const ParsingEnvironment* /*environment*/) const
{
    ENSURE_READ_LOCKED
    return d_func()->m_allModificationRevisions.needsUpdate();
}

bool ParsingEnvironmentFile::matchEnvironment(const ParsingEnvironment* /*environment*/) const
{
    ENSURE_READ_LOCKED
    return true;
}

void ParsingEnvironmentFile::setTopContext(KDevelop::IndexedTopDUContext context)
{
    if (d_func()->m_topContext == context)
        return;
    ENSURE_WRITE_LOCKED
        d_func_dynamic()->m_topContext = context;

    //Enforce an update of the 'features satisfied' caches
    TopDUContext::Features oldFeatures = features();
    setFeatures(TopDUContext::Empty);
    setFeatures(oldFeatures);
}

KDevelop::IndexedTopDUContext ParsingEnvironmentFile::indexedTopContext() const
{
    return d_func()->m_topContext;
}

const ModificationRevisionSet& ParsingEnvironmentFile::allModificationRevisions() const
{
    ENSURE_READ_LOCKED
    return d_func()->m_allModificationRevisions;
}

void ParsingEnvironmentFile::addModificationRevisions(const ModificationRevisionSet& revisions)
{
    ENSURE_WRITE_LOCKED
        d_func_dynamic()->m_allModificationRevisions += revisions;
}

ParsingEnvironmentFile::ParsingEnvironmentFile(ParsingEnvironmentFileData& data,
                                               const IndexedString& url) : DUChainBase(data)
{
    d_func_dynamic()->m_url = url;
    d_func_dynamic()->m_modificationTime = ModificationRevision::revisionForFile(url);

    addModificationRevision(url, d_func_dynamic()->m_modificationTime);
    Q_ASSERT(d_func()->m_allModificationRevisions.index());
}

ParsingEnvironmentFile::ParsingEnvironmentFile(const IndexedString& url) : DUChainBase(*new ParsingEnvironmentFileData())
{
    d_func_dynamic()->setClassId(this);

    d_func_dynamic()->m_url = url;
    d_func_dynamic()->m_modificationTime = ModificationRevision::revisionForFile(url);

    addModificationRevision(url, d_func_dynamic()->m_modificationTime);
    Q_ASSERT(d_func()->m_allModificationRevisions.index());
}

TopDUContext* ParsingEnvironmentFile::topContext() const
{
    ENSURE_READ_LOCKED
    return indexedTopContext().data();
}

ParsingEnvironmentFile::~ParsingEnvironmentFile()
{
}

ParsingEnvironmentFile::ParsingEnvironmentFile(ParsingEnvironmentFileData& data) : DUChainBase(data)
{
    //If this triggers, the item has most probably not been initialized with the correct constructor that takes an IndexedString.
    Q_ASSERT(d_func()->m_allModificationRevisions.index());
}

int ParsingEnvironment::type() const
{
    return StandardParsingEnvironment;
}

int ParsingEnvironmentFile::type() const
{
    ENSURE_READ_LOCKED
    return StandardParsingEnvironment;
}

bool ParsingEnvironmentFile::isProxyContext() const
{
    ENSURE_READ_LOCKED
    return d_func()->m_isProxyContext;
}

void ParsingEnvironmentFile::setIsProxyContext(bool is)
{
    ENSURE_WRITE_LOCKED
        d_func_dynamic()->m_isProxyContext = is;
}

QList<QExplicitlySharedDataPointer<ParsingEnvironmentFile>> ParsingEnvironmentFile::imports() const
{
    ENSURE_READ_LOCKED

    QList<IndexedDUContext> imp;
    IndexedTopDUContext top = indexedTopContext();
    if (top.isLoaded()) {
        TopDUContext* topCtx = top.data();
        imp.reserve(topCtx->d_func()->m_importedContextsSize());
        FOREACH_FUNCTION(const DUContext::Import& import, topCtx->d_func()->m_importedContexts)
        imp << import.indexedContext();
    } else {
        imp = TopDUContextDynamicData::loadImports(top.index());
    }

    QList<QExplicitlySharedDataPointer<ParsingEnvironmentFile>> ret;
    for (const IndexedDUContext ctx : std::as_const(imp)) {
        QExplicitlySharedDataPointer<ParsingEnvironmentFile> item = DUChain::self()->environmentFileForDocument(
            ctx.topContextIndex());
        if (item) {
            ret << item;
        } else {
            qCDebug(LANGUAGE) << url().str() << indexedTopContext().index() << ": invalid import" <<
                ctx.topContextIndex();
        }
    }

    return ret;
}

QList<QExplicitlySharedDataPointer<ParsingEnvironmentFile>> ParsingEnvironmentFile::importers() const
{
    ENSURE_READ_LOCKED

    QList<IndexedDUContext> imp;
    IndexedTopDUContext top = indexedTopContext();
    if (top.isLoaded()) {
        TopDUContext* topCtx = top.data();
        FOREACH_FUNCTION(const IndexedDUContext &ctx, topCtx->d_func()->m_importers)
        imp << ctx;
    } else {
        imp = TopDUContextDynamicData::loadImporters(top.index());
    }

    QList<QExplicitlySharedDataPointer<ParsingEnvironmentFile>> ret;
    for (const IndexedDUContext ctx : std::as_const(imp)) {
        QExplicitlySharedDataPointer<ParsingEnvironmentFile> f = DUChain::self()->environmentFileForDocument(
            ctx.topContextIndex());
        if (f)
            ret << f;
        else
            qCDebug(LANGUAGE) << url().str() << indexedTopContext().index() << ": invalid importer context" <<
                ctx.topContextIndex();
    }

    return ret;
}

QMutex featureSatisfactionMutex;

inline bool satisfied(TopDUContext::Features features, TopDUContext::Features required)
{
    return (features & required) == required;
}

///Makes sure the file has the correct features attached, and if minimumFeatures contains AllDeclarationsContextsAndUsesForRecursive, then also checks all imports.
bool ParsingEnvironmentFile::featuresMatch(TopDUContext::Features minimumFeatures,
                                           QSet<const ParsingEnvironmentFile*>& checked) const
{
    if (checked.contains(this))
        return true;

    checked.insert(this);

    auto localRequired = minimumFeatures | ParseJob::staticMinimumFeatures(url());

    //Check other 'local' requirements
    localRequired &= (TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::AST);

    if (!satisfied(features(), localRequired))
        return false;

    if (ParseJob::hasStaticMinimumFeatures()) {
        //Do a manual recursion to check whether any of the relevant contexts has static minimum features set
        ///@todo Only do this if one of the imports actually has static features attached (by RecursiveImports set intersection)
        const auto imports = this->imports();
        for (const ParsingEnvironmentFilePointer& import : imports) {
            if (!import->featuresMatch(minimumFeatures &
                                       TopDUContext::Recursive ? minimumFeatures : TopDUContext::Features{},
                                       checked))
                return false;
        }
    } else if (minimumFeatures & TopDUContext::Recursive) {
        QMutexLocker lock(&featureSatisfactionMutex);

        TopDUContext::IndexedRecursiveImports recursiveImportIndices = d_func()->m_importsCache;
        if (recursiveImportIndices.isEmpty()) {
            //Unfortunately, we have to load the top-context
            TopDUContext* top = topContext();
            if (top)
                recursiveImportIndices = top->recursiveImportIndices();
        }

        ///@todo Do not create temporary intersected sets

        //Use the features-cache to efficiently check the recursive satisfaction of the features
        if (satisfied(minimumFeatures,
                      TopDUContext::AST) &&
            !((m_staticData->ASTSatisfied & recursiveImportIndices) == recursiveImportIndices))
            return false;

        if (satisfied(minimumFeatures, TopDUContext::AllDeclarationsContextsAndUses))
            return (m_staticData->allDeclarationsAndUsesSatisfied & recursiveImportIndices) == recursiveImportIndices;
        else if (satisfied(minimumFeatures, TopDUContext::AllDeclarationsAndContexts))
            return (m_staticData->allDeclarationsSatisfied & recursiveImportIndices) == recursiveImportIndices;
        else if (satisfied(minimumFeatures, TopDUContext::VisibleDeclarationsAndContexts))
            return (m_staticData->visibleDeclarationsSatisfied & recursiveImportIndices) == recursiveImportIndices;
        else if (satisfied(minimumFeatures, TopDUContext::SimplifiedVisibleDeclarationsAndContexts))
            return (m_staticData->simplifiedVisibleDeclarationsSatisfied & recursiveImportIndices) ==
                   recursiveImportIndices;
    }

    return true;
}

void ParsingEnvironmentFile::setFeatures(TopDUContext::Features features)
{
    if (d_func()->m_features == features)
        return;
    ENSURE_WRITE_LOCKED
        d_func_dynamic()->m_features = features;

    if (indexedTopContext().isValid()) {
        QMutexLocker lock(&featureSatisfactionMutex);

        if (!satisfied(features, TopDUContext::SimplifiedVisibleDeclarationsAndContexts))
            m_staticData->simplifiedVisibleDeclarationsSatisfied.remove(indexedTopContext());
        else
            m_staticData->simplifiedVisibleDeclarationsSatisfied.insert(indexedTopContext());

        if (!satisfied(features, TopDUContext::VisibleDeclarationsAndContexts))
            m_staticData->visibleDeclarationsSatisfied.remove(indexedTopContext());
        else
            m_staticData->visibleDeclarationsSatisfied.insert(indexedTopContext());

        if (!satisfied(features, TopDUContext::AllDeclarationsAndContexts))
            m_staticData->allDeclarationsSatisfied.remove(indexedTopContext());
        else
            m_staticData->allDeclarationsSatisfied.insert(indexedTopContext());

        if (!satisfied(features, TopDUContext::AllDeclarationsContextsAndUses))
            m_staticData->allDeclarationsAndUsesSatisfied.remove(indexedTopContext());
        else
            m_staticData->allDeclarationsAndUsesSatisfied.insert(indexedTopContext());

        if (!satisfied(features, TopDUContext::AST))
            m_staticData->ASTSatisfied.remove(indexedTopContext());
        else
            m_staticData->ASTSatisfied.insert(indexedTopContext());
    }
}

bool ParsingEnvironmentFile::featuresSatisfied(KDevelop::TopDUContext::Features minimumFeatures) const
{
    ENSURE_READ_LOCKED
    QSet<const ParsingEnvironmentFile*> checked;
    if (minimumFeatures & TopDUContext::ForceUpdate)
        return false;
    return featuresMatch(minimumFeatures, checked);
}

void ParsingEnvironmentFile::clearModificationRevisions()
{
    ENSURE_WRITE_LOCKED
        d_func_dynamic()->m_allModificationRevisions.clear();
    d_func_dynamic()->m_allModificationRevisions.addModificationRevision(d_func()->m_url, d_func()->m_modificationTime);
}

void ParsingEnvironmentFile::addModificationRevision(const IndexedString& url, const ModificationRevision& revision)
{
    ENSURE_WRITE_LOCKED
        d_func_dynamic()->m_allModificationRevisions.addModificationRevision(url, revision);
    {
        //Test
        Q_ASSERT(d_func_dynamic()->m_allModificationRevisions.index());
        bool result = d_func_dynamic()->m_allModificationRevisions.removeModificationRevision(url, revision);
        Q_UNUSED(result);
        Q_ASSERT(result);
        d_func_dynamic()->m_allModificationRevisions.addModificationRevision(url, revision);
    }
}

void ParsingEnvironmentFile::setModificationRevision(const KDevelop::ModificationRevision& rev)
{
    ENSURE_WRITE_LOCKED

        Q_ASSERT(d_func_dynamic()->m_allModificationRevisions.index());
    bool result = d_func_dynamic()->m_allModificationRevisions.removeModificationRevision(d_func()->m_url,
                                                                                          d_func()->m_modificationTime);
    Q_ASSERT(result);
    Q_UNUSED(result);

  #ifdef LEXERCACHE_DEBUG
    if (debugging()) {
        qCDebug(LANGUAGE) <<  id(this) << "setting modification-revision" << rev.toString();
    }
#endif
    d_func_dynamic()->m_modificationTime = rev;
#ifdef LEXERCACHE_DEBUG
    if (debugging()) {
        qCDebug(LANGUAGE) <<  id(this) << "new modification-revision" << m_modificationTime;
    }
#endif
    d_func_dynamic()->m_allModificationRevisions.addModificationRevision(d_func()->m_url, d_func()->m_modificationTime);
}

KDevelop::ModificationRevision ParsingEnvironmentFile::modificationRevision() const
{
    ENSURE_READ_LOCKED
    return d_func()->m_modificationTime;
}

IndexedString ParsingEnvironmentFile::language() const
{
    return d_func()->m_language;
}

void ParsingEnvironmentFile::setLanguage(const IndexedString& language)
{
    d_func_dynamic()->m_language = language;
}

const KDevelop::TopDUContext::IndexedRecursiveImports& ParsingEnvironmentFile::importsCache() const
{
    return d_func()->m_importsCache;
}

void ParsingEnvironmentFile::setImportsCache(const KDevelop::TopDUContext::IndexedRecursiveImports& importsCache)
{
    d_func_dynamic()->m_importsCache = importsCache;
}
} //KDevelop
