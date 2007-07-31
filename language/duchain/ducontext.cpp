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
#include "typesystem.h"
#include "topducontext.h"
#include "symboltable.h"

///It is fine to use one global static mutex here

using namespace KTextEditor;

#define ENSURE_CAN_WRITE {if(inDUChain()) { ENSURE_CHAIN_WRITE_LOCKED }}
#define ENSURE_CAN_READ {if(inDUChain()) { ENSURE_CHAIN_READ_LOCKED }}

#define ENSURE_CAN_WRITE_(x) {if(x->inDUChain()) { ENSURE_CHAIN_WRITE_LOCKED }}
#define ENSURE_CAN_READ_(x) {if(x->inDUChain()) { ENSURE_CHAIN_READ_LOCKED }}

namespace KDevelop
{
QMutex DUContextPrivate::m_localDeclarationsMutex(QMutex::Recursive);

DUContextPrivate::DUContextPrivate( DUContext* d)
  : m_declaration(0), m_context(d), m_anonymousInParent(false)
{
}

void DUContextPrivate::addUse(Use* use)
{
  m_uses.append(use);

  DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::Uses, use);
}

void DUContextPrivate::removeUse(Use* use)
{
  Q_ASSERT(m_uses.contains(use));
  m_uses.removeAll(use);

  DUChain::contextChanged(m_context, DUChainObserver::Removal, DUChainObserver::Uses, use);
}

void DUContextPrivate::addDeclaration( Declaration * newDeclaration )
{
  // The definition may not have its identifier set when it's assigned... allow dupes here, TODO catch the error elsewhere
  {
    QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);

    m_localDeclarations.append(newDeclaration);
    m_localDeclarationsHash.insert( newDeclaration->identifier(), DeclarationPointer(newDeclaration) );
  }

  DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::LocalDeclarations, newDeclaration);
}

bool DUContextPrivate::removeDeclaration(Declaration* declaration)
{
  QMutexLocker lock(&m_localDeclarationsMutex);
  
  m_localDeclarationsHash.remove( declaration->identifier(), DeclarationPointer(declaration) );
  
  if( m_localDeclarations.removeAll(declaration) ) {
    DUChain::contextChanged(m_context, DUChainObserver::Removal, DUChainObserver::LocalDeclarations, declaration);
    return true;
  }else {
    return false;
  }
}

void DUContext::changingIdentifier( Declaration* decl, const Identifier& from, const Identifier& to ) {
  QMutexLocker lock(&d->m_localDeclarationsMutex);
  d->m_localDeclarationsHash.remove( from, DeclarationPointer(decl) );
  d->m_localDeclarationsHash.insert( to, DeclarationPointer(decl) );
}

void DUContextPrivate::addChildContext( DUContext * context )
{
  // Internal, don't need to assert a lock

  for (int i = 0; i < m_childContexts.count(); ++i) {
    DUContext* child = m_childContexts.at(i);
    if (context->textRange().start() < child->textRange().start()) {
      m_childContexts.insert(i, context);
      context->d->m_parentContext = m_context;
      return;
    }
  }
  m_childContexts.append(context);
  context->d->m_parentContext = m_context;

  DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::ChildContexts, context);
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

  DUChain::contextChanged(m_context, DUChainObserver::Addition, DUChainObserver::ImportedChildContexts, context);
}

void DUContextPrivate::removeImportedChildContext( DUContext * context )
{
  Q_ASSERT(m_importedChildContexts.contains(context));

  m_importedChildContexts.removeAll(context);

  DUChain::contextChanged(m_context, DUChainObserver::Removal, DUChainObserver::ImportedChildContexts, context);
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
  if( d->m_declaration && d->m_declaration->internalContext() == this )
    d->m_declaration->setInternalContext(0);
  
  if (inSymbolTable())
    SymbolTable::self()->removeContext(this);

  if (d->m_parentContext)
    d->m_parentContext->d->removeChildContext(this);

  foreach (DUContext* context, importedChildContexts())
    context->removeImportedParentContext(this);

  QList<DUContextPointer> importedParentContexts = d->m_importedParentContexts;
  foreach (DUContextPointer context, importedParentContexts)
    removeImportedParentContext(context.data());

  deleteChildContextsRecursively();

  deleteUses();

  deleteLocalDefinitions();

  deleteLocalDeclarations();

  QList<Use*> useList = uses();
  foreach (Use* use, useList)
    use->setContext(0);

  clearNamespaceAliases();

  DUChain::contextChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
  delete d;
}

const QList< DUContext * > & DUContext::childContexts( ) const
{
  ENSURE_CAN_READ

  return d->m_childContexts;
}

Declaration* DUContext::declaration() const {
  ENSURE_CAN_READ
  return d->m_declaration.data();
}

void DUContext::setDeclaration(Declaration* decl) {
  ENSURE_CAN_WRITE

  if( decl == d->m_declaration.data() )
    return;

  Declaration* oldDeclaration = d->m_declaration.data();
  
  d->m_declaration = decl;
  
  if( oldDeclaration && oldDeclaration->internalContext() == this )
    oldDeclaration->setInternalContext(0);
    

  //The context set as internal context should always be the last opened context
  if( decl )
    decl->setInternalContext(this);
}

DUContext* DUContext::parentContext( ) const
{
  ENSURE_CAN_READ

  return d->m_parentContext.data();
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
        kDebug() << "DUContext::findLocalDeclarationsInternal: Invalid declaration in local-declaration-hash" << endl;
        continue;
      }
        
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

void DUContext::findDeclarationsInternal( const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, QList<NamespaceAlias*>& usingNS, QList<Declaration*>& ret, SearchFlags flags ) const
{
  findLocalDeclarationsInternal(identifier, position, dataType, flags & InImportedParentContext, ret, flags);
  
  if( foundEnough(ret) )
    return;

  if (!identifier.explicitlyGlobal())
    acceptUsingNamespaces(position, usingNS);

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
    
    context->findDeclarationsInternal(identifier,  url() == context->url() ? position : context->textRange().end(), dataType, usingNS, ret, flags | InImportedParentContext);
    if (!ret.isEmpty())
      return;
  }

  if (!(flags & DontSearchInParent) && !(flags & InImportedParentContext) && parentContext())
    parentContext()->findDeclarationsInternal(identifier, url() == parentContext()->url() ? position : parentContext()->textRange().end(), dataType, usingNS, ret, flags);
}

QList<Declaration*> DUContext::findDeclarations( const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, SearchFlags flags) const
{
  ENSURE_CAN_READ

  QList<NamespaceAlias*> usingStatements;
  QList<Declaration*> ret;
  findDeclarationsInternal(identifier, position.isValid() ? position : textRange().end(), dataType, usingStatements, ret, flags);
  return ret;
}

void DUContext::addImportedParentContext( DUContext * context, bool anonymous )
{
  ENSURE_CAN_WRITE
  
  if (d->m_importedParentContexts.contains(context))
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
      d->m_importedParentContexts.insert(i, context);
      return;
    }
  }
  d->m_importedParentContexts.append(context);

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::ImportedParentContexts, context);
}

void DUContext::removeImportedParentContext( DUContext * context )
{
  ENSURE_CAN_WRITE

  d->m_importedParentContexts.removeAll(context);

  if( !context )
    return;
  
  context->d->removeImportedChildContext(this);

  DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::ImportedParentContexts, context);
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

QHash<QualifiedIdentifier, Declaration*> DUContext::allDeclarations(const KTextEditor::Cursor& position) const
{
  ENSURE_CAN_READ

  QHash<QualifiedIdentifier, Declaration*> ret;

  // Iterate back up the chain
  mergeDeclarationsInternal(ret, position);

  return ret;
}

const QList<Declaration*> DUContext::localDeclarations() const
{
  ENSURE_CAN_READ

  QMutexLocker lock(&DUContextPrivate::m_localDeclarationsMutex);
  return d->m_localDeclarations;
}

void DUContext::mergeDeclarationsInternal(QHash<QualifiedIdentifier, Declaration*>& definitions, const KTextEditor::Cursor& position, bool inImportedContext) const
{
  foreach (Declaration* definition, localDeclarations())
    if (!definitions.contains(definition->qualifiedIdentifier()) && (inImportedContext || definition->textRange().start() <= position))
      definitions.insert(definition->qualifiedIdentifier(), definition);

  QListIterator<DUContextPointer> it = d->m_importedParentContexts;
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* context = it.previous().data();
    while( !context && it.hasPrevious() )
      context = it.previous().data();
    if( !context )
      break;

    context->mergeDeclarationsInternal(definitions, position, true);
  }

  if (!inImportedContext && parentContext())
    parentContext()->mergeDeclarationsInternal(definitions, position);
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

  if (includeClasses || type() == Namespace)
    ret = localScopeIdentifier();

  if (parentContext())
    ret = ret.merge(parentContext()->scopeIdentifier());

  return ret;
}

void DUContext::setLocalScopeIdentifier(const QualifiedIdentifier & identifier)
{
  ENSURE_CAN_WRITE

  d->m_scopeIdentifier = identifier;

  DUChain::contextChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

const QualifiedIdentifier & DUContext::localScopeIdentifier() const
{
  ENSURE_CAN_READ

  return d->m_scopeIdentifier;
}

DUContext::NamespaceAlias::NamespaceAlias(KTextEditor::Cursor* cursor)
  : KDevelop::DocumentCursorObject(cursor)
{
}

void DUContext::addNamespaceAlias(KTextEditor::Cursor* cursor, const QualifiedIdentifier& id, const QualifiedIdentifier& aliasName)
{
  ENSURE_CAN_WRITE

  NamespaceAlias* use = new NamespaceAlias(cursor);
  use->nsIdentifier = id;
  use->aliasIdentifier = aliasName;
  use->scope = scopeIdentifier();

  QMutableListIterator<NamespaceAlias*> it = d->m_namespaceAliases;
  while (it.hasPrevious())
    if (use->textCursor() > it.previous()->textCursor()) {
      it.next();
      it.insert(use);
      return;
    }

  d->m_namespaceAliases.prepend(use);

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::UsingNamespaces);
}

const QList<DUContext::NamespaceAlias*>& DUContext::namespaceAliases() const
{
  ENSURE_CAN_READ

  return d->m_namespaceAliases;
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

  DUChain::contextChanged(this, DUChainObserver::Change, DUChainObserver::ContextType);
}

QList<Declaration*> DUContext::findDeclarations(const Identifier& identifier, const KTextEditor::Cursor& position, SearchFlags flags) const
{
  ENSURE_CAN_READ

  QList<NamespaceAlias*> usingStatements;
  QList<Declaration*> ret;
  findDeclarationsInternal(QualifiedIdentifier(identifier), position.isValid() ? position : textRange().end(), AbstractType::Ptr(), usingStatements, ret, flags);
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

  QList<NamespaceAlias*> usingStatements;
  QList<DUContext*> ret;
  findContextsInternal(contextType, identifier, position.isValid() ? position : textRange().end(), usingStatements, ret, flags);
  return ret;
}

void DUContext::findContextsInternal(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, QList<NamespaceAlias*>& usingNS, QList<DUContext*>& ret, SearchFlags flags) const
{
  if (contextType == type())
    if (identifier == scopeIdentifier(true))
      ret.append(const_cast<DUContext*>(this));

  if (!identifier.explicitlyGlobal())
    acceptUsingNamespaces(position, usingNS);

  QListIterator<DUContextPointer> it = d->m_importedParentContexts;
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* context = it.previous().data();
    
    while( !context && it.hasPrevious() ) {
      context = it.previous().data();
    }
    if( !context )
      break;

    context->findContextsInternal(contextType, identifier, position, usingNS, ret, flags | InImportedParentContext);
  }

  if ( !(flags & DontSearchInParent) && !(flags & InImportedParentContext) && parentContext())
    parentContext()->findContextsInternal(contextType, identifier, position, usingNS, ret);
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

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::LocalDefinitions, definition);

  return definition;
}

Definition* DUContext::takeDefinition(Definition* definition)
{
  ENSURE_CAN_WRITE

  d->m_localDefinitions.removeAll(definition);

  DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::LocalDefinitions, definition);

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

void DUContext::acceptUsingNamespaces(const KTextEditor::Cursor & position, QList< NamespaceAlias * > & usingNS) const
{
  foreach (NamespaceAlias* ns, namespaceAliases())
    if (ns->textCursor() <= position) {
      // TODO: inefficient... needs a hash??
      foreach (NamespaceAlias* ns2, usingNS)
        if (ns2->nsIdentifier == ns->nsIdentifier)
          goto duplicate;

      usingNS.append(ns);

      duplicate:
      continue;

    } else {
      break;
    }
}

void DUContext::acceptUsingNamespace(NamespaceAlias* ns, QList<NamespaceAlias*>& usingNS) const
{
  // TODO: inefficient... needs a hash??
  foreach (NamespaceAlias* ns2, usingNS)
    if (ns2->nsIdentifier == ns->nsIdentifier)
      return;

  usingNS.append(ns);
}

void DUContext::clearNamespaceAliases()
{
  ENSURE_CAN_WRITE

  qDeleteAll(d->m_namespaceAliases);
  d->m_namespaceAliases.clear();

  DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::UsingNamespaces);
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

}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
