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

#include "typesystem.h"
#include "definition.h"
#include "duchain.h"
#include "editorintegrator.h"

using namespace KTextEditor;

DUContext::DUContext(KTextEditor::Range* range, DUContext* parent)
  : RangeObject(range)
  , m_contextType(Other)
{
  if (parent)
    parent->addChildContext(this);
}

DUContext::~DUContext( )
{
  deleteChildContextsRecursively(url());

  qDeleteAll(m_localDefinitions);

  QList<Cursor*> usingNS = m_usingNamespaces.values();
  m_usingNamespaces.clear();
  qDeleteAll(usingNS);
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
  if (type() == Namespace) {
    Q_ASSERT(parentContexts().count() == 1);
    parentContexts().first()->addDefinition(newDefinition);
  }

  newDefinition->setContext(this);
  m_localDefinitions.append(newDefinition);
  return newDefinition;
}

Definition * DUContext::findLocalDefinition( const QualifiedIdentifier& identifier ) const
{
  if (identifier.count() > 1) {
    QualifiedIdentifier scope = scopeIdentifier();

    if (identifier.explicitlyGlobal()) {
      QualifiedIdentifier id = identifier;
      id.pop();
      if (scope != id)
        goto Continue;

      return 0;
    }

    if (scope.count() < identifier.count() - 1)
      return 0;

    for (int i = identifier.count() - 2, j = 0; i >= 0; --i, ++j)
      if (identifier.at(i) == scope.at(scope.count() - j - 1))
        continue;
      else
        // The requested identifier is not within the local scope.
        return 0;
  }

  Continue:

  foreach (Definition* definition, m_localDefinitions)
    if (definition->identifier() == identifier.top())
      return definition;

  return 0;
}

DUContext * DUContext::definitionContext( const QualifiedIdentifier& identifier ) const
{
  if (findLocalDefinition(identifier))
    return const_cast<DUContext*>(this);

  QListIterator<DUContext*> it = m_parentContexts;
  it.toBack();
  while (it.hasPrevious()) {
    if (DUContext* context = it.previous()->definitionContext(identifier))
      return context;
  }

  return 0;
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

DUContext * DUContext::findContext( const DocumentCursor& position, DUContext* parent) const
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

QHash<QualifiedIdentifier, Definition*> DUContext::allDefinitions(const DocumentCursor& position) const
{
  QHash<QualifiedIdentifier, Definition*> ret;

  DUContext* context = findContext(position, const_cast<DUContext*>(this));

  // Iterate back up the chain
  mergeDefinitions(context, ret);

  return ret;
}

const QList<Definition*> DUContext::localDefinitions() const
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
  foreach (DUContext* context, m_childContexts) {
    takeChildContext(context);
    context->deleteChildContextsRecursively(url);
    delete context;
  }

  Q_ASSERT(m_childContexts.isEmpty());
}

QList< Definition * > DUContext::clearLocalDefinitions( )
{
  QList< Definition * > ret = m_localDefinitions;
  m_localDefinitions.clear();
  return ret;
}

Definition * DUContext::findDefinition( const QualifiedIdentifier & identifier, const DocumentCursor & position ) const
{
  if (Definition* definition = findLocalDefinition(identifier))
    return definition;

  QListIterator<DUContext*> it = parentContexts();
  it.toBack();
  while (it.hasPrevious())
    if (Definition* definition = it.previous()->findDefinition(identifier, position))
      return definition;

  return 0;
}

Definition * DUContext::findDefinition( const QualifiedIdentifier& identifier ) const
{
  return findDefinition(identifier, DocumentCursor(textRangePtr(), DocumentCursor::Start));
}

Definition* DUContext::takeDefinition(Definition* definition)
{
  // The definition may not have its identifier set when it's assigned... allow dupes here, TODO catch the error elsewhere
  if (type() == Namespace) {
    Q_ASSERT(parentContexts().count() == 1);
    return parentContexts().first()->takeDefinition(definition);
  }

  m_localDefinitions.removeAll(definition);
  return definition;
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

void DUContext::addUsingNamespace(Cursor * cursor, const QualifiedIdentifier& nsIdentifier)
{
  m_usingNamespaces.insert(nsIdentifier, cursor);
}

const QHash<QualifiedIdentifier, KTextEditor::Cursor*>& DUContext::usingNamespaces() const
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

Definition* DUContext::findDefinition(const Identifier& identifier, const DocumentCursor& position) const
{
  return findDefinition(QualifiedIdentifier(identifier), position);
}

// kate: indent-width 2;
