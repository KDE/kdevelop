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

DUContext::DUContext(KTextEditor::Range* range)
  : RangeObject(range)
{
}

DUContext::~DUContext( )
{
  deleteChildContextsRecursively(url());

  qDeleteAll(m_localDefinitions);
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
  foreach (Definition* definition, m_localDefinitions)
    if (definition->identifier() == newDefinition->identifier()) {
      kWarning() << k_funcinfo << "Attempted to add definition with identical identifier to a context." << endl;
      // Shouldn't ever hit this, but return the old definition for now...
      return definition;
    }

  m_localDefinitions.append(newDefinition);
  return newDefinition;
}

Definition * DUContext::findLocalDefinition( const QString & identifier ) const
{
  foreach (Definition* definition, m_localDefinitions)
    if (definition->identifier() == identifier)
      return definition;

  return 0;
}

Definition * DUContext::findDefinition( const QString & identifier ) const
{
  return findDefinition(identifier, DocumentCursor(textRangePtr(), DocumentCursor::Start));
}

DUContext * DUContext::definitionContext( const QString & identifier ) const
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

QHash<QString, Definition*> DUContext::allDefinitions(const DocumentCursor& position) const
{
  QHash<QString, Definition*> ret;

  DUContext* context = findContext(position, const_cast<DUContext*>(this));

  // Iterate back up the chain
  mergeDefinitions(context, ret);

  return ret;
}

const QList<Definition*> DUContext::localDefinitions() const
{
  return m_localDefinitions;
}

void DUContext::mergeDefinitions(DUContext* context, QHash<QString, Definition*>& definitions) const
{
  foreach (Definition* definition, context->localDefinitions())
    if (!definitions.contains(definition->identifier()))
      definitions.insert(definition->identifier(), definition);

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

Definition * DUContext::findDefinition( const QString & identifier, const DocumentCursor & position ) const
{
  return findDefinitionInternal(identifier, position, this);
}

Definition * DUContext::findDefinitionInternal( const QString & identifier, const DocumentCursor & position, const DUContext * const context ) const
{
  if (Definition* definition = context->findLocalDefinition(identifier))
    return definition;

  QListIterator<DUContext*> it = context->parentContexts();
  it.toBack();
  while (it.hasPrevious())
    if (Definition* definition = findDefinitionInternal(identifier, position, it.previous()))
      return definition;

  return 0;
}

Definition* DUContext::takeDefinition(Definition* definition)
{
  m_localDefinitions.removeAll(definition);
  return definition;
}

void DUContext::deleteDefinition(Definition* definition)
{
  m_localDefinitions.removeAll(definition);
  delete definition;
}

// kate: indent-width 2;
