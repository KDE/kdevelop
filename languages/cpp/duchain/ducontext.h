/* This file is part of KDevelop
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

#ifndef DUCONTEXT_H
#define DUCONTEXT_H

#include <QHash>

#include "kdevdocumentrangeobject.h"
#include "kdevdocumentcursor.h"
#include "identifier.h"

class AbstractType;
class Definition;
class DUChain;

/**
 * A single context in source code, represented as a node in a
 * directed acyclic graph.
 *
 * \todo change child relationships to a linked list within the context?
 * \todo check that redefinition of local variables within a base function context
 *       is supported by the current code.
 */
class DUContext : public KDevDocumentRangeObject
{
public:
  /**
   * Constructor. No convenience methods, as the initialisation order is important,
   * and providing all permutations would be overkill.
   */
  DUContext(KTextEditor::Range* range, DUContext* parent = 0);

  /**
   * Destructor. Will delete all child contexts which are defined within
   * the same file as this context.
   */
  virtual ~DUContext();

  enum ContextType {
    Namespace,
    Class,
    Function,
    Other
  };

  ContextType type() const;
  void setType(ContextType type);

  inline int depth() const { if (parentContexts().isEmpty()) return 0; return parentContexts().first()->depth() + 1; }

  /**
   * Calculate the fully qualified scope identifier
   */
  QualifiedIdentifier scopeIdentifier() const;

  /**
   * Scope identifier, used to qualify the identifiers occurring in each context
   */
  const QualifiedIdentifier& localScopeIdentifier() const;

  /**
   * Scope identifier, used to qualify the identifiers occurring in each context
   */
  void setLocalScopeIdentifier(const QualifiedIdentifier& identifier);

  /**
   * Returns the list of immediate parent contexts for this context.
   */
  const QList<DUContext*>& parentContexts() const;

  /**
   * Returns the list of immediate child contexts for this context.
   */
  const QList<DUContext*>& childContexts() const;

  /**
   * Adds a child context.
   *
   * \note Be sure to have set the text location first, so that
   * the chain is sorted correctly.
   */
  void addChildContext(DUContext* context);

  /**
   * Removes a child context.  Ownership is transferred to the caller, so if you don't
   * want it, delete it.
   */
  DUContext* takeChildContext(DUContext* context);

  /**
   * Clears but does not delete the child contexts.  The caller has the responsibility
   * of using or deleting them.
   */
  QList<DUContext*> takeChildContexts();

  /**
   * Clears and deletes all child contexts recursively.
   * This will not cross file boundaries.
   */
  void deleteChildContextsRecursively();

  struct UsingNS {
    KTextEditor::Cursor* origin;
    QualifiedIdentifier nsIdentifier;
  };
  void addUsingNamespace(KTextEditor::Cursor* cursor, const QualifiedIdentifier& nsIdentifier);
  const QList<UsingNS*>& usingNamespaces() const;

  /**
   * Searches for and returns a definition with a given \a identifier in this context, which
   * is currently active at the given text \a position.
   *
   * \param identifier the identifier of the definition to search for
   * \param location the text position to search for
   *
   * \returns the requested definition if one was found, otherwise null.
   */
  Definition* findDefinition(const QualifiedIdentifier& identifier, const KDevDocumentCursor& position, const DUContext* sourceChild = 0, const QList<UsingNS*>& usingNamespaces = QList<UsingNS*>()) const;

  /**
   * Searches for and returns a definition with a given \a identifier in this context, which
   * is currently active at the given text \a position.
   *
   * \param identifier the identifier of the definition to search for
   * \param location the text position to search for
   *
   * \returns the requested definition if one was found, otherwise null.
   *
   * \overload
   */
  Definition* findDefinition(const Identifier& identifier, const KDevDocumentCursor& position) const;

  /**
   * Returns the type of any existing valid \a identifier anywhere this context, or
   * null if one is not found.
   *
   * \overload
   */
  Definition* findDefinition(const QualifiedIdentifier& identifier) const;

  /**
   * Returns the type of any existing valid \a identifier anywhere this context, or
   * null if one is not found.
   *
   * \overload
   */
  Definition* findDefinition(const Identifier& identifier) const;

  /**
   * Returns the type of any \a identifier defined in this context, or
   * null if one is not found.
   */
  Definition* findLocalDefinition(const QualifiedIdentifier& identifier, const KDevDocumentCursor & position, bool allowUnqualifiedMatch = false, const QList<UsingNS*>& usingNamespaces = QList<UsingNS*>()) const;

  /**
   * Clears all local definitions. Does not delete the definitions; the caller
   * assumes ownership.
   */
  QList<Definition*> clearLocalDefinitions();

  /**
   * Clears all local definitions. Deletes these definitions, as the context has
   * ownership.
   */
  void deleteLocalDefinitions();

  /**
   * Returns all local definitions
   */
  const QList<Definition*>& localDefinitions() const;

  /**
   * Adds a new definition to this context. Passes back that definition for convenience.
   */
  Definition* addDefinition(Definition* definition);

  /**
   * Take a specified \a definition from this context and pass ownership to the caller.
   */
  Definition* takeDefinition(Definition* definition);

  /**
   * Remove the specified \a definition from this context and delete it.
   */
  void deleteDefinition(Definition* definition);

  /**
   * Searches for the most specific context for the given cursor \a position in the given \a url.
   *
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested context if one was found, otherwise null.
   */
  DUContext* findContext(const KDevDocumentCursor& position, DUContext* parent = 0) const;

  /**
   * Return a list of definitions for a given cursor \a position in a given \a url.
   *
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested definitions, if any were active at that location.
   */
  QHash<QualifiedIdentifier, Definition*> allDefinitions(const KDevDocumentCursor& position) const;

  /**
   * Return a list of uses which don't have a corresponding definition.
   */
  const QList<KTextEditor::Range*>& orphanUses() const;

  /**
   * Add an orphan use (a use which doesn't have a corresponding definition)
   * to this context.
   */
  void addOrphanUse(KTextEditor::Range* orphan);

  /**
   * Clear and delete all orphan uses.
   */
  void deleteOrphanUses();

private:
  /**
   * \internal
   * Adds a parent context.
   */
  void addParentContext(DUContext* context);

  /**
   * \internal
   * Removes a parent context.
   */
  void removeParentContext(DUContext* context);

  /**
   * Merges definitions up all branches of the definition-use chain into one hash.
   */
  void mergeDefinitions(DUContext* context, QHash<QualifiedIdentifier, Definition*>& definitions) const;

  /// Deletion function which respects file boundaries.
  void deleteChildContextsRecursively(const KUrl& url);

  /// Logic for calculating the fully qualified scope name
  QualifiedIdentifier scopeIdentifierInternal(DUContext* context) const;

  /// Search closed contexts which have not necessarily lost scope
  Definition* findDefinitionInChildren(const QualifiedIdentifier& identifier, const KDevDocumentCursor& position, const DUContext* sourceChild, const QList<UsingNS*>& usingNamespaces) const;

  ContextType m_contextType;

  QualifiedIdentifier m_scopeIdentifier;

  QList<DUContext*> m_parentContexts;
  QList<DUContext*> m_childContexts;

  QList<Definition*> m_localDefinitions;

  QList<UsingNS*> m_usingNamespaces;
  QList<KTextEditor::Range*> m_orphanUses;
};

#endif // DUCONTEXT_H

// kate: indent-width 2;
