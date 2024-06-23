/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "topducontext.h"
#include "topducontextutils.h"

#include <limits>

#include "persistentsymboltable.h"
#include "problem.h"
#include "declaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "parsingenvironment.h"
#include "duchainpointer.h"
#include "declarationid.h"
#include "namespacealiasdeclaration.h"
#include "aliasdeclaration.h"
#include "uses.h"
#include "topducontextdata.h"
#include "duchainregister.h"
#include "topducontextdynamicdata.h"
#include <debug.h>

#include <language/interfaces/iastcontainer.h>

#include <QMutexLocker>
#include <QRecursiveMutex>

// #define DEBUG_SEARCH

const uint maxApplyAliasesRecursion = 100;

namespace KDevelop {
Utils::BasicSetRepository* RecursiveImportRepository::repository()
{
    static QRecursiveMutex mutex;
    static Utils::BasicSetRepository recursiveImportRepositoryObject(QStringLiteral("Recursive Imports"), &mutex,
                                                                     &KDevelop::globalItemRepositoryRegistry());
    return &recursiveImportRepositoryObject;
}

ReferencedTopDUContext::ReferencedTopDUContext(TopDUContext* context) : m_topContext(context)
{
    if (m_topContext)
        DUChain::self()->refCountUp(m_topContext);
}

ReferencedTopDUContext::ReferencedTopDUContext(const ReferencedTopDUContext& rhs) : m_topContext(rhs.m_topContext)
{
    if (m_topContext)
        DUChain::self()->refCountUp(m_topContext);
}

ReferencedTopDUContext::~ReferencedTopDUContext()
{
    if (m_topContext && !DUChain::deleted())
        DUChain::self()->refCountDown(m_topContext);
}

ReferencedTopDUContext& ReferencedTopDUContext::operator=(const ReferencedTopDUContext& rhs)
{
    if (m_topContext == rhs.m_topContext)
        return *this;

    if (m_topContext)
        DUChain::self()->refCountDown(m_topContext);

    m_topContext = rhs.m_topContext;

    if (m_topContext)
        DUChain::self()->refCountUp(m_topContext);
    return *this;
}

DEFINE_LIST_MEMBER_HASH(TopDUContextData, m_usedDeclarationIds, DeclarationId)
DEFINE_LIST_MEMBER_HASH(TopDUContextData, m_problems, LocalIndexedProblem)
REGISTER_DUCHAIN_ITEM(TopDUContext);

QRecursiveMutex importStructureMutex;

//Contains data that is not shared among top-contexts that share their duchain entries
class TopDUContextLocalPrivate
{
public:
    TopDUContextLocalPrivate (TopDUContext* ctxt, uint index) :
        m_ctxt(ctxt)
        , m_ownIndex(index)
        , m_inDuChain(false)
    {
        m_indexedRecursiveImports.insert(index);
    }

    ~TopDUContextLocalPrivate()
    {
        //Either we use some other contexts data and have no users, or we own the data and have users that share it.
        QMutexLocker lock(&importStructureMutex);

        for (const DUContext::Import& import : qAsConst(m_importedContexts)) {
            if (DUChain::self()->isInMemory(import.topContextIndex()) &&
                dynamic_cast<TopDUContext*>(import.context(nullptr)))
                dynamic_cast<TopDUContext*>(import.context(nullptr))->m_local->m_directImporters.remove(m_ctxt);
        }
    }

    ///@todo Make all this work consistently together with import-caching

    //After loading, should rebuild the links
    void rebuildDynamicImportStructure()
    {
        //Currently we do not store the whole data in TopDUContextLocalPrivate, so we reconstruct it from what was stored by DUContext.
        Q_ASSERT(m_importedContexts.isEmpty());

        FOREACH_FUNCTION(const DUContext::Import& import, m_ctxt->d_func()->m_importedContexts) {
            if (DUChain::self()->isInMemory(import.topContextIndex())) {
                Q_ASSERT(import.context(nullptr));
                TopDUContext* top = import.context(nullptr)->topContext();
                Q_ASSERT(top);
                addImportedContextRecursively(top, false, true);
            }
        }
        FOREACH_FUNCTION(const IndexedDUContext &importer, m_ctxt->d_func()->m_importers) {
            if (DUChain::self()->isInMemory(importer.topContextIndex())) {
                Q_ASSERT(importer.context());
                TopDUContext* top = importer.context()->topContext();
                Q_ASSERT(top);
                top->m_local->addImportedContextRecursively(m_ctxt, false, true);
            }
        }
    }

    //Index of this top-context within the duchain
    //Since the data of top-contexts can be shared among multiple, this can be used to add imports that are local to this top-context.
    QVector<DUContext::Import> m_importedContexts;
//   mutable bool m_haveImportStructure : 1;
    TopDUContext* m_ctxt;

    QSet<DUContext*> m_directImporters;

    ParsingEnvironmentFilePointer m_file;

    QExplicitlySharedDataPointer<IAstContainer> m_ast;

    uint m_ownIndex;

    bool m_inDuChain;

    void clearImportedContextsRecursively()
    {
        QMutexLocker lock(&importStructureMutex);

//     Q_ASSERT(m_recursiveImports.size() == m_indexedRecursiveImports.count()-1);

        QSet<QPair<TopDUContext*, const TopDUContext*>> rebuild;

        for (const DUContext::Import& import : qAsConst(m_importedContexts)) {
            auto* top = dynamic_cast<TopDUContext*>(import.context(nullptr));
            if (top) {
                top->m_local->m_directImporters.remove(m_ctxt);

                if (!m_ctxt->usingImportsCache()) {
                    removeImportedContextRecursion(top, top, 1, rebuild);

                    QHash<const TopDUContext*, QPair<int, const TopDUContext*>> b = top->m_local->m_recursiveImports;
                    for (RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it) {
                        if (m_recursiveImports.contains(it.key()) && m_recursiveImports[it.key()].second == top)
                            removeImportedContextRecursion(top, it.key(), it->first + 1, rebuild); //Remove all contexts that are imported through the context
                    }
                }
            }
        }

        m_importedContexts.clear();

        rebuildImportStructureRecursion(rebuild);

        Q_ASSERT(m_recursiveImports.isEmpty());
//     Q_ASSERT(m_recursiveImports.size() == m_indexedRecursiveImports.count()-1);
    }

    //Adds the context to this and all contexts that import this, and manages m_recursiveImports
    void addImportedContextRecursively(TopDUContext* context, bool temporary, bool local)
    {
        QMutexLocker lock(&importStructureMutex);

        context->m_local->m_directImporters.insert(m_ctxt);

        if (local) {
            // note: m_importedContexts may end up with duplicate entries -- not sure whether we should protect against this --Kevin
            m_importedContexts << DUContext::Import(context, m_ctxt);
        }

        if (!m_ctxt->usingImportsCache()) {
            addImportedContextRecursion(context, context, 1, temporary);

            QHash<const TopDUContext*, QPair<int, const TopDUContext*>> b = context->m_local->m_recursiveImports;
            for (RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it)
                addImportedContextRecursion(context, it.key(), (*it).first + 1, temporary); //Add contexts that were imported earlier into the given one
        }
    }

    //Removes the context from this and all contexts that import this, and manages m_recursiveImports
    void removeImportedContextRecursively(TopDUContext* context, bool local)
    {
        QMutexLocker lock(&importStructureMutex);

        context->m_local->m_directImporters.remove(m_ctxt);

        if (local)
            m_importedContexts.removeAll(DUContext::Import(context, m_ctxt));

        QSet<QPair<TopDUContext*, const TopDUContext*>> rebuild;
        if (!m_ctxt->usingImportsCache()) {
            removeImportedContextRecursion(context, context, 1, rebuild);

            QHash<const TopDUContext*, QPair<int, const TopDUContext*>> b = context->m_local->m_recursiveImports;
            for (RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it) {
                if (m_recursiveImports.contains(it.key()) && m_recursiveImports[it.key()].second == context)
                    removeImportedContextRecursion(context, it.key(), it->first + 1, rebuild); //Remove all contexts that are imported through the context
            }
        }

        rebuildImportStructureRecursion(rebuild);
    }

    void removeImportedContextsRecursively(const QList<TopDUContext*>& contexts, bool local)
    {
        QMutexLocker lock(&importStructureMutex);

        QSet<QPair<TopDUContext*, const TopDUContext*>> rebuild;
        for (TopDUContext* context : contexts) {
            context->m_local->m_directImporters.remove(m_ctxt);

            if (local)
                m_importedContexts.removeAll(DUContext::Import(context, m_ctxt));

            if (!m_ctxt->usingImportsCache()) {
                removeImportedContextRecursion(context, context, 1, rebuild);

                QHash<const TopDUContext*, QPair<int, const TopDUContext*>> b = context->m_local->m_recursiveImports;
                for (RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it) {
                    const auto recursiveImportIt = m_recursiveImports.constFind(it.key());
                    if (recursiveImportIt != m_recursiveImports.constEnd() && recursiveImportIt->second == context)
                        removeImportedContextRecursion(context, it.key(), it->first + 1, rebuild); //Remove all contexts that are imported through the context
                }
            }
        }

        rebuildImportStructureRecursion(rebuild);
    }

    //Has an entry for every single recursively imported file, that contains the shortest path, and the next context on that path to the imported context.
    //This does not need to be stored to disk, because it is defined implicitly.
    //What makes this most complicated is the fact that loops are allowed in the import structure.
    using RecursiveImports = QHash<const TopDUContext*, QPair<int, const TopDUContext*>>;
    mutable RecursiveImports m_recursiveImports;
    mutable TopDUContext::IndexedRecursiveImports m_indexedRecursiveImports;

private:
    void addImportedContextRecursion(const TopDUContext* traceNext, const TopDUContext* imported, int depth,
                                     bool temporary = false)
    {
        if (m_ctxt->usingImportsCache())
            return;

//     if(!m_haveImportStructure)
//       return;

        if (imported == m_ctxt)
            return;

        const bool computeShortestPaths = false; ///@todo We do not compute the shortest path. Think what's right.

//     traceNext->m_local->needImportStructure();
//     imported->m_local->needImportStructure();

        RecursiveImports::iterator it = m_recursiveImports.find(imported);
        if (it == m_recursiveImports.end()) {
            //Insert new path to "imported"
            m_recursiveImports[imported] = qMakePair(depth, traceNext);

            m_indexedRecursiveImports.insert(imported->indexed());
//       Q_ASSERT(m_indexedRecursiveImports.size() == m_recursiveImports.size()+1);

            Q_ASSERT(traceNext != m_ctxt);
        } else {
            if (!computeShortestPaths)
                return;

            if (temporary) //For temporary imports, we don't record the best path.
                return;
            //It would be better if we would use the following code, but it creates too much cost in updateImportedContextRecursion when imports are removed again.

            //Check whether the new way to "imported" is shorter than the stored one
            if ((*it).first > depth) {
                //Add a shorter path
                (*it).first = depth;
                Q_ASSERT(traceNext);
                (*it).second = traceNext;
                Q_ASSERT(traceNext == imported ||
                         (traceNext->m_local->m_recursiveImports.contains(imported) &&
                          traceNext->m_local->m_recursiveImports[imported].first < (*it).first));
            } else {
                //The imported context is already imported through a same/better path, so we can just stop processing. This saves us from endless recursion.
                return;
            }
        }

        if (temporary)
            return;

        for (auto* context : qAsConst(m_directImporters)) {
            auto* top = dynamic_cast<TopDUContext*>(context);
            if (top) ///@todo also record this for local imports
                top->m_local->addImportedContextRecursion(m_ctxt, imported, depth + 1);
        }
    }

    void removeImportedContextRecursion(const TopDUContext* traceNext, const TopDUContext* imported, int distance,
                                        QSet<QPair<TopDUContext*, const TopDUContext*>>& rebuild)
    {
        if (m_ctxt->usingImportsCache())
            return;

        if (imported == m_ctxt)
            return;

//     if(!m_haveImportStructure)
//       return;

        RecursiveImports::iterator it = m_recursiveImports.find(imported);
        if (it == m_recursiveImports.end()) {
            //We don't import. Just return, this saves us from endless recursion.
            return;
        } else {
            //Check whether we have imported "imported" through "traceNext". If not, return. Else find a new trace.
            if ((*it).second == traceNext && (*it).first == distance) {
                //We need to remove the import through traceNext. Check whether there is another imported context that imports it.

                m_recursiveImports.erase(it); //In order to prevent problems, we completely remove everything, and re-add it.
                                              //Just updating these complex structures is very hard.
                Q_ASSERT(imported != m_ctxt);

                m_indexedRecursiveImports.remove(imported->indexed());
//         Q_ASSERT(m_indexedRecursiveImports.size() == m_recursiveImports.size());

                rebuild.insert(qMakePair(m_ctxt, imported));
                //We MUST do this before finding another trace, because else we would create loops
                for (QSet<DUContext*>::const_iterator childIt = m_directImporters.constBegin();
                     childIt != m_directImporters.constEnd(); ++childIt) {
                    auto* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(*childIt)); //Avoid detaching, so use const iterator
                    if (top)
                        top->m_local->removeImportedContextRecursion(m_ctxt, imported, distance + 1, rebuild); //Don't use 'it' from here on, it may be invalid
                }
            }
        }
    }

    //Updates the trace to 'imported'
    void rebuildStructure(const TopDUContext* imported);

    void rebuildImportStructureRecursion(const QSet<QPair<TopDUContext*, const TopDUContext*>>& rebuild)
    {
        for (auto& rebuildPair : rebuild) {
            //for(int a = rebuild.size()-1; a >= 0; --a) {
            //Find the best imported parent
            rebuildPair.first->m_local->rebuildStructure(rebuildPair.second);
        }
    }
};

const TopDUContext::IndexedRecursiveImports& TopDUContext::recursiveImportIndices() const
{
//   No lock-check for performance reasons
    QMutexLocker lock(&importStructureMutex);
    if (!d_func()->m_importsCache.isEmpty())
        return d_func()->m_importsCache;

    return m_local->m_indexedRecursiveImports;
}

void TopDUContextData::updateImportCacheRecursion(uint baseIndex, IndexedTopDUContext currentContext,
                                                  TopDUContext::IndexedRecursiveImports& visited)
{
    if (visited.contains(currentContext.index()))
        return;
    Q_ASSERT(currentContext.index()); //The top-context must be in the repository when this is called
    if (!currentContext.data()) {
        qCDebug(LANGUAGE) << "importing invalid context";
        return;
    }
    visited.insert(currentContext.index());

    const TopDUContextData* currentData = currentContext.data()->topContext()->d_func();
    if (currentData->m_importsCache.contains(baseIndex) || currentData->m_importsCache.isEmpty()) {
        //If we have a loop or no imports-cache is used, we have to look at each import separately.
        const KDevelop::DUContext::Import* imports = currentData->m_importedContexts();
        uint importsSize = currentData->m_importedContextsSize();
        for (uint a = 0; a < importsSize; ++a) {
            IndexedTopDUContext next(imports[a].topContextIndex());
            if (next.isValid())
                updateImportCacheRecursion(baseIndex, next, visited);
        }
    } else {
        //If we don't have a loop with baseIndex, we can safely just merge with the imported importscache
        visited += currentData->m_importsCache;
    }
}

void TopDUContextData::updateImportCacheRecursion(IndexedTopDUContext currentContext, std::set<uint>& visited)
{
    if (visited.find(currentContext.index()) != visited.end())
        return;
    Q_ASSERT(currentContext.index()); //The top-context must be in the repository when this is called
    if (!currentContext.data()) {
        qCDebug(LANGUAGE) << "importing invalid context";
        return;
    }
    visited.insert(currentContext.index());
    const TopDUContextData* currentData = currentContext.data()->topContext()->d_func();
    const KDevelop::DUContext::Import* imports = currentData->m_importedContexts();
    uint importsSize = currentData->m_importedContextsSize();
    for (uint a = 0; a < importsSize; ++a) {
        IndexedTopDUContext next(imports[a].topContextIndex());
        if (next.isValid())
            updateImportCacheRecursion(next, visited);
    }
}

void TopDUContext::updateImportsCache()
{
    QMutexLocker lock(&importStructureMutex);

    const bool use_fully_recursive_import_cache_computation = false;

    if (use_fully_recursive_import_cache_computation) {
        std::set<uint> visited;
        TopDUContextData::updateImportCacheRecursion(this, visited);
        Q_ASSERT(visited.find(ownIndex()) != visited.end());
        d_func_dynamic()->m_importsCache = IndexedRecursiveImports(visited);
    } else {
        d_func_dynamic()->m_importsCache = IndexedRecursiveImports();
        TopDUContextData::updateImportCacheRecursion(ownIndex(), this, d_func_dynamic()->m_importsCache);
    }
    Q_ASSERT(d_func_dynamic()->m_importsCache.contains(IndexedTopDUContext(this)));
    Q_ASSERT(usingImportsCache());
    Q_ASSERT(imports(this, CursorInRevision::invalid()));

    if (parsingEnvironmentFile())
        parsingEnvironmentFile()->setImportsCache(d_func()->m_importsCache);
}

bool TopDUContext::usingImportsCache() const
{
    return !d_func()->m_importsCache.isEmpty();
}

CursorInRevision TopDUContext::importPosition(const DUContext* target) const
{
    ENSURE_CAN_READ
        DUCHAIN_D(DUContext);
    Import import(const_cast<DUContext*>(target), const_cast<TopDUContext*>(this), CursorInRevision::invalid());
    for (unsigned int a = 0; a < d->m_importedContextsSize(); ++a)
        if (d->m_importedContexts()[a] == import)
            return d->m_importedContexts()[a].position;

    return DUContext::importPosition(target);
}

void TopDUContextLocalPrivate::rebuildStructure(const TopDUContext* imported)
{
    if (m_ctxt == imported)
        return;

    for (auto& importedContext : qAsConst(m_importedContexts)) {
        auto* top = dynamic_cast<TopDUContext*>(importedContext.context(nullptr));
        if (top) {
//       top->m_local->needImportStructure();
            if (top == imported) {
                addImportedContextRecursion(top, imported, 1);
            } else {
                RecursiveImports::const_iterator it2 = top->m_local->m_recursiveImports.constFind(imported);
                if (it2 != top->m_local->m_recursiveImports.constEnd()) {
                    addImportedContextRecursion(top, imported, (*it2).first + 1);
                }
            }
        }
    }

    for (unsigned int a = 0; a < m_ctxt->d_func()->m_importedContextsSize(); ++a) {
        auto* top =
            dynamic_cast<TopDUContext*>(const_cast<DUContext*>(m_ctxt->d_func()->m_importedContexts()[a].context(nullptr)));           //To avoid detaching, use const iterator
        if (top) {
//       top->m_local->needImportStructure();
            if (top == imported) {
                addImportedContextRecursion(top, imported, 1);
            } else {
                RecursiveImports::const_iterator it2 = top->m_local->m_recursiveImports.constFind(imported);
                if (it2 != top->m_local->m_recursiveImports.constEnd()) {
                    addImportedContextRecursion(top, imported, (*it2).first + 1);
                }
            }
        }
    }
}

void TopDUContext::rebuildDynamicImportStructure()
{
    m_local->rebuildDynamicImportStructure();
}

void TopDUContext::rebuildDynamicData(DUContext* parent, uint ownIndex)
{
    Q_ASSERT(parent == nullptr && ownIndex != 0);
    m_local->m_ownIndex = ownIndex;

    DUContext::rebuildDynamicData(parent, 0);
}

IndexedTopDUContext TopDUContext::indexed() const
{
    return IndexedTopDUContext(m_local->m_ownIndex);
}

uint TopDUContext::ownIndex() const
{
    return m_local->m_ownIndex;
}

TopDUContext::TopDUContext(TopDUContextData& data) : DUContext(data)
    , m_local(new TopDUContextLocalPrivate(this, data.m_ownIndex))
    , m_dynamicData(new TopDUContextDynamicData(this))
{
    initFromTopContext();
}

TopDUContext::TopDUContext(const IndexedString& url, const RangeInRevision& range, ParsingEnvironmentFile* file)
    : DUContext(*new TopDUContextData(url), range)
    , m_local(new TopDUContextLocalPrivate(this, DUChain::newTopContextIndex()))
    , m_dynamicData(new TopDUContextDynamicData(this))
{
    initFromTopContext();

    Q_ASSERT(url.toUrl().isValid() && !url.toUrl().isRelative());
    d_func_dynamic()->setClassId(this);
    setType(Global);

    DUCHAIN_D_DYNAMIC(TopDUContext);
    d->m_features = VisibleDeclarationsAndContexts;
    d->m_ownIndex = m_local->m_ownIndex;
    setParsingEnvironmentFile(file);
    setInSymbolTable(true);
}

QExplicitlySharedDataPointer<ParsingEnvironmentFile> TopDUContext::parsingEnvironmentFile() const
{
    return m_local->m_file;
}

TopDUContext::~TopDUContext()
{
    m_dynamicData->m_deleting = true;

    //Clear the AST, so that the 'feature satisfaction' cache is eventually updated
    clearAst();

    if (!isOnDisk()) {
        //Clear the 'feature satisfaction' cache which is managed in ParsingEnvironmentFile
        setFeatures(Empty);

        clearUsedDeclarationIndices();
    }

    deleteChildContextsRecursively();
    deleteLocalDeclarations();
    m_dynamicData->clear();
}

void TopDUContext::deleteSelf()
{
    //We've got to make sure that m_dynamicData and m_local are still valid while all the sub-contexts are destroyed
    TopDUContextLocalPrivate* local = m_local;
    TopDUContextDynamicData* dynamicData = m_dynamicData;

    m_dynamicData->m_deleting = true;

    delete this;

    delete local;
    delete dynamicData;
}

TopDUContext::Features TopDUContext::features() const
{
    auto ret = d_func()->m_features;

    if (ast())
        ret |= TopDUContext::AST;

    return ret;
}

void TopDUContext::setFeatures(Features features)
{
    features &= ~Recursive; //Remove the "Recursive" flag since that's only for searching
    features &= ~ForceUpdateRecursive; //Remove the update flags
    features &= ~AST; //Remove the AST flag, it's only used while updating
    d_func_dynamic()->m_features = features;

    //Replicate features to ParsingEnvironmentFile
    if (parsingEnvironmentFile())
        parsingEnvironmentFile()->setFeatures(this->features());
}

void TopDUContext::setAst(const QExplicitlySharedDataPointer<IAstContainer>& ast)
{
    ENSURE_CAN_WRITE
    m_local->m_ast = ast;

    if (parsingEnvironmentFile())
        parsingEnvironmentFile()->setFeatures(features());
}

void TopDUContext::setParsingEnvironmentFile(ParsingEnvironmentFile* file)
{
    if (m_local->m_file) //Clear the "feature satisfaction" cache
        m_local->m_file->setFeatures(Empty);

    //We do not enforce a duchain lock here, since this is also used while loading a top-context
    m_local->m_file = QExplicitlySharedDataPointer<ParsingEnvironmentFile>(file);

    //Replicate features to ParsingEnvironmentFile
    if (file) {
        file->setTopContext(IndexedTopDUContext(ownIndex()));
        Q_ASSERT(file->indexedTopContext().isValid());
        file->setFeatures(d_func()->m_features);

        file->setImportsCache(d_func()->m_importsCache);
    }
}

struct TopDUContext::FindDeclarationsAcceptor
{
    FindDeclarationsAcceptor(const TopDUContext* _top, DeclarationList& _target, const DeclarationChecker& _check,
                             SearchFlags _flags) : top(_top)
        , target(_target)
        , check(_check)
    {
        flags = _flags;
    }

    bool operator()(const QualifiedIdentifier& id)
    {
#ifdef DEBUG_SEARCH
        qCDebug(LANGUAGE) << "accepting" << id.toString();
#endif

        auto visitDeclaration = [&](const IndexedDeclaration& iDecl) {
            Declaration* decl = iDecl.data();

            if (!decl)
                return PersistentSymbolTable::VisitorState::Continue;

            if (!check(decl))
                return PersistentSymbolTable::VisitorState::Continue;

            if (!(flags & DontResolveAliases) && decl->kind() == Declaration::Alias) {
                //Apply alias declarations
                auto* alias = static_cast<AliasDeclaration*>(decl);
                if (alias->aliasedDeclaration().isValid()) {
                    decl = alias->aliasedDeclaration().declaration();
                } else {
                    qCDebug(LANGUAGE) << "lost aliased declaration";
                }
            }

            target.append(decl);
            return PersistentSymbolTable::VisitorState::Continue;
        };

        if (check.flags & DUContext::NoImportsCheck) {
            PersistentSymbolTable::self().visitDeclarations(id, visitDeclaration);
        } else {
            PersistentSymbolTable::self().visitFilteredDeclarations(id, top->recursiveImportIndices(),
                                                                    visitDeclaration);
        }

        check.createVisibleCache = nullptr;

        return !top->foundEnough(target, flags);
    }

    const TopDUContext* top;
    DeclarationList& target;
    const DeclarationChecker& check;
    QFlags<KDevelop::DUContext::SearchFlag> flags;
};

bool TopDUContext::findDeclarationsInternal(const SearchItem::PtrList& identifiers, const CursorInRevision& position,
                                            const AbstractType::Ptr& dataType, DeclarationList& ret,
                                            const TopDUContext* /*source*/, SearchFlags flags, uint /*depth*/) const
{
    ENSURE_CAN_READ

#ifdef DEBUG_SEARCH
    for (const SearchItem::Ptr& idTree : identifiers) {
        const auto ids = idTree->toList();
        for (const QualifiedIdentifier& id : ids) {
            qCDebug(LANGUAGE) << "searching item" << id.toString();
        }
    }

#endif

    DeclarationChecker check(this, position, dataType, flags);
    FindDeclarationsAcceptor storer(this, ret, check, flags);

    ///The actual scopes are found within applyAliases, and each complete qualified identifier is given to FindDeclarationsAcceptor.
    ///That stores the found declaration to the output.
    applyAliases(identifiers, storer, position, false);

    return true;
}

//This is used to prevent endless recursion due to "using namespace .." declarations, by storing all imports that are already being used.
struct TopDUContext::ApplyAliasesBuddyInfo
{
    ApplyAliasesBuddyInfo(uint importChainType, ApplyAliasesBuddyInfo* predecessor,
                          const IndexedQualifiedIdentifier& importId) : m_importChainType(importChainType)
        , m_predecessor(predecessor)
        , m_importId(importId)
    {
        if (m_predecessor && m_predecessor->m_importChainType != importChainType)
            m_predecessor = nullptr;
    }

    bool alreadyImporting(const IndexedQualifiedIdentifier& id)
    {
        ApplyAliasesBuddyInfo* current = this;
        while (current) {
            if (current->m_importId == id)
                return true;
            current = current->m_predecessor;
        }
        return false;
    }

    uint m_importChainType;
    ApplyAliasesBuddyInfo* m_predecessor;
    IndexedQualifiedIdentifier m_importId;
};

///@todo Implement a cache so at least the global import checks don't need to be done repeatedly. The cache should be thread-local, using DUChainPointer for the hashed items, and when an item was deleted, it should be discarded
template <class Acceptor>
bool TopDUContext::applyAliases(const QualifiedIdentifier& previous, const SearchItem::Ptr& identifier,
                                Acceptor& accept, const CursorInRevision& position, bool canBeNamespace,
                                ApplyAliasesBuddyInfo* buddy, uint recursionDepth) const
{
    if (recursionDepth > maxApplyAliasesRecursion) {
        const auto searches = identifier->toList();
        QualifiedIdentifier id;
        if (!searches.isEmpty())
            id = searches.first();

        qCDebug(LANGUAGE) << "maximum apply-aliases recursion reached while searching" << id;
    }
    bool foundAlias = false;

    QualifiedIdentifier id(previous);
    id.push(identifier->identifier);

    if (!id.inRepository())
        return true; //If the qualified identifier is not in the identifier repository, it cannot be registered anywhere, so there's nothing we need to do

    if (!identifier->next.isEmpty() || canBeNamespace) { //If it cannot be a namespace, the last part of the scope will be ignored
        //Search for namespace-aliases, by using globalAliasIdentifier, which is inserted into the symbol-table by NamespaceAliasDeclaration
        QualifiedIdentifier aliasId(id);
        aliasId.push(globalIndexedAliasIdentifier());

#ifdef DEBUG_SEARCH
        qCDebug(LANGUAGE) << "checking" << id.toString();
#endif

        if (aliasId.inRepository()) {
            DeclarationChecker check(this, position, AbstractType::Ptr(), NoSearchFlags, nullptr);

            bool isDone = false;
            // The first part of the identifier has been found as a namespace-alias.
            // In c++, we only need the first alias. However, just to be correct, follow them all for now.
            // This efficiently filters the visible declarations out of all declarations
            PersistentSymbolTable::self().visitFilteredDeclarations(
                aliasId, recursiveImportIndices(), [&](const IndexedDeclaration& indexedAliasDecl) {
                    auto* aliasDecl = indexedAliasDecl.data();
                    if (!aliasDecl)
                        return PersistentSymbolTable::VisitorState::Continue;

                    if (!check(aliasDecl))
                        return PersistentSymbolTable::VisitorState::Continue;

                    if (aliasDecl->kind() != Declaration::NamespaceAlias)
                        return PersistentSymbolTable::VisitorState::Continue;

                    if (foundAlias)
                        return PersistentSymbolTable::VisitorState::Break;

                    Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(aliasDecl));

                    auto* alias = static_cast<NamespaceAliasDeclaration*>(aliasDecl);

                    foundAlias = true;

                    QualifiedIdentifier importIdentifier = alias->importIdentifier();

                    if (importIdentifier.isEmpty()) {
                        qCDebug(LANGUAGE) << "found empty import";
                        return PersistentSymbolTable::VisitorState::Continue;
                    }

                    if (buddy && buddy->alreadyImporting(importIdentifier)) {
                        // This import has already been applied to this search
                        return PersistentSymbolTable::VisitorState::Continue;
                    }

                    ApplyAliasesBuddyInfo info(1, buddy, importIdentifier);

                    if (identifier->next.isEmpty()) {
                        // Just insert the aliased namespace identifier
                        if (!accept(importIdentifier)) {
                            isDone = true;
                            return PersistentSymbolTable::VisitorState::Break;
                        }
                    } else {
                        // Create an identifiers where namespace-alias part is replaced with the alias target
                        for (const SearchItem::Ptr& item : qAsConst(identifier->next)) {
                            if (!applyAliases(importIdentifier, item, accept, position, canBeNamespace, &info,
                                              recursionDepth + 1)) {
                                isDone = true;
                                return PersistentSymbolTable::VisitorState::Break;
                            }
                        }
                    }
                    return PersistentSymbolTable::VisitorState::Continue;
                });

            if (isDone) {
                return false;
            }
        }
    }

    if (!foundAlias) { //If we haven't found an alias, put the current versions into the result list. Additionally we will compute the identifiers transformed through "using".
        if (identifier->next.isEmpty()) {
            if (!accept(id)) //We're at the end of a qualified identifier, accept it
                return false;
        } else {
            for (const SearchItem::Ptr& next : qAsConst(identifier->next)) {
                if (!applyAliases(id, next, accept, position, canBeNamespace, nullptr, recursionDepth + 1))
                    return false;
            }
        }
    }

    /*if( !prefix.explicitlyGlobal() || !prefix.isEmpty() ) {*/ ///@todo check iso c++ if using-directives should be respected on top-level when explicitly global
    ///@todo this is bad for a very big repository(the chains should be walked for the top-context instead)

    //Find all namespace-imports at given scope

    {
        QualifiedIdentifier importId(previous);
        importId.push(globalIndexedImportIdentifier());

#ifdef DEBUG_SEARCH
//   qCDebug(LANGUAGE) << "checking imports in" << (backPointer ? id.toString() : QStringLiteral("global"));
#endif

        if (importId.inRepository()) {
            DeclarationChecker check(this, position, AbstractType::Ptr(), NoSearchFlags, nullptr);
            bool isDone = false;
            //This iterator efficiently filters the visible declarations out of all declarations
            PersistentSymbolTable::self().visitFilteredDeclarations(
                importId, recursiveImportIndices(), [&](const IndexedDeclaration& indexedImportDecl) {
                    Declaration* importDecl = indexedImportDecl.data();
                    if (!importDecl)
                        return PersistentSymbolTable::VisitorState::Continue;

                    //We must never break or return from this loop, because else we might be creating a bad cache
                    if (!check(importDecl))
                        return PersistentSymbolTable::VisitorState::Continue;

                    //Search for the identifier with the import-identifier prepended
                    Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(importDecl));
                    auto* alias = static_cast<NamespaceAliasDeclaration*>(importDecl);

#ifdef DEBUG_SEARCH
                    qCDebug(LANGUAGE) << "found import of" << alias->importIdentifier().toString();
#endif

                    QualifiedIdentifier importIdentifier = alias->importIdentifier();

                    if (importIdentifier.isEmpty()) {
                        qCDebug(LANGUAGE) << "found empty import";
                        return PersistentSymbolTable::VisitorState::Continue;
                    }

                    if (buddy && buddy->alreadyImporting(importIdentifier)) {
                        // This import has already been applied to this search
                        return PersistentSymbolTable::VisitorState::Continue;
                    }

                    ApplyAliasesBuddyInfo info(2, buddy, importIdentifier);

                    if (previous != importIdentifier) {
                        if (!applyAliases(importIdentifier, identifier, accept,
                                          importDecl->topContext() == this ? importDecl->range().start : position,
                                          canBeNamespace, &info, recursionDepth + 1)) {
                            isDone = true;
                            return PersistentSymbolTable::VisitorState::Break;
                        }
                    }
                    return PersistentSymbolTable::VisitorState::Continue;
                });

            if (isDone) {
                return false;
            }
        }
    }
    return true;
}

template <class Acceptor>
void TopDUContext::applyAliases(const SearchItem::PtrList& identifiers, Acceptor& acceptor,
                                const CursorInRevision& position, bool canBeNamespace) const
{
    QualifiedIdentifier emptyId;

    for (const SearchItem::Ptr& item : identifiers)
        applyAliases(emptyId, item, acceptor, position, canBeNamespace, nullptr, 0);
}

TopDUContext* TopDUContext::topContext() const
{
    return const_cast<TopDUContext*>(this);
}

bool TopDUContext::deleting() const
{
    return m_dynamicData->m_deleting;
}

QList<ProblemPointer> TopDUContext::problems() const
{
    ENSURE_CAN_READ

    const auto data = d_func();
    QList<ProblemPointer> ret;
    ret.reserve(data->m_problemsSize());
    for (uint i = 0; i < data->m_problemsSize(); ++i) {
        ret << ProblemPointer(data->m_problems()[i].data(this));
    }

    return ret;
}

void TopDUContext::setProblems(const QList<ProblemPointer>& problems)
{
    ENSURE_CAN_WRITE
        clearProblems();
    for (const auto& problem : problems) {
        addProblem(problem);
    }
}

void TopDUContext::addProblem(const ProblemPointer& problem)
{
    ENSURE_CAN_WRITE

        Q_ASSERT(problem);

    auto data = d_func_dynamic();
    // store for indexing
    LocalIndexedProblem indexedProblem(problem, this);
    Q_ASSERT(indexedProblem.isValid());
    data->m_problemsList().append(indexedProblem);
    Q_ASSERT(indexedProblem.data(this));
}

void TopDUContext::clearProblems()
{
    ENSURE_CAN_WRITE
        d_func_dynamic()->m_problemsList().clear();
    m_dynamicData->clearProblems();
}

QVector<DUContext*> TopDUContext::importers() const
{
    ENSURE_CAN_READ
    const QSet<DUContext*>& directImporters = m_local->m_directImporters;
    return QVector<DUContext*>(directImporters.begin(), directImporters.end());
}

QList<DUContext*> TopDUContext::loadedImporters() const
{
    ENSURE_CAN_READ
    return m_local->m_directImporters.values();
}

QVector<DUContext::Import> TopDUContext::importedParentContexts() const
{
    ENSURE_CAN_READ
    return DUContext::importedParentContexts();
}

bool TopDUContext::imports(const DUContext* origin, const CursorInRevision& position) const
{
    return importsPrivate(origin, position);
}

bool TopDUContext::importsPrivate(const DUContext* origin, const CursorInRevision& position) const
{
    Q_UNUSED(position);

    if (const auto* top = dynamic_cast<const TopDUContext*>(origin)) {
        QMutexLocker lock(&importStructureMutex);
        bool ret = recursiveImportIndices().contains(IndexedTopDUContext(const_cast<TopDUContext*>(top)));
        if (top == this)
            Q_ASSERT(ret);
        return ret;
    } else {
        //Cannot import a non top-context
        return false;
    }
}

void TopDUContext::clearImportedParentContexts()
{
    if (usingImportsCache()) {
        d_func_dynamic()->m_importsCache = IndexedRecursiveImports();
        d_func_dynamic()->m_importsCache.insert(IndexedTopDUContext(this));
    }

    DUContext::clearImportedParentContexts();

    m_local->clearImportedContextsRecursively();

    Q_ASSERT(m_local->m_recursiveImports.count() == 0);

    Q_ASSERT(m_local->m_indexedRecursiveImports.count() == 1);

    Q_ASSERT(imports(this, CursorInRevision::invalid()));
}

void TopDUContext::addImportedParentContext(DUContext* context, const CursorInRevision& position, bool anonymous,
                                            bool temporary)
{
    if (context == this)
        return;

    if (!dynamic_cast<TopDUContext*>(context)) {
        //We cannot do this, because of the extended way we treat top-context imports.
        qCDebug(LANGUAGE) << "tried to import a non top-context into a top-context. This is not possible.";
        return;
    }

    //Always make the contexts anonymous, because we care about importers in TopDUContextLocalPrivate
    DUContext::addImportedParentContext(context, position, anonymous, temporary);

    m_local->addImportedContextRecursively(static_cast<TopDUContext*>(context), temporary, true);
}

void TopDUContext::removeImportedParentContext(DUContext* context)
{
    DUContext::removeImportedParentContext(context);

    m_local->removeImportedContextRecursively(static_cast<TopDUContext*>(context), true);
}

void TopDUContext::addImportedParentContexts(const QVector<QPair<TopDUContext*, CursorInRevision>>& contexts,
                                             bool temporary)
{
    using Pair = QPair<TopDUContext*, CursorInRevision>;

    for (const Pair &pair : contexts) {
        addImportedParentContext(pair.first, pair.second, false, temporary);
    }
}

void TopDUContext::removeImportedParentContexts(const QList<TopDUContext*>& contexts)
{
    for (TopDUContext* context : contexts) {
        DUContext::removeImportedParentContext(context);
    }

    m_local->removeImportedContextsRecursively(contexts, true);
}

/// Returns true if this object is registered in the du-chain. If it is not, all sub-objects(context, declarations, etc.)
bool TopDUContext::inDUChain() const
{
    return m_local->m_inDuChain;
}

/// This flag is only used by DUChain, never change it from outside.
void TopDUContext::setInDuChain(bool b)
{
    m_local->m_inDuChain = b;
}

bool TopDUContext::isOnDisk() const
{
    ///@todo Change this to releasingToDisk, and only enable it while saving a top-context to disk.
    return m_dynamicData->isOnDisk();
}

void TopDUContext::clearUsedDeclarationIndices()
{
    ENSURE_CAN_WRITE
    for (unsigned int a = 0; a < d_func()->m_usedDeclarationIdsSize(); ++a)
        DUChain::uses()->removeUse(d_func()->m_usedDeclarationIds()[a], this);

    d_func_dynamic()->m_usedDeclarationIdsList().clear();
}

void TopDUContext::deleteUsesRecursively()
{
    clearUsedDeclarationIndices();
    KDevelop::DUContext::deleteUsesRecursively();
}

Declaration* TopDUContext::usedDeclarationForIndex(unsigned int declarationIndex) const
{
    ENSURE_CAN_READ
    if (declarationIndex & (1 << 31)) {
        //We use the highest bit to mark direct indices into the local declarations
        declarationIndex &= ~(1 << 31); //unset the highest bit
        return m_dynamicData->declarationForIndex(declarationIndex);
    } else if (declarationIndex < d_func()->m_usedDeclarationIdsSize())
        return d_func()->m_usedDeclarationIds()[declarationIndex].declaration(this);
    else
        return nullptr;
}

int TopDUContext::indexForUsedDeclaration(Declaration* declaration, bool create)
{
    if (create) {
        ENSURE_CAN_WRITE
    } else {
        ENSURE_CAN_READ
    }

    if (!declaration) {
        return std::numeric_limits<int>::max();
    }

    if (declaration->topContext() == this && !declaration->inSymbolTable() &&
        !m_dynamicData->isTemporaryDeclarationIndex(declaration->ownIndex())) {
        uint index = declaration->ownIndex();
        Q_ASSERT(!(index & (1 << 31)));
        return ( int )(index | (1 << 31)); //We don't put context-local declarations into the list, that's a waste. We just use the mark them with the highest bit.
    }

    // if the declaration can not be found from this top-context, we create a direct
    // reference by index, to ensure that the use can be resolved in
    // usedDeclarationForIndex
    bool useDirectId = !recursiveImportIndices().contains(declaration->topContext());
    DeclarationId id(declaration->id(useDirectId));

    int index = -1;

    uint size = d_func()->m_usedDeclarationIdsSize();
    const DeclarationId* ids = d_func()->m_usedDeclarationIds();

    ///@todo Make m_usedDeclarationIds sorted, and find the decl. using binary search
    for (unsigned int a = 0; a < size; ++a)
        if (ids[a] == id) {
            index = a;
            break;
        }

    if (index != -1)
        return index;
    if (!create)
        return std::numeric_limits<int>::max();

    d_func_dynamic()->m_usedDeclarationIdsList().append(id);

    if (declaration->topContext() != this)
        DUChain::uses()->addUse(id, this);

    return d_func()->m_usedDeclarationIdsSize() - 1;
}

QVector<RangeInRevision> allUses(TopDUContext* context, Declaration* declaration, bool noEmptyRanges)
{
    QVector<RangeInRevision> ret;
    int declarationIndex = context->indexForUsedDeclaration(declaration, false);
    if (declarationIndex == std::numeric_limits<int>::max())
        return ret;
    return allUses(context, declarationIndex, noEmptyRanges);
}

QExplicitlySharedDataPointer<IAstContainer> TopDUContext::ast() const
{
    return m_local->m_ast;
}

void TopDUContext::clearAst()
{
    setAst(QExplicitlySharedDataPointer<IAstContainer>(nullptr));
}

IndexedString TopDUContext::url() const
{
    return d_func()->m_url;
}
}
