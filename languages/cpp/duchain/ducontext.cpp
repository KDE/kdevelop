/* This  is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include <QMutableLinkedListIterator>
#include <QReadLocker>

#include "declaration.h"
#include "definition.h"
#include "duchain.h"
#include "use.h"
#include "typesystem.h"
#include "topducontext.h"
#include "symboltable.h"

#include "dumpchain.h"

using namespace KTextEditor;

DUContext::DUContext(KTextEditor::Range* range, DUContext* parent)
  : DUChainBase(parent->topContext())
  , KDevDocumentRangeObject(range)
  , m_contextType(Other)
  , m_parentContext(0)
  , m_inSymbolTable(false)
{
  if (parent)
    parent->addChildContext(this);
}

DUContext::DUContext(KTextEditor::Range* range, TopDUContext* top)
  : DUChainBase(top)
  , KDevDocumentRangeObject(range)
  , m_contextType(Other)
  , m_parentContext(0)
  , m_inSymbolTable(false)
{
  if (top && top != this)
    top->addChildContext(this);
}

DUContext::~DUContext( )
{
  if (inSymbolTable())
    SymbolTable::self()->removeContext(this);

  if (m_parentContext)
    m_parentContext->m_childContexts.removeAll(this);

  foreach (DUContext* context, importedChildContexts())
    context->removeImportedParentContext(this);

  QList<DUContext*> importedParentContexts = m_importedParentContexts;
  foreach (DUContext* context, importedParentContexts)
    removeImportedParentContext(context);

  deleteChildContextsRecursively();

  deleteUses();

  deleteLocalDefinitions();

  deleteLocalDeclarations();

  QList<Use*> useList = uses();
  foreach (Use* use, useList)
    use->setContext(0);

  clearUsingNamespaces();

  DUChain::contextChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
}

const QList< DUContext * > & DUContext::childContexts( ) const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_childContexts;
}

DUContext* DUContext::parentContext( ) const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_parentContext;
}

void DUContext::addDeclaration( Declaration * newDeclaration )
{
  ENSURE_CHAIN_WRITE_LOCKED

  // The definition may not have its identifier set when it's assigned... allow dupes here, TODO catch the error elsewhere

  m_localDeclarations.append(newDeclaration);

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::LocalDeclarations, newDeclaration);
}

void DUContext::removeDeclaration(Declaration* declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_localDeclarations.removeAll(declaration);

  DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::LocalDeclarations, declaration);
}

QList<Declaration*> DUContext::findLocalDeclarations( const QualifiedIdentifier& identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch ) const
{
  ENSURE_CHAIN_READ_LOCKED

  QList<Declaration*> ret;
  findLocalDeclarationsInternal(identifier, position.isValid() ? position : textRange().end(), dataType, allowUnqualifiedMatch, ret);
  return ret;
}

void DUContext::findLocalDeclarationsInternal( const QualifiedIdentifier& identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch, QList<Declaration*>& ret ) const
{
  QLinkedList<Declaration*> tryToResolve;
  QLinkedList<Declaration*> ensureResolution;
  QList<Declaration*> resolved;

  QList<Declaration*>::ConstIterator end = m_localDeclarations.constEnd();
  for (QList<Declaration*>::ConstIterator it = m_localDeclarations.constBegin(); it != end; ++it) {
    Declaration* declaration = *it;
    QualifiedIdentifier::MatchTypes m = identifier.match(declaration->identifier());
    switch (m) {
      case QualifiedIdentifier::NoMatch:
        continue;

      case QualifiedIdentifier::Contains:
        // identifier is a more complete specification...
        // Try again with a qualified definition identifier
        ensureResolution.append(declaration);
        continue;

      case QualifiedIdentifier::ContainedBy:
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

  foreach (Declaration* declaration, resolved)
    if (!dataType || dataType == declaration->abstractType())
      if (type() == Class || position >= declaration->textRange().start())
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
      case QualifiedIdentifier::Contains:
        break;

      case QualifiedIdentifier::ContainedBy:
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
      case QualifiedIdentifier::Contains:
        break;

      case QualifiedIdentifier::ContainedBy:
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

void DUContext::findDeclarationsInternal( const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, QList<UsingNS*>& usingNS, QList<Declaration*>& ret, bool inImportedContext ) const
{
  // TODO we're missing ambiguous references by not checking every resolution before returning...
  // but is that such a bad thing? (might be good performance-wise)
  findLocalDeclarationsInternal(identifier, position, dataType, inImportedContext, ret);
  if (ret.count())
    return;

  if (!identifier.explicitlyGlobal())
    acceptUsingNamespaces(position, usingNS);

  QListIterator<DUContext*> it = importedParentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* context = it.previous();
    context->findDeclarationsInternal(identifier, position, dataType, usingNS, ret, true);
    if (!ret.isEmpty())
      return;
  }

  if (!inImportedContext && parentContext())
    parentContext()->findDeclarationsInternal(identifier, position, dataType, usingNS, ret);
}

QList<Declaration*> DUContext::findDeclarations( const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType) const
{
  ENSURE_CHAIN_READ_LOCKED

  QList<UsingNS*> usingStatements;
  QList<Declaration*> ret;
  findDeclarationsInternal(identifier, position.isValid() ? position : textRange().end(), dataType, usingStatements, ret);
  return ret;
}

void DUContext::addChildContext( DUContext * context )
{
  // Internal, don't need to assert a lock

  for (int i = 0; i < m_childContexts.count(); ++i) {
    DUContext* child = m_childContexts.at(i);
    if (context->textRange().start() < child->textRange().start()) {
      m_childContexts.insert(i, context);
      context->m_parentContext = this;
      return;
    }
  }
  m_childContexts.append(context);
  context->m_parentContext = this;

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::ChildContexts, context);
}

void DUContext::addImportedParentContext( DUContext * context )
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (m_importedParentContexts.contains(context))
    return;

  context->addImportedChildContext(this);

  for (int i = 0; i < m_importedParentContexts.count(); ++i) {
    DUContext* parent = m_importedParentContexts.at(i);
    if (context->textRange().start() < parent->textRange().start()) {
      m_importedParentContexts.insert(i, context);
      return;
    }
  }
  m_importedParentContexts.append(context);

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::ImportedParentContexts, context);
}

void DUContext::removeImportedParentContext( DUContext * context )
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_importedParentContexts.removeAll(context);

  context->removeImportedChildContext(this);

  DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::ImportedParentContexts, context);
}

void DUContext::addImportedChildContext( DUContext * context )
{
  ENSURE_CHAIN_WRITE_LOCKED

  Q_ASSERT(!m_importedChildContexts.contains(context));

  m_importedChildContexts.append(context);

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::ImportedChildContexts, context);
}

void DUContext::removeImportedChildContext( DUContext * context )
{
  ENSURE_CHAIN_WRITE_LOCKED

  Q_ASSERT(m_importedChildContexts.contains(context));

  m_importedChildContexts.removeAll(context);

  DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::ImportedChildContexts, context);
}

const QList<DUContext*>& DUContext::importedChildContexts() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_importedChildContexts;
}

DUContext * DUContext::findContext( const KTextEditor::Cursor& position, DUContext* parent) const
{
  ENSURE_CHAIN_READ_LOCKED

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
  ENSURE_CHAIN_READ_LOCKED

  QHash<QualifiedIdentifier, Declaration*> ret;

  // Iterate back up the chain
  mergeDeclarationsInternal(ret, position);

  return ret;
}

const QList<Declaration*>& DUContext::localDeclarations() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_localDeclarations;
}

void DUContext::mergeDeclarationsInternal(QHash<QualifiedIdentifier, Declaration*>& definitions, const KTextEditor::Cursor& position, bool inImportedContext) const
{
  foreach (Declaration* definition, localDeclarations())
    if (!definitions.contains(definition->qualifiedIdentifier()) && (inImportedContext || definition->textRange().start() <= position))
      definitions.insert(definition->qualifiedIdentifier(), definition);

  QListIterator<DUContext*> it = importedParentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* context = it.previous();

    context->mergeDeclarationsInternal(definitions, position, true);
  }

  if (!inImportedContext && parentContext())
    parentContext()->mergeDeclarationsInternal(definitions, position);
}

void DUContext::deleteLocalDeclarations()
{
  ENSURE_CHAIN_WRITE_LOCKED

  qDeleteAll(m_localDeclarations);
  Q_ASSERT(m_localDeclarations.isEmpty());
}

void DUContext::deleteChildContextsRecursively()
{
  ENSURE_CHAIN_WRITE_LOCKED

  qDeleteAll(m_childContexts);

  Q_ASSERT(m_childContexts.isEmpty());
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
  ENSURE_CHAIN_READ_LOCKED

  QualifiedIdentifier ret;

  if (includeClasses || type() == Namespace)
    ret = localScopeIdentifier();

  if (parentContext())
    ret = ret.merge(parentContext()->scopeIdentifier());

  return ret;
}

void DUContext::setLocalScopeIdentifier(const QualifiedIdentifier & identifier)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_scopeIdentifier = identifier;

  DUChain::contextChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

const QualifiedIdentifier & DUContext::localScopeIdentifier() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_scopeIdentifier;
}

DUContext::UsingNS::UsingNS(KTextEditor::Cursor* cursor)
  : KDevDocumentCursorObject(cursor)
{
}

void DUContext::addUsingNamespace(KTextEditor::Cursor* cursor, const QualifiedIdentifier& id)
{
  ENSURE_CHAIN_WRITE_LOCKED

  UsingNS* use = new UsingNS(cursor);
  use->nsIdentifier = id;

  QMutableListIterator<UsingNS*> it = m_usingNamespaces;
  while (it.hasPrevious())
    if (use->textCursor() > it.previous()->textCursor()) {
      it.next();
      it.insert(use);
      return;
    }

  m_usingNamespaces.prepend(use);

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::UsingNamespaces);
}

const QList<DUContext::UsingNS*>& DUContext::usingNamespaces() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_usingNamespaces;
}

DUContext::ContextType DUContext::type() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_contextType;
}

void DUContext::setType(ContextType type)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_contextType = type;

  DUChain::contextChanged(this, DUChainObserver::Change, DUChainObserver::ContextType);
}

QList<Declaration*> DUContext::findDeclarations(const Identifier& identifier, const KTextEditor::Cursor& position) const
{
  ENSURE_CHAIN_READ_LOCKED

  QList<UsingNS*> usingStatements;
  QList<Declaration*> ret;
  findDeclarationsInternal(QualifiedIdentifier(identifier), position.isValid() ? position : textRange().end(), AbstractType::Ptr(), usingStatements, ret);
  return ret;
}

void DUContext::addOrphanUse(Use* orphan)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_orphanUses.append(orphan);
}

void DUContext::deleteUses()
{
  ENSURE_CHAIN_WRITE_LOCKED

  qDeleteAll(m_uses);
  Q_ASSERT(m_uses.isEmpty());
}

const QList<Use*>& DUContext::orphanUses() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_orphanUses;
}

const QList<DUContext*>& DUContext::importedParentContexts() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_importedParentContexts;
}

QList<DUContext*> DUContext::findContexts(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position) const
{
  ENSURE_CHAIN_READ_LOCKED

  QList<UsingNS*> usingStatements;
  QList<DUContext*> ret;
  findContextsInternal(contextType, identifier, position.isValid() ? position : textRange().end(), usingStatements, ret);
  return ret;
}

void DUContext::findContextsInternal(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, QList<UsingNS*>& usingNS, QList<DUContext*>& ret, bool inImportedContext) const
{
  if (contextType == type())
    if (identifier == scopeIdentifier(true))
      ret.append(const_cast<DUContext*>(this));

  if (!identifier.explicitlyGlobal())
    acceptUsingNamespaces(position, usingNS);

  QListIterator<DUContext*> it = m_importedParentContexts;
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* context = it.previous();

    context->findContextsInternal(contextType, identifier, position, usingNS, ret, true);
  }

  if (!inImportedContext && parentContext())
    parentContext()->findContextsInternal(contextType, identifier, position, usingNS, ret);
}

const QList<Definition*>& DUContext::localDefinitions() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_localDefinitions;
}

Definition* DUContext::addDefinition(Definition* definition)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_localDefinitions.append(definition);

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::LocalDefinitions, definition);

  return definition;
}

Definition* DUContext::takeDefinition(Definition* definition)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_localDefinitions.removeAll(definition);

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
  ENSURE_CHAIN_READ_LOCKED

  return m_uses;
}

void DUContext::addUse(Use* use)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_uses.append(use);

  DUChain::contextChanged(this, DUChainObserver::Addition, DUChainObserver::Uses, use);
}

void DUContext::removeUse(Use* use)
{
  ENSURE_CHAIN_WRITE_LOCKED

  Q_ASSERT(m_uses.contains(use));
  m_uses.removeAll(use);

  DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::Uses, use);
}

DUContext * DUContext::findContextAt(const KTextEditor::Cursor & position) const
{
  ENSURE_CHAIN_READ_LOCKED

  if (!textRange().contains(position))
    return 0;

  foreach (DUContext* child, m_childContexts)
    if (DUContext* specific = child->findContextAt(position))
      return specific;

  return const_cast<DUContext*>(this);
}

Use* DUContext::findUseAt(const KTextEditor::Cursor & position) const
{
  ENSURE_CHAIN_READ_LOCKED

  if (!textRange().contains(position))
    return 0;

  foreach (Use* use, m_uses)
    if (use->textRange().contains(position))
      return use;

  return 0;
}

bool DUContext::inSymbolTable() const
{
  // Only one symbol table, no need for a lock

  return m_inSymbolTable;
}

void DUContext::setInSymbolTable(bool inSymbolTable)
{
  // Only one symbol table, no need for a lock

  m_inSymbolTable = inSymbolTable;
}

void DUContext::acceptUsingNamespaces(const KTextEditor::Cursor & position, QList< UsingNS * > & usingNS) const
{
  foreach (UsingNS* ns, usingNamespaces())
    if (ns->textCursor() <= position) {
      // TODO: inefficient... needs a hash??
      foreach (UsingNS* ns2, usingNS)
        if (ns2->nsIdentifier == ns->nsIdentifier)
          goto duplicate;

      usingNS.append(ns);

      duplicate:
      continue;

    } else {
      break;
    }
}

void DUContext::acceptUsingNamespace(UsingNS* ns, QList<UsingNS*>& usingNS) const
{
  // TODO: inefficient... needs a hash??
  foreach (UsingNS* ns2, usingNS)
    if (ns2->nsIdentifier == ns->nsIdentifier)
      return;

  usingNS.append(ns);
}

void DUContext::clearUsingNamespaces()
{
  ENSURE_CHAIN_WRITE_LOCKED

  qDeleteAll(m_usingNamespaces);
  m_usingNamespaces.clear();

  DUChain::contextChanged(this, DUChainObserver::Removal, DUChainObserver::UsingNamespaces);
}

// kate: indent-width 2;

void DUContext::clearImportedParentContexts()
{
  ENSURE_CHAIN_WRITE_LOCKED

  foreach (DUContext* parent, m_importedParentContexts)
    removeImportedParentContext(parent);

  Q_ASSERT(m_importedParentContexts.isEmpty());
}

void DUContext::cleanIfNotEncountered(uint encountered, bool firstPass)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (firstPass) {
    foreach (DUContext* childContext, childContexts())
      if (childContext->lastEncountered() != encountered)
        delete childContext;

    foreach (Declaration* dec, localDeclarations())
      if (dec->lastEncountered() != encountered)
        delete dec;

    foreach (Definition* def, localDefinitions())
      if (def->lastEncountered() != encountered)
        delete def;

  } else {
    foreach (Use* use, uses())
      if (use->lastEncountered() != encountered)
        delete use;
  }
}
