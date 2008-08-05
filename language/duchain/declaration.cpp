/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "declaration.h"
#include "declarationdata.h"

#include <QByteArray>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include <limits>

#include "topducontext.h"
#include "topducontextdynamicdata.h"
#include "use.h"
#include "symboltable.h"
#include "forwarddeclaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "ducontextdata.h"
#include "declarationid.h"
#include "uses.h"
#include "indexedstring.h"
#include "duchainregister.h"
#include "persistentsymboltable.h"
#include "repositories/stringrepository.h"
#include "types/identifiedtype.h"
#include "functiondefinition.h"

using namespace KTextEditor;

namespace KDevelop
{

Repositories::StringRepository commentRepository("Comment Repository");

REGISTER_DUCHAIN_ITEM(Declaration);

DeclarationData::DeclarationData() 
  : m_isDefinition(false), m_inSymbolTable(false),  
    m_isTypeAlias(false), m_anonymousInContext(false), m_comment(0)
{
  m_kind = Declaration::Instance;
}
  
DeclarationData::DeclarationData( const DeclarationData& rhs ) : DUChainBaseData(rhs)
{
  m_identifier = rhs.m_identifier;
  m_declaration = rhs.m_declaration;
  m_type = rhs.m_type;
  m_kind = rhs.m_kind;
  m_isDefinition = rhs.m_isDefinition;
  m_isTypeAlias = rhs.m_isTypeAlias;
  m_inSymbolTable = rhs.m_inSymbolTable;
  m_comment = rhs.m_comment;
  m_anonymousInContext = rhs.m_anonymousInContext;
  m_internalContext = rhs.m_internalContext;
}
  
Declaration::Kind Declaration::kind() const {
  DUCHAIN_D(Declaration);
  return d->m_kind;
}

void Declaration::setKind(Kind kind) {
  DUCHAIN_D_DYNAMIC(Declaration);
  d->m_kind = kind;
}

bool Declaration::inDUChain() const {
  DUCHAIN_D(Declaration);
  if( d->m_anonymousInContext )
    return false;
  if( !context() )
    return false;
  TopDUContext* top = topContext();
  return top && top->inDuChain();
}

Declaration::Declaration( const SimpleRange& range, DUContext* context )
  : DUChainBase(*new DeclarationData, range)
{
  d_func_dynamic()->setClassId(this);
  m_topContext = 0;
  m_context = 0;
  m_indexInTopContext = 0;
  
  if(context)
    setContext(context);
}

uint Declaration::ownIndex() const
{
  ENSURE_CAN_READ
  return m_indexInTopContext;
}

Declaration::Declaration(const Declaration& rhs) 
  : DUChainBase(*new DeclarationData( *rhs.d_func() )) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
  m_topContext = 0;
  m_context = 0;
  m_indexInTopContext = 0;
}

Declaration::Declaration( DeclarationData & dd ) : DUChainBase(dd)
{
  m_topContext = 0;
  m_context = 0;
  m_indexInTopContext = 0;
}

Declaration::Declaration( DeclarationData & dd, const SimpleRange& range )
  : DUChainBase(dd, range)
{
  m_topContext = 0;
  m_context = 0;
  m_indexInTopContext = 0;
}

Declaration::~Declaration()
{
  DUCHAIN_D_DYNAMIC(Declaration);
  // Inserted by the builder after construction has finished.
  if(!topContext()->isOnDisk()) {
    if( d->m_internalContext.context() )
      d->m_internalContext.context()->setOwner(0);
  }
  
  if (d->m_inSymbolTable && !d->m_identifier.isEmpty()) {
    SymbolTable::self()->removeDeclaration(this);
    if(!topContext()->isOnDisk())
      PersistentSymbolTable::self().removeDeclaration(qualifiedIdentifier(), this);
  }
  
  d->m_inSymbolTable = false;

  // context is only null in the test cases
  if (context() && !d->m_anonymousInContext) {
    Q_ASSERT(context()->m_dynamicData->removeDeclaration(this));
  }

  clearOwnIndex();
  
  setContext(0);

  setAbstractType(AbstractType::Ptr());
  //DUChain::declarationChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
}

QByteArray Declaration::comment() const {
  DUCHAIN_D(Declaration);
  if(!d->m_comment)
    return 0;
  else
    return Repositories::arrayFromItem(commentRepository.itemFromIndex(d->m_comment));
}

void Declaration::setComment(const QByteArray& str) {
  DUCHAIN_D_DYNAMIC(Declaration);
  if(str.isEmpty())
    d->m_comment = 0;
  else
    d->m_comment = commentRepository.index(Repositories::StringRepositoryItemRequest(str, IndexedString::hashString(str, str.length()), str.length()));
}

void Declaration::setComment(const QString& str) {
  setComment(str.toUtf8());
}

Identifier Declaration::identifier( ) const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return d_func()->m_identifier.identifier();
}

LocalIndexedDeclaration::LocalIndexedDeclaration(Declaration* decl) {
  if(!decl)
    m_declarationIndex = 0;
  else
    m_declarationIndex = decl->m_indexInTopContext;
}

LocalIndexedDeclaration::LocalIndexedDeclaration(uint declarationIndex) : m_declarationIndex(declarationIndex) {
}

Declaration* LocalIndexedDeclaration::data(TopDUContext* top) const {
  if(m_declarationIndex)
    return top->m_dynamicData->getDeclarationForIndex(m_declarationIndex);
  else
    return 0;
}

IndexedDeclaration::IndexedDeclaration(uint topContext, uint declarationIndex) : m_topContext(topContext), m_declarationIndex(declarationIndex) {
}

IndexedDeclaration::IndexedDeclaration(Declaration* decl) {
  if(decl) {
    m_topContext = decl->topContext()->ownIndex();
    m_declarationIndex = decl->m_indexInTopContext;
  }else{
    m_topContext = 0;
    m_declarationIndex = 0;
  }
}

Declaration* IndexedDeclaration::declaration() const {
  ENSURE_CHAIN_READ_LOCKED
  if(!m_topContext || !m_declarationIndex)
    return 0;
  
  TopDUContext* ctx = DUChain::self()->chainForIndex(m_topContext);
  if(!ctx)
    return 0;
  
  return ctx->m_dynamicData->getDeclarationForIndex(m_declarationIndex);
}

void Declaration::rebuildDynamicData(DUContext* parent, uint ownIndex)
{
  DUChainBase::rebuildDynamicData(parent, ownIndex);
  
  m_context = parent;
  m_topContext = parent->topContext();
  m_indexInTopContext = ownIndex;
  
  if(d_func()->m_inSymbolTable && !d_func()->m_identifier.isEmpty())
      SymbolTable::self()->addDeclaration(this);
}

void Declaration::setIdentifier(const Identifier& identifier)
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(Declaration);
  bool wasInSymbolTable = d->m_inSymbolTable;
  
  setInSymbolTable(false);
    
  if( m_context && !d->m_anonymousInContext )
    m_context->changingIdentifier( this, d->m_identifier, identifier );

  d->m_identifier = identifier;
  
  setInSymbolTable(wasInSymbolTable);
  //DUChain::declarationChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

IndexedType Declaration::indexedType() const
{
  return d_func()->m_type;
}

AbstractType::Ptr Declaration::abstractType( ) const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return d_func()->m_type.type();
}

void Declaration::setAbstractType(AbstractType::Ptr type)
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(Declaration);
  //if (d->m_type)
    //DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::DataType);

  d->m_type = type->indexed();

  //if (d->m_type)
    //DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::DataType);
}

Declaration* Declaration::specialize(uint /*specialization*/, const TopDUContext* /*topContext*/)
{
  return this;
}

QualifiedIdentifier Declaration::qualifiedIdentifier() const
{
  ENSURE_CAN_READ
  
  QualifiedIdentifier ret;
  DUContext* ctx = m_context;
  if(ctx)
    ret = ctx->scopeIdentifier(true);
  ret.push(d_func()->m_identifier);
  return ret;
}

// QString Declaration::mangledIdentifier() const
// {
//   //GNU mangling specs from http://theory.uwinnipeg.ca/gnu/gcc/gxxint_15.html
// 
//   if (abstractType())
//     return abstractType()->mangled();
// 
//   // Error...
//   return qualifiedIdentifier().mangled();
// }

DUContext * Declaration::context() const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return m_context;
}

void Declaration::setContext(DUContext* context, bool anonymous)
{
  Q_ASSERT(!context || context->topContext());
  ///@todo re-enable. In C++ support we need a short window to put visible declarations into template contexts
  if(!specialization()) {
    ENSURE_CAN_WRITE
  }

  setInSymbolTable(false);

  //We don't need to clear, because it's not allowed to move from one top-context into another
//   clearOwnIndex();
  
  
  DUCHAIN_D_DYNAMIC(Declaration);
  if (m_context && context) {
    Q_ASSERT(m_context->topContext() == context->topContext());
  }

  if (m_context) {
    if( !d->m_anonymousInContext ) {
      m_context->m_dynamicData->removeDeclaration(this);
        //DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::Context, m_context);
    }
  }

  if(context)
    m_topContext = context->topContext();
  else
    m_topContext = 0;
  
  m_context = context;
  d->m_anonymousInContext = anonymous;

  if (context) {
    if(!m_indexInTopContext)
      allocateOwnIndex();
    
    if(!d->m_anonymousInContext) {
      context->m_dynamicData->addDeclaration(this);
      //DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::Context, m_context);
    }

    if(context->inSymbolTable() && !anonymous)
      setInSymbolTable(true);
  }
}

void Declaration::clearOwnIndex() {
  
  if(!m_indexInTopContext)
    return;
  
  if(!context() || (!context()->isAnonymous() && !d_func()->m_anonymousInContext)) {
    ENSURE_CAN_WRITE
  }
  
  if(m_indexInTopContext) {
    Q_ASSERT(m_topContext);
    m_topContext->m_dynamicData->clearDeclarationIndex(this);
  }
  m_indexInTopContext = 0;
}

void Declaration::allocateOwnIndex() {
  
  if(context() && (!context()->isAnonymous() && !d_func()->m_anonymousInContext)) {
    ENSURE_CAN_WRITE
  }

  Q_ASSERT(m_topContext);

  m_indexInTopContext = m_topContext->m_dynamicData->allocateDeclarationIndex(this, d_func()->m_anonymousInContext || !context() || context()->isAnonymous());
  Q_ASSERT(m_indexInTopContext);
}

const Declaration* Declaration::logicalDeclaration(const TopDUContext* topContext) const {
  ENSURE_CAN_READ
  if(isForwardDeclaration()) {
    const ForwardDeclaration* dec = toForwardDeclaration();
    Declaration* ret = dec->resolve(topContext);
    if(ret)
      return ret;
  }
  return this;
}

Declaration* Declaration::logicalDeclaration(const TopDUContext* topContext) {
  ENSURE_CAN_READ
  if(isForwardDeclaration()) {
    ForwardDeclaration* dec = toForwardDeclaration();
    Declaration* ret = dec->resolve(topContext);
    if(ret)
      return ret;
  }
  return this;
}

DUContext * Declaration::logicalInternalContext(const TopDUContext* topContext) const {
  ENSURE_CAN_READ

  if(!isDefinition()) {
    Declaration* def = FunctionDefinition::definition(this);
    if( def )
      return def->internalContext();
  }

  if( d_func()->m_isTypeAlias ) {
    ///If this is a type-alias, return the internal context of the actual type.
    AbstractType::Ptr t = abstractType();
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(t.unsafeData());
    if( idType && idType->declaration(topContext) && idType->declaration(topContext) != this )
      return idType->declaration(topContext)->logicalInternalContext( topContext );
  }
  
  return internalContext();
}

DUContext * Declaration::internalContext() const
{
  ENSURE_CAN_READ
  return d_func()->m_internalContext.context();
}

void Declaration::setInternalContext(DUContext* context)
{
  if(this->context()) {
    ENSURE_CAN_WRITE
  }
  DUCHAIN_D_DYNAMIC(Declaration);

  if( context == d->m_internalContext.context() )
    return;

  if(!m_topContext) {
    //Take the top-context from the other side. We need to allocate an index, so we can safely call setOwner(..)
    m_topContext = context->topContext();
    allocateOwnIndex();
  }
  
  DUContext* oldInternalContext = d->m_internalContext.context();
  
  d->m_internalContext = context;

  //Q_ASSERT( !oldInternalContext || oldInternalContext->owner() == this );
  if( oldInternalContext && oldInternalContext->owner() == this )
    oldInternalContext->setOwner(0);
  

  if( context )
    context->setOwner(this);
}


bool Declaration::operator ==(const Declaration & other) const
{
  ENSURE_CAN_READ

  return this == &other;
}

QString Declaration::toString() const
{
  return QString("%3 %4").arg(abstractType() ? abstractType()->toString() : QString("<notype>")).arg(identifier().toString());
}

// kate: indent-width 2;

bool Declaration::isDefinition() const
{
  ENSURE_CAN_READ
  DUCHAIN_D(Declaration);

  return d->m_isDefinition;
}

void Declaration::setDeclarationIsDefinition(bool dd)
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(Declaration);
  d->m_isDefinition = dd;
//   if (d->m_isDefinition && definition()) {
//     setDefinition(0);
//   }
}

///@todo see whether it would be useful to create an own TypeAliasDeclaration sub-class for this
bool Declaration::isTypeAlias() const {
  DUCHAIN_D(Declaration);
  return d->m_isTypeAlias;
}

void Declaration::setIsTypeAlias(bool isTypeAlias) {
  DUCHAIN_D_DYNAMIC(Declaration);
  d->m_isTypeAlias = isTypeAlias;
}

uint Declaration::specialization() const {
  return 0;
}

DeclarationId Declaration::id(bool forceDirect) const
{
  ENSURE_CAN_READ
  if(inSymbolTable() && !forceDirect)
    return DeclarationId(qualifiedIdentifier(), additionalIdentity(), specialization());
  else
    return DeclarationId(IndexedDeclaration(const_cast<Declaration*>(this)), specialization());
}

bool Declaration::inSymbolTable() const
{
  DUCHAIN_D(Declaration);
  return d->m_inSymbolTable;
}

void Declaration::setInSymbolTable(bool inSymbolTable)
{
  DUCHAIN_D_DYNAMIC(Declaration);
  if(!d->m_identifier.isEmpty()) {
    if(!d->m_inSymbolTable && inSymbolTable) {
      SymbolTable::self()->addDeclaration(this);
      PersistentSymbolTable::self().addDeclaration(qualifiedIdentifier(), this);
    }
  
    else if(d->m_inSymbolTable && !inSymbolTable) {
      SymbolTable::self()->removeDeclaration(this);
      PersistentSymbolTable::self().removeDeclaration(qualifiedIdentifier(), this);
    }
  }
  d->m_inSymbolTable = inSymbolTable;
}

ForwardDeclaration* Declaration::toForwardDeclaration()
{
  return static_cast<ForwardDeclaration*>(this);
}

const ForwardDeclaration* Declaration::toForwardDeclaration() const
{
  return static_cast<const ForwardDeclaration*>(this);
}

TopDUContext * Declaration::topContext() const
{
  return m_topContext;
}

Declaration* Declaration::clonePrivate() const  {
  return new Declaration(*this);
}

Declaration* Declaration::clone() const  {
  Declaration* ret = clonePrivate();
  ret->d_func_dynamic()->m_inSymbolTable = false;
  return ret;
}

bool Declaration::isForwardDeclaration() const
{
  return false;
}

bool Declaration::isFunctionDeclaration() const
{
  return false;
}

uint Declaration::additionalIdentity() const
{
  return 0;
}

bool Declaration::equalQualifiedIdentifier(const Declaration* rhs) const {
  ENSURE_CAN_READ
  DUCHAIN_D(Declaration);
  if(d->m_identifier != rhs->d_func()->m_identifier)
    return false;
  
  return m_context->equalScopeIdentifier(m_context);
}

QList<KTextEditor::SmartRange*> Declaration::smartUses() const
{
  Q_ASSERT(topContext());
  ENSURE_CAN_READ
  QSet<KTextEditor::SmartRange*> tempUses;
  //First, search for uses within the own context
  {
    foreach(KTextEditor::SmartRange* range, allSmartUses(topContext(), const_cast<Declaration*>(this)))
      tempUses.insert(range);
  }

  KDevVarLengthArray<IndexedTopDUContext> useContexts = DUChain::uses()->uses(id());

  FOREACH_ARRAY(IndexedTopDUContext indexedContext, useContexts) {
    TopDUContext* context = indexedContext.data();
    if(context) {
      foreach(KTextEditor::SmartRange* range, allSmartUses(context, const_cast<Declaration*>(this)))
        tempUses.insert(range);
    }
  }

  return tempUses.toList();
}

QMap<IndexedString, QList<SimpleRange> > Declaration::uses() const
{
  ENSURE_CAN_READ
  QMap<IndexedString, QMap<SimpleRange, bool> > tempUses;

  //First, search for uses within the own context
  {
    QMap<SimpleRange, bool>& ranges(tempUses[topContext()->url()]);
    foreach(const SimpleRange& range, allUses(topContext(), const_cast<Declaration*>(this)))
      ranges[range] = true;
  }

  KDevVarLengthArray<IndexedTopDUContext> useContexts = DUChain::uses()->uses(id());

  FOREACH_ARRAY(IndexedTopDUContext indexedContext, useContexts) {
    TopDUContext* context = indexedContext.data();
    if(context) {
      QMap<SimpleRange, bool>& ranges(tempUses[context->url()]);
      foreach(const SimpleRange& range, allUses(context, const_cast<Declaration*>(this)))
        ranges[range] = true;
    }
  }

  QMap<IndexedString, QList<SimpleRange> > ret;

  for(QMap<IndexedString, QMap<SimpleRange, bool> >::const_iterator it = tempUses.begin(); it != tempUses.end(); ++it) {
    if(!(*it).isEmpty()) {
      QList<SimpleRange>& list(ret[it.key()]);
      for(QMap<SimpleRange, bool>::const_iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
        list << it2.key();
    }
  }
  return ret;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on


