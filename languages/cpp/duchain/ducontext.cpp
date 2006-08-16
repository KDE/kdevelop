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

#include "typesystem.h"
#include "definition.h"
#include "duchain.h"

using namespace KTextEditor;

DUContext::DUContext(KTextEditor::Range* range, DUContext* parent)
  : KDevDocumentRangeObject(range)
  , m_contextType(Other)
{
  if (parent)
    parent->addChildContext(this);
}

DUContext::~DUContext( )
{
  deleteChildContextsRecursively(url());

  qDeleteAll(m_localDefinitions);

  deleteOrphanUses();

  foreach (UsingNS* use, m_usingNamespaces) {
    delete use->origin;
    delete use;
  }
}

const QList< DUContext * > & DUContext::childContexts( ) const
{
  return m_childContexts;
}

const QList< DUContext * > & DUContext::parentContexts( ) const
{
  return m_parentContexts;
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
  QSet<Definition*> resolved;

  foreach (Definition* definition, m_localDefinitions) {
    if (identifier.top() == definition->identifier()) {
      if (identifier.explicitlyGlobal() || identifier.count() > 1) {
        tryToResolve.append(definition);
      } else if (!allowUnqualifiedMatch) {
        tryToResolve.append(definition);
      } else {
        resolved.insert(definition);
      }
    }
  }

  if (resolved.count() == 1) {
    Definition* definition  = *resolved.constBegin();
    if (position >= definition->textRange().start())
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

  } else if (tryToResolve.isEmpty()) {
    return 0;
  }

  QualifiedIdentifier scope = scopeIdentifier();

  QMutableLinkedListIterator<Definition*> it = tryToResolve;
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

Definition * DUContext::findDefinition( const QualifiedIdentifier & identifier, const KDevDocumentCursor & position, const DUContext * sourceChild, const QList<UsingNS*>& usingNS ) const
{
  // TODO we're missing ambiguous references by not checking every resolution before returning...
  // but is that such a bad thing? (might be good performance-wise)
  if (Definition* definition = findLocalDefinition(identifier, position, sourceChild, usingNS))
    return definition;

  if (identifier.isQualified()) {
    if (Definition* definition = findDefinitionInChildren(identifier, position, sourceChild, usingNS))
      return definition;

  } else if (!usingNamespaces().isEmpty() && !identifier.explicitlyGlobal()) {
    QList<UsingNS*> currentUsingNS = usingNS;

    foreach (UsingNS* use, usingNamespaces())
      if (position >= *use->origin)
        currentUsingNS.append(use);

    if (!currentUsingNS.isEmpty())
      if (Definition* definition = findDefinitionInChildren(identifier, position, sourceChild, currentUsingNS))
        return definition;
  }

  QListIterator<DUContext*> it = parentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* parent = it.previous();

    if (Definition* definition = parent->findDefinition(identifier, position, this))
      return definition;
  }

  return 0;
}

Definition * DUContext::findDefinitionInChildren(const QualifiedIdentifier & identifier, const KDevDocumentCursor & position, const DUContext * sourceChild, const QList<UsingNS*>& usingNamespaces) const
{
  foreach (DUContext* context, childContexts()) {
    if (context == sourceChild)
      continue;

    if (context->type() != DUContext::Namespace)
      continue;

    if (Definition* match = context->findLocalDefinition(identifier, position, false, usingNamespaces))
      return match;

    if (Definition* match = context->findDefinitionInChildren(identifier, position, false, usingNamespaces))
      return match;
  }

  // todo nested using definitions

  return 0;
}

Definition * DUContext::findDefinition( const QualifiedIdentifier& identifier ) const
{
  return findDefinition(identifier, KDevDocumentCursor(textRangePtr(), KDevDocumentCursor::Start));
}

void DUContext::addChildContext( DUContext * context )
{
  for (int i = 0; i < m_childContexts.count(); ++i) {
    DUContext* child = m_childContexts.at(i);
    if (context->textRange().start() < child->textRange().start()) {
      m_childContexts.insert(i, context);
      context->addParentContext(this);
      return;
    }
  }
  m_childContexts.append(context);
  context->addParentContext(this);
}

DUContext* DUContext::takeChildContext( DUContext * context )
{
  m_childContexts.removeAll(context);
  context->removeParentContext(this);
  return context;
}

void DUContext::removeParentContext( DUContext * context )
{
  m_parentContexts.removeAll(context);
}

void DUContext::addParentContext( DUContext * context )
{
  for (int i = 0; i < m_parentContexts.count(); ++i) {
    DUContext* parent = m_parentContexts.at(i);
    if (context->textRange().start() < parent->textRange().start()) {
      m_parentContexts.insert(i, context);
      return;
    }
  }
  m_parentContexts.append(context);
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

  QListIterator<DUContext*> it = context->parentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    mergeDefinitions(it.previous(), definitions);
  }
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

void DUContext::deleteChildContextsRecursively(const KUrl& url)
{
  QList<DUContext*> childContexts = m_childContexts;
  foreach (DUContext* context, m_childContexts) {
    takeChildContext(context);
    context->deleteChildContextsRecursively(url);
  }

  qDeleteAll(childContexts);

  Q_ASSERT(m_childContexts.isEmpty());
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

QualifiedIdentifier DUContext::scopeIdentifier() const
{
  QualifiedIdentifier ret = localScopeIdentifier();

  QListIterator<DUContext*> it = parentContexts();
  it.toBack();
  while (it.hasPrevious())
    ret.merge(it.previous()->scopeIdentifier());

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

void DUContext::addUsingNamespace(KTextEditor::Cursor* cursor, const QualifiedIdentifier& id)
{
  UsingNS* use = new UsingNS;
  use->origin = cursor;
  use->nsIdentifier = id;

  QMutableListIterator<UsingNS*> it = m_usingNamespaces;
  while (it.hasPrevious())
    if (*use->origin > *it.previous()->origin) {
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

void DUContext::addOrphanUse(KTextEditor::Range* orphan)
{
  m_orphanUses.append(orphan);
}

void DUContext::deleteOrphanUses()
{
  qDeleteAll(m_orphanUses);
  m_orphanUses.clear();
}

const QList<KTextEditor::Range*>& DUContext::orphanUses() const
{
  return m_orphanUses;
}


// kate: indent-width 2;
