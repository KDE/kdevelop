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
#include "kdevdocumentcursorobject.h"
#include "identifier.h"
#include "typesystem.h"

class Declaration;
class Definition;
class DUChain;
class Use;
class TopDUContext;

/**
 * A single context in source code, represented as a node in a
 * directed acyclic graph.
 *
 * \todo change child relationships to a linked list within the context?
 */
class DUContext : public KDevDocumentRangeObject
{
  friend class Use;

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
    Global,
    Namespace,
    Class,
    Function,
    Other
  };

  ContextType type() const;
  void setType(ContextType type);

  /**
   * Calculate the depth of this context, from the top level context in the file.
   */
  inline int depth() const { if (!parentContext()) return 0; return parentContext()->depth() + 1; }

  /**
   * Retrieve the top level context for this context.
   */
  TopDUContext* topContext() const;

  /**
   * Find the context which most specifically covers \a position.
   */
  DUContext* findContextAt(const KTextEditor::Cursor& position) const;

  /**
   * Find the use which encompasses \a position, if one exists.
   */
  Use* findUseAt(const KTextEditor::Cursor& position) const;

  /**
   * Calculate the fully qualified scope identifier
   */
  QualifiedIdentifier scopeIdentifier(bool includeClasses = false) const;

  /**
   * Scope identifier, used to qualify the identifiers occurring in each context
   */
  const QualifiedIdentifier& localScopeIdentifier() const;

  /**
   * Scope identifier, used to qualify the identifiers occurring in each context
   */
  void setLocalScopeIdentifier(const QualifiedIdentifier& identifier);

  /**
   * Returns whether this context is listed in the symbol table (Namespaces and classes)
   */
  bool inSymbolTable() const;

  /**
   * Tell this object when it is in the symbol table, so it can deregister itself
   */
  void setInSymbolTable(bool inSymbolTable);

  /**
   * Returns the immediate parent context of this context.
   */
  DUContext* parentContext() const;

  /**
   * Returns the list of imported parent contexts for this context.
   */
  const QList<DUContext*>& importedParentContexts() const;

  /**
   * Adds an imported child context.
   *
   * \note Be sure to have set the text location first, so that
   * the chain is sorted correctly.
   */
  void addImportedParentContext(DUContext* context);

  /**
   * Removes a child context.
   */
  void removeImportedParentContext(DUContext* context);

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

  class UsingNS : public KDevDocumentCursorObject
  {
  public:
    UsingNS(KTextEditor::Cursor* cursor);

    QualifiedIdentifier nsIdentifier;
  };
  void addUsingNamespace(KTextEditor::Cursor* cursor, const QualifiedIdentifier& nsIdentifier);
  const QList<UsingNS*>& usingNamespaces() const;

  /**
   * Searches for and returns a declaration with a given \a identifier in this context, which
   * is currently active at the given text \a position, with the given type \a dataType.
   *
   * \param identifier the identifier of the definition to search for
   * \param location the text position to search for
   * \param type the type to match, or null for no type matching.
   *
   * \returns the requested declaration if one was found, otherwise null.
   */
  Declaration* findDeclaration(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType = AbstractType::Ptr()) const;

  /**
   * Searches for and returns a declaration with a given \a identifier in this context, which
   * is currently active at the given text \a position.
   *
   * \param identifier the identifier of the definition to search for
   * \param location the text position to search for
   *
   * \returns the requested declaration if one was found, otherwise null.
   *
   * \overload
   */
  Declaration* findDeclaration(const Identifier& identifier, const KTextEditor::Cursor& position) const;

  /**
   * Returns the declaration of any existing valid \a identifier anywhere this context, or
   * null if one is not found.
   *
   * \overload
   */
  Declaration* findDeclaration(const QualifiedIdentifier& identifier) const;

  /**
   * Returns the declaration of any existing valid \a identifier anywhere this context, or
   * null if one is not found.
   *
   * \overload
   */
  Declaration* findDeclaration(const Identifier& identifier) const;

  /**
   * Returns the type of any \a identifier defined in this context, or
   * null if one is not found.
   */
  Declaration* findLocalDeclaration(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType = AbstractType::Ptr(), bool allowUnqualifiedMatch = false) const;

  /**
   * Clears all local declarations. Does not delete the declaration; the caller
   * assumes ownership.
   */
  QList<Declaration*> clearLocalDeclarations();

  /**
   * Clears all local declarations. Deletes these declarations, as the context has
   * ownership.
   */
  void deleteLocalDeclarations();

  /**
   * Returns all local declarations
   */
  const QList<Declaration*>& localDeclarations() const;

  /**
   * Adds a new declaration to this context. Passes back that definition for convenience.
   */
  Declaration* addDeclaration(Declaration* declaration);

  /**
   * Take a specified \a declaration from this context and pass ownership to the caller.
   */
  Declaration* takeDeclaration(Declaration* declaration);

  /**
   * Remove the specified \a declaration from this context and delete it.
   */
  void deleteDeclaration(Declaration* declaration);

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
   * Clears all local definitions. Deletes these definitions, as the context has
   * ownership.
   */
  void deleteLocalDefinitions();

  /**
   * Searches for the most specific context for the given cursor \a position in the given \a url.
   *
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested context if one was found, otherwise null.
   */
  DUContext* findContext(const KTextEditor::Cursor& position, DUContext* parent = 0) const;

  /**
   * Searches for the context with the given \a type and \a identifier.
   *
   * \param contextType type of context to locate; usually Namespace or Class.
   * \param identifier identifier of the context which is being searched for.
   * \param position cursor position to search from, or invalid to search the whole context.
   *
   * \returns the requested context if one was found, otherwise null.
   */
  QList<DUContext*> findContexts(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position = KTextEditor::Cursor::invalid()) const;

  /**
   * Return a list of definitions for a given cursor \a position in a given \a url.
   *
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested definitions, if any were active at that location.
   */
  QHash<QualifiedIdentifier, Declaration*> allDeclarations(const KTextEditor::Cursor& position) const;

  /**
   * Return a list of all uses which occur in this context.
   */
  const QList<Use*>& uses() const;

  /**
   * Return a list of uses which don't have a corresponding definition.
   */
  const QList<Use*>& orphanUses() const;

  /**
   * Add an orphan use (a use which doesn't have a corresponding definition)
   * to this context.
   */
  void addOrphanUse(Use* orphan);

  /**
   * Clear and delete all orphan uses.
   */
  void deleteOrphanUses();

private:
  /**
   * Merges definitions up all branches of the definition-use chain into one hash.
   */
  void mergeDeclarations(QHash<QualifiedIdentifier, Declaration*>& definitions, const KTextEditor::Cursor& position, bool inImportedContext = false) const;

  /// Logic for calculating the fully qualified scope name
  QualifiedIdentifier scopeIdentifierInternal(DUContext* context) const;

  /// Declaration search implementation
  virtual Declaration* findDeclarationInternal(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<UsingNS*>* usingNamespaces, bool inImportedContext = false) const;

  /// Context search implementation
  virtual void findContextsInternal(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, QList<UsingNS*>& usingNS, QList<DUContext*>& ret, bool inImportedContext = false) const;


  ContextType m_contextType;

  QualifiedIdentifier m_scopeIdentifier;

  DUContext* m_parentContext;
  QList<DUContext*> m_importedParentContexts;
  QList<DUContext*> m_childContexts;
  QList<DUContext*> m_importedChildContexts;

  QList<Declaration*> m_localDeclarations;
  QList<Definition*> m_localDefinitions;

  QList<UsingNS*> m_usingNamespaces;
  QList<Use*> m_uses;
  QList<Use*> m_orphanUses;

  bool m_inSymbolTable : 1;
};

#endif // DUCONTEXT_H

// kate: indent-width 2;
