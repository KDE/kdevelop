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

#include "declaration.h"
#include "definition.h"
#include "duchain.h"
#include "use.h"
#include "typesystem.h"

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

  deleteChildContextsRecursively(url());

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

Declaration* DUContext::takeDeclaration(Declaration* definition)
{
  definition->setContext(0);
  m_localDeclarations.removeAll(definition);
  return definition;
}

Declaration * DUContext::findLocalDeclaration( const QualifiedIdentifier& identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch, const QList<UsingNS*>& usingNamespaces ) const
{
  QLinkedList<Declaration*> tryToResolve;
  QLinkedList<Declaration*> ensureResolution;
  QList<Declaration*> resolved;

  //kDebug() << k_funcinfo << "Searching for " << identifier << endl;

  foreach (Declaration* definition, m_localDeclarations) {
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
          resolved.append(definition);
        }
        continue;
        //kDebug() << k_funcinfo << identifier << " contained by " << it.value()->qualifiedIdentifier() << endl;

      case QualifiedIdentifier::ExactMatch:
        //kDebug() << "Identifier " << definition->identifier() << " (" << definition->qualifiedIdentifier() << ") matched, accepted match." << endl;
        if (!allowUnqualifiedMatch) {
          ensureResolution.append(definition);
        } else {
          resolved.append(definition);
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

  foreach (UsingNS* use, usingNamespaces) {
    QualifiedIdentifier id = identifier.merge(use->nsIdentifier);

    QMutableLinkedListIterator<Declaration*> it = tryToResolve;
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
          resolved.append(it.value());
          break;
      }
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

Declaration * DUContext::findDeclaration( const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, const DUContext * sourceChild, const QList<UsingNS*>& usingNS, bool inImportedContext ) const
{
  // TODO we're missing ambiguous references by not checking every resolution before returning...
  // but is that such a bad thing? (might be good performance-wise)
  if (Declaration* definition = findLocalDeclaration(identifier, position, dataType, sourceChild || inImportedContext, usingNS))
    return definition;

  if (identifier.isQualified()) {
    if (Declaration* definition = findDeclarationInChildren(identifier, position, dataType, sourceChild, usingNS))
      return definition;

  } else if (!usingNamespaces().isEmpty() && !identifier.explicitlyGlobal()) {
    QList<UsingNS*> currentUsingNS = usingNS;

    foreach (UsingNS* use, usingNamespaces())
      if (position >= use->textCursor())
        currentUsingNS.append(use);

    if (!currentUsingNS.isEmpty())
      if (Declaration* definition = findDeclarationInChildren(identifier, position, dataType, sourceChild, currentUsingNS))
        return definition;
  }

  QListIterator<DUContext*> it = importedParentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    DUContext* parent = it.previous();

    // FIXME should have the current namespace list??
    if (Declaration* definition = parent->findDeclaration(identifier, position, dataType, this, QList<UsingNS*>(), true))
      return definition;
  }

  if (!inImportedContext && parentContext())
    // FIXME should have the current namespace list??
    if (Declaration* definition = parentContext()->findDeclaration(identifier, position, dataType, this))
      return definition;

  return 0;
}

Declaration * DUContext::findDeclarationInChildren(const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, const DUContext * sourceChild, const QList<UsingNS*>& usingNamespaces) const
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
      if (Declaration* match = context->findLocalDeclaration(identifier, position, dataType, false, usingNamespaces))
        return match;
    }

    if (Declaration* match = context->findDeclarationInChildren(identifier, position, dataType, false, usingNamespaces))
      return match;

    // FIXME nested using definitions
  }

  return 0;
}

Declaration * DUContext::findDeclaration( const QualifiedIdentifier& identifier ) const
{
  return findDeclaration(identifier, textRangePtr()->end());
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

  DUContext* context = findContext(position, const_cast<DUContext*>(this));

  // Iterate back up the chain
  mergeDeclarations(context, ret);

  return ret;
}

const QList<Declaration*>& DUContext::localDeclarations() const
{
  return m_localDeclarations;
}

void DUContext::mergeDeclarations(DUContext* context, QHash<QualifiedIdentifier, Declaration*>& definitions) const
{
  foreach (Declaration* definition, context->localDeclarations())
    if (!definitions.contains(definition->qualifiedIdentifier()))
      definitions.insert(definition->qualifiedIdentifier(), definition);

  QListIterator<DUContext*> it = context->importedParentContexts();
  it.toBack();
  while (it.hasPrevious()) {
    mergeDeclarations(it.previous(), definitions);
  }

  if (parentContext())
    mergeDeclarations(parentContext(), definitions);
}

void DUContext::deleteLocalDeclarations()
{
  qDeleteAll(m_localDeclarations);
  m_localDeclarations.clear();
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
  return findDeclaration(QualifiedIdentifier(identifier));
}

Declaration* DUContext::findDeclaration(const Identifier& identifier, const KTextEditor::Cursor& position) const
{
  return findDeclaration(QualifiedIdentifier(identifier), position);
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

QString DUContext::mangledIdentifier() const
{
  QString ret;
  if (parentContext())
    ret = parentContext()->mangledIdentifier();

  if (type() != Other)
    foreach (const Identifier& id, localScopeIdentifier()) {
      switch (type()) {
        case Namespace:
          ret += "N";
          break;
        case Class:
          ret += "C";
          break;
        case Function:
          ret += "F";
          break;
        default:
          Q_ASSERT(false);
      }
      ret += id.toString();
      ret += "::";
    }

  return ret;
}

// kate: indent-width 2;

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
