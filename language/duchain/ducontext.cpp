/* This  is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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
#include "ducontext_p.h"

#include <QMutableLinkedListIterator>
#include <QSet>

#include "declaration.h"
#include "definition.h"
#include "duchain.h"
#include "duchainlock.h"
#include "use.h"
#include "identifier.h"
#include "typesystem.h"
#include "topducontext.h"
#include "symboltable.h"
#include "contextowner.h"
#include "namespacealiasdeclaration.h"

///It is fine to use one global static mutex here

using namespace KTextEditor;

//Stored statically for performance-reasons

#define ENSURE_CAN_WRITE_(x) {if(x->inDUChain()) { ENSURE_CHAIN_WRITE_LOCKED }}
#define ENSURE_CAN_READ_(x) {if(x->inDUChain()) { ENSURE_CHAIN_READ_LOCKED }}

namespace KDevelop
{
QMutex DUContextPrivate::m_localDeclarationsMutex(QMutex::Recursive);

const Identifier globalImportIdentifier("{...import...}");

DUContextPrivate::DUContextPrivate( DUContext* d)
  : m_owner(0), m_context(d), m_anonymousInParent(false), m_propagateDeclarations(false)
{
}

bool DUContextPrivate::isThisImportedBy(const DUContext* context) const {
  if( this == context->d )
    return true;

  foreach( const DUContext* ctx, m_importedChildContexts ) {
    if( ctx->d->isThisImportedBy(context) )
      return true;
  }

  return false;
}

void DUContextPrivate::addUse(Use* use)
{
  m_uses.append(use);

  //DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::Uses, use);
}

void DUContextPrivate::addDeclarationToHash(const Identifier& identifier, Declaration* declaration)
{
  m_localDeclarationsHash.insert( identifier, DeclarationPointer(declaration) );
  
  if( m_propagateDeclarations && m_parentContext )
    m_parentContext->d->addDeclarationToHash(identifier, declaration);
}

void DUContextPrivate::removeDeclarationFromHash(const Identifier& identifier, Declaration* declaration)
{
  m_localDeclarationsHash.remove( identifier, DeclarationPointer(declaration) );
  
  if( m_propagateDeclarations && m_parentContext )
    m_parentContext->d->removeDeclarationFromHash(identifier,  declaration);
}

void DUContextPrivate::removeUse(Use* use)
{
  Q_ASSERT(m_uses.contains(use));
  m_uses.removeAll(use);

  //DUChain::contextChanged(m_context, DUChainObserver::Removal, DUChainObserver::Uses, use);
}

void DUContextPrivate::addDeclaration( Declaration * newDeclaration )
{
  // The definition may not have its identifier set when it's assigned... allow dupes here, TODO catch the error elsewhere
  {
    QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);

//     m_localDeclarations.append(newDeclaration);

  bool inserted = false;
  for (int i = 0; i < m_localDeclarations.count(); ++i) {
    Declaration* child = m_localDeclarations.at(i);
    if (newDeclaration->textRange().start() < child->textRange().start()) {
      m_localDeclarations.insert(i, newDeclaration);
      inserted = true;
      break;
    }
  }
  if( !inserted )
    m_localDeclarations.append(newDeclaration);
      
    addDeclarationToHash(newDeclaration->identifier(), newDeclaration);
  }

  //DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::LocalDeclarations, newDeclaration);
}

bool DUContextPrivate::removeDeclaration(Declaration* declaration)
{
  QMutexLocker lock(&m_localDeclarationsMutex);
  
  removeDeclarationFromHash(declaration->identifier(), declaration);
  
  if( m_localDeclarations.removeAll(declaration) ) {
    //DUChain::contextChanged(m_context, DUChainObserver::Removal, DUChainObserver::LocalDeclarations, declaration);
    return true;
  }else {
    return false;
  }
}

void DUContext::changingIdentifier( Declaration* decl, const Identifier& from, const Identifier& to ) {
  QMutexLocker lock(&d->m_localDeclarationsMutex);
  d->removeDeclarationFromHash(from, decl);
  d->addDeclarationToHash(to, decl);
}

void DUContextPrivate::addChildContext( DUContext * context )
{
  // Internal, don't need to assert a lock

  bool inserted = false;
  for (int i = 0; i < m_childContexts.count(); ++i) {
    DUContext* child = m_childContexts.at(i);
    if (context->textRange().start() < child->textRange().start()) {
      m_childContexts.insert(i, context);
      context->d->m_parentContext = m_context;
      inserted = true;
      break;
    }
  }
  if( !inserted ) {
    m_childContexts.append(context);
    context->d->m_parentContext = m_context;
  }

  //DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::ChildContexts, context);
}

bool DUContextPrivate::removeChildContext( DUContext* context ) {
  
  if( m_childContexts.removeAll(context) )
    return true;
  else
    return false;
}

void DUContextPrivate::addImportedChildContext( DUContext * context )
{
  Q_ASSERT(!m_importedChildContexts.contains(context));

  m_importedChildContexts.append(context);

  //DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::ImportedChildContexts, context);
}

//Can also be called with a context that is not in the list
void DUContextPrivate::removeImportedChildContext( DUContext * context )
{
  m_importedChildContexts.removeAll(context);
  //if( != 0 )
    //DUChain::contextChanged(m_context, DUChainObserver::Removal, DUChainObserver::ImportedChildContexts, context);
}

int DUContext::depth() const
{
  { if (!parentContext()) return 0; return parentContext()->depth() + 1; }
}

DUContext::DUContext(KTextEditor::Range* range, DUContext* parent, bool anonymous)
  : DUChainBase(range)
  , d(new DUContextPrivate(this))
{
  d->m_contextType = Other;
  d->m_parentContext = 0;
  d->m_inSymbolTable = false;
  d->m_anonymousInParent = anonymous;
  if (parent) {
    if( !anonymous )
      parent->d->addChildContext(this);
    else
      d->m_parentContext = parent;
  }
}

DUContext::~DUContext( )
{
  if(d->m_owner)
    d->m_owner->setInternalContext(0);
  
  if (inSymbolTable())
    SymbolTable::self()->removeContext(this);

  if (d->m_parentContext)
    d->m_parentContext->d->removeChildContext(this);

  while( !d->m_importedChildContexts.isEmpty() )
    importedChildContexts().first()->removeImportedParentContext(this);

  while( !d->m_importedParentContexts.isEmpty() )
    if( d->m_importedParentContexts.front() )
      removeImportedParentContext(d->m_importedParentContexts.front().data());
    else
      d->m_importedParentContexts.pop_front();
    
  deleteChildContextsRecursively();

  deleteUses();

  deleteLocalDefinitions();

  deleteLocalDeclarations();

  QList<Use*> useList = uses();
  foreach (Use* use, useList)
    use->setContext(0);

  //DUChain::contextChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
  delete d;
}

const QList< DUContext * > & DUContext::childContexts( ) const
{
  ENSURE_CAN_READ

  return d->m_childContexts;
}

ContextOwner* DUContext::owner() const {
  ENSURE_CAN_READ
  return d->m_owner;
}

void DUContext::setOwner(ContextOwner* owner) {
  ENSURE_CAN_WRITE

  if( owner == d->m_owner )
    return;

  ContextOwner* oldOwner = d->m_owner;
  
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
  ENSURE_CAN_READ

  return d->m_parentContext.data();
}

void DUContext::setPropagateDeclarations(bool propagate)
{
  ENSURE_CAN_WRITE
  QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);
  
  bool oldPropagate = d->m_propagateDeclarations;

  if( oldPropagate && !propagate && d->m_parentContext )
    foreach(const DeclarationPointer& decl, d->m_localDeclarationsHash)
      if(decl)
        d->m_parentContext->d->removeDeclarationFromHash(decl->identifier(), decl.data());
  
  d->m_propagateDeclarations = propagate;

  if( !oldPropagate && propagate && d->m_parentContext )
    foreach(const DeclarationPointer& decl, d->m_localDeclarationsHash)
      if(decl)
        d->m_parentContext->d->addDeclarationToHash(decl->identifier(), decl.data());
}

bool DUContext::isPropagateDeclarations() const
{
  return d->m_propagateDeclarations;
}

QList<Declaration*> DUContext::findLocalDeclarations( const QualifiedIdentifier& identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch, SearchFlags flags ) const
{
  ENSURE_CAN_READ

  QList<Declaration*> ret;
  findLocalDeclarationsInternal(identifier, position.isValid() ? position : textRange().end(), dataType, allowUnqualifiedMatch, ret, flags);
  return ret;
}

void DUContext::findLocalDeclarationsInternal( const QualifiedIdentifier& identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch, QList<Declaration*>& ret, SearchFlags /*flags*/ ) const
{
  if( identifier.explicitlyGlobal() && parentContext() )
    return;
  
  ///@todo use flags
  QLinkedList<Declaration*> tryToResolve;
  QLinkedList<Declaration*> ensureResolution;
  QList<Declaration*> resolved;

  {
     QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);
    Identifier lastIdentifier = identifier.last();

    QHash<Identifier, DeclarationPointer>::const_iterator it = d->m_localDeclarationsHash.find(lastIdentifier);
    QHash<Identifier, DeclarationPointer>::const_iterator end = d->m_localDeclarationsHash.end();

    for( ; it != end && it.key() == lastIdentifier; ++it ) {
      Declaration* declaration = (*it).data();

      if(!declaration) {
        //This should never happen, but let's see
        kDebug(9505) << "DUContext::findLocalDeclarationsInternal: Invalid declaration in local-declaration-hash";
        continue;
      }
      if( dynamic_cast<NamespaceAliasDeclaration*>(declaration) )
        continue; //Do not include NamespaceAliasDeclarations here, they are handled by DUContext directly.
        
      QualifiedIdentifier::MatchTypes m = identifier.match(declaration->identifier());
      switch (m) {
        case QualifiedIdentifier::NoMatch:
          continue;

        case QualifiedIdentifier::EndsWith:
          // identifier is a more complete specification...
          // Try again with a qualified definition identifier
          ensureResolution.append(declaration);
          continue;

      case QualifiedIdentifier::TargetEndsWith : ///NOTE: This cannot happen, because declaration() identifier is of type Identifier
          // definition is a more complete specification...
          if (!allowUnqualifiedMatch)
            tryToResolve.append(declaration);
          else
            resolved.append(declaration);
          continue;

        case QualifiedIdentifier::ExactMatch:
          if (!allowUnqualifiedMatch)
            ensureResolution.append(declaration);
          else
            resolved.append(declaration);
          continue;
      }
    }
  }

  foreach (Declaration* declaration, resolved)
    if (!dataType || dataType == declaration->abstractType())
      if (type() == Class || type() == Template || position >= declaration->textRange().start() || !position.isValid())
        ret.append(declaration);

  if (!ret.isEmpty())
    // Match(es)
    return;

  if (tryToResolve.isEmpty() && ensureResolution.isEmpty())
    return;

  QMutableLinkedListIterator<Declaration*> it = ensureResolution;
  while (it.hasNext()) {
    QualifiedIdentifier::MatchTypes m = identifier.match(it.next()->qualifiedIdentifier());
    switch (m) {
      case QualifiedIdentifier::NoMatch:
      case QualifiedIdentifier::EndsWith:
        break;

      case QualifiedIdentifier::TargetEndsWith:
      case QualifiedIdentifier::ExactMatch:
        resolved.append(it.value());
        break;
    }
  }

  foreach (Declaration* declaration, resolved)
    if (!dataType || dataType == declaration->abstractType())
      if (type() == Class || position >= declaration->textRange().start())
        ret.append(declaration);

  if (!ret.isEmpty())
    // Match(es)
    return;

  it = tryToResolve;
  while (it.hasNext()) {
    QualifiedIdentifier::MatchTypes m = identifier.match(it.next()->qualifiedIdentifier());
    switch (m) {
      case QualifiedIdentifier::NoMatch:
      case QualifiedIdentifier::EndsWith:
        break;

      case QualifiedIdentifier::TargetEndsWith:
      case QualifiedIdentifier::ExactMatch:
        resolved.append(it.value());
        break;
    }
  }

  foreach (Declaration* declaration, resolved)
    if (!dataType || dataType == declaration->abstractType())
      if (type() == Class || position >= declaration->textRange().start())
        ret.append(declaration);

  //if (!ret.isEmpty())
    // Match(es)... don't need to check, returning anyway


  // TODO: namespace abbreviations

  return;
}

bool DUContext::foundEnough( const QList<Declaration*>& ret ) const {
  if( !ret.isEmpty() )
    return true;
  else
    return false;
}

void DUContext::findDeclarationsInternal( const QList<QualifiedIdentifier> & baseIdentifiers, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, QList<Declaration*>& ret, SearchFlags flags ) const
{
  foreach( const QualifiedIdentifier& identifier, baseIdentifiers )
      findLocalDeclarationsInternal(identifier, position, dataType, flags & InImportedParentContext, ret, flags);
  
  if( foundEnough(ret) )
    return;

  ///Step 1: Apply namespace-aliases and -imports
  QList<QualifiedIdentifier> aliasedIdentifiers;
  //Because of namespace-imports and aliases, this identifier may need to be searched under multiple names
  if( type() == Namespace )
    applyAliases(baseIdentifiers, aliasedIdentifiers, position, false);
  else
    aliasedIdentifiers = baseIdentifiers;


  if( !d->m_importedParentContexts.isEmpty() ) {
    ///Step 2: Give identifiers that are not marked as explicitly-global to imported contexts(explicitly global ones are treatead in TopDUContext)
    QList<QualifiedIdentifier> nonGlobalIdentifiers;
    foreach( const QualifiedIdentifier& identifier, aliasedIdentifiers )
      if( !identifier.explicitlyGlobal() )
        nonGlobalIdentifiers << identifier;

    if( !nonGlobalIdentifiers.isEmpty() ) {
      QList<DUContextPointer>::iterator it = d->m_importedParentContexts.end();
      QList<DUContextPointer>::iterator begin = d->m_importedParentContexts.begin();
      while( it != begin ) {
        --it;
        DUContext* context = (*it).data();

        while( !context && it != begin ) {
          --it;
          context = (*it).data();
        }

        if( !context )
          break;

        QMap<DUContextPointer, KTextEditor::Cursor>::const_iterator it2 = d->m_importedParentContextPositions.find(*it);
        if( it2 != d->m_importedParentContextPositions.end() && (*it2).isValid() ) {
          if( position < *it2 )
            continue; ///Respect the import-positions
        }
        context->findDeclarationsInternal(nonGlobalIdentifiers,  url() == context->url() ? position : context->textRange().end(), dataType, ret, flags | InImportedParentContext);
      }
    }
  }
  
  if( foundEnough(ret) )
    return;

  ///Step 3: Continue search in parent-context
  if (!(flags & DontSearchInParent) && !(flags & InImportedParentContext) && parentContext()) {
    parentContext()->findDeclarationsInternal(aliasedIdentifiers, url() == parentContext()->url() ? position : parentContext()->textRange().end(), dataType, ret, flags);
  }
}

QList<Declaration*> DUContext::findDeclarations( const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, SearchFlags flags) const
{
  ENSURE_CAN_READ

  QList<Declaration*> ret;
  QList<QualifiedIdentifier> identifiers;
  identifiers << identifier;
  findDeclarationsInternal(identifiers, position.isValid() ? position : textRange().end(), dataType, ret, flags);
  return ret;
}

bool DUContext::imports(const DUContext* origin, const KTextEditor::Cursor& position ) const
{
  ENSURE_CAN_READ
  
  return origin->d->isThisImportedBy(this);
}


void DUContext::addImportedParentContext( DUContext * context, const KTextEditor::Cursor& position, bool anonymous )
{
  ENSURE_CAN_WRITE

  if( context->imports(this) ) {
    kDebug(9505) << "DUContext::addImportedParentContext: Tried to create circular import-structure by importing " << context << " (" << context->url() << ") into " << this << " (" << url() << ")";
    return;
  }

  if( position.isValid() )
    d->m_importedParentContextPositions[DUContextPointer(context)] = position;

    
  if (d->m_importedParentContexts.contains(DUContextPointer(context)))
    return;

  if( !anonymous ) {
    ENSURE_CAN_WRITE_(context)
    context->d->addImportedChildContext(this);
  }

  for (int i = 0; i < d->m_importedParentContexts.count(); ++i) {
    DUContext* parent = d->m_importedParentContexts.at(i).data();
    if( !parent )
      continue;
    if (context->textRange().start() < parent->textRange().start()) {
      d->m_importedParentContexts.insert(i, DUContextPointer(context));
      return;
    }
  }
  d->m_importedParentContexts.append(DUContextPointer(context));

  //DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::ImportedParentContexts, context);
}

void DUContext::removeImportedParentContext( DUContext * context )
{
  ENSURE_CAN_WRITE

  d->m_importedParentContextPositions.remove(DUContextPointer(context));
  d->m_importedParentContexts.removeAll(DUContextPointer(context));

  if( !context )
    return;
  
  context->d->removeImportedChildContext(this);

  //DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::ImportedParentContexts, context);
}

const QList<DUContext*>& DUContext::importedChildContexts() const
{
  ENSURE_CAN_READ

  return d->m_importedChildContexts;
}

DUContext * DUContext::findContext( const KTextEditor::Cursor& position, DUContext* parent) const
{
  ENSURE_CAN_READ

  if (!parent)
    parent = const_cast<DUContext*>(this);

  foreach (DUContext* context, parent->childContexts())
    if (context->textRange().contains(position)) {
      DUContext* ret = findContext(position, context);
      if (!ret)
        ret = context;

      return ret;
    }

  return 0;
}

bool DUContext::parentContextOf(DUContext* context) const
{
  if (this == context)
    return true;

  foreach (DUContext* child, childContexts())
    if (child->parentContextOf(context))
      return true;

  return false;
}

QList<Declaration*> DUContext::allLocalDeclarations(const Identifier& identifier) const
{
  ENSURE_CAN_READ
  QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);

  QList<Declaration*> ret;
  
  QHash<Identifier, DeclarationPointer>::const_iterator it = d->m_localDeclarationsHash.find(identifier);
  QHash<Identifier, DeclarationPointer>::const_iterator end = d->m_localDeclarationsHash.end();

  for( ; it != end && it.key() == identifier; ++it )
    ret << (*it).data();

  return ret;
}

QList< QPair<Declaration*, int> > DUContext::allDeclarations(const KTextEditor::Cursor& position, bool searchInParents) const
{
  ENSURE_CAN_READ

  QList< QPair<Declaration*, int> > ret;

  QHash<const DUContext*, bool> hadContexts;
  // Iterate back up the chain
  mergeDeclarationsInternal(ret, type() == DUContext::Class ? KTextEditor::Cursor::invalid() : position, hadContexts, searchInParents);

  return ret;
}

const QList<Declaration*> DUContext::localDeclarations() const
{
  ENSURE_CAN_READ

  QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);
  return d->m_localDeclarations;
}

void DUContext::mergeDeclarationsInternal(QList< QPair<Declaration*, int> >& definitions, const KTextEditor::Cursor& position, QHash<const DUContext*, bool>& hadContexts, bool searchInParents, int currentDepth) const
{
  if(hadContexts.contains(this))
    return;
  hadContexts[this] = true;
  
  if( type() == DUContext::Namespace || type() == DUContext::Global && currentDepth < 1000 )
    currentDepth += 1000;

  {
    QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);
      foreach (DeclarationPointer decl, d->m_localDeclarationsHash)
        if ( decl && (!position.isValid() || decl->textRange().start() <= position) )
          definitions << qMakePair(decl.data(), currentDepth);
  }

  QListIterator<DUContextPointer> it = d->m_importedParentContexts;
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* context = it.previous().data();
    while( !context && it.hasPrevious() )
      context = it.previous().data();
    if( !context )
      break;

    context->mergeDeclarationsInternal(definitions, KTextEditor::Cursor::invalid(), hadContexts, false, currentDepth+1);
  }

  if (searchInParents && parentContext())                            ///Only respect the position if the parent-context is not a class(@todo this is language-dependent)
    parentContext()->mergeDeclarationsInternal(definitions, (parentContext()->type() != DUContext::Class) ? position : KTextEditor::Cursor::invalid(), hadContexts, true, currentDepth+1);
}

void DUContext::deleteLocalDeclarations()
{
  ENSURE_CAN_WRITE

  QList<Declaration*> declarations;
  {
    QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);
    declarations = d->m_localDeclarations;
  }
  
  qDeleteAll(declarations);
  Q_ASSERT(d->m_localDeclarations.isEmpty());
}

void DUContext::deleteChildContextsRecursively()
{
  ENSURE_CAN_WRITE

  qDeleteAll(d->m_childContexts);

  Q_ASSERT(d->m_childContexts.isEmpty());
}

QList< Declaration * > DUContext::clearLocalDeclarations( )
{
  QList< Declaration * > ret = localDeclarations();
  foreach (Declaration* dec, ret)
    dec->setContext(0);
  return ret;
}

QualifiedIdentifier DUContext::scopeIdentifier(bool includeClasses) const
{
  ENSURE_CAN_READ

  QualifiedIdentifier ret;
  if (parentContext())
    ret = parentContext()->scopeIdentifier();

  if (includeClasses || type() == Namespace)
    ret += localScopeIdentifier();


  return ret;
}

void DUContext::setLocalScopeIdentifier(const QualifiedIdentifier & identifier)
{
  ENSURE_CAN_WRITE

  d->m_scopeIdentifier = identifier;

  //DUChain::contextChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

const QualifiedIdentifier & DUContext::localScopeIdentifier() const
{
  ENSURE_CAN_READ

  return d->m_scopeIdentifier;
}

DUContext::ContextType DUContext::type() const
{
  ENSURE_CAN_READ

  return d->m_contextType;
}

void DUContext::setType(ContextType type)
{
  ENSURE_CAN_WRITE

  d->m_contextType = type;

  //DUChain::contextChanged(this, DUChainObserver::Change, DUChainObserver::ContextType);
}

QList<Declaration*> DUContext::findDeclarations(const Identifier& identifier, const KTextEditor::Cursor& position, SearchFlags flags) const
{
  ENSURE_CAN_READ

  QList<Declaration*> ret;
  QList<QualifiedIdentifier> identifiers;
  identifiers << QualifiedIdentifier(identifier);
  findDeclarationsInternal(identifiers, position.isValid() ? position : textRange().end(), AbstractType::Ptr(), ret, flags);
  return ret;
}

void DUContext::addOrphanUse(Use* orphan)
{
  ENSURE_CAN_WRITE

  d->m_orphanUses.append(orphan);
}

void DUContext::deleteUses()
{
  ENSURE_CAN_WRITE

  qDeleteAll(d->m_uses);
  Q_ASSERT(d->m_uses.isEmpty());
}

const QList<Use*>& DUContext::orphanUses() const
{
  ENSURE_CAN_READ

  return d->m_orphanUses;
}

bool DUContext::inDUChain() const {
  if( d->m_anonymousInParent )
    return false;

  TopDUContext* top = topContext();
  return top && top->inDuChain();
}

const QList<DUContextPointer>& DUContext::importedParentContexts() const
{
  ENSURE_CAN_READ

  return d->m_importedParentContexts;
}

QList<DUContext*> DUContext::findContexts(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, SearchFlags flags) const
{
  ENSURE_CAN_READ

  QList<DUContext*> ret;
  QList<QualifiedIdentifier> identifiers;
  identifiers << QualifiedIdentifier(identifier);
  
  findContextsInternal(contextType, identifiers, position.isValid() ? position : textRange().end(), ret, flags);
  return ret;
}

void DUContext::applyAliases(const QList<QualifiedIdentifier>& baseIdentifiers, QList<QualifiedIdentifier>& identifiers, const KTextEditor::Cursor& position, bool canBeNamespace) const {

  foreach( const QualifiedIdentifier& identifier, baseIdentifiers ) {
    bool addUnmodified = true;

    if( !identifier.explicitlyGlobal() ) {
      QList<Declaration*> imports = allLocalDeclarations(globalImportIdentifier);

      if( !imports.isEmpty() )
      {
        //We have namespace-imports.
        foreach( Declaration* importDecl, imports )
        {
          if( importDecl->textRange().end() > position )
            continue;
          //Search for the identifier with the import-identifier prepended
          Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(importDecl));
          NamespaceAliasDeclaration* alias = static_cast<NamespaceAliasDeclaration*>(importDecl);
          QualifiedIdentifier identifierInImport = alias->importIdentifier();
          identifierInImport.push(identifier);
          identifiers << identifierInImport;
        }
      }

      if( !identifier.isEmpty() && (identifier.count() > 1 || canBeNamespace) ) {
        QList<Declaration*> aliases = allLocalDeclarations(identifier.first());
        if(!aliases.isEmpty()) {
          //The first part of the identifier has been found as a namespace-alias.
          //In c++, we only need the first alias. However, just to be correct, follow them all for now.
          foreach( Declaration* aliasDecl, aliases )
          {
            if( aliasDecl->textRange().end() > position )
              continue;
            if(!dynamic_cast<NamespaceAliasDeclaration*>(aliasDecl))
              continue;

            addUnmodified = false; //The un-modified identifier can be ignored, because it will be replaced with the resolved alias
            NamespaceAliasDeclaration* alias = static_cast<NamespaceAliasDeclaration*>(aliasDecl);

            //Create an identifier where namespace-alias part is replaced with the alias target
            QualifiedIdentifier aliasedIdentifier = alias->importIdentifier();
            for( int a = 1; a <  identifier.count(); a++ )
              aliasedIdentifier.push(identifier.at(a));

            identifiers << aliasedIdentifier;
          }
        }
      }
    }

    if( addUnmodified )
        identifiers << identifier;
  }
}

void DUContext::findContextsInternal(ContextType contextType, const QList<QualifiedIdentifier>& baseIdentifiers, const KTextEditor::Cursor& position, QList<DUContext*>& ret, SearchFlags flags) const
{
  if (contextType == type()) {
    foreach( const QualifiedIdentifier& identifier, baseIdentifiers )
      if (identifier == scopeIdentifier(true) && (!parentContext() || !identifier.explicitlyGlobal()) )
        ret.append(const_cast<DUContext*>(this));
  }

  ///Step 1: Apply namespace-aliases and -imports
  QList<QualifiedIdentifier> aliasedIdentifiers;
  //Because of namespace-imports and aliases, this identifier may need to be searched as under multiple names
  applyAliases(baseIdentifiers, aliasedIdentifiers, position, contextType == Namespace);

  if( !d->m_importedParentContexts.isEmpty() ) {
    ///Step 2: Give identifiers that are not marked as explicitly-global to imported contexts(explicitly global ones are treatead in TopDUContext)
    QList<QualifiedIdentifier> nonGlobalIdentifiers;
    foreach( const QualifiedIdentifier& identifier, aliasedIdentifiers )
      if( !identifier.explicitlyGlobal() )
        nonGlobalIdentifiers << identifier;
    
    if( !nonGlobalIdentifiers.isEmpty() ) {
      QListIterator<DUContextPointer> it = d->m_importedParentContexts;
      it.toBack();
      while (it.hasPrevious()) {
        DUContext* context = it.previous().data();

        while( !context && it.hasPrevious() ) {
          context = it.previous().data();
        }
        if( !context )
          break;

        context->findContextsInternal(contextType, nonGlobalIdentifiers, url() == context->url() ? position : context->textRange().end(), ret, flags | InImportedParentContext);
      }
    }
  }

  ///Step 3: Continue search in parent
  if ( !(flags & DontSearchInParent) && !(flags & InImportedParentContext) && parentContext())
    parentContext()->findContextsInternal(contextType, aliasedIdentifiers, url() == parentContext()->url() ? position : parentContext()->textRange().end(), ret, flags);
}

const QList<Definition*>& DUContext::localDefinitions() const
{
  ENSURE_CAN_READ

  return d->m_localDefinitions;
}

Definition* DUContext::addDefinition(Definition* definition)
{
  ENSURE_CAN_WRITE

  d->m_localDefinitions.append(definition);

  //DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::LocalDefinitions, definition);

  return definition;
}

Definition* DUContext::takeDefinition(Definition* definition)
{
  ENSURE_CAN_WRITE

  d->m_localDefinitions.removeAll(definition);

  //DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::LocalDefinitions, definition);

  return definition;
}

void DUContext::deleteLocalDefinitions()
{
  // No need to assert a lock

  QList<Definition*> definitions = localDefinitions();
  qDeleteAll(definitions);

  Q_ASSERT(localDefinitions().isEmpty());
}

const QList< Use * > & DUContext::uses() const
{
  ENSURE_CAN_READ

  return d->m_uses;
}

DUContext * DUContext::findContextAt(const KTextEditor::Cursor & position) const
{
  ENSURE_CAN_READ

  if (!textRange().contains(position))
    return 0;

  foreach (DUContext* child, d->m_childContexts)
    if (DUContext* specific = child->findContextAt(position))
      return specific;

  return const_cast<DUContext*>(this);
}

DUContext* DUContext::findContextIncluding(const KTextEditor::Range& range) const
{
  ENSURE_CAN_READ

  if (!textRange().contains(range))
    return 0;

  foreach (DUContext* child, d->m_childContexts)
    if (DUContext* specific = child->findContextIncluding(range))
      return specific;

  return const_cast<DUContext*>(this);
}

Use* DUContext::findUseAt(const KTextEditor::Cursor & position) const
{
  ENSURE_CAN_READ

  if (!textRange().contains(position))
    return 0;

  foreach (Use* use, d->m_uses)
    if (use->textRange().contains(position))
      return use;

  return 0;
}

bool DUContext::inSymbolTable() const
{
  // Only one symbol table, no need for a lock

  return d->m_inSymbolTable;
}

void DUContext::setInSymbolTable(bool inSymbolTable)
{
  // Only one symbol table, no need for a lock

  d->m_inSymbolTable = inSymbolTable;
}

// kate: indent-width 2;

void DUContext::clearImportedParentContexts()
{
  ENSURE_CAN_WRITE

  foreach (DUContextPointer parent, d->m_importedParentContexts)
      if( parent.data() )
        removeImportedParentContext(parent.data());

  Q_ASSERT(d->m_importedParentContexts.isEmpty());
}

void DUContext::cleanIfNotEncountered(const QSet<DUChainBase*>& encountered, bool firstPass)
{
  ENSURE_CAN_WRITE

  if (firstPass) {
    foreach (DUContext* childContext, childContexts())
      if (!encountered.contains(childContext))
        delete childContext;

    foreach (Declaration* dec, localDeclarations())
      if (!encountered.contains(dec))
        delete dec;

    foreach (Definition* def, localDefinitions())
      if ( !encountered.contains(def))
        delete def;

  } else {
    foreach (Use* use, uses())
      if (!encountered.contains(use))
        delete use;
  }
}

TopDUContext* DUContext::topContext() const
{
  if (d->m_parentContext.data())
    return d->m_parentContext.data()->topContext();

  return 0;
}

QWidget* DUContext::createNavigationWidget(Declaration* decl, const QString& htmlPrefix, const QString& htmlSuffix) const
{
  return 0;
}


}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
