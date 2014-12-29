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

#include <QtCore/QByteArray>

#include <limits>

#include "topducontext.h"
#include "topducontextdynamicdata.h"
#include "use.h"
#include "forwarddeclaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "ducontextdata.h"
#include "declarationid.h"
#include "uses.h"
#include <serialization/indexedstring.h>
#include "duchainregister.h"
#include "persistentsymboltable.h"
#include "types/identifiedtype.h"
#include "types/structuretype.h"
#include "functiondefinition.h"
#include "codemodel.h"
#include "specializationstore.h"
#include "types/typeutils.h"
#include "types/typealiastype.h"
#include "classdeclaration.h"
#include "serialization/stringrepository.h"
#include "ducontextdynamicdata.h"

namespace KDevelop
{

REGISTER_DUCHAIN_ITEM(Declaration);

DeclarationData::DeclarationData()
  : m_comment(0)
  , m_isDefinition(false)
  , m_inSymbolTable(false)
  , m_isTypeAlias(false)
  , m_anonymousInContext(false)
  , m_isDeprecated(false)
  , m_alwaysForceDirect(false)
  , m_isAutoDeclaration(false)
  , m_isExplicitlyDeleted(false)
{
  m_kind = Declaration::Instance;
}

DeclarationData::DeclarationData( const DeclarationData& rhs ) : DUChainBaseData(rhs),
m_internalContext(rhs.m_internalContext),
m_type(rhs.m_type),
m_identifier(rhs.m_identifier),
m_declaration(rhs.m_declaration),
m_comment(rhs.m_comment),
m_kind(rhs.m_kind),
m_isDefinition(rhs.m_isDefinition),
m_inSymbolTable(rhs.m_inSymbolTable),
m_isTypeAlias(rhs.m_isTypeAlias),
m_anonymousInContext(rhs.m_anonymousInContext),
m_isDeprecated(rhs.m_isDeprecated),
m_alwaysForceDirect(rhs.m_alwaysForceDirect),
m_isAutoDeclaration(rhs.m_isAutoDeclaration),
m_isExplicitlyDeleted(rhs.m_isExplicitlyDeleted)
{
}

///@todo Use reference counting
static Repositories::StringRepository& commentRepository() {
    static Repositories::StringRepository commentRepositoryObject("Comment Repository");
    return commentRepositoryObject;
}

void initDeclarationRepositories() {
  commentRepository();
}

Declaration::Kind Declaration::kind() const {
  DUCHAIN_D(Declaration);
  return d->m_kind;
}

void Declaration::setKind(Kind kind) {
  DUCHAIN_D_DYNAMIC(Declaration);
  d->m_kind = kind;
  updateCodeModel();
}

bool Declaration::inDUChain() const {
  DUCHAIN_D(Declaration);
  if( d->m_anonymousInContext )
    return false;
  if( !context() )
    return false;
  TopDUContext* top = topContext();
  return top && top->inDUChain();
}

Declaration::Declaration( const RangeInRevision& range, DUContext* context )
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

Declaration::Declaration( DeclarationData & dd, const RangeInRevision& range )
  : DUChainBase(dd, range)
{
  m_topContext = 0;
  m_context = 0;
  m_indexInTopContext = 0;
}

bool Declaration::persistentlyDestroying() const
{
  TopDUContext* topContext = this->topContext();
  return !topContext->deleting() || !topContext->isOnDisk();
}

Declaration::~Declaration()
{
  uint oldOwnIndex = m_indexInTopContext;

  TopDUContext* topContext = this->topContext();

  //Only perform the actions when the top-context isn't being deleted, or when it hasn't been stored to disk
  if(persistentlyDestroying()) {
    DUCHAIN_D_DYNAMIC(Declaration);

    // Inserted by the builder after construction has finished.
    if( d->m_internalContext.context() )
      d->m_internalContext.context()->setOwner(0);

    setInSymbolTable(false);
  }

  // If the parent-context already has dynamic data, like for example any temporary context,
  // always delete the declaration, to not create crashes within more complex code like C++ template stuff.
  if (context() && !d_func()->m_anonymousInContext) {
    if(!topContext->deleting() || !topContext->isOnDisk() || context()->d_func()->isDynamic())
      context()->m_dynamicData->removeDeclaration(this);
  }

  clearOwnIndex();

  if(!topContext->deleting() || !topContext->isOnDisk()) {
    setContext(0);

    setAbstractType(AbstractType::Ptr());
  }
  Q_ASSERT(d_func()->isDynamic() == (!topContext->deleting() || !topContext->isOnDisk() || topContext->m_dynamicData->isTemporaryDeclarationIndex(oldOwnIndex)));
  Q_UNUSED(oldOwnIndex);
}

QByteArray Declaration::comment() const {
  DUCHAIN_D(Declaration);
  if(!d->m_comment)
    return 0;
  else
    return Repositories::arrayFromItem(commentRepository().itemFromIndex(d->m_comment));
}

void Declaration::setComment(const QByteArray& str) {
  DUCHAIN_D_DYNAMIC(Declaration);
  if(str.isEmpty())
    d->m_comment = 0;
  else
    d->m_comment = commentRepository().index(Repositories::StringRepositoryItemRequest(str, IndexedString::hashString(str, str.length()), str.length()));
}

void Declaration::setComment(const QString& str) {
  setComment(str.toUtf8());
}

Identifier Declaration::identifier( ) const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return d_func()->m_identifier.identifier();
}

const IndexedIdentifier& Declaration::indexedIdentifier( ) const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return d_func()->m_identifier;
}

void Declaration::rebuildDynamicData(DUContext* parent, uint ownIndex)
{
  DUChainBase::rebuildDynamicData(parent, ownIndex);

  m_context = parent;
  m_topContext = parent->topContext();
  m_indexInTopContext = ownIndex;
}

void Declaration::setIdentifier(const Identifier& identifier)
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(Declaration);
  bool wasInSymbolTable = d->m_inSymbolTable;

  setInSymbolTable(false);

  d->m_identifier = identifier;

  setInSymbolTable(wasInSymbolTable);
}

IndexedType Declaration::indexedType() const
{
  return d_func()->m_type;
}

AbstractType::Ptr Declaration::abstractType( ) const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return d_func()->m_type.abstractType();
}

void Declaration::setAbstractType(AbstractType::Ptr type)
{
  ENSURE_CAN_WRITE
  DUCHAIN_D_DYNAMIC(Declaration);

  d->m_type = type->indexed();

  updateCodeModel();
}

Declaration* Declaration::specialize(const IndexedInstantiationInformation& /*specialization*/, const TopDUContext* topContext, int /*upDistance*/)
{
  if(!topContext)
    return 0;
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

DUContext * Declaration::context() const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return m_context;
}

bool Declaration::isAnonymous() const
{
  return d_func()->m_anonymousInContext;
}

void Declaration::setContext(DUContext* context, bool anonymous)
{
  Q_ASSERT(!context || context->topContext());

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
    }
  }

  if(context)
    m_topContext = context->topContext();
  else
    m_topContext = 0;

  d->m_anonymousInContext = anonymous;
  m_context = context;

  if (context) {
    if(!m_indexInTopContext)
      allocateOwnIndex();

    if(!d->m_anonymousInContext) {
      context->m_dynamicData->addDeclaration(this);
    }

    if(context->inSymbolTable() && !anonymous)
      setInSymbolTable(true);
  }
}

void Declaration::clearOwnIndex() {

  if(!m_indexInTopContext)
    return;

  if(!context() || (!d_func()->m_anonymousInContext && !context()->isAnonymous())) {
    ENSURE_CAN_WRITE
  }

  if(m_indexInTopContext) {
    Q_ASSERT(m_topContext);
    m_topContext->m_dynamicData->clearDeclarationIndex(this);
  }
  m_indexInTopContext = 0;
}

void Declaration::allocateOwnIndex() {

  ///@todo Fix multithreading stuff with template instantiation, preferably using some internal mutexes
//   if(context() && (!context()->isAnonymous() && !d_func()->m_anonymousInContext)) {
//     ENSURE_CAN_WRITE
//   }

  Q_ASSERT(m_topContext);

  m_indexInTopContext = m_topContext->m_dynamicData->allocateDeclarationIndex(this, d_func()->m_anonymousInContext || !context() || context()->isAnonymous());
  Q_ASSERT(m_indexInTopContext);

  if(!m_topContext->m_dynamicData->getDeclarationForIndex(m_indexInTopContext))
    qFatal("Could not re-retrieve declaration\nindex: %d", m_indexInTopContext);

}

const Declaration* Declaration::logicalDeclaration(const TopDUContext* topContext) const {
  ENSURE_CAN_READ
  if(isForwardDeclaration()) {
    const auto dec = static_cast<const ForwardDeclaration*>(this);
    Declaration* ret = dec->resolve(topContext);
    if(ret)
      return ret;
  }
  return this;
}

Declaration* Declaration::logicalDeclaration(const TopDUContext* topContext) {
  ENSURE_CAN_READ
  if(isForwardDeclaration()) {
    const auto dec = static_cast<const ForwardDeclaration*>(this);
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
    TypeAliasType::Ptr t = type<TypeAliasType>();
    if(t) {
      AbstractType::Ptr target = t->type();
      
      IdentifiedType* idType = dynamic_cast<IdentifiedType*>(target.data());
      if( idType ) {
        Declaration* decl = idType->declaration(topContext);
        if(decl && decl != this) {
          return decl->logicalInternalContext( topContext );
        }
      }
    }
  }

  return internalContext();
}

DUContext * Declaration::internalContext() const
{
//   ENSURE_CAN_READ
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

bool Declaration::isAutoDeclaration() const
{
  return d_func()->m_isAutoDeclaration;
}

void Declaration::setAutoDeclaration(bool _auto)
{
  d_func_dynamic()->m_isAutoDeclaration = _auto;
}

bool Declaration::isDeprecated() const
{
  return d_func()->m_isDeprecated;
}

void Declaration::setDeprecated(bool deprecated)
{
  d_func_dynamic()->m_isDeprecated = deprecated;
}

bool Declaration::alwaysForceDirect() const
{
  return d_func()->m_alwaysForceDirect;
}

void Declaration::setAlwaysForceDirect(bool direct)
{
  d_func_dynamic()->m_alwaysForceDirect = direct;
}

bool Declaration::isExplicitlyDeleted() const
{
  return d_func()->m_isExplicitlyDeleted;
}

void Declaration::setExplicitlyDeleted(bool deleted)
{
  d_func_dynamic()->m_isExplicitlyDeleted = deleted;
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

IndexedInstantiationInformation Declaration::specialization() const {
  return IndexedInstantiationInformation();
}

void Declaration::activateSpecialization()
{
  if(specialization().index()) {
    DeclarationId baseId(id());
    baseId.setSpecialization(IndexedInstantiationInformation());
    SpecializationStore::self().set(baseId, specialization());
  }
}

DeclarationId Declaration::id(bool forceDirect) const
{
  ENSURE_CAN_READ
  if(inSymbolTable() && !forceDirect && !alwaysForceDirect())
    return DeclarationId(qualifiedIdentifier(), additionalIdentity(), specialization());
  else
    return DeclarationId(IndexedDeclaration(const_cast<Declaration*>(this)), specialization());
}

bool Declaration::inSymbolTable() const
{
  DUCHAIN_D(Declaration);
  return d->m_inSymbolTable;
}

CodeModelItem::Kind kindForDeclaration(Declaration* decl) {
  CodeModelItem::Kind kind = CodeModelItem::Unknown;

  if(decl->kind() == Declaration::Namespace)
    return CodeModelItem::Namespace;

  if(decl->isFunctionDeclaration()) {
    kind = CodeModelItem::Function;
  }

  if(decl->kind() == Declaration::Type && (decl->type<StructureType>() || dynamic_cast<ClassDeclaration*>(decl)))
    kind = CodeModelItem::Class;

  if(kind == CodeModelItem::Unknown && decl->kind() == Declaration::Instance)
    kind = CodeModelItem::Variable;

  if(decl->isForwardDeclaration())
    kind = (CodeModelItem::Kind)(kind | CodeModelItem::ForwardDeclaration);

  if ( decl->context() && decl->context()->type() == DUContext::Class )
    kind = (CodeModelItem::Kind)(kind | CodeModelItem::ClassMember);

  return kind;
}

void Declaration::updateCodeModel()
{
  DUCHAIN_D(Declaration);
  if(!d->m_identifier.isEmpty() && d->m_inSymbolTable) {
    QualifiedIdentifier id(qualifiedIdentifier());
    CodeModel::self().updateItem(url(), id, kindForDeclaration(this));
  }
}

void Declaration::setInSymbolTable(bool inSymbolTable)
{
  DUCHAIN_D_DYNAMIC(Declaration);
  if(!d->m_identifier.isEmpty()) {
    if(!d->m_inSymbolTable && inSymbolTable) {
      QualifiedIdentifier id(qualifiedIdentifier());
      PersistentSymbolTable::self().addDeclaration(id, this);

      CodeModel::self().addItem(url(), id, kindForDeclaration(this));
    }

    else if(d->m_inSymbolTable && !inSymbolTable) {
      QualifiedIdentifier id(qualifiedIdentifier());
      PersistentSymbolTable::self().removeDeclaration(id, this);

      CodeModel::self().removeItem(url(), id);
    }
  }
  d->m_inSymbolTable = inSymbolTable;
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

QMap<IndexedString, QList<RangeInRevision> > Declaration::uses() const
{
  ENSURE_CAN_READ
  QMap<IndexedString, QMap<RangeInRevision, bool> > tempUses;

  //First, search for uses within the own context
  {
    QMap<RangeInRevision, bool>& ranges(tempUses[topContext()->url()]);
    foreach(const RangeInRevision& range, allUses(topContext(), const_cast<Declaration*>(this)))
      ranges[range] = true;
  }

  KDevVarLengthArray<IndexedTopDUContext> useContexts = DUChain::uses()->uses(id());

  for (const IndexedTopDUContext& indexedContext : useContexts) {
    TopDUContext* context = indexedContext.data();
    if(context) {
      QMap<RangeInRevision, bool>& ranges(tempUses[context->url()]);
      foreach(const RangeInRevision& range, allUses(context, const_cast<Declaration*>(this)))
        ranges[range] = true;
    }
  }

  QMap<IndexedString, QList<RangeInRevision> > ret;

  for(QMap<IndexedString, QMap<RangeInRevision, bool> >::const_iterator it = tempUses.constBegin(); it != tempUses.constEnd(); ++it) {
    if(!(*it).isEmpty()) {
      QList<RangeInRevision>& list(ret[it.key()]);
      for(QMap<RangeInRevision, bool>::const_iterator it2 = (*it).constBegin(); it2 != (*it).constEnd(); ++it2)
        list << it2.key();
    }
  }
  return ret;
}

bool hasDeclarationUse(DUContext* context, int declIdx)
{
  bool ret=false;
  int usescount=context->usesCount();
  const Use* uses=context->uses();
  
  for(int i=0; !ret && i<usescount; ++i) {
    ret = uses[i].m_declarationIndex==declIdx;
  }
  
  foreach(DUContext* child, context->childContexts()) {
    ret = ret || hasDeclarationUse(child, declIdx);
    if(ret)
      break;
  }
  
  return ret;
}

bool Declaration::hasUses() const
{
  ENSURE_CAN_READ
  int idx = topContext()->indexForUsedDeclaration(const_cast<Declaration*>(this), false);
  bool ret = idx != std::numeric_limits<int>::max() && (idx>=0 || hasDeclarationUse(topContext(), idx)); //hasLocalUses
  ret =  ret || DUChain::uses()->hasUses(id()); //hasUsesInOtherContexts 
  
  return ret;
}

QMap<IndexedString, QList<KTextEditor::Range> > Declaration::usesCurrentRevision() const
{
  ENSURE_CAN_READ
  QMap<IndexedString, QMap<KTextEditor::Range, bool> > tempUses;

  //First, search for uses within the own context
  {
    QMap<KTextEditor::Range, bool>& ranges(tempUses[topContext()->url()]);
    foreach(const RangeInRevision& range, allUses(topContext(), const_cast<Declaration*>(this)))
    {
      ranges[topContext()->transformFromLocalRevision(range)] = true;
    }
  }

  KDevVarLengthArray<IndexedTopDUContext> useContexts = DUChain::uses()->uses(id());

  for (const IndexedTopDUContext& indexedContext : useContexts) {
    TopDUContext* context = indexedContext.data();
    if(context) {
      QMap<KTextEditor::Range, bool>& ranges(tempUses[context->url()]);
      foreach(const RangeInRevision& range, allUses(context, const_cast<Declaration*>(this)))
        ranges[context->transformFromLocalRevision(range)] = true;
    }
  }

QMap<IndexedString, QList<KTextEditor::Range> > ret;

  for(QMap<IndexedString, QMap<KTextEditor::Range, bool> >::const_iterator it = tempUses.constBegin(); it != tempUses.constEnd(); ++it) {
    if(!(*it).isEmpty()) {
      QList<KTextEditor::Range>& list(ret[it.key()]);
      for(QMap<KTextEditor::Range, bool>::const_iterator it2 = (*it).constBegin(); it2 != (*it).constEnd(); ++it2)
        list << it2.key();
    }
  }
  return ret;
}

}
