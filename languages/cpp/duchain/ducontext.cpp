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
#include <QSet>

#include "typeinstance.h"
#include "definition.h"
#include "duchain.h"
#include "definitionuse.h"

#include "dumpchain.h"

using namespace KTextEditor;

DUContext::DUContext(KTextEditor::Range* range, DUContext* parent)
  : KDevDocumentRangeObject(range)
  , m_contextType(Other)
  , m_parentContext(0)
{
  if (parent)
    parent->addChildContext(this);
}

DUContext::~DUContext( )
{
  KUrl thisUrl = url();

  if (m_parentContext)
    m_parentContext->m_childContexts.removeAll(this);

  foreach (DUContext* context, m_importedChildContexts)
    context->removeImportedParentContext(this);

  QList<DUContext*> importedParentContexts = m_importedParentContexts;
  foreach (DUContext* context, importedParentContexts)
    removeImportedParentContext(context);

  //deleteImportedParentContextsRecursively(url());

  deleteChildContextsRecursively(url());

  deleteAllTypes();

  qDeleteAll(m_localDefinitions);

  deleteOrphanUses();

  qDeleteAll(m_usingNamespaces);
}

const QList< DUContext * > & DUContext::childContexts( ) const
{
  return m_childContexts;
}

DUContext* DUContext::parentContext( ) const
{
  return m_parentContext;
}

Definition * DUContext::addDefinition( Definition * newDefinition )
{
  // The definition may not have its identifier set when it's assigned... allow dupes here, TODO catch the error elsewhere

  newDefinition->setContext(this);
  m_localDefinitions.append(newDefinition);
  return newDefinition;
}

Definition* DUContext::takeDefinition(Definition* definition)
{
  definition->setContext(0);
  m_localDefinitions.removeAll(definition);
  return definition;
}

Definition * DUContext::findLocalDefinition( const QualifiedIdentifier& identifier, const KDevDocumentCursor & position, bool allowUnqualifiedMatch, const QList<UsingNS*>& usingNamespaces ) const
{
  QLinkedList<Definition*> tryToResolve;
  QLinkedList<Definition*> ensureResolution;
  QSet<Definition*> resolved;

  //kDebug() << k_funcinfo << "Searching for " << identifier << endl;

  foreach (Definition* definition, m_localDefinitions) {
    QualifiedIdentifier::MatchTypes m = identifier.match(QualifiedIdentifier(definition->identifier()));
    switch (m) {
      case QualifiedIdentifier::NoMatch:
        //kDebug() << "Identifier does not match " << definition->identifier() << endl;
        continue;

      case QualifiedIdentifier::Contains:
        // identifier is a more complete specification...
        // Try again with a qualified definition identifier
        //kDebug() << "Identifier contains " << definition->identifier() << ", plan to confirm that it is contained by " << definition->qualifiedIdentifier() << endl;
        ensureResolution.append(definition);
        continue;

      case QualifiedIdentifier::ContainedBy:
        // definition is a more complete specification...
        if (!allowUnqualifiedMatch) {
          //kDebug() << "Identifier contained by " << definition->identifier() << ", plan to try again with " << definition->qualifiedIdentifier() << endl;
          tryToResolve.append(definition);
        } else {
          //kDebug() << "Identifier contained by " << definition->identifier() << " (" << definition->qualifiedIdentifier() << "), accepted match." << endl;
          resolved.insert(definition);
        }
        continue;
        //kDebug() << k_funcinfo << identifier << " contained by " << it.value()->qualifiedIdentifier() << endl;

      case QualifiedIdentifier::ExactMatch:
        //kDebug() << "Identifier " << definition->identifier() << " (" << definition->qualifiedIdentifier() << ") matched, accepted match." << endl;
        if (!allowUnqualifiedMatch) {
          ensureResolution.append(definition);
        } else {
          resolved.insert(definition);
        }
        continue;
    }
  }

  if (resolved.count() == 1) {
    Definition* definition  = *resolved.constBegin();
    if (type() == Class || position >= definition->textRange().start())
      return definition;

    return 0;

  } else if (resolved.count() > 1) {
    /*kWarning() << k_funcinfo << "Multiple matching definitions (shouldn't happen, code error)" << endl;
    QSetIterator<Definition*> it = resolved;
    while (it.hasNext()) {
      Definition* def = it.next();
      kDebug() << " Definition: " << def->qualifiedIdentifier() << " range " << def->textRange() << endl;
    }*/

    return 0;

  } else if (tryToResolve.isEmpty() && ensureResolution.isEmpty()) {
    return 0;
  }

  QMutableLinkedListIterator<Definition*> it = ensureResolution;
  while (it.hasNext()) {
    QualifiedIdentifier::MatchTypes m = identifier.match(it.next()->qualifiedIdentifier());
    switch (m) {
      case QualifiedIdentifier::NoMatch:
      case QualifiedIdentifier::Contains:
        //kDebug() << k_funcinfo << identifier << " mismatched " << m << ": " << it.value()->qualifiedIdentifier() << endl;
        break;

      case QualifiedIdentifier::ContainedBy:
        //kDebug() << k_funcinfo << identifier << " contained by " << it.value()->qualifiedIdentifier() << endl;
      case QualifiedIdentifier::ExactMatch:
        resolved.insert(it.value());
        break;
    }
  }

  if (resolved.count() == 1) {
    Definition* definition  = *resolved.constBegin();
    if (position >= definition->textRange().start())
      return definition;

    return 0;

  } else if (resolved.count() > 1) {
    return 0;
  }


  it = tryToResolve;
  while (it.hasNext()) {
    QualifiedIdentifier::MatchTypes m = identifier.match(it.next()->qualifiedIdentifier());
    switch (m) {
      case QualifiedIdentifier::NoMatch:
      case QualifiedIdentifier::Contains:
        //kDebug() << k_funcinfo << identifier << " mismatched " << m << ": " << it.value()->qualifiedIdentifier() << endl;
        break;

      case QualifiedIdentifier::ContainedBy:
        //kDebug() << k_funcinfo << identifier << " contained by " << it.value()->qualifiedIdentifier() << endl;
      case QualifiedIdentifier::ExactMatch:
        resolved.insert(it.value());
        break;
    }
  }

  foreach (UsingNS* use, usingNamespaces) {
    QualifiedIdentifier id = identifier.merge(use->nsIdentifier);

    QMutableLinkedListIterator<Definition*> it = tryToResolve;
    while (it.hasNext()) {
      QualifiedIdentifier::MatchTypes m = id.match(it.next()->qualifiedIdentifier());
      switch (m) {
        case QualifiedIdentifier::NoMatch:
        case QualifiedIdentifier::Contains:
          //kDebug() << k_funcinfo << identifier << " mismatched " << m << ": " << it.value()->qualifiedIdentifier() << endl;
          break;

        case QualifiedIdentifier::ContainedBy:
          //kDebug() << k_funcinfo << identifier << " contained by " << it.value()->qualifiedIdentifier() << endl;
        case QualifiedIdentifier::ExactMatch:
          resolved.insert(it.value());
          break;
      }
    }
  }

  if (resolved.count() == 1) {
    return *resolved.constBegin();
  } else if (resolved.count() > 1) {
    /*kWarning() << k_funcinfo << "Multiple matching definitions (shouldn't happen, code error)" << endl;
    QSetIterator<Definition*> it = resolved;
    while (it.hasNext()) {
      Definition* def = it.next();
      kDebug() << " Definition: " << def->qualifiedIdentifier() << " range " << def->textRange() << endl;
    }*/

    return 0;
  }

  // todo: namespace abbreviations

  return 0;
}

Definition * DUContext::findDefinition( const QualifiedIdentifier & identifier, const KDevDocumentCursor & position, const DUContext * sourceChild, const QList<UsingNS*>& usingNS, bool inImportedContext ) const
{
  // TODO we're missing ambiguous references by not checking every resolution before returning...
  // but is that such a bad thing? (might be good performance-wise)
  if (Definition* definition = findLocalDefinition(identifier, position, sourceChild || inImportedContext, usingNS))
    return definition;

  if (identifier.isQualified()) {
    if (Definition* definition = findDefinitionInChildren(identifier, position, sourceChild, usingNS))
      return definition;

  } else if (!usingNamespaces().isEmpty() && !identifier.explicitlyGlobal()) {
    QList<UsingNS*> currentUsingNS = usingNS;

    foreach (UsingNS* use, usingNamespaces())
      if (position >= use->textCursor())
        currentUsingNS.append(use);

    if (!currentUsingNS.isEmpty())
      if (Definition* definition = findDefinitionInChildren(identifier, position, sourceChild, currentUsingNS))
        return definition;
  }

  QListIterator<DUContext*> it = importedParentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* parent = it.previous();

    // FIXME should have the current namespace list??
    if (Definition* definition = parent->findDefinition(identifier, position, this, QList<UsingNS*>(), true))
      return definition;
  }

  if (!inImportedContext && parentContext())
    // FIXME should have the current namespace list??
    if (Definition* definition = parentContext()->findDefinition(identifier, position, this))
      return definition;

  return 0;
}

Definition * DUContext::findDefinitionInChildren(const QualifiedIdentifier & identifier, const KDevDocumentCursor & position, const DUContext * sourceChild, const QList<UsingNS*>& usingNamespaces) const
{
  foreach (DUContext* context, childContexts()) {
    if (context == sourceChild)
      break;

    if (context->type() != DUContext::Namespace)
      continue;

    QualifiedIdentifier nsId = context->scopeIdentifier();
    QualifiedIdentifier::MatchTypes bestMatch = QualifiedIdentifier::NoMatch;

    if (identifier.beginsWith(nsId))
      goto found;

    foreach (UsingNS* usingNS, usingNamespaces) {
      QualifiedIdentifier::MatchTypes m = usingNS->nsIdentifier.match(nsId);
      switch (m) {
        case QualifiedIdentifier::Contains:
          // using namespace is a more complete specification...
          bestMatch = m;
          continue;

        case QualifiedIdentifier::ContainedBy:
          // namespace is a more complete specification...
        case QualifiedIdentifier::NoMatch:
          continue;

        case QualifiedIdentifier::ExactMatch:
          goto found;
      }
    }

    if (bestMatch == QualifiedIdentifier::NoMatch)
      continue;

    if (false) {
      found:
      if (Definition* match = context->findLocalDefinition(identifier, position, false, usingNamespaces))
        return match;
    }

    if (Definition* match = context->findDefinitionInChildren(identifier, position, false, usingNamespaces))
      return match;

    // FIXME nested using definitions
  }

  return 0;
}

Definition * DUContext::findDefinition( const QualifiedIdentifier& identifier ) const
{
  return findDefinition(identifier, KDevDocumentCursor(textRangePtr(), KDevDocumentCursor::End));
}

void DUContext::addChildContext( DUContext * context )
{
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

DUContext* DUContext::takeChildContext( DUContext * context )
{
  Q_ASSERT(m_childContexts.contains(context));
  m_childContexts.removeAll(context);
  context->m_parentContext = 0;
  return context;
}

void DUContext::addImportedParentContext( DUContext * context )
{
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
  m_importedParentContexts.removeAll(context);
  context->m_importedChildContexts.removeAll(this);
}

DUContext * DUContext::findContext( const KDevDocumentCursor& position, DUContext* parent) const
{
  if (!parent)
    parent = const_cast<DUContext*>(this);

  foreach (DUContext* context, parent->childContexts())
    if (context->contains(position)) {
      DUContext* ret = findContext(position, context);
      if (!ret)
        ret = context;

      return ret;
    }

  return 0;
}

QHash<QualifiedIdentifier, Definition*> DUContext::allDefinitions(const KDevDocumentCursor& position) const
{
  QHash<QualifiedIdentifier, Definition*> ret;

  DUContext* context = findContext(position, const_cast<DUContext*>(this));

  // Iterate back up the chain
  mergeDefinitions(context, ret);

  return ret;
}

const QList<Definition*>& DUContext::localDefinitions() const
{
  return m_localDefinitions;
}

void DUContext::mergeDefinitions(DUContext* context, QHash<QualifiedIdentifier, Definition*>& definitions) const
{
  foreach (Definition* definition, context->localDefinitions())
    if (!definitions.contains(definition->qualifiedIdentifier()))
      definitions.insert(definition->qualifiedIdentifier(), definition);

  QListIterator<DUContext*> it = context->importedParentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    mergeDefinitions(it.previous(), definitions);
  }

  if (parentContext())
    mergeDefinitions(parentContext(), definitions);
}

void DUContext::deleteLocalDefinitions()
{
  qDeleteAll(m_localDefinitions);
  m_localDefinitions.clear();
}

QList<DUContext*> DUContext::takeChildContexts()
{
  QList<DUContext*> ret = m_childContexts;

  foreach (DUContext* context, m_childContexts)
    takeChildContext(context);

  Q_ASSERT(m_childContexts.isEmpty());

  return ret;
}

void DUContext::deleteChildContextsRecursively()
{
  deleteChildContextsRecursively(url());
}

void DUContext::deleteImportedParentContextsRecursively(const KUrl& url)
{
  QList<DUContext*> importedParentContexts = m_importedParentContexts;
  foreach (DUContext* importedParent, importedParentContexts) {
    if (importedParent->url() == url)
      delete importedParent;
  }

  Q_ASSERT(m_importedParentContexts.isEmpty());
}

void DUContext::deleteChildContextsRecursively(const KUrl& url)
{
  QList<DUContext*> childContexts = m_childContexts;
  foreach (DUContext* context, childContexts)
    if (context->url() == url)
      delete context;

  //Q_ASSERT(m_childContexts.isEmpty());
}

QList< Definition * > DUContext::clearLocalDefinitions( )
{
  QList< Definition * > ret = m_localDefinitions;
  m_localDefinitions.clear();
  return ret;
}

void DUContext::deleteDefinition(Definition* definition)
{
  m_localDefinitions.removeAll(definition);
  delete definition;
}

QualifiedIdentifier DUContext::scopeIdentifier(bool includeClasses) const
{
  QualifiedIdentifier ret;

  if (includeClasses || type() == Namespace)
    ret = localScopeIdentifier();

  if (parentContext())
    ret.merge(parentContext()->scopeIdentifier());

  return ret;
}

void DUContext::setLocalScopeIdentifier(const QualifiedIdentifier & identifier)
{
  m_scopeIdentifier = identifier;
}

const QualifiedIdentifier & DUContext::localScopeIdentifier() const
{
  return m_scopeIdentifier;
}

DUContext::UsingNS::UsingNS(KTextEditor::Cursor* cursor)
  : KDevDocumentCursorObject(cursor)
{
}

void DUContext::addUsingNamespace(KTextEditor::Cursor* cursor, const QualifiedIdentifier& id)
{
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
  return m_usingNamespaces;
}

DUContext::ContextType DUContext::type() const
{
  return m_contextType;
}

void DUContext::setType(ContextType type)
{
  m_contextType = type;
}

Definition * DUContext::findDefinition(const Identifier & identifier) const
{
  return findDefinition(QualifiedIdentifier(identifier));
}

Definition* DUContext::findDefinition(const Identifier& identifier, const KDevDocumentCursor& position) const
{
  return findDefinition(QualifiedIdentifier(identifier), position);
}

void DUContext::addOrphanUse(DefinitionUse* orphan)
{
  m_orphanUses.append(orphan);
}

void DUContext::deleteOrphanUses()
{
  qDeleteAll(m_orphanUses);
  m_orphanUses.clear();
}

const QList<DefinitionUse*>& DUContext::orphanUses() const
{
  return m_orphanUses;
}

const QList<DUContext*>& DUContext::importedParentContexts() const
{
  return m_importedParentContexts;
}

DUContext* DUContext::findContext(ContextType contextType, const QualifiedIdentifier& identifier, const DUContext* sourceChild, const QList<UsingNS*>& usingNS, bool inImportedContext) const
{
  QList<UsingNS*> currentUsingNS = usingNS;

  if (!identifier.explicitlyGlobal()) {
    foreach (UsingNS* use, usingNamespaces())
      if (!sourceChild || sourceChild->textRange().start() >= use->textCursor())
        currentUsingNS.append(use);
  }

  if (contextType == type())
    if (identifier == scopeIdentifier(true))
      return const_cast<DUContext*>(this);

  foreach (DUContext* child, m_childContexts) {
    if (child == sourceChild)
      break;

    if (contextType == child->type())
      if (identifier == child->scopeIdentifier(true))
        return child;

    if (child->type() == Namespace) {
      QualifiedIdentifier nsIdentifier = identifier;
      nsIdentifier.pop();
      foreach (UsingNS* use, currentUsingNS) {
        switch (use->nsIdentifier.match(nsIdentifier)) {
          case QualifiedIdentifier::ContainedBy:
          case QualifiedIdentifier::ExactMatch:
          case QualifiedIdentifier::Contains:
            if (DUContext* context = child->findContext(contextType, identifier, 0L, currentUsingNS))
              return context;
            continue;

          default:
            continue;
        }
      }
    }
  }

  // FIXME currentUsingNS is not exactly correct
  foreach (UsingNS* use, currentUsingNS) {
    QualifiedIdentifier id = identifier.merge(use->nsIdentifier);

    if (contextType == type())
      if (id == scopeIdentifier(true))
        return const_cast<DUContext*>(this);
  }

  QListIterator<DUContext*> it = m_importedParentContexts;
  it.toBack();
  while (it.hasPrevious())
    if (DUContext* context = it.previous()->findContext(contextType, identifier, this, currentUsingNS, true))
      return context;

  if (!inImportedContext && parentContext())
    if (DUContext* context = parentContext()->findContext(contextType, identifier, this, currentUsingNS))
      return context;

  return 0;
}

// kate: indent-width 2;

const QList<TypeInstance*>& DUContext::types() const
{
  return m_types;
}

void DUContext::addType(TypeInstance* type)
{
  m_types.append(type);
}

void DUContext::deleteAllTypes()
{
  qDeleteAll(m_types);
}
