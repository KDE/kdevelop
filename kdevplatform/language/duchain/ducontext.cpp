/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ducontext.h"

#include <limits>
#include <algorithm>

#include <QSet>

#include "ducontextdata.h"
#include "declaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "use.h"
#include "identifier.h"
#include "topducontext.h"
#include "persistentsymboltable.h"
#include "aliasdeclaration.h"
#include "namespacealiasdeclaration.h"
#include "abstractfunctiondeclaration.h"
#include "duchainregister.h"
#include "topducontextdynamicdata.h"
#include "importers.h"
#include "uses.h"
#include "navigation/abstractdeclarationnavigationcontext.h"
#include "navigation/abstractnavigationwidget.h"
#include "ducontextdynamicdata.h"
#include <debug.h>

// maximum depth for DUContext::findDeclarationsInternal searches
const uint maxParentDepth = 20;

using namespace KTextEditor;

#ifndef NDEBUG
#define ENSURE_CAN_WRITE_(x) {if (x->inDUChain()) { ENSURE_CHAIN_WRITE_LOCKED }}
#define ENSURE_CAN_READ_(x) {if (x->inDUChain()) { ENSURE_CHAIN_READ_LOCKED }}
#else
#define ENSURE_CAN_WRITE_(x)
#define ENSURE_CAN_READ_(x)
#endif

QDebug operator<<(QDebug dbg, const KDevelop::DUContext::Import& import)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Import(" << import.indexedContext().data() << ')';
    return dbg;
}

namespace KDevelop {
DEFINE_LIST_MEMBER_HASH(DUContextData, m_childContexts, LocalIndexedDUContext)
DEFINE_LIST_MEMBER_HASH(DUContextData, m_importers, IndexedDUContext)
DEFINE_LIST_MEMBER_HASH(DUContextData, m_importedContexts, DUContext::Import)
DEFINE_LIST_MEMBER_HASH(DUContextData, m_localDeclarations, LocalIndexedDeclaration)
DEFINE_LIST_MEMBER_HASH(DUContextData, m_uses, Use)

REGISTER_DUCHAIN_ITEM(DUContext);

DUChainVisitor::~DUChainVisitor()
{
}

/**
 * We leak here, to prevent a possible crash during destruction, as the destructor
 * of Identifier is not safe to be called after the duchain has been destroyed
 */
const Identifier& globalImportIdentifier()
{
    static const Identifier globalImportIdentifierObject(QStringLiteral("{...import...}"));
    return globalImportIdentifierObject;
}

const Identifier& globalAliasIdentifier()
{
    static const Identifier globalAliasIdentifierObject(QStringLiteral("{...alias...}"));
    return globalAliasIdentifierObject;
}

const IndexedIdentifier& globalIndexedImportIdentifier()
{
    static const IndexedIdentifier id(globalImportIdentifier());
    return id;
}

const IndexedIdentifier& globalIndexedAliasIdentifier()
{
    static const IndexedIdentifier id(globalAliasIdentifier());
    return id;
}

void DUContext::rebuildDynamicData(DUContext* parent, uint ownIndex)
{
    Q_ASSERT(!parent || ownIndex);
    m_dynamicData->m_topContext = parent ? parent->topContext() : static_cast<TopDUContext*>(this);
    m_dynamicData->m_indexInTopContext = ownIndex;
    m_dynamicData->m_parentContext = DUContextPointer(parent);
    m_dynamicData->m_context = this;

    m_dynamicData->m_childContexts.clear();
    m_dynamicData->m_childContexts.reserve(d_func()->m_childContextsSize());
    FOREACH_FUNCTION(const LocalIndexedDUContext &ctx, d_func()->m_childContexts) {
        m_dynamicData->m_childContexts << ctx.data(m_dynamicData->m_topContext);
    }

    m_dynamicData->m_localDeclarations.clear();
    m_dynamicData->m_localDeclarations.reserve(d_func()->m_localDeclarationsSize());
    FOREACH_FUNCTION(const LocalIndexedDeclaration &idx, d_func()->m_localDeclarations) {
        auto declaration = idx.data(m_dynamicData->m_topContext);
        if (!declaration) {
            qCWarning(LANGUAGE) << "child declaration number" << idx.localIndex() << "of" <<
                d_func_dynamic()->m_localDeclarationsSize() << "is invalid";
            continue;
        }
        m_dynamicData->m_localDeclarations << declaration;
    }

    DUChainBase::rebuildDynamicData(parent, ownIndex);
}

DUContextData::DUContextData()
    : m_inSymbolTable(false)
    , m_anonymousInParent(false)
    , m_propagateDeclarations(false)
{
    initializeAppendedLists();
}

DUContextData::~DUContextData()
{
    freeAppendedLists();
}

DUContextData::DUContextData(const DUContextData& rhs)
    : DUChainBaseData(rhs)
    , m_inSymbolTable(rhs.m_inSymbolTable)
    , m_anonymousInParent(rhs.m_anonymousInParent)
    , m_propagateDeclarations(rhs.m_propagateDeclarations)
{
    initializeAppendedLists();
    copyListsFrom(rhs);
    m_scopeIdentifier = rhs.m_scopeIdentifier;
    m_contextType = rhs.m_contextType;
    m_owner = rhs.m_owner;
}

DUContextDynamicData::DUContextDynamicData(DUContext* d)
    : m_topContext(nullptr)
    , m_indexInTopContext(0)
    , m_context(d)
{
}

void DUContextDynamicData::scopeIdentifier(bool includeClasses, QualifiedIdentifier& target) const
{
    if (m_parentContext)
        m_parentContext->m_dynamicData->scopeIdentifier(includeClasses, target);

    if (includeClasses || d_func()->m_contextType != DUContext::Class)
        target += d_func()->m_scopeIdentifier;
}

bool DUContextDynamicData::imports(const DUContext* context, const TopDUContext* source,
                                   QSet<const DUContextDynamicData*>* recursionGuard) const
{
    if (this == context->m_dynamicData)
        return true;

    if (recursionGuard->contains(this)) {
        return false;
    }
    recursionGuard->insert(this);

    FOREACH_FUNCTION(const DUContext::Import& ctx, d_func()->m_importedContexts) {
        DUContext* import = ctx.context(source);
        if (import == context || (import && import->m_dynamicData->imports(context, source, recursionGuard)))
            return true;
    }

    return false;
}

inline bool isContextTemporary(uint index)
{
    return index > (0xffffffff / 2);
}

void DUContextDynamicData::addDeclaration(Declaration* newDeclaration)
{
    // The definition may not have its identifier set when it's assigned...
    // allow dupes here, TODO catch the error elsewhere

    //If this context is temporary, added declarations should be as well, and viceversa
    Q_ASSERT(isContextTemporary(m_indexInTopContext) == isContextTemporary(newDeclaration->ownIndex()));

    CursorInRevision start = newDeclaration->range().start;

    bool inserted = false;
    ///@todo Do binary search to find the position
    for (int i = m_localDeclarations.size() - 1; i >= 0; --i) {
        Declaration* child = m_localDeclarations[i];
        Q_ASSERT(d_func()->m_localDeclarations()[i].data(m_topContext) == child);
        if (child == newDeclaration)
            return;
        //TODO: All declarations in a macro will have the same empty range, and just get appended
        //that may not be Good Enough in complex cases.
        if (start >= child->range().start) {
            m_localDeclarations.insert(i + 1, newDeclaration);
            d_func_dynamic()->m_localDeclarationsList().insert(i + 1, newDeclaration);
            Q_ASSERT(d_func()->m_localDeclarations()[i + 1].data(m_topContext) == newDeclaration);

            inserted = true;
            break;
        }
    }

    if (!inserted) {
        // We haven't found any child that is before this one, so prepend it
        m_localDeclarations.insert(0, newDeclaration);
        auto& declarations = d_func_dynamic()->m_localDeclarationsList();
        declarations.insert(declarations.begin(), newDeclaration);
        Q_ASSERT(declarations[0].data(m_topContext) == newDeclaration);
    }
}

bool DUContextDynamicData::removeDeclaration(Declaration* declaration)
{
    const int idx = m_localDeclarations.indexOf(declaration);
    if (idx != -1) {
        Q_ASSERT(d_func()->m_localDeclarations()[idx].data(m_topContext) == declaration);
        m_localDeclarations.remove(idx);
        d_func_dynamic()->m_localDeclarationsList().remove(idx);
        return true;
    } else {
        Q_ASSERT(d_func_dynamic()->m_localDeclarationsList().indexOf(LocalIndexedDeclaration(declaration)) == -1);
        return false;
    }
}

void DUContextDynamicData::addChildContext(DUContext* context)
{
    // Internal, don't need to assert a lock
    Q_ASSERT(!context->m_dynamicData->m_parentContext
             || context->m_dynamicData->m_parentContext.data()->m_dynamicData == this);

    LocalIndexedDUContext indexed(context->m_dynamicData->m_indexInTopContext);

    //If this context is temporary, added declarations should be as well, and viceversa
    Q_ASSERT(isContextTemporary(m_indexInTopContext) == isContextTemporary(indexed.localIndex()));

    bool inserted = false;

    int childCount = m_childContexts.size();

    for (int i = childCount - 1; i >= 0; --i) {///@todo Do binary search to find the position
        DUContext* child = m_childContexts[i];
        Q_ASSERT(d_func_dynamic()->m_childContexts()[i] == LocalIndexedDUContext(child));
        if (context == child)
            return;
        if (context->range().start >= child->range().start) {
            m_childContexts.insert(i + 1, context);
            d_func_dynamic()->m_childContextsList().insert(i + 1, indexed);
            context->m_dynamicData->m_parentContext = m_context;
            inserted = true;
            break;
        }
    }

    if (!inserted) {
        m_childContexts.insert(0, context);

        auto& childContexts = d_func_dynamic()->m_childContextsList();
        childContexts.insert(childContexts.begin(), indexed);

        context->m_dynamicData->m_parentContext = m_context;
    }
}

bool DUContextDynamicData::removeChildContext(DUContext* context)
{
//   ENSURE_CAN_WRITE

    const int idx = m_childContexts.indexOf(context);
    if (idx != -1) {
        m_childContexts.remove(idx);
        Q_ASSERT(d_func()->m_childContexts()[idx] == LocalIndexedDUContext(context));
        d_func_dynamic()->m_childContextsList().remove(idx);
        return true;
    } else {
        Q_ASSERT(d_func_dynamic()->m_childContextsList().indexOf(LocalIndexedDUContext(context)) == -1);
        return false;
    }
}

void DUContextDynamicData::addImportedChildContext(DUContext* context)
{
//   ENSURE_CAN_WRITE
    DUContext::Import import(m_context, context);

    if (import.isDirect()) {
        //Direct importers are registered directly within the data
        if (d_func_dynamic()->m_importersList().contains(IndexedDUContext(context))) {
            qCDebug(LANGUAGE) << m_context->scopeIdentifier(true).toString() << "importer added multiple times:" <<
                context->scopeIdentifier(true).toString();
            return;
        }

        d_func_dynamic()->m_importersList().append(context);
    } else {
        //Indirect importers are registered separately
        Importers::self().addImporter(import.indirectDeclarationId(), IndexedDUContext(context));
    }
}

//Can also be called with a context that is not in the list
void DUContextDynamicData::removeImportedChildContext(DUContext* context)
{
//   ENSURE_CAN_WRITE
    DUContext::Import import(m_context, context);

    if (import.isDirect()) {
        d_func_dynamic()->m_importersList().removeOne(IndexedDUContext(context));
    } else {
        //Indirect importers are registered separately
        Importers::self().removeImporter(import.indirectDeclarationId(), IndexedDUContext(context));
    }
}

int DUContext::depth() const
{
    if (!parentContext()) {
        return 0;
    }

    return parentContext()->depth() + 1;
}

DUContext::DUContext(DUContextData& data)
    : DUChainBase(data)
    , m_dynamicData(new DUContextDynamicData(this))
{
}

DUContext::DUContext(const RangeInRevision& range, DUContext* parent, bool anonymous)
    : DUChainBase(*new DUContextData(), range)
    , m_dynamicData(new DUContextDynamicData(this))
{
    d_func_dynamic()->setClassId(this);
    if (parent)
        m_dynamicData->m_topContext = parent->topContext();

    d_func_dynamic()->setClassId(this);
    DUCHAIN_D_DYNAMIC(DUContext);
    d->m_contextType = Other;
    m_dynamicData->m_parentContext = nullptr;

    d->m_anonymousInParent = anonymous;
    d->m_inSymbolTable = false;

    if (parent) {
        m_dynamicData->m_indexInTopContext = parent->topContext()->m_dynamicData->allocateContextIndex(this,
                                                                                                       parent->isAnonymous() ||
                                                                                                       anonymous);
        Q_ASSERT(m_dynamicData->m_indexInTopContext);

        if (!anonymous)
            parent->m_dynamicData->addChildContext(this);
        else
            m_dynamicData->m_parentContext = parent;
    }

    if (parent && !anonymous && parent->inSymbolTable())
        setInSymbolTable(true);
}

bool DUContext::isAnonymous() const
{
    return d_func()->m_anonymousInParent ||
           (m_dynamicData->m_parentContext && m_dynamicData->m_parentContext->isAnonymous());
}

void DUContext::initFromTopContext()
{
    Q_ASSERT(dynamic_cast<TopDUContext*>(this));
    m_dynamicData->m_topContext = static_cast<TopDUContext*>(this);
}

DUContext::DUContext(DUContextData& dd, const RangeInRevision& range, DUContext* parent, bool anonymous)
    : DUChainBase(dd, range)
    , m_dynamicData(new DUContextDynamicData(this))
{
    if (parent)
        m_dynamicData->m_topContext = parent->topContext();
    // else initTopContext must be called, doing a static_cast here is UB

    DUCHAIN_D_DYNAMIC(DUContext);
    d->m_contextType = Other;
    m_dynamicData->m_parentContext = nullptr;
    d->m_inSymbolTable = false;
    d->m_anonymousInParent = anonymous;
    if (parent) {
        m_dynamicData->m_indexInTopContext = parent->topContext()->m_dynamicData->allocateContextIndex(this,
                                                                                                       parent->isAnonymous() ||
                                                                                                       anonymous);

        if (!anonymous)
            parent->m_dynamicData->addChildContext(this);
        else
            m_dynamicData->m_parentContext = parent;
    }
}

DUContext::DUContext(DUContext& useDataFrom)
    : DUChainBase(useDataFrom)
    , m_dynamicData(useDataFrom.m_dynamicData)
{
}

DUContext::~DUContext()
{
    TopDUContext* top = topContext();

    if (top != this) {
        const auto doCleanup = !top->deleting() || !top->isOnDisk();

        if (doCleanup) {
            DUCHAIN_D_DYNAMIC(DUContext);

            if (d->m_owner.declaration())
                d->m_owner.declaration()->setInternalContext(nullptr);

            while (d->m_importersSize() != 0) {
                if (d->m_importers()[0].data())
                    d->m_importers()[0].data()->removeImportedParentContext(this);
                else {
                    qCDebug(LANGUAGE) << "importer disappeared";
                    d->m_importersList().removeOne(d->m_importers()[0]);
                }
            }

            clearImportedParentContexts();
        }

        deleteChildContextsRecursively();

        if (doCleanup)
            deleteUses();

        deleteLocalDeclarations();

        //If the top-context is being delete, we don't need to spend time rebuilding the inner structure.
        //That's expensive, especially when the data is not dynamic.
        if (doCleanup && m_dynamicData->m_parentContext) {
            m_dynamicData->m_parentContext->m_dynamicData->removeChildContext(this);
        }

        top->m_dynamicData->clearContextIndex(this);

        Q_ASSERT(d_func()->isDynamic() ==
                (doCleanup ||
                top->m_dynamicData->isTemporaryContextIndex(m_dynamicData->m_indexInTopContext)));
    }

    delete m_dynamicData;
}

QVector<DUContext*> DUContext::childContexts() const
{
    ENSURE_CAN_READ

    return m_dynamicData->m_childContexts;
}

Declaration* DUContext::owner() const
{
    ENSURE_CAN_READ
    return d_func()->m_owner.declaration();
}

void DUContext::setOwner(Declaration* owner)
{
    ENSURE_CAN_WRITE
        DUCHAIN_D_DYNAMIC(DUContext);
    if (owner == d->m_owner.declaration())
        return;

    Declaration* oldOwner = d->m_owner.declaration();

    d->m_owner = owner;

    //Q_ASSERT(!oldOwner || oldOwner->internalContext() == this);
    if (oldOwner && oldOwner->internalContext() == this)
        oldOwner->setInternalContext(nullptr);

    //The context set as internal context should always be the last opened context
    if (owner)
        owner->setInternalContext(this);
}

DUContext* DUContext::parentContext() const
{
    //ENSURE_CAN_READ Commented out for performance reasons

    return m_dynamicData->m_parentContext.data();
}

void DUContext::setPropagateDeclarations(bool propagate)
{
    ENSURE_CAN_WRITE
        DUCHAIN_D_DYNAMIC(DUContext);

    if (propagate == d->m_propagateDeclarations)
        return;

    d->m_propagateDeclarations = propagate;
}

bool DUContext::isPropagateDeclarations() const
{
    return d_func()->m_propagateDeclarations;
}

QList<Declaration*> DUContext::findLocalDeclarations(const IndexedIdentifier& identifier,
                                                     const CursorInRevision& position,
                                                     const TopDUContext* topContext,
                                                     const AbstractType::Ptr& dataType,
                                                     SearchFlags flags) const
{
    ENSURE_CAN_READ

    DeclarationList ret;
    findLocalDeclarationsInternal(identifier,
                                  position.isValid() ? position : range().end, dataType, ret,
                                  topContext ? topContext : this->topContext(), flags);
    return ret;
}

QList<Declaration*> DUContext::findLocalDeclarations(const Identifier& identifier,
                                                     const CursorInRevision& position,
                                                     const TopDUContext* topContext,
                                                     const AbstractType::Ptr& dataType,
                                                     SearchFlags flags) const
{
    return findLocalDeclarations(IndexedIdentifier(identifier), position, topContext, dataType, flags);
}

namespace {
bool contextIsChildOrEqual(const DUContext* childContext, const DUContext* context)
{
    if (childContext == context)
        return true;

    if (childContext->parentContext())
        return contextIsChildOrEqual(childContext->parentContext(), context);
    else
        return false;
}

struct Checker
{
    Checker(DUContext::SearchFlags flags, const AbstractType::Ptr& dataType,
            const CursorInRevision& position, DUContext::ContextType ownType)
        : m_flags(flags)
        , m_dataType(dataType)
        , m_position(position)
        , m_ownType(ownType)
    {
    }

    Declaration* check(Declaration* declaration) const
    {
        ///@todo This is C++-specific
        if (m_ownType != DUContext::Class && m_ownType != DUContext::Template
            && m_position.isValid() && m_position <= declaration->range().start) {
            return nullptr;
        }

        if (declaration->kind() == Declaration::Alias && !(m_flags & DUContext::DontResolveAliases)) {
            //Apply alias declarations
            auto* alias = static_cast<AliasDeclaration*>(declaration);
            if (alias->aliasedDeclaration().isValid()) {
                declaration = alias->aliasedDeclaration().declaration();
            } else {
                qCDebug(LANGUAGE) << "lost aliased declaration";
            }
        }

        if (declaration->kind() == Declaration::NamespaceAlias && !(m_flags & DUContext::NoFiltering)) {
            return nullptr;
        }

        if (( m_flags& DUContext::OnlyFunctions ) && !declaration->isFunctionDeclaration()) {
            return nullptr;
        }

        if (m_dataType && m_dataType->indexed() != declaration->indexedType()) {
            return nullptr;
        }

        return declaration;
    }

    DUContext::SearchFlags m_flags;
    const AbstractType::Ptr m_dataType;
    const CursorInRevision m_position;
    DUContext::ContextType m_ownType;
};
}

void DUContext::findLocalDeclarationsInternal(const Identifier& identifier, const CursorInRevision& position,
                                              const AbstractType::Ptr& dataType, DeclarationList& ret,
                                              const TopDUContext* source, SearchFlags flags) const
{
    findLocalDeclarationsInternal(IndexedIdentifier(identifier), position, dataType, ret, source, flags);
}

void DUContext::findLocalDeclarationsInternal(const IndexedIdentifier& identifier,
                                              const CursorInRevision& position,
                                              const AbstractType::Ptr& dataType,
                                              DeclarationList& ret, const TopDUContext* /*source*/,
                                              SearchFlags flags) const
{
    Checker checker(flags, dataType, position, type());

    DUCHAIN_D(DUContext);
    if (d->m_inSymbolTable && !d->m_scopeIdentifier.isEmpty() && !identifier.isEmpty()) {
        //This context is in the symbol table, use the symbol-table to speed up the search
        QualifiedIdentifier id(scopeIdentifier(true) + identifier);

        TopDUContext* top = topContext();

        PersistentSymbolTable::self().visitDeclarations(id, [&](const IndexedDeclaration& indexedDecl) {
            ///@todo Eventually do efficient iteration-free filtering
            if (indexedDecl.topContextIndex() == top->ownIndex()) {
                Declaration* decl = indexedDecl.declaration();
                if (decl && contextIsChildOrEqual(decl->context(), this)) {
                    Declaration* checked = checker.check(decl);
                    if (checked) {
                        ret.append(checked);
                    }
                }
            }
            return PersistentSymbolTable::VisitorState::Continue;
        });
    } else {
        //Iterate through all declarations
        DUContextDynamicData::VisibleDeclarationIterator it(m_dynamicData);
        while (it) {
            Declaration* declaration = *it;
            if (declaration && declaration->indexedIdentifier() == identifier) {
                Declaration* checked = checker.check(declaration);
                if (checked)
                    ret.append(checked);
            }
            ++it;
        }
    }
}

bool DUContext::foundEnough(const DeclarationList& ret, SearchFlags flags) const
{
    if (!ret.isEmpty() && !(flags & DUContext::NoFiltering))
        return true;
    else
        return false;
}

bool DUContext::findDeclarationsInternal(const SearchItem::PtrList& baseIdentifiers,
                                         const CursorInRevision& position,
                                         const AbstractType::Ptr& dataType,
                                         DeclarationList& ret, const TopDUContext* source,
                                         SearchFlags flags, uint depth) const
{
    if (depth > maxParentDepth) {
        qCDebug(LANGUAGE) << "maximum depth reached in" << scopeIdentifier(true);
        return false;
    }

    DUCHAIN_D(DUContext);
    if (d->m_contextType != Namespace) {
        // If we're in a namespace, delay all the searching into the top-context, because only that has the overview to pick the correct declarations.
        for (auto& baseIdentifier : baseIdentifiers) {
            if (!baseIdentifier->isExplicitlyGlobal && baseIdentifier->next.isEmpty()) {
                // It makes no sense searching locally for qualified identifiers
                findLocalDeclarationsInternal(baseIdentifier->identifier, position, dataType, ret, source, flags);
            }
        }

        if (foundEnough(ret, flags)) {
            return true;
        }
    }

    ///Step 1: Apply namespace-aliases and -imports
    SearchItem::PtrList aliasedIdentifiers;
    //Because of namespace-imports and aliases, this identifier may need to be searched under multiple names
    applyAliases(baseIdentifiers, aliasedIdentifiers, position, false,
                 type() != DUContext::Namespace && type() != DUContext::Global);

    if (d->m_importedContextsSize() != 0) {
        ///Step 2: Give identifiers that are not marked as explicitly-global to imported contexts(explicitly global ones are treatead in TopDUContext)
        SearchItem::PtrList nonGlobalIdentifiers;
        for (const SearchItem::Ptr& identifier : std::as_const(aliasedIdentifiers)) {
            if (!identifier->isExplicitlyGlobal) {
                nonGlobalIdentifiers << identifier;
            }
        }

        if (!nonGlobalIdentifiers.isEmpty()) {
            const auto& url = this->url();
            for (int import = d->m_importedContextsSize() - 1; import >= 0; --import) {
                if (position.isValid() && d->m_importedContexts()[import].position.isValid() &&
                    position < d->m_importedContexts()[import].position) {
                    continue;
                }

                DUContext* context = d->m_importedContexts()[import].context(source);

                if (!context) {
                    continue;
                } else if (context == this) {
                    qCDebug(LANGUAGE) << "resolved self as import:" << scopeIdentifier(true);
                    continue;
                }

                if (!context->findDeclarationsInternal(nonGlobalIdentifiers,
                                                       url == context->url() ? position : context->range().end,
                                                       dataType, ret, source, flags | InImportedParentContext,
                                                       depth + 1)) {
                    return false;
                }
            }
        }
    }

    if (foundEnough(ret, flags)) {
        return true;
    }

    ///Step 3: Continue search in parent-context
    if (!(flags & DontSearchInParent) && shouldSearchInParent(flags) && m_dynamicData->m_parentContext) {
        applyUpwardsAliases(aliasedIdentifiers, source);
        return m_dynamicData->m_parentContext->findDeclarationsInternal(aliasedIdentifiers,
                                                                        url() == m_dynamicData->m_parentContext->url() ? position : m_dynamicData->m_parentContext->range().end,
                                                                        dataType, ret, source, flags, depth);
    }
    return true;
}

QVector<QualifiedIdentifier> DUContext::fullyApplyAliases(const QualifiedIdentifier& id,
                                                          const TopDUContext* source) const
{
    ENSURE_CAN_READ

    if (!source)
        source = topContext();

    SearchItem::PtrList identifiers;
    identifiers << SearchItem::Ptr(new SearchItem(id));

    const DUContext* current = this;
    while (current) {
        SearchItem::PtrList aliasedIdentifiers;
        current->applyAliases(identifiers, aliasedIdentifiers, CursorInRevision::invalid(), true, false);
        current->applyUpwardsAliases(identifiers, source);

        current = current->parentContext();
    }

    QVector<QualifiedIdentifier> ret;
    for (const SearchItem::Ptr& item : std::as_const(identifiers)) {
        ret += item->toList();
    }

    return ret;
}

QList<Declaration*> DUContext::findDeclarations(const QualifiedIdentifier& identifier,
                                                const CursorInRevision& position,
                                                const AbstractType::Ptr& dataType,
                                                const TopDUContext* topContext, SearchFlags flags) const
{
    ENSURE_CAN_READ

    DeclarationList ret;
    // optimize: we don't want to allocate the top node always
    // so create it on stack but ref it so its not deleted by the smart pointer
    SearchItem item(identifier);
    item.ref.ref();

    SearchItem::PtrList identifiers{SearchItem::Ptr(&item)};

    findDeclarationsInternal(identifiers,
                             position.isValid() ? position : range().end, dataType, ret,
                             topContext ? topContext : this->topContext(), flags, 0);

    return ret;
}

bool DUContext::imports(const DUContext* origin, const CursorInRevision& /*position*/) const
{
    ENSURE_CAN_READ

    QSet<const DUContextDynamicData*> recursionGuard;
    recursionGuard.reserve(8);
    return m_dynamicData->imports(origin, topContext(), &recursionGuard);
}

bool DUContext::addIndirectImport(const DUContext::Import& import)
{
    ENSURE_CAN_WRITE
        DUCHAIN_D_DYNAMIC(DUContext);

    for (unsigned int a = 0; a < d->m_importedContextsSize(); ++a) {
        if (d->m_importedContexts()[a] == import) {
            d->m_importedContextsList()[a].position = import.position;
            return true;
        }
    }

    ///Do not sort the imported contexts by their own line-number, it makes no sense.
    ///Contexts added first, aka template-contexts, should stay in first place, so they are searched first.

    d->m_importedContextsList().append(import);
    return false;
}

void DUContext::addImportedParentContext(DUContext* context, const CursorInRevision& position, bool anonymous,
                                         bool /*temporary*/)
{
    ENSURE_CAN_WRITE

    if (context == this) {
        qCDebug(LANGUAGE) << "Tried to import self";
        return;
    }
    if (!context) {
        qCDebug(LANGUAGE) << "Tried to import invalid context";
        return;
    }

    Import import(context, this, position);
    if (addIndirectImport(import))
        return;

    if (!anonymous) {
        ENSURE_CAN_WRITE_(context)
        context->m_dynamicData->addImportedChildContext(this);
    }
}

void DUContext::removeImportedParentContext(DUContext* context)
{
    ENSURE_CAN_WRITE
        DUCHAIN_D_DYNAMIC(DUContext);

    Import import(context, this, CursorInRevision::invalid());

    for (unsigned int a = 0; a < d->m_importedContextsSize(); ++a) {
        if (d->m_importedContexts()[a] == import) {
            d->m_importedContextsList().remove(a);
            break;
        }
    }

    if (!context)
        return;

    context->m_dynamicData->removeImportedChildContext(this);
}

KDevVarLengthArray<IndexedDUContext> DUContext::indexedImporters() const
{
    KDevVarLengthArray<IndexedDUContext> ret;
    if (owner())
        ret = Importers::self().importers(owner()->id()); //Add indirect importers to the list

    FOREACH_FUNCTION(const IndexedDUContext &ctx, d_func()->m_importers)
    ret.append(ctx);

    return ret;
}

QVector<DUContext*> DUContext::importers() const
{
    ENSURE_CAN_READ

    QVector<DUContext*> ret;
    ret.reserve(d_func()->m_importersSize());
    FOREACH_FUNCTION(const IndexedDUContext &ctx, d_func()->m_importers)
    ret << ctx.context();

    if (owner()) {
        //Add indirect importers to the list
        const KDevVarLengthArray<IndexedDUContext> indirect = Importers::self().importers(owner()->id());
        ret.reserve(ret.size() + indirect.size());
        for (const IndexedDUContext ctx : indirect) {
            ret << ctx.context();
        }
    }

    return ret;
}

DUContext* DUContext::findContext(const CursorInRevision& position, DUContext* parent) const
{
    ENSURE_CAN_READ

    if (!parent)
        parent = const_cast<DUContext*>(this);

    for (DUContext* context : std::as_const(parent->m_dynamicData->m_childContexts)) {
        if (context->range().contains(position)) {
            DUContext* ret = findContext(position, context);
            if (!ret) {
                ret = context;
            }

            return ret;
        }
    }

    return nullptr;
}

bool DUContext::parentContextOf(DUContext* context) const
{
    if (this == context)
        return true;

    const auto& childContexts = m_dynamicData->m_childContexts;
    return std::any_of(childContexts.begin(), childContexts.end(), [&](DUContext* child) {
        return child->parentContextOf(context);
    });
}

QVector<QPair<Declaration*, int>> DUContext::allDeclarations(const CursorInRevision& position,
                                                             const TopDUContext* topContext,
                                                             bool searchInParents) const
{
    ENSURE_CAN_READ

    QVector<QPair<Declaration*, int>> ret;

    QHash<const DUContext*, bool> hadContexts;
    // Iterate back up the chain
    mergeDeclarationsInternal(ret, position, hadContexts, topContext ? topContext : this->topContext(),
                              searchInParents);

    return ret;
}

QVector<Declaration*> DUContext::localDeclarations(const TopDUContext* source) const
{
    ENSURE_CAN_READ
    // TODO: remove this parameter once we kill old-cpp
        Q_UNUSED(source);
    return m_dynamicData->m_localDeclarations;
}

void DUContext::mergeDeclarationsInternal(QVector<QPair<Declaration*, int>>& definitions,
                                          const CursorInRevision& position,
                                          QHash<const DUContext*, bool>& hadContexts,
                                          const TopDUContext* source,
                                          bool searchInParents, int currentDepth) const
{
    ENSURE_CAN_READ

    if ((currentDepth > 300 && currentDepth < 1000) || currentDepth > 1300) {
        qCDebug(LANGUAGE) << "too much depth";
        return;
    }
    DUCHAIN_D(DUContext);

    if (hadContexts.contains(this) && !searchInParents)
        return;

    if (!hadContexts.contains(this)) {
        hadContexts[this] = true;

        if ((type() == DUContext::Namespace || type() == DUContext::Global) && currentDepth < 1000)
            currentDepth += 1000;

        {
            DUContextDynamicData::VisibleDeclarationIterator it(m_dynamicData);
            while (it) {
                Declaration* decl = *it;

                if (decl && (!position.isValid() || decl->range().start <= position))
                    definitions << qMakePair(decl, currentDepth);
                ++it;
            }
        }

        for (int a = d->m_importedContextsSize() - 1; a >= 0; --a) {
            const Import* import(&d->m_importedContexts()[a]);
            DUContext* context = import->context(source);
            while (!context && a > 0) {
                --a;
                import = &d->m_importedContexts()[a];
                context = import->context(source);
            }
            if (!context)
                break;

            if (context == this) {
                qCDebug(LANGUAGE) << "resolved self as import:" << scopeIdentifier(true);
                continue;
            }

            if (position.isValid() && import->position.isValid() && position < import->position)
                continue;

            context->mergeDeclarationsInternal(definitions,
                                               CursorInRevision::invalid(), hadContexts, source,
                                               searchInParents && context->shouldSearchInParent(
                                                   InImportedParentContext) &&  context->parentContext()->type() == DUContext::Helper,
                                               currentDepth + 1);
        }
    }

    ///Only respect the position if the parent-context is not a class(@todo this is language-dependent)
    if (parentContext() && searchInParents)
        parentContext()->mergeDeclarationsInternal(definitions,
                                                   parentContext()->type() == DUContext::Class ? parentContext()->range().end : position, hadContexts, source, searchInParents,
                                                   currentDepth + 1);
}

void DUContext::deleteLocalDeclarations()
{
    ENSURE_CAN_WRITE
    // It may happen that the deletion of one declaration triggers the deletion of another one
    // Therefore we copy the list of indexed declarations and work on those. Indexed declarations
    // will return zero for already deleted declarations.
    KDevVarLengthArray<LocalIndexedDeclaration> indexedLocal;
    if (d_func()->m_localDeclarations()) {
        indexedLocal.append(d_func()->m_localDeclarations(), d_func()->m_localDeclarationsSize());
    }
    for (const LocalIndexedDeclaration& indexed : indexedLocal) {
        delete indexed.data(topContext());
    }

    m_dynamicData->m_localDeclarations.clear();
}

void DUContext::deleteChildContextsRecursively()
{
    ENSURE_CAN_WRITE

    // note: operate on copy here because child ctx deletion changes m_dynamicData->m_childContexts
    const auto currentChildContexts = m_dynamicData->m_childContexts;
    qDeleteAll(currentChildContexts);

    m_dynamicData->m_childContexts.clear();
}

QVector<Declaration*> DUContext::clearLocalDeclarations()
{
    auto copy = m_dynamicData->m_localDeclarations;
    for (Declaration* dec : std::as_const(copy)) {
        dec->setContext(nullptr);
    }

    return copy;
}

QualifiedIdentifier DUContext::scopeIdentifier(bool includeClasses) const
{
    ENSURE_CAN_READ

    QualifiedIdentifier ret;
    m_dynamicData->scopeIdentifier(includeClasses, ret);

    return ret;
}

bool DUContext::equalScopeIdentifier(const DUContext* rhs) const
{
    ENSURE_CAN_READ

    const DUContext* left = this;
    const DUContext* right = rhs;

    while (left || right) {
        if (!left || !right)
            return false;

        if (!(left->d_func()->m_scopeIdentifier == right->d_func()->m_scopeIdentifier))
            return false;

        left = left->parentContext();
        right = right->parentContext();
    }

    return true;
}

void DUContext::setLocalScopeIdentifier(const QualifiedIdentifier& identifier)
{
    ENSURE_CAN_WRITE
    bool wasInSymbolTable = inSymbolTable();
    setInSymbolTable(false);
    d_func_dynamic()->m_scopeIdentifier = identifier;
    setInSymbolTable(wasInSymbolTable);
}

QualifiedIdentifier DUContext::localScopeIdentifier() const
{
    //ENSURE_CAN_READ Commented out for performance reasons

    return d_func()->m_scopeIdentifier;
}

IndexedQualifiedIdentifier DUContext::indexedLocalScopeIdentifier() const
{
    return d_func()->m_scopeIdentifier;
}

DUContext::ContextType DUContext::type() const
{
    //ENSURE_CAN_READ This is disabled, because type() is called very often while searching, and it costs us performance

    return d_func()->m_contextType;
}

void DUContext::setType(ContextType type)
{
    ENSURE_CAN_WRITE

        d_func_dynamic()->m_contextType = type;
}

QList<Declaration*> DUContext::findDeclarations(const Identifier& identifier, const CursorInRevision& position,
                                                const TopDUContext* topContext, SearchFlags flags) const
{
    return findDeclarations(IndexedIdentifier(identifier), position, topContext, flags);
}

QList<Declaration*> DUContext::findDeclarations(const IndexedIdentifier& identifier, const CursorInRevision& position,
                                                const TopDUContext* topContext, SearchFlags flags) const
{
    ENSURE_CAN_READ

    DeclarationList ret;
    SearchItem::PtrList identifiers;
    identifiers << SearchItem::Ptr(new SearchItem(false, identifier, SearchItem::PtrList()));
    findDeclarationsInternal(identifiers, position.isValid() ? position : range().end,
                             AbstractType::Ptr(), ret, topContext ? topContext : this->topContext(), flags, 0);
    return ret;
}

void DUContext::deleteUse(int index)
{
    ENSURE_CAN_WRITE
        DUCHAIN_D_DYNAMIC(DUContext);
    d->m_usesList().remove(index);
}

void DUContext::deleteUses()
{
    ENSURE_CAN_WRITE

        DUCHAIN_D_DYNAMIC(DUContext);
    d->m_usesList().clear();
}

void DUContext::deleteUsesRecursively()
{
    deleteUses();

    for (DUContext* childContext : std::as_const(m_dynamicData->m_childContexts)) {
        childContext->deleteUsesRecursively();
    }
}

bool DUContext::inDUChain() const
{
    if (d_func()->m_anonymousInParent || !m_dynamicData->m_parentContext)
        return false;

    TopDUContext* top = topContext();
    return top && top->inDUChain();
}

DUContext* DUContext::specialize(const IndexedInstantiationInformation& /*specialization*/,
                                 const TopDUContext* topContext, int /*upDistance*/)
{
    if (!topContext)
        return nullptr;
    return this;
}

CursorInRevision DUContext::importPosition(const DUContext* target) const
{
    ENSURE_CAN_READ
        DUCHAIN_D(DUContext);
    Import import(const_cast<DUContext*>(target), this, CursorInRevision::invalid());
    for (unsigned int a = 0; a < d->m_importedContextsSize(); ++a)
        if (d->m_importedContexts()[a] == import)
            return d->m_importedContexts()[a].position;

    return CursorInRevision::invalid();
}

QVector<DUContext::Import> DUContext::importedParentContexts() const
{
    ENSURE_CAN_READ
    QVector<DUContext::Import> ret;
    ret.reserve(d_func()->m_importedContextsSize());
    FOREACH_FUNCTION(const DUContext::Import& import, d_func()->m_importedContexts)
    ret << import;
    return ret;
}

void DUContext::applyAliases(const SearchItem::PtrList& baseIdentifiers, SearchItem::PtrList& identifiers,
                             const CursorInRevision& position, bool canBeNamespace, bool onlyImports) const
{
    DeclarationList imports;
    findLocalDeclarationsInternal(globalIndexedImportIdentifier(), position, AbstractType::Ptr(), imports,
                                  topContext(), DUContext::NoFiltering);

    if (imports.isEmpty() && onlyImports) {
        identifiers = baseIdentifiers;
        return;
    }

    for (const SearchItem::Ptr& identifier : baseIdentifiers) {
        bool addUnmodified = true;

        if (!identifier->isExplicitlyGlobal) {
            if (!imports.isEmpty()) {
                //We have namespace-imports.
                for (Declaration* importDecl : std::as_const(imports)) {
                    //Search for the identifier with the import-identifier prepended
                    if (dynamic_cast<NamespaceAliasDeclaration*>(importDecl)) {
                        auto* alias = static_cast<NamespaceAliasDeclaration*>(importDecl);
                        identifiers.append(SearchItem::Ptr(new SearchItem(alias->importIdentifier(), identifier)));
                    } else {
                        qCDebug(LANGUAGE) << "Declaration with namespace alias identifier has the wrong type" <<
                            importDecl->url().str() << importDecl->range().castToSimpleRange();
                    }
                }
            }

            if (!identifier->isEmpty() && (identifier->hasNext() || canBeNamespace)) {
                DeclarationList aliases;
                findLocalDeclarationsInternal(identifier->identifier, position,
                                              AbstractType::Ptr(), imports, nullptr, DUContext::NoFiltering);

                if (!aliases.isEmpty()) {
                    //The first part of the identifier has been found as a namespace-alias.
                    //In c++, we only need the first alias. However, just to be correct, follow them all for now.
                    for (Declaration* aliasDecl : std::as_const(aliases)) {
                        if (!dynamic_cast<NamespaceAliasDeclaration*>(aliasDecl))
                            continue;

                        addUnmodified = false; //The un-modified identifier can be ignored, because it will be replaced with the resolved alias
                        auto* alias = static_cast<NamespaceAliasDeclaration*>(aliasDecl);

                        //Create an identifier where namespace-alias part is replaced with the alias target
                        identifiers.append(SearchItem::Ptr(new SearchItem(alias->importIdentifier(),
                                                                          identifier->next)));
                    }
                }
            }
        }

        if (addUnmodified)
            identifiers.append(identifier);
    }
}

void DUContext::applyUpwardsAliases(SearchItem::PtrList& identifiers, const TopDUContext* /*source*/) const
{
    if (type() == Namespace) {
        if (d_func()->m_scopeIdentifier.isEmpty())
            return;

        //Make sure we search for the items in all namespaces of the same name, by duplicating each one with the namespace-identifier prepended.
        //We do this by prepending items to the current identifiers that equal the local scope identifier.
        SearchItem::Ptr newItem(new SearchItem(d_func()->m_scopeIdentifier.identifier()));

        //This will exclude explicitly global identifiers
        newItem->addToEachNode(identifiers);

        if (!newItem->next.isEmpty()) {
            //Prepend the full scope before newItem
            DUContext* parent = m_dynamicData->m_parentContext.data();
            while (parent) {
                newItem = SearchItem::Ptr(new SearchItem(parent->d_func()->m_scopeIdentifier, newItem));
                parent = parent->m_dynamicData->m_parentContext.data();
            }

            newItem->isExplicitlyGlobal = true;
            identifiers.insert(identifiers.begin(), newItem);
        }
    }
}

bool DUContext::shouldSearchInParent(SearchFlags flags) const
{
    return (parentContext() && parentContext()->type() == DUContext::Helper && (flags & InImportedParentContext))
           || !(flags & InImportedParentContext);
}

const Use* DUContext::uses() const
{
    ENSURE_CAN_READ

    return d_func()->m_uses();
}

bool DUContext::declarationHasUses(Declaration* decl)
{
    return DUChain::uses()->hasUses(decl->id());
}

int DUContext::usesCount() const
{
    return d_func()->m_usesSize();
}

bool usesRangeLessThan(const Use& left, const Use& right)
{
    return left.m_range.start < right.m_range.start;
}

int DUContext::createUse(int declarationIndex, const RangeInRevision& range, int insertBefore)
{
    DUCHAIN_D_DYNAMIC(DUContext);
    ENSURE_CAN_WRITE

    Use use(range, declarationIndex);
    if (insertBefore == -1) {
        //Find position where to insert
        const unsigned int size = d->m_usesSize();
        const Use* uses = d->m_uses();
        const Use* lowerBound = std::lower_bound(uses, uses + size, use, usesRangeLessThan);
        insertBefore = lowerBound - uses;
        // comment out to test this:
        /*
           unsigned int a = 0;
           for(; a < size && range.start > uses[a].m_range.start; ++a) {
           }
           Q_ASSERT(a == insertBefore);
         */
    }

    d->m_usesList().insert(insertBefore, use);

    return insertBefore;
}

void DUContext::changeUseRange(int useIndex, const RangeInRevision& range)
{
    ENSURE_CAN_WRITE
        d_func_dynamic()->m_usesList()[useIndex].m_range = range;
}

void DUContext::setUseDeclaration(int useNumber, int declarationIndex)
{
    ENSURE_CAN_WRITE
        d_func_dynamic()->m_usesList()[useNumber].m_declarationIndex = declarationIndex;
}

DUContext* DUContext::findContextAt(const CursorInRevision& position, bool includeRightBorder) const
{
    ENSURE_CAN_READ

//   qCDebug(LANGUAGE) << "searching" << position << "in:" << scopeIdentifier(true).toString() << range() << includeRightBorder;

    if (!range().contains(position) && (!includeRightBorder || range().end != position)) {
//     qCDebug(LANGUAGE) << "mismatch";
        return nullptr;
    }

    const auto childContexts = m_dynamicData->m_childContexts;
    for (int a = childContexts.size() - 1; a >= 0; --a) {
        if (DUContext* specific = childContexts[a]->findContextAt(position, includeRightBorder)) {
            return specific;
        }
    }

    return const_cast<DUContext*>(this);
}

Declaration* DUContext::findDeclarationAt(const CursorInRevision& position) const
{
    ENSURE_CAN_READ

    if (!range().contains(position))
        return nullptr;

    for (Declaration* child : std::as_const(m_dynamicData->m_localDeclarations)) {
        if (child->range().contains(position)) {
            return child;
        }
    }

    return nullptr;
}

DUContext* DUContext::findContextIncluding(const RangeInRevision& range) const
{
    ENSURE_CAN_READ

    if (!this->range().contains(range))
        return nullptr;

    for (DUContext* child : std::as_const(m_dynamicData->m_childContexts)) {
        if (DUContext* specific = child->findContextIncluding(range)) {
            return specific;
        }
    }

    return const_cast<DUContext*>(this);
}

int DUContext::findUseAt(const CursorInRevision& position) const
{
    ENSURE_CAN_READ

    if (!range().contains(position))
        return -1;

    for (unsigned int a = 0; a < d_func()->m_usesSize(); ++a)
        if (d_func()->m_uses()[a].m_range.contains(position))
            return a;

    return -1;
}

bool DUContext::inSymbolTable() const
{
    return d_func()->m_inSymbolTable;
}

void DUContext::setInSymbolTable(bool inSymbolTable)
{
    d_func_dynamic()->m_inSymbolTable = inSymbolTable;
}

void DUContext::clearImportedParentContexts()
{
    ENSURE_CAN_WRITE
        DUCHAIN_D_DYNAMIC(DUContext);

    while (d->m_importedContextsSize() != 0) {
        DUContext* ctx = d->m_importedContexts()[0].context(nullptr, false);
        if (ctx)
            ctx->m_dynamicData->removeImportedChildContext(this);

        d->m_importedContextsList().removeOne(d->m_importedContexts()[0]);
    }
}

void DUContext::cleanIfNotEncountered(const QSet<DUChainBase*>& encountered)
{
    ENSURE_CAN_WRITE

    // It may happen that the deletion of one declaration triggers the deletion of another one
    // Therefore we copy the list of indexed declarations and work on those. Indexed declarations
    // will return zero for already deleted declarations.
    KDevVarLengthArray<LocalIndexedDeclaration> indexedLocal;
    if (d_func()->m_localDeclarations()) {
        indexedLocal.append(d_func()->m_localDeclarations(), d_func()->m_localDeclarationsSize());
    }
    for (const LocalIndexedDeclaration& indexed : indexedLocal) {
        auto dec = indexed.data(topContext());
        if (dec && !encountered.contains(dec) && (!dec->isAutoDeclaration() || !dec->hasUses())) {
            delete dec;
        }
    }

    const auto currentChildContexts = m_dynamicData->m_childContexts;
    for (DUContext* childContext : currentChildContexts) {
        if (!encountered.contains(childContext)) {
            delete childContext;
        }
    }
}

TopDUContext* DUContext::topContext() const
{
    return m_dynamicData->m_topContext;
}

AbstractNavigationWidget* DUContext::createNavigationWidget(Declaration* decl, TopDUContext* topContext,
                                                            AbstractNavigationWidget::DisplayHints hints) const
{
    if (decl) {
        auto* widget = new AbstractNavigationWidget;
        widget->setDisplayHints(hints);
        auto* context = new AbstractDeclarationNavigationContext(DeclarationPointer(decl),
                                                                 TopDUContextPointer(topContext));
        widget->setContext(NavigationContextPointer(context));
        return widget;
    } else {
        return nullptr;
    }
}

QVector<RangeInRevision> allUses(DUContext* context, int declarationIndex, bool noEmptyUses)
{
    QVector<RangeInRevision> ret;
    for (int a = 0; a < context->usesCount(); ++a)
        if (context->uses()[a].m_declarationIndex == declarationIndex)
            if (!noEmptyUses || !context->uses()[a].m_range.isEmpty())
                ret << context->uses()[a].m_range;

    const auto childContexts = context->childContexts();
    for (DUContext* child : childContexts) {
        ret += allUses(child, declarationIndex, noEmptyUses);
    }

    return ret;
}

DUContext::SearchItem::SearchItem(const QualifiedIdentifier& id, const Ptr& nextItem, int start)
    : isExplicitlyGlobal(start == 0 ? id.explicitlyGlobal() : false)
{
    if (!id.isEmpty()) {
        if (id.count() > start)
            identifier = id.indexedAt(start);

        if (id.count() > start + 1)
            addNext(Ptr(new SearchItem(id, nextItem, start + 1)));
        else if (nextItem)
            next.append(nextItem);
    } else if (nextItem) {
        ///If there is no prefix, just copy nextItem
        isExplicitlyGlobal = nextItem->isExplicitlyGlobal;
        identifier = nextItem->identifier;
        next = nextItem->next;
    }
}

DUContext::SearchItem::SearchItem(const QualifiedIdentifier& id, const PtrList& nextItems, int start)
    : isExplicitlyGlobal(start == 0 ? id.explicitlyGlobal() : false)
{
    if (id.count() > start)
        identifier = id.indexedAt(start);

    if (id.count() > start + 1)
        addNext(Ptr(new SearchItem(id, nextItems, start + 1)));
    else
        next = nextItems;
}

DUContext::SearchItem::SearchItem(bool explicitlyGlobal, const IndexedIdentifier& id, const PtrList& nextItems)
    : isExplicitlyGlobal(explicitlyGlobal)
    , identifier(id)
    , next(nextItems)
{
}

DUContext::SearchItem::SearchItem(bool explicitlyGlobal, const IndexedIdentifier& id, const Ptr& nextItem)
    : isExplicitlyGlobal(explicitlyGlobal)
    , identifier(id)
{
    next.append(nextItem);
}

bool DUContext::SearchItem::match(const QualifiedIdentifier& id, int offset) const
{
    if (id.isEmpty()) {
        if (identifier.isEmpty() && next.isEmpty())
            return true;
        else
            return false;
    }

    if (id.at(offset) != identifier) //The identifier is different
        return false;

    if (offset == id.count() - 1) {
        if (next.isEmpty())
            return true; //match
        else
            return false; //id is too short
    }

    for (int a = 0; a < next.size(); ++a)
        if (next[a]->match(id, offset + 1))
            return true;

    return false;
}

bool DUContext::SearchItem::isEmpty() const
{
    return identifier.isEmpty();
}

bool DUContext::SearchItem::hasNext() const
{
    return !next.isEmpty();
}

QVector<QualifiedIdentifier> DUContext::SearchItem::toList(const QualifiedIdentifier& prefix) const
{
    QVector<QualifiedIdentifier> ret;

    QualifiedIdentifier id = prefix;
    if (id.isEmpty())
        id.setExplicitlyGlobal(isExplicitlyGlobal);
    if (!identifier.isEmpty())
        id.push(identifier);

    if (next.isEmpty()) {
        ret << id;
    } else {
        for (int a = 0; a < next.size(); ++a)
            ret += next[a]->toList(id);
    }
    return ret;
}

void DUContext::SearchItem::addNext(const SearchItem::Ptr& other)
{
    next.append(other);
}

void DUContext::SearchItem::addToEachNode(const SearchItem::Ptr& other)
{
    if (other->isExplicitlyGlobal)
        return;

    next.append(other);
    for (int a = 0; a < next.size() - 1; ++a)
        next[a]->addToEachNode(other);
}

void DUContext::SearchItem::addToEachNode(const SearchItem::PtrList& other)
{
    int added = 0;
    for (const SearchItem::Ptr& o : other) {
        if (!o->isExplicitlyGlobal) {
            next.append(o);
            ++added;
        }
    }

    for (int a = 0; a < next.size() - added; ++a)
        next[a]->addToEachNode(other);
}

DUContext::Import::Import(DUContext* _context, const DUContext* importer, const CursorInRevision& _position)
    : position(_position)
{
    if (_context && _context->owner() &&
        (_context->owner()->specialization().index() ||
         (importer && importer->topContext() != _context->topContext()))) {
        m_declaration = _context->owner()->id();
    } else {
        m_context = _context;
    }
}

DUContext::Import::Import(const DeclarationId& id, const CursorInRevision& _position)
    : position(_position)
    , m_declaration(id)
{
}

DUContext* DUContext::Import::context(const TopDUContext* topContext, bool instantiateIfRequired) const
{
    if (m_declaration.isValid()) {
        Declaration* decl = m_declaration.declaration(topContext, instantiateIfRequired);
        //This first case rests on the assumption that no context will ever import a function's expression context
        //More accurately, that no specialized or cross-topContext imports will, but if the former assumption fails the latter will too
        if (auto* functionDecl = dynamic_cast<AbstractFunctionDeclaration*>(decl)) {
            if (functionDecl->internalFunctionContext()) {
                return functionDecl->internalFunctionContext();
            } else {
                qCWarning(LANGUAGE) << "Import of function declaration without internal function context encountered!";
            }
        }
        if (decl)
            return decl->logicalInternalContext(topContext);
        else
            return nullptr;
    } else {
        return m_context.data();
    }
}

bool DUContext::Import::isDirect() const
{
    return m_context.isValid();
}

void DUContext::visit(DUChainVisitor& visitor)
{
    ENSURE_CAN_READ

    visitor.visit(this);

    for (Declaration* decl : std::as_const(m_dynamicData->m_localDeclarations)) {
        visitor.visit(decl);
    }

    for (DUContext* childContext : std::as_const(m_dynamicData->m_childContexts)) {
        childContext->visit(visitor);
    }
}

static bool sortByRange(const DUChainBase* lhs, const DUChainBase* rhs)
{
    return lhs->range() < rhs->range();
}

void DUContext::resortLocalDeclarations()
{
    ENSURE_CAN_WRITE

    std::sort(m_dynamicData->m_localDeclarations.begin(), m_dynamicData->m_localDeclarations.end(), sortByRange);

    auto top = topContext();
    auto& declarations = d_func_dynamic()->m_localDeclarationsList();
    std::sort(declarations.begin(), declarations.end(),
              [top](const LocalIndexedDeclaration& lhs, const LocalIndexedDeclaration& rhs) {
            return lhs.data(top)->range() < rhs.data(top)->range();
        });
}

void DUContext::resortChildContexts()
{
    ENSURE_CAN_WRITE

    std::sort(m_dynamicData->m_childContexts.begin(), m_dynamicData->m_childContexts.end(), sortByRange);

    auto top = topContext();
    auto& contexts = d_func_dynamic()->m_childContextsList();
    std::sort(contexts.begin(), contexts.end(),
              [top](const LocalIndexedDUContext& lhs, const LocalIndexedDUContext& rhs) {
            return lhs.data(top)->range() < rhs.data(top)->range();
        });
}
}
