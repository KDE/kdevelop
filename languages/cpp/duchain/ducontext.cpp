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

#include "declaration.h"
#include "definition.h"
#include "duchain.h"
#include "use.h"
#include "typesystem.h"
#include "topducontext.h"

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

  deleteChildContextsRecursively();

  deleteLocalDefinitions();

  deleteLocalDeclarations();

  deleteOrphanUses();

  foreach (Use* use, m_uses)
    use->setContext(0);

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

Declaration * DUContext::addDeclaration( Declaration * newDeclaration )
{
  // The definition may not have its identifier set when it's assigned... allow dupes here, TODO catch the error elsewhere

  newDeclaration->setContext(this);
  m_localDeclarations.append(newDeclaration);
  return newDeclaration;
}

Declaration* DUContext::takeDeclaration(Declaration* declaration)
{
  declaration->setContext(0);
  m_localDeclarations.removeAll(declaration);
  return declaration;
}

Declaration * DUContext::findLocalDeclaration( const QualifiedIdentifier& identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch ) const
{
  QLinkedList<Declaration*> tryToResolve;
  QLinkedList<Declaration*> ensureResolution;
  QList<Declaration*> resolved;

  //kDebug() << k_funcinfo << "Searching for " << identifier << endl;

  QList<Declaration*>::ConstIterator end = m_localDeclarations.constEnd();
  for (QList<Declaration*>::ConstIterator it = m_localDeclarations.constBegin(); it != end; ++it) {
    Declaration* declaration = *it;
    QualifiedIdentifier::MatchTypes m = identifier.match(declaration->identifier());
    switch (m) {
      case QualifiedIdentifier::NoMatch:
        //kDebug() << "Identifier does not match " << definition->identifier() << endl;
        continue;

      case QualifiedIdentifier::Contains:
        // identifier is a more complete specification...
        // Try again with a qualified definition identifier
        //kDebug() << "Identifier contains " << definition->identifier() << ", plan to confirm that it is contained by " << definition->qualifiedIdentifier() << endl;
        ensureResolution.append(declaration);
        continue;

      case QualifiedIdentifier::ContainedBy:
        // definition is a more complete specification...
        if (!allowUnqualifiedMatch) {
          //kDebug() << "Identifier contained by " << definition->identifier() << ", plan to try again with " << definition->qualifiedIdentifier() << endl;
          tryToResolve.append(declaration);
        } else {
          //kDebug() << "Identifier contained by " << definition->identifier() << " (" << definition->qualifiedIdentifier() << "), accepted match." << endl;
          resolved.append(declaration);
        }
        continue;
        //kDebug() << k_funcinfo << identifier << " contained by " << it.value()->qualifiedIdentifier() << endl;

      case QualifiedIdentifier::ExactMatch:
        //kDebug() << "Identifier " << definition->identifier() << " (" << definition->qualifiedIdentifier() << ") matched, accepted match." << endl;
        if (!allowUnqualifiedMatch) {
          ensureResolution.append(declaration);
        } else {
          resolved.append(declaration);
        }
        continue;
    }
  }

  if (resolved.count() == 1) {
    Declaration* definition  = resolved.first();
    if (dataType && dataType != definition->abstractType())
      return 0;

    if (type() == Class || position >= definition->textRange().start())
      return definition;

    return 0;

  } else if (resolved.count() > 1) {
    if (dataType) {
      QList<Declaration*> resolved2;

      foreach (Declaration* declaration, resolved)
        if (dataType == declaration->abstractType())
          resolved2.append(declaration);

      if (resolved2.count() == 1) {
        Declaration* definition  = resolved2.first();
        if (type() == Class || position >= definition->textRange().start())
          return definition;
      }
    }

    /*kWarning() << k_funcinfo << "Multiple matching definitions (shouldn't happen, code error)" << endl;
    QSetIterator<Declaration*> it = resolved;
    while (it.hasNext()) {
      Declaration* def = it.next();
      kDebug() << " Declaration: " << def->qualifiedIdentifier() << " range " << def->textRange() << endl;
    }*/

    return 0;

  } else if (tryToResolve.isEmpty() && ensureResolution.isEmpty()) {
    return 0;
  }

  QMutableLinkedListIterator<Declaration*> it = ensureResolution;
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
        resolved.append(it.value());
        break;
    }
  }

  if (resolved.count() == 1) {
    Declaration* definition  = *resolved.constBegin();
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
        resolved.append(it.value());
        break;
    }
  }

  if (resolved.count() == 1) {
    return *resolved.constBegin();
  } else if (resolved.count() > 1) {
    /*kWarning() << k_funcinfo << "Multiple matching definitions (shouldn't happen, code error)" << endl;
    QSetIterator<Declaration*> it = resolved;
    while (it.hasNext()) {
      Declaration* def = it.next();
      kDebug() << " Declaration: " << def->qualifiedIdentifier() << " range " << def->textRange() << endl;
    }*/

    return 0;
  }

  // todo: namespace abbreviations

  return 0;
}

Declaration* DUContext::findDeclarationInternal( const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, QList<UsingNS*>* usingNS, bool inImportedContext ) const
{
  Q_ASSERT(usingNS);

  // TODO we're missing ambiguous references by not checking every resolution before returning...
  // but is that such a bad thing? (might be good performance-wise)
  if (Declaration* definition = findLocalDeclaration(identifier, position, dataType, inImportedContext))
    return definition;

  foreach (UsingNS* ns, usingNamespaces())
    if (ns->textCursor() <= position)
      usingNS->append(ns);

  Declaration* ret = 0;

  QListIterator<DUContext*> it = importedParentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* parent = it.previous();

    ret = parent->findDeclarationInternal(identifier, position, dataType, usingNS, true);
    if (ret)
      break;
  }

  if (!ret && !inImportedContext && parentContext())
    if (Declaration* definition = parentContext()->findDeclarationInternal(identifier, position, dataType, usingNS))
      ret = definition;

  return ret;
}

Declaration * DUContext::findDeclaration( const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType) const
{
  QList<UsingNS*> usingStatements;
  return findDeclarationInternal(identifier, position, dataType, &usingStatements);
}

Declaration * DUContext::findDeclaration( const QualifiedIdentifier& identifier ) const
{
  QList<UsingNS*> usingStatements;
  return findDeclarationInternal(identifier, textRange().end(), AbstractType::Ptr(), &usingStatements);
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

DUContext * DUContext::findContext( const KTextEditor::Cursor& position, DUContext* parent) const
{
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
  QHash<QualifiedIdentifier, Declaration*> ret;

  // Iterate back up the chain
  mergeDeclarations(ret, position);

  return ret;
}

const QList<Declaration*>& DUContext::localDeclarations() const
{
  return m_localDeclarations;
}

void DUContext::mergeDeclarations(QHash<QualifiedIdentifier, Declaration*>& definitions, const KTextEditor::Cursor& position, bool inImportedContext) const
{
  foreach (Declaration* definition, localDeclarations())
    if (!definitions.contains(definition->qualifiedIdentifier()) && (inImportedContext || definition->textRange().start() <= position))
      definitions.insert(definition->qualifiedIdentifier(), definition);

  QListIterator<DUContext*> it = importedParentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    it.previous()->mergeDeclarations(definitions, position, true);
  }

  if (!inImportedContext && parentContext())
    parentContext()->mergeDeclarations(definitions, position);
}

void DUContext::deleteLocalDeclarations()
{
  QList<Declaration*> declarations = m_localDeclarations;

  qDeleteAll(declarations);

  Q_ASSERT(m_localDeclarations.isEmpty());
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
  QList<DUContext*> childContexts = m_childContexts;
  foreach (DUContext* context, childContexts)
    delete context;

  //Q_ASSERT(m_childContexts.isEmpty());
}

QList< Declaration * > DUContext::clearLocalDeclarations( )
{
  QList< Declaration * > ret = m_localDeclarations;
  m_localDeclarations.clear();
  return ret;
}

void DUContext::deleteDeclaration(Declaration* definition)
{
  m_localDeclarations.removeAll(definition);
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

Declaration * DUContext::findDeclaration(const Identifier & identifier) const
{
  QList<UsingNS*> usingStatements;
  return findDeclarationInternal(QualifiedIdentifier(identifier), textRange().end(), AbstractType::Ptr(), &usingStatements);
}

Declaration* DUContext::findDeclaration(const Identifier& identifier, const KTextEditor::Cursor& position) const
{
  QList<UsingNS*> usingStatements;
  return findDeclarationInternal(QualifiedIdentifier(identifier), position, AbstractType::Ptr(), &usingStatements);
}

void DUContext::addOrphanUse(Use* orphan)
{
  m_orphanUses.append(orphan);
}

void DUContext::deleteOrphanUses()
{
  qDeleteAll(m_orphanUses);
  m_orphanUses.clear();
}

const QList<Use*>& DUContext::orphanUses() const
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

const QList<Definition*>& DUContext::localDefinitions() const
{
  return m_localDefinitions;
}

Definition* DUContext::addDefinition(Definition* definition)
{
  m_localDefinitions.append(definition);
  return definition;
}

Definition* DUContext::takeDefinition(Definition* definition)
{
  m_localDefinitions.removeAll(definition);
  return definition;
}

void DUContext::deleteLocalDefinitions()
{
  QList<Definition*> localDefinitions = m_localDefinitions;
  qDeleteAll(localDefinitions);
  Q_ASSERT(m_localDefinitions.isEmpty());
}

const QList< Use * > & DUContext::uses() const
{
  return m_uses;
}

DUContext * DUContext::findContextAt(const KTextEditor::Cursor & position) const
{
  if (!textRange().contains(position))
    return 0;

  foreach (DUContext* child, m_childContexts)
    if (DUContext* specific = child->findContextAt(position))
      return specific;

  return const_cast<DUContext*>(this);
}

Use* DUContext::findUseAt(const KTextEditor::Cursor & position) const
{
  if (!textRange().contains(position))
    return 0;

  foreach (Use* use, m_uses)
    if (use->textRange().contains(position))
      return use;

  return 0;
}

TopDUContext* DUContext::topContext() const
{
  if (parentContext())
    return parentContext()->topContext();

  // This must be the top level context
  Q_ASSERT(dynamic_cast<TopDUContext*>(const_cast<DUContext*>(this)));

  return static_cast<TopDUContext*>(const_cast<DUContext*>(this));
}

// kate: indent-width 2;
