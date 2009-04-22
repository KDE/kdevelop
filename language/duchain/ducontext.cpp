/* This  is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ducontext.h"

#include <limits>

#include <QMutableLinkedListIterator>
#include <QSet>

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "../editor/editorintegrator.h"

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
#include "indexedstring.h"
#include "duchainregister.h"
#include "topducontextdynamicdata.h"
#include "arrayhelpers.h"

///It is fine to use one global static mutex here

using namespace KTextEditor;

//Stored statically for performance-reasons

#ifndef NDEBUG
#define ENSURE_CAN_WRITE_(x) {if(x->inDUChain()) { ENSURE_CHAIN_WRITE_LOCKED }}
#define ENSURE_CAN_READ_(x) {if(x->inDUChain()) { ENSURE_CHAIN_READ_LOCKED }}
#else
#define ENSURE_CAN_WRITE_(x)
#define ENSURE_CAN_READ_(x)
#endif

namespace KDevelop
{
QMutex DUContextDynamicData::m_localDeclarationsMutex(QMutex::Recursive);

DEFINE_LIST_MEMBER_HASH(DUContextData, m_childContexts, LocalIndexedDUContext)
DEFINE_LIST_MEMBER_HASH(DUContextData, m_importers, IndexedDUContext)
DEFINE_LIST_MEMBER_HASH(DUContextData, m_importedContexts, DUContext::Import)
DEFINE_LIST_MEMBER_HASH(DUContextData, m_localDeclarations, LocalIndexedDeclaration)
DEFINE_LIST_MEMBER_HASH(DUContextData, m_uses, Use)

REGISTER_DUCHAIN_ITEM(DUContext);

const Identifier globalImportIdentifier("{...import...}");

void DUContext::rebuildDynamicData(DUContext* parent, uint ownIndex) {

  Q_ASSERT(!parent || ownIndex);
  m_dynamicData->m_topContext = parent ? parent->topContext() : static_cast<TopDUContext*>(this);
  m_dynamicData->m_indexInTopContext = ownIndex;
  m_dynamicData->m_parentContext = DUContextPointer(parent);
  m_dynamicData->m_context = this;

  DUChainBase::rebuildDynamicData(parent, ownIndex);
}

DUContextData::DUContextData()
  : m_inSymbolTable(false), m_anonymousInParent(false), m_propagateDeclarations(false)
{
  initializeAppendedLists();
}

DUContextData::~DUContextData() {
  freeAppendedLists();
}

DUContextData::DUContextData(const DUContextData& rhs)  : DUChainBaseData(rhs), m_inSymbolTable(rhs.m_inSymbolTable), m_anonymousInParent(rhs.m_anonymousInParent), m_propagateDeclarations(rhs.m_propagateDeclarations) {
  initializeAppendedLists();
  copyListsFrom(rhs);
  m_scopeIdentifier = rhs.m_scopeIdentifier;
  m_contextType = rhs.m_contextType;
  m_owner = rhs.m_owner;
}

DUContextDynamicData::DUContextDynamicData(DUContext* d)
  : m_topContext(0), m_hasLocalDeclarationsHash(false), m_indexInTopContext(0), m_context(d), m_rangesChanged(true)
{
}

void DUContextDynamicData::scopeIdentifier(bool includeClasses, QualifiedIdentifier& target) const {
  if (m_parentContext)
    m_parentContext->m_dynamicData->scopeIdentifier(includeClasses, target);

  if (includeClasses || m_context->d_func()->m_contextType != DUContext::Class)
    target += m_context->d_func()->m_scopeIdentifier;
}

bool DUContextDynamicData::importsSafeButSlow(const DUContext* context, const TopDUContext* source, ImportsHash& checked) const {
  if( this == context->m_dynamicData )
    return true;

  if(checked.find(this) != checked.end())
    return false;
  checked.insert(std::make_pair(this, true));

  FOREACH_FUNCTION( const DUContext::Import& ctx, m_context->d_func()->m_importedContexts ) {
    DUContext* import = ctx.context(source);
    if(import == context || (import && import->m_dynamicData->importsSafeButSlow(context, source, checked)))
      return true;
  }

  return false;
}

bool DUContextDynamicData::imports(const DUContext* context, const TopDUContext* source, int maxDepth) const {
  if( this == context->m_dynamicData )
    return true;

  if(maxDepth == 0) {
    ImportsHash checked(500);
    checked.set_empty_key(0);
    return importsSafeButSlow(context, source, checked);
  }

  FOREACH_FUNCTION( const DUContext::Import& ctx, m_context->d_func()->m_importedContexts ) {
    DUContext* import = ctx.context(source);
    if(import == context || (import && import->m_dynamicData->imports(context, source, maxDepth-1)))
      return true;
  }

  return false;
}

IndexedDUContext::IndexedDUContext(uint topContext, uint contextIndex) : m_topContext(topContext), m_contextIndex(contextIndex) {
}

IndexedDUContext::IndexedDUContext(DUContext* ctx) {
  if(ctx) {
    m_topContext = ctx->topContext()->ownIndex();
    m_contextIndex = ctx->m_dynamicData->m_indexInTopContext;
  }else{
    m_topContext = 0;
    m_contextIndex = 0;
  }
}

IndexedTopDUContext IndexedDUContext::indexedTopContext() const {
  if(isDummy())
    return IndexedTopDUContext();
  return IndexedTopDUContext(m_topContext);
}

LocalIndexedDUContext::LocalIndexedDUContext(uint contextIndex) : m_contextIndex(contextIndex) {
}

LocalIndexedDUContext::LocalIndexedDUContext(DUContext* ctx) {
  if(ctx) {
    m_contextIndex = ctx->m_dynamicData->m_indexInTopContext;
  }else{
    m_contextIndex = 0;
  }
}

bool LocalIndexedDUContext::isLoaded(TopDUContext* top) const {
  if(!m_contextIndex)
    return false;
  else
    return top->m_dynamicData->isContextForIndexLoaded(m_contextIndex);
}

DUContext* LocalIndexedDUContext::data(TopDUContext* top) const {
  if(!m_contextIndex)
    return 0;
  else
    return top->m_dynamicData->getContextForIndex(m_contextIndex);
}

DUContext* IndexedDUContext::context() const {
  if(isDummy())
    return 0;
//   ENSURE_CHAIN_READ_LOCKED
  if(!m_topContext)
    return 0;

  TopDUContext* ctx = DUChain::self()->chainForIndex(m_topContext);
  if(!ctx)
    return 0;

  if(!m_contextIndex)
    return ctx;

  return ctx->m_dynamicData->getContextForIndex(m_contextIndex);
}

void DUContext::synchronizeUsesFromSmart() const
{
  DUCHAIN_D(DUContext);

  if(m_dynamicData->m_rangesForUses.isEmpty() || !m_dynamicData->m_rangesChanged)
    return;

  Q_ASSERT(uint(m_dynamicData->m_rangesForUses.count()) == d->m_usesSize());

  for(unsigned int a = 0; a < d->m_usesSize(); a++)
    if(m_dynamicData->m_rangesForUses[a]) ///@todo somehow signalize the change
      const_cast<Use&>(d->m_uses()[a]).m_range = SimpleRange(*m_dynamicData->m_rangesForUses[a]);

  m_dynamicData->m_rangesChanged = false;
}

void DUContext::synchronizeUsesToSmart() const
{
  DUCHAIN_D(DUContext);
  if(m_dynamicData->m_rangesForUses.isEmpty())
    return;
  Q_ASSERT(uint(m_dynamicData->m_rangesForUses.count()) == d->m_usesSize());

  // TODO: file close race? from here
  KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( smartRange()->document() );
  Q_ASSERT(iface);

  // TODO: file close race to here
  QMutexLocker l(iface->smartMutex());

  for(unsigned int a = 0; a < d->m_usesSize(); a++) {
    if(a % 10 == 0) { //Unlock the smart-lock time by time, to increase responsiveness
      l.unlock();
      l.relock();
    }
    if(m_dynamicData->m_rangesForUses[a]) {
      m_dynamicData->m_rangesForUses[a]->start() = d->m_uses()[a].m_range.start.textCursor();
      m_dynamicData->m_rangesForUses[a]->end() = d->m_uses()[a].m_range.end.textCursor();
    }else{
      kDebug() << "bad smart-range";
    }
  }
}

void DUContext::rangePositionChanged(KTextEditor::SmartRange* range)
{
  if(range != smartRange())
    m_dynamicData->m_rangesChanged = true;
}

void DUContext::rangeDeleted(KTextEditor::SmartRange* range)
{
  if(range == smartRange()) {
    DocumentRangeObject::rangeDeleted(range);
  } else {
    range->removeWatcher(this);
    int index = m_dynamicData->m_rangesForUses.indexOf(range);
    if(index != -1) {
      d_func_dynamic()->m_usesList()[index].m_range = SimpleRange(*range);
      m_dynamicData->m_rangesForUses[index] = 0;
    }

    if(m_dynamicData->m_rangesForUses.count(0) == m_dynamicData->m_rangesForUses.size())
      m_dynamicData->m_rangesForUses.clear();
  }
}

void DUContextDynamicData::enableLocalDeclarationsHash(DUContext* ctx, const Identifier& currentIdentifier, Declaration* currentDecl)
{
  m_hasLocalDeclarationsHash = true;

  FOREACH_FUNCTION(LocalIndexedDeclaration indexedDecl, ctx->d_func()->m_localDeclarations) {
    Declaration* decl = indexedDecl.data(m_topContext);
    Q_ASSERT(decl);
    if(currentDecl != decl)
      m_localDeclarationsHash.insert( decl->identifier(), DeclarationPointer(decl) );
    else
      m_localDeclarationsHash.insert( currentIdentifier, DeclarationPointer(decl) );
  }

  FOREACH_FUNCTION(LocalIndexedDUContext child, ctx->d_func()->m_childContexts) {
    DUContext* childCtx = child.data(m_topContext);
    Q_ASSERT(childCtx);
    if(childCtx->d_func()->m_propagateDeclarations)
      enableLocalDeclarationsHash(childCtx, currentIdentifier, currentDecl);
  }
}

void DUContextDynamicData::disableLocalDeclarationsHash()
{
  m_hasLocalDeclarationsHash = false;
  m_localDeclarationsHash.clear();
}

bool DUContextDynamicData::needsLocalDeclarationsHash()
{
  ///@todo Do this again, it brings a large performance boost
  //For now disable the hash, until we make sure that all declarations needed for the hash are loaded first
  //including those in propagating sub-contexts.
  //Then, also make sure that we create the declaration hash after loading if needed
  return false;

  if(m_context->d_func()->m_localDeclarationsSize() > 15)
    return true;

  uint propagatingChildContexts = 0;

  FOREACH_FUNCTION(LocalIndexedDUContext child, m_context->d_func()->m_childContexts) {
    DUContext* childCtx = child.data(m_topContext);
    Q_ASSERT(childCtx);
    if(childCtx->d_func()->m_propagateDeclarations)
      ++propagatingChildContexts;
  }

  return propagatingChildContexts > 4;
}

void DUContextDynamicData::addDeclarationToHash(const Identifier& identifier, Declaration* declaration)
{
  if(m_hasLocalDeclarationsHash)
    m_localDeclarationsHash.insert( identifier, DeclarationPointer(declaration) );

  if( m_context->d_func()->m_propagateDeclarations && m_parentContext )
    m_parentContext->m_dynamicData->addDeclarationToHash(identifier, declaration);

  if(!m_hasLocalDeclarationsHash && needsLocalDeclarationsHash())
    enableLocalDeclarationsHash(m_context, identifier, declaration);
}

void DUContextDynamicData::removeDeclarationFromHash(const Identifier& identifier, Declaration* declaration)
{
  if(m_hasLocalDeclarationsHash)
    m_localDeclarationsHash.remove( identifier, DeclarationPointer(declaration) );

  if( m_context->d_func()->m_propagateDeclarations && m_parentContext )
    m_parentContext->m_dynamicData->removeDeclarationFromHash(identifier,  declaration);

  if(m_hasLocalDeclarationsHash && !needsLocalDeclarationsHash())
      disableLocalDeclarationsHash();
}

void DUContextDynamicData::addDeclaration( Declaration * newDeclaration )
{
  // The definition may not have its identifier set when it's assigned... allow dupes here, TODO catch the error elsewhere
  {
    QMutexLocker lock(&m_localDeclarationsMutex);

//     m_localDeclarations.append(newDeclaration);

  if(m_indexInTopContext < (0xffffffff/2)) {
    //If this context is not temporary, added declarations shouldn't be either
    Q_ASSERT(newDeclaration->ownIndex() < (0xffffffff/2));
  }
  if(m_indexInTopContext > (0xffffffff/2)) {
    //If this context is temporary, added declarations should be as well
    Q_ASSERT(newDeclaration->ownIndex() > (0xffffffff/2));
  }

  SimpleCursor start = newDeclaration->range().start;
///@todo Do binary search to find the position
  bool inserted = false;
  for (int i = m_context->d_func_dynamic()->m_localDeclarationsSize()-1; i >= 0; --i) {
    Declaration* child = m_context->d_func_dynamic()->m_localDeclarations()[i].data(m_topContext);
    if(!child) {
      kWarning() << "child declaration number" << i << "of" << m_context->d_func_dynamic()->m_localDeclarationsSize() << "is invalid";
      continue;
    }
    if(child == newDeclaration)
      return;
    if (start > child->range().start) {
      insertToArray(m_context->d_func_dynamic()->m_localDeclarationsList(), newDeclaration, i+1);
      if(!m_context->d_func()->m_localDeclarations()[i+1].data(m_topContext))
        kFatal() << "Inserted a not addressable declaration";

      inserted = true;
      break;
    }
  }
  if( !inserted ) //We haven't found any child that is before this one, so prepend it
    insertToArray(m_context->d_func_dynamic()->m_localDeclarationsList(), newDeclaration, 0);

    addDeclarationToHash(newDeclaration->identifier(), newDeclaration);
  }

  //DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::LocalDeclarations, newDeclaration);
}

bool DUContextDynamicData::removeDeclaration(Declaration* declaration)
{
  QMutexLocker lock(&m_localDeclarationsMutex);

  if(!m_topContext->deleting()) //We can save a lot of time by just not caring about the hash while deleting
    removeDeclarationFromHash(declaration->identifier(), declaration);

  if( removeOne(m_context->d_func_dynamic()->m_localDeclarationsList(), LocalIndexedDeclaration(declaration)) ) {
    //DUChain::contextChanged(m_context, DUChainObserver::Removal, DUChainObserver::LocalDeclarations, declaration);
    return true;
  }else {
    return false;
  }
}

void DUContext::changingIdentifier( Declaration* decl, const Identifier& from, const Identifier& to ) {
  QMutexLocker lock(&DUContextDynamicData::m_localDeclarationsMutex);
  m_dynamicData->removeDeclarationFromHash(from, decl);
  m_dynamicData->addDeclarationToHash(to, decl);
}

void DUContextDynamicData::addChildContext( DUContext * context )
{
  // Internal, don't need to assert a lock
  Q_ASSERT(!context->m_dynamicData->m_parentContext || context->m_dynamicData->m_parentContext.data()->m_dynamicData == this );

  LocalIndexedDUContext indexed(context->m_dynamicData->m_indexInTopContext);

  if(m_indexInTopContext < (0xffffffff/2)) {
    //If this context is not temporary, added declarations shouldn't be either
    Q_ASSERT(indexed.localIndex() < (0xffffffff/2));
  }
  if(m_indexInTopContext > (0xffffffff/2)) {
    //If this context is temporary, added declarations should be as well
    Q_ASSERT(indexed.localIndex() > (0xffffffff/2));
  }

  bool inserted = false;

  int childCount = m_context->d_func()->m_childContextsSize();

  for (int i = childCount-1; i >= 0; --i) {///@todo Do binary search to find the position
    DUContext* child = m_context->d_func()->m_childContexts()[i].data(m_topContext);
    if (context == child)
      return;
    if (context->range().start > child->range().start) {
      insertToArray(m_context->d_func_dynamic()->m_childContextsList(), indexed, i+1);
      context->m_dynamicData->m_parentContext = m_context;
      inserted = true;
      break;
    }
  }

  if( !inserted ) {
    m_context->d_func_dynamic()->m_childContextsList().insert(indexed, 0);
    context->m_dynamicData->m_parentContext = m_context;
  }

  if(context->d_func()->m_propagateDeclarations) {
    QMutexLocker lock(&DUContextDynamicData::m_localDeclarationsMutex);
    disableLocalDeclarationsHash();
    if(needsLocalDeclarationsHash())
      enableLocalDeclarationsHash(m_context);
  }

  //DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::ChildContexts, context);
}

bool DUContextDynamicData::removeChildContext( DUContext* context ) {
//   ENSURE_CAN_WRITE

  if( removeOne(m_context->d_func_dynamic()->m_childContextsList(), LocalIndexedDUContext(context)) )
    return true;
  else
    return false;
}

void DUContextDynamicData::addImportedChildContext( DUContext * context )
{
//   ENSURE_CAN_WRITE
  if(arrayContains(m_context->d_func_dynamic()->m_importersList(), IndexedDUContext(context))) {
    kDebug(9505) << m_context->scopeIdentifier(true).toString() << "importer added multiple times:" << context->scopeIdentifier(true).toString();
    return;
  }

  m_context->d_func_dynamic()->m_importersList().append(context);

  //DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::ImportedChildContexts, context);
}

//Can also be called with a context that is not in the list
void DUContextDynamicData::removeImportedChildContext( DUContext * context )
{
//   ENSURE_CAN_WRITE
  removeOne(m_context->d_func_dynamic()->m_importersList(), IndexedDUContext(context));
  //if( != 0 )
    //DUChain::contextChanged(m_context, DUChainObserver::Removal, DUChainObserver::ImportedChildContexts, context);
}

int DUContext::depth() const
{
  { if (!parentContext()) return 0; return parentContext()->depth() + 1; }
}

DUContext::DUContext(DUContextData& data) : DUChainBase(data), m_dynamicData(new DUContextDynamicData(this)) {
}


DUContext::DUContext(const SimpleRange& range, DUContext* parent, bool anonymous)
  : DUChainBase(*new DUContextData(), range), m_dynamicData(new DUContextDynamicData(this))
{
  d_func_dynamic()->setClassId(this);
  if(parent)
    m_dynamicData->m_topContext = parent->topContext();
  else
    m_dynamicData->m_topContext = static_cast<TopDUContext*>(this);

  d_func_dynamic()->setClassId(this);
  DUCHAIN_D_DYNAMIC(DUContext);
  d->m_contextType = Other;
  m_dynamicData->m_parentContext = 0;

  d->m_anonymousInParent = anonymous;
  d->m_inSymbolTable = false;

  if (parent) {
    m_dynamicData->m_indexInTopContext = parent->topContext()->m_dynamicData->allocateContextIndex(this, parent->isAnonymous() || anonymous);
    Q_ASSERT(m_dynamicData->m_indexInTopContext);

    if( !anonymous )
      parent->m_dynamicData->addChildContext(this);
    else
      m_dynamicData->m_parentContext = parent;
  }

  if(parent && !anonymous && parent->inSymbolTable())
    setInSymbolTable(true);
}

bool DUContext::isAnonymous() const {
  return d_func()->m_anonymousInParent || (m_dynamicData->m_parentContext && m_dynamicData->m_parentContext->isAnonymous());
}

DUContext::DUContext( DUContextData& dd, const SimpleRange& range, DUContext * parent, bool anonymous )
  : DUChainBase(dd, range), m_dynamicData(new DUContextDynamicData(this))
{
  if(parent)
    m_dynamicData->m_topContext = parent->topContext();
  else
    m_dynamicData->m_topContext = static_cast<TopDUContext*>(this);

  DUCHAIN_D_DYNAMIC(DUContext);
  d->m_contextType = Other;
  m_dynamicData->m_parentContext = 0;
  d->m_inSymbolTable = false;
  d->m_anonymousInParent = anonymous;
  if (parent) {
    m_dynamicData->m_indexInTopContext = parent->topContext()->m_dynamicData->allocateContextIndex(this, parent->isAnonymous() || anonymous);

    if( !anonymous )
      parent->m_dynamicData->addChildContext(this);
    else
      m_dynamicData->m_parentContext = parent;
  }
}

DUContext::DUContext(DUContext& useDataFrom)
  : DUChainBase(useDataFrom), m_dynamicData(useDataFrom.m_dynamicData)
{
}

DUContext::~DUContext( )
{
  TopDUContext* top = topContext();

  if(!top->deleting() || !top->isOnDisk()) {
    DUCHAIN_D_DYNAMIC(DUContext);
    QualifiedIdentifier id(scopeIdentifier(true));
    if(d->m_inSymbolTable && parentContext()) {
      PersistentSymbolTable::self().removeContext(id, this);
    }

    if(d->m_owner.declaration())
      d->m_owner.declaration()->setInternalContext(0);

    while( d->m_importersSize() != 0 ) {
      if(d->m_importers()[0].data())
        d->m_importers()[0].data()->removeImportedParentContext(this);
      else {
        kDebug() << "importer disappeared";
        d->m_importersList().removeOne(d->m_importers()[0]);
      }
    }

    clearImportedParentContexts();
  }

  deleteChildContextsRecursively();

  if(!topContext()->deleting() || !topContext()->isOnDisk())
    deleteUses();
  else
    clearUseSmartRanges();

  deleteLocalDeclarations();

  //If the top-context is being delete, we don't need to spend time rebuilding the inner structure.
  //That's expensive, especially when the data is not dynamic.
  if(!top->deleting() || !top->isOnDisk()) {
    if (m_dynamicData->m_parentContext)
      m_dynamicData->m_parentContext->m_dynamicData->removeChildContext(this);
    //DUChain::contextChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
  }

  top->m_dynamicData->clearContextIndex(this);

  Q_ASSERT(d_func()->isDynamic() == (!top->deleting() || !top->isOnDisk() || top->m_dynamicData->isTemporaryContextIndex(m_dynamicData->m_indexInTopContext)));
}

QVector< DUContext * > DUContext::childContexts( ) const
{
  ENSURE_CAN_READ

  QVector< DUContext * > ret;
  FOREACH_FUNCTION(LocalIndexedDUContext ctx, d_func()->m_childContexts)
    ret << ctx.data(topContext());
  return ret;
}

Declaration* DUContext::owner() const {
  ENSURE_CAN_READ
  return d_func()->m_owner.declaration();
}

void DUContext::setOwner(Declaration* owner) {
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(DUContext);
  if( owner == d->m_owner.declaration() )
    return;

  Declaration* oldOwner = d->m_owner.declaration();

  d->m_owner = owner;

  //Q_ASSERT(!oldOwner || oldOwner->internalContext() == this);
  if( oldOwner && oldOwner->internalContext() == this )
    oldOwner->setInternalContext(0);


  //The context set as internal context should always be the last opened context
  if( owner )
    owner->setInternalContext(this);
}

DUContext* DUContext::parentContext( ) const
{
  //ENSURE_CAN_READ Commented out for performance reasons

  return m_dynamicData->m_parentContext.data();
}

void DUContext::setPropagateDeclarations(bool propagate)
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(DUContext);
  QMutexLocker lock(&DUContextDynamicData::m_localDeclarationsMutex);

  m_dynamicData->m_parentContext->m_dynamicData->disableLocalDeclarationsHash();

  d->m_propagateDeclarations = propagate;

  if(m_dynamicData->m_parentContext->m_dynamicData->needsLocalDeclarationsHash())
    m_dynamicData->m_parentContext->m_dynamicData->enableLocalDeclarationsHash(m_dynamicData->m_parentContext.data());
}

bool DUContext::isPropagateDeclarations() const
{
  return d_func()->m_propagateDeclarations;
}

QList<Declaration*> DUContext::findLocalDeclarations( const Identifier& identifier, const SimpleCursor & position, const TopDUContext* topContext, const AbstractType::Ptr& dataType, SearchFlags flags ) const
{
  ENSURE_CAN_READ

  DeclarationList ret;
  findLocalDeclarationsInternal(identifier, position.isValid() ? position : range().end, dataType, ret, topContext ? topContext : this->topContext(), flags);
  return arrayToList(ret);
}

bool contextIsChildOrEqual(const DUContext* childContext, const DUContext* context) {
  if(childContext == context)
    return true;

  if(childContext->parentContext())
    return contextIsChildOrEqual(childContext->parentContext(), context);
  else
    return false;
}

void DUContext::findLocalDeclarationsInternal( const Identifier& identifier, const SimpleCursor & position, const AbstractType::Ptr& dataType, DeclarationList& ret, const TopDUContext* /*source*/, SearchFlags flags ) const
{
  IndexedIdentifier indexedIdentifier(identifier);
  {
     QMutexLocker lock(&DUContextDynamicData::m_localDeclarationsMutex);

     struct Checker {
       Checker(SearchFlags flags, const AbstractType::Ptr& dataType, const SimpleCursor & position, DUContext::ContextType ownType) : m_flags(flags), m_dataType(dataType), m_position(position), m_ownType(ownType) {
       }

       Declaration* check(Declaration* declaration) {
          if( declaration->kind() == Declaration::Alias ) {
            //Apply alias declarations
            AliasDeclaration* alias = static_cast<AliasDeclaration*>(declaration);
            if(alias->aliasedDeclaration().isValid()) {
              declaration = alias->aliasedDeclaration().declaration();
            } else {
#ifndef Q_CC_MSVC
              kDebug() << "lost aliased declaration";
#endif
            }
          }

          if( declaration->kind() == Declaration::NamespaceAlias && !(m_flags & NoFiltering) )
            return 0;

          if((m_flags & OnlyFunctions) && !declaration->isFunctionDeclaration())
            return 0;

          if (!m_dataType || m_dataType->indexed() == declaration->abstractType()->indexed())
            if (m_ownType == Class || m_ownType == Template || m_position > declaration->range().start || !m_position.isValid()) ///@todo This is C++-specific
              return declaration;
          return 0;
       }

       SearchFlags m_flags;
       const AbstractType::Ptr& m_dataType;
       const SimpleCursor& m_position;
       DUContext::ContextType m_ownType;
     };

     Checker checker(flags, dataType, position, type());

     if(m_dynamicData->m_hasLocalDeclarationsHash) {
       //Use a special hash that contains all declarations visible in this context
      QHash<Identifier, DeclarationPointer>::const_iterator it = m_dynamicData->m_localDeclarationsHash.find(identifier);
      QHash<Identifier, DeclarationPointer>::const_iterator end = m_dynamicData->m_localDeclarationsHash.end();

      for( ; it != end && it.key() == identifier; ++it ) {
        Declaration* declaration = (*it).data();

        if( !declaration ) {
          //This should never happen, but let's see
          kDebug(9505) << "DUContext::findLocalDeclarationsInternal: Invalid declaration in local-declaration-hash";
          continue;
        }

        Declaration* checked = checker.check(declaration);
        if(checked)
            ret.append(checked);
      }
    }else if(d_func()->m_inSymbolTable && !this->localScopeIdentifier().isEmpty() && !identifier.isEmpty()) {
       //This context is in the symbol table, use the symbol-table to speed up the search
       QualifiedIdentifier id(scopeIdentifier(true) + identifier);

       TopDUContext* top = topContext();

       uint count;
       const IndexedDeclaration* declarations;
       PersistentSymbolTable::self().declarations(id, count, declarations);
       for(uint a = 0; a < count; ++a) {
         ///@todo Eventually do efficient iteration-free filtering
         if(declarations[a].topContextIndex() == top->ownIndex()) {
           Declaration* decl = LocalIndexedDeclaration(declarations[a].localIndex()).data(top);
           if(decl && contextIsChildOrEqual(decl->context(), this)) {
             Declaration* checked = checker.check(decl);
             if(checked)
               ret.append(checked);
           }
         }
       }
     }else {
       //Iterate through all declarations
      DUContextDynamicData::VisibleDeclarationIterator it(m_dynamicData);
      while(it) {
        Declaration* declaration = *it;
        if(declaration->indexedIdentifier() == indexedIdentifier) {
          Declaration* checked = checker.check(declaration);
          if(checked)
              ret.append(checked);
        }
        ++it;
      }
     }
  }
}

bool DUContext::foundEnough( const DeclarationList& ret, SearchFlags flags ) const {
  if( !ret.isEmpty() && !(flags & DUContext::NoFiltering))
    return true;
  else
    return false;
}

bool DUContext::findDeclarationsInternal( const SearchItem::PtrList & baseIdentifiers, const SimpleCursor & position, const AbstractType::Ptr& dataType, DeclarationList& ret, const TopDUContext* source, SearchFlags flags ) const
{
  DUCHAIN_D(DUContext);
  if( d_func()->m_contextType != Namespace ) { //If we're in a namespace, delay all the searching into the top-context, because only that has the overview to pick the correct declarations.
    for(int a = 0; a < baseIdentifiers.size(); ++a)
      if(!baseIdentifiers[a]->isExplicitlyGlobal && baseIdentifiers[a]->next.isEmpty()) //It makes no sense searching locally for qualified identifiers
        findLocalDeclarationsInternal(baseIdentifiers[a]->identifier, position, dataType, ret, source, flags);

    if( foundEnough(ret, flags) )
      return true;
  }

  ///Step 1: Apply namespace-aliases and -imports
  SearchItem::PtrList aliasedIdentifiers;
  //Because of namespace-imports and aliases, this identifier may need to be searched under multiple names
  applyAliases(baseIdentifiers, aliasedIdentifiers, position, false,  type() != DUContext::Namespace && type() != DUContext::Global);


  if( d->m_importedContextsSize() != 0 ) {
    ///Step 2: Give identifiers that are not marked as explicitly-global to imported contexts(explicitly global ones are treatead in TopDUContext)
    SearchItem::PtrList nonGlobalIdentifiers;
    FOREACH_ARRAY( const SearchItem::Ptr& identifier, aliasedIdentifiers )
      if( !identifier->isExplicitlyGlobal )
        nonGlobalIdentifiers << identifier;

    if( !nonGlobalIdentifiers.isEmpty() ) {
      for(int import = d->m_importedContextsSize()-1; import >= 0; --import ) {
        DUContext* context = d->m_importedContexts()[import].context(source);

        while( !context && import > 0 ) {
          --import;
          context = d->m_importedContexts()[import].context(source);
        }

        if(context == this) {
          kDebug() << "resolved self as import:" << scopeIdentifier(true);
          continue;
        }

        if( !context )
          break;

        if( position.isValid() && d->m_importedContexts()[import].position.isValid() && position < d->m_importedContexts()[import].position )
          continue;

        if( !context->findDeclarationsInternal(nonGlobalIdentifiers,  url() == context->url() ? position : context->range().end, dataType, ret, source, flags | InImportedParentContext) )
          return false;
      }
    }
  }

  if( foundEnough(ret, flags) )
    return true;

  ///Step 3: Continue search in parent-context
  if (!(flags & DontSearchInParent) && shouldSearchInParent(flags) && m_dynamicData->m_parentContext) {
    applyUpwardsAliases(aliasedIdentifiers, source);
    return m_dynamicData->m_parentContext->findDeclarationsInternal(aliasedIdentifiers, url() == m_dynamicData->m_parentContext->url() ? position : m_dynamicData->m_parentContext->range().end, dataType, ret, source, flags);
  }
  return true;
}

QList<Declaration*> DUContext::findDeclarations( const QualifiedIdentifier & identifier, const SimpleCursor & position, const AbstractType::Ptr& dataType, const TopDUContext* topContext, SearchFlags flags) const
{
  ENSURE_CAN_READ

  DeclarationList ret;
  SearchItem::PtrList identifiers;
  identifiers << SearchItem::Ptr(new SearchItem(identifier));

  findDeclarationsInternal(identifiers, position.isValid() ? position : range().end, dataType, ret, topContext ? topContext : this->topContext(), flags);

  return arrayToList(ret);
}

bool DUContext::imports(const DUContext* origin, const SimpleCursor& /*position*/ ) const
{
  ENSURE_CAN_READ

  return m_dynamicData->imports(origin, topContext(), 4);
}

bool DUContext::addIndirectImport(const DUContext::Import& import) {
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(DUContext);

  for(unsigned int a = 0; a < d->m_importedContextsSize(); ++a) {
    if(d->m_importedContexts()[a] == import) {
      d->m_importedContextsList()[a].position = import.position;
      return true;
    }
  }

  ///Do not sort the imported contexts by their own line-number, it makes no sense.
  ///Contexts added first, aka template-contexts, should stay in first place, so they are searched first.

  d->m_importedContextsList().append(import);
  return false;
}

void DUContext::addImportedParentContext( DUContext * context, const SimpleCursor& position, bool anonymous, bool /*temporary*/ )
{
  ENSURE_CAN_WRITE

  if(context == this) {
    kDebug() << "Tried to import self";
    return;
  }

  Import import(context, position);
  if(addIndirectImport(import))
    return;

  if( !anonymous && import.isBackwardMapped() ) {
    ENSURE_CAN_WRITE_(context)
    context->m_dynamicData->addImportedChildContext(this);
  }
  //DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::ImportedParentContexts, context);
}

void DUContext::removeImportedParentContext( DUContext * context )
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(DUContext);

  Import import(context, SimpleCursor::invalid());

  for(unsigned int a = 0; a < d->m_importedContextsSize(); ++a) {
    if(d->m_importedContexts()[a] == import) {
      removeFromArray(d->m_importedContextsList(), a);
      break;
    }
  }

  if( !context )
    return;

  context->m_dynamicData->removeImportedChildContext(this);

  //DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::ImportedParentContexts, context);
}

const IndexedDUContext* DUContext::indexedImporters() const
{
  return d_func()->m_importers();
}

uint DUContext::indexedImportersSize() const
{
  return d_func()->m_importersSize();
}

QVector<DUContext*> DUContext::importers() const
{
  ENSURE_CAN_READ

  QVector<DUContext*> ret;
  FOREACH_FUNCTION(IndexedDUContext ctx, d_func()->m_importers)
    ret << ctx.context();

  return ret;
}

DUContext * DUContext::findContext( const SimpleCursor& position, DUContext* parent) const
{
  ENSURE_CAN_READ

  if (!parent)
    parent = const_cast<DUContext*>(this);

  FOREACH_FUNCTION(LocalIndexedDUContext context, parent->d_func()->m_childContexts)
    if (context.data(topContext())->range().contains(position)) {
      DUContext* ret = findContext(position, context.data(topContext()));
      if (!ret)
        ret = context.data(topContext());

      return ret;
    }

  return 0;
}

bool DUContext::parentContextOf(DUContext* context) const
{
  if (this == context)
    return true;

  FOREACH_FUNCTION(LocalIndexedDUContext child, d_func()->m_childContexts) {
    if (child.data(topContext())->parentContextOf(context))
      return true;
  }

  return false;
}

QList<Declaration*> DUContext::allLocalDeclarations(const Identifier& identifier) const
{
  IndexedIdentifier indexedIdentifier(identifier);

  ENSURE_CAN_READ
  QMutexLocker lock(&DUContextDynamicData::m_localDeclarationsMutex);

  QList<Declaration*> ret;

  if(m_dynamicData->m_hasLocalDeclarationsHash) {
    QHash<Identifier, DeclarationPointer>::const_iterator it = m_dynamicData->m_localDeclarationsHash.find(identifier);
    QHash<Identifier, DeclarationPointer>::const_iterator end = m_dynamicData->m_localDeclarationsHash.end();

    for( ; it != end && it.key() == identifier; ++it )
      ret << (*it).data();
  }else{
    DUContextDynamicData::VisibleDeclarationIterator it(m_dynamicData);
    while(it) {
      Declaration* decl = *it;
      if(decl->indexedIdentifier() == indexedIdentifier)
        ret << decl;
      ++it;
    }
  }
  return ret;
}

QList< QPair<Declaration*, int> > DUContext::allDeclarations(const SimpleCursor& position, const TopDUContext* topContext, bool searchInParents) const
{
  ENSURE_CAN_READ

  QList< QPair<Declaration*, int> > ret;

  QHash<const DUContext*, bool> hadContexts;
  // Iterate back up the chain
  mergeDeclarationsInternal(ret, position, hadContexts, topContext ? topContext : this->topContext(), searchInParents);

  return ret;
}

QVector<Declaration*> DUContext::localDeclarations(const TopDUContext* source) const
{
  Q_UNUSED(source);
  ENSURE_CAN_READ

  QMutexLocker lock(&DUContextDynamicData::m_localDeclarationsMutex);
  QVector<Declaration*> ret;
  FOREACH_FUNCTION(LocalIndexedDeclaration decl, d_func()->m_localDeclarations) {
    ret << decl.data(topContext());
    Q_ASSERT(ret.back()->context() == this);
    ret.back()->identifier().isEmpty();
  }

  return ret;
}

void DUContext::mergeDeclarationsInternal(QList< QPair<Declaration*, int> >& definitions, const SimpleCursor& position, QHash<const DUContext*, bool>& hadContexts, const TopDUContext* source, bool searchInParents, int currentDepth) const
{
  if((currentDepth > 300 && currentDepth < 1000) || currentDepth > 1300) {
    kDebug() << "too much depth";
    return;
  }
  DUCHAIN_D(DUContext);
  
    if(hadContexts.contains(this) && !searchInParents)
      return;
  
    if(!hadContexts.contains(this)) {
      hadContexts[this] = true;

      if( (type() == DUContext::Namespace || type() == DUContext::Global) && currentDepth < 1000 )
        currentDepth += 1000;

      {
        QMutexLocker lock(&DUContextDynamicData::m_localDeclarationsMutex);
        DUContextDynamicData::VisibleDeclarationIterator it(m_dynamicData);
        while(it) {
          Declaration* decl = *it;

          if ( decl && (!position.isValid() || decl->range().start <= position) )
            definitions << qMakePair(decl, currentDepth);
          ++it;
        }
      }

      for(int a = d->m_importedContextsSize()-1; a >= 0; --a) {
        const Import* import(&d->m_importedContexts()[a]);
        DUContext* context = import->context(source);
        while( !context && a > 0 ) {
          --a;
          import = &d->m_importedContexts()[a];
          context = import->context(source);
        }
        if( !context )
          break;

        if(context == this) {
          kDebug() << "resolved self as import:" << scopeIdentifier(true);
          continue;
        }


        if( position.isValid() && import->position.isValid() && position < import->position )
          continue;

        context->mergeDeclarationsInternal(definitions, SimpleCursor::invalid(), hadContexts, source, searchInParents && context->shouldSearchInParent(InImportedParentContext) &&  context->parentContext()->type() == DUContext::Helper, currentDepth+1);
      }
    }
    
  ///Only respect the position if the parent-context is not a class(@todo this is language-dependent)
  if (parentContext() && searchInParents )
    parentContext()->mergeDeclarationsInternal(definitions, parentContext()->type() == DUContext::Class ? parentContext()->range().end : position, hadContexts, source, searchInParents, currentDepth+1);
}

void DUContext::deleteLocalDeclarations()
{
  ENSURE_CAN_WRITE
  KDevVarLengthArray<LocalIndexedDeclaration> declarations;
  {
    QMutexLocker lock(&DUContextDynamicData::m_localDeclarationsMutex);
    FOREACH_FUNCTION(const LocalIndexedDeclaration& decl, d_func()->m_localDeclarations)
      declarations.append(decl);
  }

  TopDUContext* top = topContext();
  //If we are deleting something that is not stored to disk, we need to create + delete the declarations,
  //so their destructor unregisters from the persistent symbol table and from TopDUContextDynamicData
  FOREACH_ARRAY(LocalIndexedDeclaration decl, declarations)
    if(decl.isLoaded(top) || !top->deleting() || !top->isOnDisk())
      delete decl.data(top);
}

void DUContext::deleteChildContextsRecursively()
{
  ENSURE_CAN_WRITE

  TopDUContext* top = topContext();

  QVector<LocalIndexedDUContext> children;
  FOREACH_FUNCTION(LocalIndexedDUContext ctx, d_func()->m_childContexts)
    children << ctx;

  //If we are deleting a context that is already stored to disk, we don't need to load not yet loaded child-contexts.
  //Else we need to, so the declarations are unregistered from symbol-table and from TopDUContextDynamicData in their destructor
  foreach(const LocalIndexedDUContext &ctx, children)
    if(ctx.isLoaded(top) || !top->deleting() || !top->isOnDisk())
      delete ctx.data(top);
}

QVector< Declaration * > DUContext::clearLocalDeclarations( )
{
  QVector< Declaration * > ret = localDeclarations();
  foreach (Declaration* dec, ret)
    dec->setContext(0);
  return ret;
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

  while(left || right) {
    if(!left || !right)
      return false;

    if(!(left->d_func()->m_scopeIdentifier == right->d_func()->m_scopeIdentifier))
      return false;

    left = left->parentContext();
    right = right->parentContext();
  }

  return true;
}

void DUContext::setLocalScopeIdentifier(const QualifiedIdentifier & identifier)
{
  ENSURE_CAN_WRITE
  //Q_ASSERT(d_func()->m_childContexts.isEmpty() && d_func()->m_localDeclarations.isEmpty());
  bool wasInSymbolTable = inSymbolTable();
  setInSymbolTable(false);
  d_func_dynamic()->m_scopeIdentifier = identifier;
  setInSymbolTable(wasInSymbolTable);
  //DUChain::contextChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

QualifiedIdentifier DUContext::localScopeIdentifier() const
{
  //ENSURE_CAN_READ Commented out for performance reasons

  return d_func()->m_scopeIdentifier;
}

IndexedQualifiedIdentifier DUContext::indexedLocalScopeIdentifier() const {
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

  //DUChain::contextChanged(this, DUChainObserver::Change, DUChainObserver::ContextType);
}

QList<Declaration*> DUContext::findDeclarations(const Identifier& identifier, const SimpleCursor& position, const TopDUContext* topContext, SearchFlags flags) const
{
  ENSURE_CAN_READ

  DeclarationList ret;
  SearchItem::PtrList identifiers;
  identifiers << SearchItem::Ptr(new SearchItem(QualifiedIdentifier(identifier)));
  findDeclarationsInternal(identifiers, position.isValid() ? position : range().end, AbstractType::Ptr(), ret, topContext ? topContext : this->topContext(), flags);
  return arrayToList(ret);
}

void DUContext::deleteUse(int index)
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(DUContext);
  removeFromArray(d->m_usesList(), index);

  if(!m_dynamicData->m_rangesForUses.isEmpty()) {
    if(m_dynamicData->m_rangesForUses[index]) {
      EditorIntegrator editor;
      editor.setCurrentUrl(url(), (bool)smartRange());
      LockedSmartInterface iface = editor.smart();
      if (iface) {
        m_dynamicData->m_rangesForUses[index]->removeWatcher(this);
        EditorIntegrator::releaseRange(m_dynamicData->m_rangesForUses[index]);
      }
    }
    m_dynamicData->m_rangesForUses.remove(index);
  }
}

QVector<KTextEditor::SmartRange*> DUContext::takeUseRanges()
{
  ENSURE_CAN_WRITE
  QVector<KTextEditor::SmartRange*> ret = m_dynamicData->m_rangesForUses;

  foreach(KTextEditor::SmartRange* range, ret)
    if(range)
      range->removeWatcher(this);

  m_dynamicData->m_rangesForUses.clear();
  return ret;
}

QVector<KTextEditor::SmartRange*> DUContext::useRanges()
{
  ENSURE_CAN_READ
  return m_dynamicData->m_rangesForUses;
}

void DUContext::deleteUses()
{
  ENSURE_CAN_WRITE

  DUCHAIN_D_DYNAMIC(DUContext);
  d->m_usesList().clear();

  clearUseSmartRanges();
}

bool DUContext::inDUChain() const {
  if( d_func()->m_anonymousInParent || !m_dynamicData->m_parentContext)
    return false;

  TopDUContext* top = topContext();
  return top && top->inDUChain();
}

DUContext* DUContext::specialize(uint /*specialization*/, const TopDUContext* /*topContext*/, int /*upDistance*/) {
  return this;
}

SimpleCursor DUContext::importPosition(const DUContext* target) const
{
  ENSURE_CAN_READ
  DUCHAIN_D(DUContext);
  Import import(const_cast<DUContext*>(target), SimpleCursor::invalid());
  for(unsigned int a = 0; a < d->m_importedContextsSize(); ++a)
    if(d->m_importedContexts()[a] == import)
      return d->m_importedContexts()[a].position;
  return SimpleCursor::invalid();
}

QVector<DUContext::Import> DUContext::importedParentContexts() const
{
  ENSURE_CAN_READ
  QVector<DUContext::Import> ret;
  FOREACH_FUNCTION(const DUContext::Import& import, d_func()->m_importedContexts)
    ret << import;
  return ret;
}

QList<DUContext*> DUContext::findContexts(ContextType contextType, const QualifiedIdentifier& identifier, const SimpleCursor& position, const TopDUContext* top, SearchFlags flags) const
{
  ENSURE_CAN_READ

  QList<DUContext*> ret;
  SearchItem::PtrList identifiers;
  identifiers << SearchItem::Ptr(new SearchItem(identifier));

  findContextsInternal(contextType, identifiers, position.isValid() ? position : range().end, ret, top ? top : topContext(), flags);
  return ret;
}

void DUContext::applyAliases(const SearchItem::PtrList& baseIdentifiers, SearchItem::PtrList& identifiers, const SimpleCursor& position, bool canBeNamespace, bool onlyImports) const {

  QList<Declaration*> imports = allLocalDeclarations(globalImportIdentifier);
  if(imports.isEmpty() && onlyImports) {
    identifiers = baseIdentifiers;
    return;
  }

  FOREACH_ARRAY( const SearchItem::Ptr& identifier, baseIdentifiers ) {
    bool addUnmodified = true;

    if( !identifier->isExplicitlyGlobal ) {

      if( !imports.isEmpty() )
      {
        //We have namespace-imports.
        foreach( Declaration* importDecl, imports )
        {
          if( importDecl->range().end > position )
            continue;
          //Search for the identifier with the import-identifier prepended
          Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(importDecl));
          NamespaceAliasDeclaration* alias = static_cast<NamespaceAliasDeclaration*>(importDecl);
          identifiers.append( SearchItem::Ptr( new SearchItem( alias->importIdentifier(), identifier ) ) ) ;
        }
      }

      if( !identifier->isEmpty() && (identifier->hasNext() || canBeNamespace) ) {
        QList<Declaration*> aliases = allLocalDeclarations(identifier->identifier);
        if(!aliases.isEmpty()) {
          //The first part of the identifier has been found as a namespace-alias.
          //In c++, we only need the first alias. However, just to be correct, follow them all for now.
          foreach( Declaration* aliasDecl, aliases )
          {
            if( aliasDecl->range().end > position )
              continue;
            if(!dynamic_cast<NamespaceAliasDeclaration*>(aliasDecl))
              continue;

            addUnmodified = false; //The un-modified identifier can be ignored, because it will be replaced with the resolved alias
            NamespaceAliasDeclaration* alias = static_cast<NamespaceAliasDeclaration*>(aliasDecl);

            //Create an identifier where namespace-alias part is replaced with the alias target
            identifiers.append( SearchItem::Ptr( new SearchItem( alias->importIdentifier(), identifier->next ) ) ) ;
          }
        }
      }
    }

    if( addUnmodified )
        identifiers.append(identifier);
  }
}

void DUContext::applyUpwardsAliases(SearchItem::PtrList& identifiers, const TopDUContext* /*source*/) const {

  if(type() == Namespace) {
    QualifiedIdentifier localId = d_func()->m_scopeIdentifier;
    if(localId.isEmpty())
      return;

    //Make sure we search for the items in all namespaces of the same name, by duplicating each one with the namespace-identifier prepended.
    //We do this by prepending items to the current identifiers that equal the local scope identifier.
    SearchItem::Ptr newItem( new SearchItem(localId) );

    //This will exclude explictly global identifiers
    newItem->addToEachNode( identifiers );

    if(!newItem->next.isEmpty()) {
      //Prepend the full scope before newItem
      DUContext* parent = m_dynamicData->m_parentContext.data();
      while(parent) {
        newItem = SearchItem::Ptr( new SearchItem(parent->d_func()->m_scopeIdentifier, newItem) );
        parent = parent->m_dynamicData->m_parentContext.data();
      }

      newItem->isExplicitlyGlobal = true;
      insertToArray(identifiers, newItem, 0);
    }
  }
}

void DUContext::findContextsInternal(ContextType contextType, const SearchItem::PtrList& baseIdentifiers, const SimpleCursor& position, QList<DUContext*>& ret, const TopDUContext* source, SearchFlags flags) const
{
  DUCHAIN_D(DUContext);
  if (contextType == type()) {
    FOREACH_ARRAY( const SearchItem::Ptr& identifier, baseIdentifiers )
      if (identifier->match(scopeIdentifier(true)) && (!parentContext() || !identifier->isExplicitlyGlobal) )
        ret.append(const_cast<DUContext*>(this));
  }
  ///@todo This doesn't seem quite correct: Local Contexts aren't found anywhere
  ///Step 1: Apply namespace-aliases and -imports
  SearchItem::PtrList aliasedIdentifiers;
  //Because of namespace-imports and aliases, this identifier may need to be searched as under multiple names
  applyAliases(baseIdentifiers, aliasedIdentifiers, position, contextType == Namespace, contextType != Namespace);

  if( d->m_importedContextsSize() != 0 ) {
    ///Step 2: Give identifiers that are not marked as explicitly-global to imported contexts(explicitly global ones are treatead in TopDUContext)
    SearchItem::PtrList nonGlobalIdentifiers;
    FOREACH_ARRAY( const SearchItem::Ptr& identifier, aliasedIdentifiers )
      if( !identifier->isExplicitlyGlobal )
        nonGlobalIdentifiers << identifier;

    if( !nonGlobalIdentifiers.isEmpty() ) {
      for(int a = d->m_importedContextsSize()-1; a >= 0; --a) {

        DUContext* context = d->m_importedContexts()[a].context(source);

        while( !context && a > 0 ) {
          --a;
          context = d->m_importedContexts()[a].context(source);
        }

        if(context == this) {
          kDebug() << "resolved self as import:" << scopeIdentifier(true);
          continue;
        }

        if( !context )
          break;

        context->findContextsInternal(contextType, nonGlobalIdentifiers, url() == context->url() ? position : context->range().end, ret, source, flags | InImportedParentContext);
      }
    }
  }

  ///Step 3: Continue search in parent
  if ( !(flags & DontSearchInParent) && shouldSearchInParent(flags) && parentContext()) {
    applyUpwardsAliases(aliasedIdentifiers, source);
    parentContext()->findContextsInternal(contextType, aliasedIdentifiers, url() == parentContext()->url() ? position : parentContext()->range().end, ret, source, flags);
  }
}

bool DUContext::shouldSearchInParent(SearchFlags flags) const
{
  return (parentContext() && parentContext()->type() == DUContext::Helper && (flags & InImportedParentContext)) ||
         !(flags & InImportedParentContext);
}

const Use* DUContext::uses() const
{
  ENSURE_CAN_READ

  synchronizeUsesFromSmart();
  return d_func()->m_uses();
}

int DUContext::usesCount() const
{
  return d_func()->m_usesSize();
}

int DUContext::createUse(int declarationIndex, const SimpleRange& range, KTextEditor::SmartRange* smartRange, int insertBefore)
{
  DUCHAIN_D_DYNAMIC(DUContext);
  ENSURE_CAN_WRITE

  if(insertBefore == -1) {
    //Find position where to insert
    unsigned int a = 0;
    for(; a < d->m_usesSize() && range.start > d->m_uses()[a].m_range.start; ++a) { ///@todo do binary search
    }
    insertBefore = a;
  }

  insertToArray(d->m_usesList(), Use(range, declarationIndex), insertBefore);
  if(smartRange) {
    ///When this assertion triggers, then the updated context probably was not smart-converted before processing. @see SmartConverter
    Q_ASSERT(uint(m_dynamicData->m_rangesForUses.size()) == d->m_usesSize()-1);
    m_dynamicData->m_rangesForUses.insert(insertBefore, smartRange);
    smartRange->addWatcher(this);
//     smartRange->setWantsDirectChanges(true);

    d->m_usesList()[insertBefore].m_range = SimpleRange(*smartRange);
  }else{
    // This can happen eg. when a document is closed during its parsing, and has no ill effects.
    //Q_ASSERT(m_dynamicData->m_rangesForUses.isEmpty());
  }

  return insertBefore;
}

KTextEditor::SmartRange* DUContext::useSmartRange(int useIndex)
{
  ENSURE_CAN_READ
  if(m_dynamicData->m_rangesForUses.isEmpty())
    return 0;
  else{
    if(useIndex >= 0 && useIndex < m_dynamicData->m_rangesForUses.size())
      return m_dynamicData->m_rangesForUses.at(useIndex);
    else
      return 0;
  }
}


void DUContext::setUseSmartRange(int useIndex, KTextEditor::SmartRange* range)
{
  ENSURE_CAN_WRITE
  if(m_dynamicData->m_rangesForUses.isEmpty())
      m_dynamicData->m_rangesForUses.insert(0, d_func()->m_usesSize(), 0);

  Q_ASSERT(uint(m_dynamicData->m_rangesForUses.size()) == d_func()->m_usesSize());

  if(m_dynamicData->m_rangesForUses[useIndex]) {
    EditorIntegrator editor;
    editor.setCurrentUrl(url(), (bool)range);
    LockedSmartInterface iface = editor.smart();
    if (iface) {
      m_dynamicData->m_rangesForUses[useIndex]->removeWatcher(this);
      EditorIntegrator::releaseRange(m_dynamicData->m_rangesForUses[useIndex]);
    }
  }

  m_dynamicData->m_rangesForUses[useIndex] = range;
  d_func_dynamic()->m_usesList()[useIndex].m_range = SimpleRange(*range);
  range->addWatcher(this);
//   range->setWantsDirectChanges(true);
}

void DUContext::clearUseSmartRanges()
{
  ENSURE_CAN_WRITE

  if (!m_dynamicData->m_rangesForUses.isEmpty()) {
    EditorIntegrator editor;
    editor.setCurrentUrl(url(), (bool)smartRange());
    LockedSmartInterface iface = editor.smart();
    if (iface) {
      foreach (SmartRange* range, m_dynamicData->m_rangesForUses) {
        range->removeWatcher(this);
        EditorIntegrator::releaseRange(range);
      }
    }

    m_dynamicData->m_rangesForUses.clear();
  }
}

void DUContext::setUseDeclaration(int useNumber, int declarationIndex)
{
  ENSURE_CAN_WRITE
  d_func_dynamic()->m_usesList()[useNumber].m_declarationIndex = declarationIndex;
}


DUContext * DUContext::findContextAt(const SimpleCursor & position, bool includeRightBorder) const
{
  ENSURE_CAN_READ
  
//   kDebug() << "searchign" << position.textCursor() << "in:" << scopeIdentifier(true).toString() << range().textRange() << includeRightBorder;

  if (!range().contains(position) && (!includeRightBorder || range().end != position)) {
//     kDebug() << "mismatch";
    return 0;
  }

  for(int a = int(d_func()->m_childContextsSize())-1; a >= 0; --a)
    if (DUContext* specific = d_func()->m_childContexts()[a].data(topContext())->findContextAt(position, includeRightBorder))
      return specific;

  return const_cast<DUContext*>(this);
}

Declaration * DUContext::findDeclarationAt(const SimpleCursor & position) const
{
  ENSURE_CAN_READ

  if (!range().contains(position))
    return 0;

  FOREACH_FUNCTION(LocalIndexedDeclaration child, d_func()->m_localDeclarations)
    if (child.data(topContext())->range().contains(position))
      return child.data(topContext());

  return 0;
}

DUContext* DUContext::findContextIncluding(const SimpleRange& range) const
{
  ENSURE_CAN_READ

  if (!this->range().contains(range))
    return 0;

  FOREACH_FUNCTION(LocalIndexedDUContext child, d_func()->m_childContexts)
    if (DUContext* specific = child.data(topContext())->findContextIncluding(range))
      return specific;

  return const_cast<DUContext*>(this);
}

int DUContext::findUseAt(const SimpleCursor & position) const
{
  ENSURE_CAN_READ

  synchronizeUsesFromSmart();

  if (!range().contains(position))
    return -1;

  for(unsigned int a = 0; a < d_func()->m_usesSize(); ++a)
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
    if(parentContext()) {
      if(!d_func()->m_inSymbolTable && inSymbolTable) {
        QualifiedIdentifier id(scopeIdentifier(true));
        PersistentSymbolTable::self().addContext(id, this);

      }else if(d_func()->m_inSymbolTable && !inSymbolTable) {
        QualifiedIdentifier id(scopeIdentifier(true));
        PersistentSymbolTable::self().removeContext(id, this);
      }
    }

  d_func_dynamic()->m_inSymbolTable = inSymbolTable;
}

// kate: indent-width 2;

void DUContext::clearImportedParentContexts()
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(DUContext);

  while( d->m_importedContextsSize() != 0 ) {
    if( d->m_importedContexts()[0].isBackwardMapped() ) {
      DUContext* ctx = d->m_importedContexts()[0].context(0);
      if(ctx)
        ctx->m_dynamicData->removeImportedChildContext(this);
    }

    d->m_importedContextsList().removeOne(d->m_importedContexts()[0]);
  }
}

void DUContext::cleanIfNotEncountered(const QSet<DUChainBase*>& encountered)
{
  ENSURE_CAN_WRITE

  foreach (Declaration* dec, localDeclarations())
    if (!encountered.contains(dec))
      delete dec;

  FOREACH_FUNCTION(LocalIndexedDUContext childContext, d_func()->m_childContexts)
    if (!encountered.contains(childContext.data(topContext())))
      delete childContext.data(topContext());
}

TopDUContext* DUContext::topContext() const
{
  return m_dynamicData->m_topContext;
}

QWidget* DUContext::createNavigationWidget(Declaration* /*decl*/, TopDUContext* /*topContext*/, const QString& /*htmlPrefix*/, const QString& /*htmlSuffix*/) const
{
  return 0;
}

void DUContext::squeeze()
{
  if(!m_dynamicData->m_rangesForUses.isEmpty())
    m_dynamicData->m_rangesForUses.squeeze();

  FOREACH_FUNCTION(LocalIndexedDUContext child, d_func()->m_childContexts)
    child.data(topContext())->squeeze();
}

QList<SimpleRange> allUses(DUContext* context, int declarationIndex, bool noEmptyUses)
{
  QList<SimpleRange> ret;
  for(int a = 0; a < context->usesCount(); ++a)
    if(context->uses()[a].m_declarationIndex == declarationIndex)
      if(!noEmptyUses || !context->uses()[a].m_range.isEmpty())
        ret << context->uses()[a].m_range;

  foreach(DUContext* child, context->childContexts())
    ret += allUses(child, declarationIndex, noEmptyUses);

  return ret;
}

QList<KTextEditor::SmartRange*> allSmartUses(DUContext* context, int declarationIndex)
{
  QList<KTextEditor::SmartRange*> ret;

  const Use* uses(context->uses());

  for(int a = 0; a < context->usesCount(); ++a)
    if(uses[a].m_declarationIndex == declarationIndex) {
      KTextEditor::SmartRange* range = context->useSmartRange(a);
      if(range)
        ret << range;
    }

  foreach(DUContext* child, context->childContexts())
    ret += allSmartUses(child, declarationIndex);

  return ret;
}

DUContext::SearchItem::SearchItem(const QualifiedIdentifier& id, Ptr nextItem, int start) : isExplicitlyGlobal(start == 0 ? id.explicitlyGlobal() : false) {
  if(!id.isEmpty()) {
    if(id.count() > start)
      identifier = id.at(start);

    if(id.count() > start+1)
      addNext(Ptr( new SearchItem(id, nextItem, start+1) ));
    else if(nextItem)
      next.append(nextItem);
  }else if(nextItem) {
    ///If there is no prefix, just copy nextItem
    isExplicitlyGlobal = nextItem->isExplicitlyGlobal;
    identifier = nextItem->identifier;
    next = nextItem->next;
  }
}

DUContext::SearchItem::SearchItem(const QualifiedIdentifier& id, const PtrList& nextItems, int start) : isExplicitlyGlobal(start == 0 ? id.explicitlyGlobal() : false) {
  if(id.count() > start)
    identifier = id.at(start);

  if(id.count() > start+1)
    addNext(Ptr( new SearchItem(id, nextItems, start+1) ));
  else
    next = nextItems;
}

DUContext::SearchItem::SearchItem(bool explicitlyGlobal, Identifier id, const PtrList& nextItems) : isExplicitlyGlobal(explicitlyGlobal), identifier(id), next(nextItems) {
}

DUContext::SearchItem::SearchItem(bool explicitlyGlobal, Identifier id, Ptr nextItem) : isExplicitlyGlobal(explicitlyGlobal), identifier(id) {
  next.append(nextItem);
}

bool DUContext::SearchItem::match(const QualifiedIdentifier& id, int offset) const {
  if(id.isEmpty()) {
    if(identifier.isEmpty() && next.isEmpty())
      return true;
    else
      return false;
  }

  if(id.at(offset) != identifier) //The identifier is different
    return false;

  if(offset == id.count()-1) {
    if(next.isEmpty())
      return true; //match
    else
      return false; //id is too short
  }

  for(int a = 0; a < next.size(); ++a)
    if(next[a]->match(id, offset+1))
      return true;

  return false;
}

bool DUContext::SearchItem::isEmpty() const {
  return identifier.isEmpty();
}

bool DUContext::SearchItem::hasNext() const {
  return !next.isEmpty();
}

QList<QualifiedIdentifier> DUContext::SearchItem::toList(const QualifiedIdentifier& prefix) const {
  QList<QualifiedIdentifier> ret;

  QualifiedIdentifier id = prefix;
  if(id.isEmpty())
  id.setExplicitlyGlobal(isExplicitlyGlobal);
  if(!identifier.isEmpty())
    id.push(identifier);

  if(next.isEmpty()) {
    ret << id;
  } else {
    for(int a = 0; a < next.size(); ++a)
      ret += next[a]->toList(id);
  }
  return ret;
}


void DUContext::SearchItem::addNext(SearchItem::Ptr other) {
  next.append(other);
}

void DUContext::SearchItem::addToEachNode(SearchItem::Ptr other) {
  if(other->isExplicitlyGlobal)
    return;

  next.append(other);
  for(int a = 0; a < next.size()-1; ++a)
    next[a]->addToEachNode(other);
}

void DUContext::SearchItem::addToEachNode(SearchItem::PtrList other) {
  int added = 0;
  FOREACH_ARRAY(SearchItem::Ptr o, other) {
    if(!o->isExplicitlyGlobal) {
      next.append(o);
      ++added;
    }
  }

  for(int a = 0; a < next.size()-added; ++a)
    next[a]->addToEachNode(other);
}

DUContext::Import::Import(DUContext* _context, const SimpleCursor& _position) : position(_position) {
  if(_context && _context->owner() && _context->owner()->specialization()) {
    m_declaration = _context->owner()->id(true);
  }else{
    m_context = _context;
  }
}

DUContext::Import::Import(const DeclarationId& id, const SimpleCursor& _position) : position(_position) {
  m_declaration = id;
}

DUContext* DUContext::Import::context(const TopDUContext* topContext) const {
  if(m_declaration.isValid()) {
    Declaration* decl = m_declaration.getDeclaration(topContext);
    if(decl)
      return decl->logicalInternalContext(topContext);
    else
      return 0;
  }else{
    return m_context.data();
  }
}

bool DUContext::Import::isBackwardMapped() const {
  return true; //Always do backward-mapping. We just have to find a solution for temporary imports.
/*  if(m_declaration.isValid())
    return !m_declaration.specialization();
  else
    return true;*/
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
