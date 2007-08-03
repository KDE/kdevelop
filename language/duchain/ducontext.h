/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include <QtCore/QHash>
#include <languageexport.h>

#include <documentcursorobject.h>
#include <identifier.h>
#include <typesystem.h>
#include <duchainbase.h>

template<class T>
class QSet;

namespace KDevelop
{

class Declaration;
class Definition;
class DUChain;
class Use;
class TopDUContext;
class DUContext;


template<class T>
class DUChainPointer;
typedef DUChainPointer<DUContext> DUContextPointer;

/**
 * A single context in source code, represented as a node in a
 * directed acyclic graph.
 *
 * Access to context objects must be serialised by holding the
 * chain lock, ie. DUChain::lock().
 *
 * NOTE: A du-context can be freely edited as long as it's parent-context is zero.
 * In the moment the parent-context is set, the context may only be edited when it
 * is allowed to edited it's top-level context(@see TopLevelContext::inDUChain()
 * 
 * \todo change child relationships to a linked list within the context?
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUContext : public DUChainBase
{
  friend class Use;
  friend class Declaration;
  friend class Definition;

public:
  /**
   * Constructor. No convenience methods, as the initialisation order is important,
   * and providing all permutations would be overkill.
   * @param anonymous Whether the context should be added as an anonymous context to the parent. That way the context can never be found through any of the parent's member-functions.
   */
  explicit DUContext(KTextEditor::Range* range, DUContext* parent = 0, bool anonymous = false);

  /**
   * Destructor. Will delete all child contexts which are defined within
   * the same file as this context.
   */
  virtual ~DUContext();

  enum ContextType {
    Global,
    Namespace,
    Class,
    Function, //A context that declares function-arguments
    Template, //A context that declares template-parameters
    Other //Represents executable code, like for example within a compound-statement
  };

  enum SearchFlag {
    NoSearchFlags = 0,
    InImportedParentContext = 1, //Internal, do not use from outside
    OnlyContainerTypes = 2, //Not implemented yet
    DontSearchInParent = 4, //IF  this flag is set, findDeclarations(..) will not search for the identifier in parent-contexts(which does not include imported parent-contexts)
    NoUndefinedTemplateParams = 8 //For languages that support templates(like C++). If this is set, the search should fail as soon as undefined template-parameters are involved.
  };

  Q_DECLARE_FLAGS(SearchFlags, SearchFlag)
  

  ContextType type() const;
  void setType(ContextType type);

  /**
   * If this context was opened by a declaration, this returns that declaration.
   * The returned declaration may have this context set as internalContext(),
   * but it does not have to, because there may be multiple contexts opened by one declaration,
   * and the declaration should have the last one assigned.
   * */
  Declaration* declaration() const;
  /**
   * Sets the declaration, and also updates the declarations internal context if necessary.
   * */
  void setDeclaration(Declaration* decl);
  
  /**
   * Calculate the depth of this context, from the top level context in the file.
   */
  int depth() const;

  /**
   * Find the top context.
   */
  virtual TopDUContext* topContext() const;

  /**
   * Find the context which most specifically covers \a position.
   */
  DUContext* findContextAt(const KTextEditor::Cursor& position) const;

  /**
   * Find the context which most specifically covers \a range.
   */
  DUContext* findContextIncluding(const KTextEditor::Range& range) const;

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
   * @warning The list may contain objects that are not valid any more(data() returns zero, but that can only happen when using anonymous imports, @see addImportedParentContext)
   */
  const QList<DUContextPointer>& importedParentContexts() const;

  /**
   * Adds an imported context.
   *
   * @param anonymous If this is true, the import will not be registered at the imported context. This allows du-chain contexts importing without having a write-lock.
   * 
   * \note Be sure to have set the text location first, so that
   * the chain is sorted correctly.
   */
  virtual void addImportedParentContext(DUContext* context, bool anonymous = false);

  /**
   * Removes a child context.
   */
  virtual void removeImportedParentContext(DUContext* context);

  /**
   * Clear all imported parent contexts.
   */
  void clearImportedParentContexts();

  /**
   * Returns the list of contexts importing this context.
   */
  const QList<DUContext*>& importedChildContexts() const;

  /**
   * Returns the list of immediate child contexts for this context.
   */
  const QList<DUContext*>& childContexts() const;

  /**
   * Clears and deletes all child contexts recursively.
   * This will not cross file boundaries.
   */
  void deleteChildContextsRecursively();

  ///Returns true if this declaration is accessible through the du-chain, and thus cannot be edited without a du-chain write lock
  virtual bool inDUChain() const;
  
  /**
   * A class which represents a "using namespace" statement, or a "namespace A = B" statement.
   */
  class NamespaceAlias : public KDevelop::DocumentCursorObject
  {
  public:
    NamespaceAlias(KTextEditor::Cursor* cursor);

    QualifiedIdentifier nsIdentifier; //The identifier that was imported
    QString aliasIdentifier; //The identifier nsIdentifier should be "renamed to" within scope, or empty if it is an import.
    QualifiedIdentifier scope; //The scope this using namespace was issued in
  };

  /**
   * Register a namespace-alias with this context.
   * @param aliasName The name as which the imported context should be found. If this is QString::null, nsIdentifier will be imported(like "using namespace ...")
   * */
  void addNamespaceAlias(KTextEditor::Cursor* cursor, const QualifiedIdentifier& nsIdentifier, const QString& aliasName = QString() );
  /// Return a list of namespace aliases for this context. They are ordered in the reversed oder of their appearance.
  const QList<NamespaceAlias*>& namespaceAliases() const;
  /// Clear namespace aliases for this context.
  void clearNamespaceAliases();

  /**
   * Searches for and returns a declaration with a given \a identifier in this context, which
   * is currently active at the given text \a position, with the given type \a dataType.
   *
   * \param identifier the identifier of the definition to search for
   * \param location the text position to search for
   * \param type the type to match, or null for no type matching.
   *
   * \returns the requested declaration if one was found, otherwise null.
   *
   * \warning this may return declarations which are not in this tree, you may need to lock them too...
   */
  QList<Declaration*> findDeclarations(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position = KTextEditor::Cursor::invalid(), const AbstractType::Ptr& dataType = AbstractType::Ptr(), SearchFlags flags = NoSearchFlags) const;

  /**
   * Searches for and returns a declaration with a given \a identifier in this context, which
   * is currently active at the given text \a position.
   *
   * \param identifier the identifier of the definition to search for
   * \param location the text position to search for
   *
   * \returns the requested declaration if one was found, otherwise null.
   *
   * \warning this may return declarations which are not in this tree, you may need to lock them too...
   *
   * \overload
   */
  QList<Declaration*> findDeclarations(const Identifier& identifier, const KTextEditor::Cursor& position = KTextEditor::Cursor::invalid(), SearchFlags flags = NoSearchFlags) const;

  /**
   * Returns the type of any \a identifier defined in this context, or
   * null if one is not found.
   * 
   * Does not search imported parent-contexts(like base-classes).
   */
  QList<Declaration*> findLocalDeclarations(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position = KTextEditor::Cursor::invalid(), const AbstractType::Ptr& dataType = AbstractType::Ptr(), bool allowUnqualifiedMatch = false, SearchFlags flags = NoSearchFlags) const;

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
  const QList<Declaration*> localDeclarations() const;

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
   *
   * \warning this may return contexts which are not in this tree, you may need to lock them too...
   */
  QList<DUContext*> findContexts(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position = KTextEditor::Cursor::invalid(), SearchFlags flags = NoSearchFlags) const;

  /**
   * Return a list of definitions for a given cursor \a position in a given \a url.
   *
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested definitions, if any were active at that location.
   *
   * \warning this may return declarations which are not in this tree, you may need to lock them too...
   */
  QHash<QualifiedIdentifier, Declaration*> allDeclarations(const KTextEditor::Cursor& position) const;

  /**
   * Find the use which encompasses \a position, if one exists.
   */
  Use* findUseAt(const KTextEditor::Cursor& position) const;

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
   * Clear and delete all uses in this context.
   */
  void deleteUses();

  /**
   * Delete and remove all slaves(uses, declarations, definitions, contexts) that are not in the given set
   */
  void cleanIfNotEncountered(const QSet<DUChainBase*>& encountered, bool firstPass);

  /**
   * Used exclusively by Declaration, do not use this.
   * */
  void changingIdentifier( Declaration* decl, const Identifier& from, const Identifier& to );
  
protected:
  /**
   * After one scope was searched, this function is asked whether more results should be collected. Override it, for example to collect overloaded functions.
   * The default-implementation returns true as soon as decls is not empty.
   * */
  virtual bool foundEnough( const QList<Declaration*>& decls ) const;
  /**
   * Merges definitions up all branches of the definition-use chain into one hash.
   */
  void mergeDeclarationsInternal(QHash<QualifiedIdentifier, Declaration*>& definitions, const KTextEditor::Cursor& position, bool inImportedContext = false) const;

  /// Logic for calculating the fully qualified scope name
  QualifiedIdentifier scopeIdentifierInternal(DUContext* context) const;

  virtual void findLocalDeclarationsInternal( const QualifiedIdentifier& identifier, const KTextEditor::Cursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch, QList<Declaration*>& ret, SearchFlags flags ) const;

  /// Declaration search implementation
  virtual void findDeclarationsInternal(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<NamespaceAlias*>& namespaceAliases, QList<Declaration*>& ret, SearchFlags flags ) const;
  
  /// Context search implementation
  virtual void findContextsInternal(ContextType contextType, const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, QList<NamespaceAlias*>& usingNS, QList<DUContext*>& ret, SearchFlags flags = NoSearchFlags) const;

  void acceptUsingNamespaces(const KTextEditor::Cursor& position, QList<NamespaceAlias*>& usingNS) const;
  void acceptUsingNamespace(NamespaceAlias* ns, QList<NamespaceAlias*>& usingNS) const;

private:
  class DUContextPrivate* const d;
  friend class DUContextPrivate;
};

}

#endif // DUCONTEXT_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
