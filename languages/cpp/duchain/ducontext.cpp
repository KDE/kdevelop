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

// Unfortunately, impossible to check this exactly as we could be in a write lock
#define ENSURE_CHAIN_READ_LOCKED \
if (!topContext()->deleting()) { \
  bool _ensure_chain_locked = chainLock()->tryLockForWrite(); \
  Q_ASSERT(!_ensure_chain_locked); \
}

#define ENSURE_CHAIN_WRITE_LOCKED \
if (!topContext()->deleting()) { \
  bool _ensure_chain_locked = chainLock()->tryLockForWrite(); \
  Q_ASSERT(!_ensure_chain_locked); \
}

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

  foreach (DUContext* context, m_importedChildContexts)
    context->removeImportedParentContext(this);

  QList<DUContext*> importedParentContexts = m_importedParentContexts;
  foreach (DUContext* context, importedParentContexts)
    removeImportedParentContext(context);

  deleteChildContextsRecursively();

  deleteLocalDefinitions();

  deleteLocalDeclarations();

  deleteOrphanUses();

  // Hmm, a lot of lock/unlock here, maybe fix?
  QList<Use*> useList = internalUses();
  foreach (Use* use, useList)
    use->setContext(0);
  useList = externalUses();
  foreach (Use* use, useList)
    use->setContext(0);

  clearUsingNamespaces();
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
}

void DUContext::removeDeclaration(Declaration* declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_localDeclarations.removeAll(declaration);
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
    DUContext* parent = it.previous();

    parent->findDeclarationsInternal(identifier, position, dataType, usingNS, ret, true);
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

  Q_ASSERT(!context->m_importedChildContexts.contains(context));

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
}

void DUContext::addImportedParentContext( DUContext * context )
{
  ENSURE_CHAIN_WRITE_LOCKED

  Q_ASSERT(!context->childContexts().contains(context));
  context->m_importedChildContexts.append(this);

  for (int i = 0; i < m_importedParentContexts.count(); ++i) {
    DUContext* parent = m_importedParentContexts.at(i);
    if (context->textRange().start() < parent->textRange().start()) {
      m_importedParentContexts.insert(i, context);
      return;
    }
  }
  m_importedParentContexts.append(context);
}

void DUContext::removeImportedParentContext( DUContext * context )
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_importedParentContexts.removeAll(context);
  context->m_importedChildContexts.removeAll(this);
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
    it.previous()->mergeDeclarationsInternal(definitions, position, true);
  }

  if (!inImportedContext && parentContext())
    parentContext()->mergeDeclarationsInternal(definitions, position);
}

void DUContext::deleteLocalDeclarations()
{
  ENSURE_CHAIN_WRITE_LOCKED

  QList<Declaration*> declarations = m_localDeclarations;

  qDeleteAll(declarations);

  Q_ASSERT(m_localDeclarations.isEmpty());
}

void DUContext::deleteChildContextsRecursively()
{
  ENSURE_CHAIN_WRITE_LOCKED

  QList<DUContext*> childContexts = m_childContexts;
  foreach (DUContext* context, childContexts)
    delete context;

  //Q_ASSERT(m_childContexts.isEmpty());
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

void DUContext::deleteOrphanUses()
{
  ENSURE_CHAIN_WRITE_LOCKED

  qDeleteAll(m_orphanUses);
  m_orphanUses.clear();
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
  while (it.hasPrevious())
    it.previous()->findContextsInternal(contextType, identifier, position, usingNS, ret, true);

  if (!inImportedContext && parentContext())
    parentContext()->findContextsInternal(contextType, identifier, position, usingNS, ret);
}

const QList<Definition*>& DUContext::localDefinitions() const
{
  QReadLocker lock(&m_definitionLock);

  return m_localDefinitions;
}

Definition* DUContext::addDefinition(Definition* definition)
{
  QWriteLocker lock(&m_definitionLock);

  m_localDefinitions.append(definition);
  return definition;
}

Definition* DUContext::takeDefinition(Definition* definition)
{
  QWriteLocker lock(&m_definitionLock);

  m_localDefinitions.removeAll(definition);
  return definition;
}

void DUContext::deleteLocalDefinitions()
{
  // No need to assert a lock

  QList<Definition*> definitions = localDefinitions();
  qDeleteAll(definitions);

  Q_ASSERT(localDefinitions().isEmpty());
}

const QList< Use * > & DUContext::internalUses() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_internalUses;
}

void DUContext::addInternalUse(Use* use)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_internalUses.append(use);
}

void DUContext::removeInternalUse(Use* use)
{
  ENSURE_CHAIN_WRITE_LOCKED

  Q_ASSERT(m_internalUses.contains(use));
  m_internalUses.removeAll(use);
}

const QList< Use * > & DUContext::externalUses() const
{
  QWriteLocker lock(&m_externalUseLock);

  return m_externalUses;
}

void DUContext::addExternalUse(Use* use)
{
  QWriteLocker lock(&m_externalUseLock);

  m_externalUses.append(use);
}

void DUContext::removeExternalUse(Use* use)
{
  QWriteLocker lock(&m_externalUseLock);

  Q_ASSERT(m_externalUses.contains(use));
  m_externalUses.removeAll(use);
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

  foreach (Use* use, m_internalUses)
    if (use->textRange().contains(position))
      return use;

  QReadLocker lock(&m_externalUseLock);

  foreach (Use* use, m_externalUses)
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
}

// kate: indent-width 2;
