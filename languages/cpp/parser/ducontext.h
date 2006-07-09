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

#include "textlocation.h"

#include <QHash>

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
class DUContext : public TextRange
{
public:
  /**
   * Constructor. No convenience methods, as the initialisation order is important,
   * and providing all permutations would be overkill.
   */
  DUContext();

  /**
   * Destructor. Will delete all child contexts which are defined within
   * the same file as this context.
   */
  virtual ~DUContext();

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

  /**
   * Returns the context in which \a identifier was defined, or
   * null if one is not found.
   */
  DUContext* definitionContext(const QString& identifier) const;

  /**
   * Returns the type of any existing valid \a identifier anywhere this context, or
   * null if one is not found.
   *
   * \overload
   */
  Definition* findDefinition(const QString& identifier) const;

  /**
   * Searches for and returns a definition with a given \a identifier, which is currently active at the given cursor \a position
   * in the given \a url.
   *
   * \param identifier the identifier of the definition to search for
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested definition if one was found, otherwise null.
   */
  Definition* findDefinition(const QString& identifier, const TextPosition& position, DUContext* parent = 0) const;

  /**
   * Returns the type of any \a identifier defined in this context, or
   * null if one is not found.
   */
  Definition* findLocalDefinition(const QString& identifier) const;

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
  const QList<Definition*> localDefinitions() const;

  /**
   * Adds a new definition to this context. Passes back that definition for convenience.
   */
  Definition* addDefinition(Definition* definition);

  /**
   * Remove a specified \a definition from this context.
   */
  void removeDefinition(Definition* definition);

  /**
   * Searches for the most specific context for the given cursor \a position in the given \a url.
   *
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested context if one was found, otherwise null.
   */
  DUContext* findContext(const TextPosition& position, DUContext* parent = 0) const;

  /**
   * Return a list of definitions for a given cursor \a position in a given \a url.
   *
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested definitions, if any were active at that location.
   */
  QHash<QString, Definition*> allDefinitions(const TextPosition& position) const;

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
  void mergeDefinitions(DUContext* context, QHash<QString, Definition*>& definitions) const;

  /// Deletion function which respects file boundaries.
  void deleteChildContextsRecursively(const KUrl& url);

  QList<DUContext*> m_parentContexts;
  QList<DUContext*> m_childContexts;

  QList<Definition*> m_localDefinitions;
};

#endif // DUCONTEXT_H

// kate: indent-width 2;
