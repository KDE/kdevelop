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

#include <editor/documentcursorobject.h>
#include "identifier.h"
#include "typesystem.h"
#include "duchainbase.h"
#include "../languageexport.h"

template<class T>
class QSet;

class QWidget;

namespace KTextEditor {
  class SmartRange;
}

namespace KDevelop
{

class ContextOwner;
class Declaration;
class DUChain;
class Use;
class TopDUContext;
class DUContext;
class DUContextPrivate;

///This class is used to trace imports while findDeclarationsInternal. The back-tracing may be needed for correctly resolving delayed types(templates)
struct ImportTraceItem {
  ImportTraceItem(const DUContext* _ctx, SimpleCursor _pos = SimpleCursor::invalid()) : ctx(_ctx), position(_pos) {
  }

  //The trace goes backwards. This means that for each imported context, it contains the context the new one is imported to, not the imported context.
  const DUContext* ctx;
  SimpleCursor position;
};

typedef QList<ImportTraceItem> ImportTrace;

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
  friend class DeclarationPrivate;
  friend class Definition;

public:
  /**
   * Constructor. No convenience methods, as the initialisation order is important,
   * and providing all permutations would be overkill.
   * @param anonymous Whether the context should be added as an anonymous context to the parent. That way the context can never be found through any of the parent's member-functions.
   */
  explicit DUContext(const HashedString& url, const SimpleRange& range, DUContext* parent = 0, bool anonymous = false);

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
    NoUndefinedTemplateParams = 8, //For languages that support templates(like C++). If this is set, the search should fail as soon as undefined template-parameters are involved.
    DirectQualifiedLookup = 16, //When this flag is used, the searched qualified identifier should NOT be split up into it's components and looked up one by one. Currently only plays a role in C++ specific parts.
    NoFiltering = 32,           //Should be set when no filtering at all is wished, not even filtering that is natural for the underlying language(For example in C++, constructors are filtered out be default)
    OnlyFunctions = 64, //When this is given, only function-declarations are returned. In case of C++, this also means that constructors can be retrieved, while normally they are filtered out.
    NoImportsCheck = 128 //With this parameter, a global search will return all matching items, from all contexts, not only from imported ones.
  };

  Q_DECLARE_FLAGS(SearchFlags, SearchFlag)


  ContextType type() const;
  void setType(ContextType type);

  /**
   * If this context was opened by a declaration or definition, this returns that item.
   * The returned declaration/definition will have this context set as internalContext()
   * */
  Declaration* owner() const;
  /**
   * Sets the declaration/definition, and also updates it's internal context(they are strictly paired together)
   * */
  void setOwner(Declaration* decl);

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
  DUContext* findContextAt(const SimpleCursor& position) const;

  /**
   * Find the context which most specifically covers \a range.
   */
  DUContext* findContextIncluding(const SimpleRange& range) const;

  /**
   * Calculate the fully qualified scope identifier
   */
  QualifiedIdentifier scopeIdentifier(bool includeClasses = false) const;

  /**
   * Scope identifier, used to qualify the identifiers occurring in each context.
   * This is the part relative to the parent context.
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
   * @warning The import structure may contain loops if this is a TopDUContext, so be careful when traversing the tree.
   */
  const QList<DUContextPointer>& importedParentContexts() const;

  /**
   * If the given context is directly imported into this one, and
   * addImportedParentContext(..) was called with a valid cursor, this will return that position.
   * Else an invalid cursor is returned.
   * */
  SimpleCursor importPosition(const DUContext* target) const;

  /**
   * Returns true if this context imports @param origin at any depth, else false.
   * */
  virtual bool imports(const DUContext* origin, const SimpleCursor& position = SimpleCursor::invalid()) const;

  /**
   * Adds an imported context.
   *
   * @param anonymous If this is true, the import will not be registered at the imported context. This allows du-chain contexts importing without having a write-lock.
   * @param position Position where the context is imported. This is mainly important in C++ with included files.
   *
   * If the context is already imported, only the position is updated.
   *
   * \note Be sure to have set the text location first, so that
   * the chain is sorted correctly.
   */
  virtual void addImportedParentContext(DUContext* context, const SimpleCursor& position = SimpleCursor::invalid(), bool anonymous = false);

  /**
   * Removes a child context.
   */
  virtual void removeImportedParentContext(DUContext* context);

  /**
   * Clear all imported parent contexts.
   */
  void clearImportedParentContexts();

  /**
   * If this is set to true, all declarations that are added to this context will also be visible in the parent-context.
   * They will be visible in the parent using findDeclarations(..), and findLocalDeclarations, but will not be in the list of localDeclarations(...).
   * */
  void setPropagateDeclarations(bool propagate);

  bool isPropagateDeclarations() const;

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
   * Searches for and returns a declaration with a given \a identifier in this context, which
   * is currently active at the given text \a position, with the given type \a dataType.
   * In fact, only items are returned that are declared BEFORE that position.
   *
   * \param identifier the identifier of the definition to search for
   * \param location the text position to search for
   * \param topContext the top-context from where a completion is triggered. This is needed so delayed types(templates in C++) can be resolved in the correct context.
   * \param type the type to match, or null for no type matching.
   *
   * \returns the requested declaration if one was found, otherwise null.
   *
   * \warning this may return declarations which are not in this tree, you may need to lock them too...
   */
  QList<Declaration*> findDeclarations(const QualifiedIdentifier& identifier, const SimpleCursor& position = SimpleCursor::invalid(), const AbstractType::Ptr& dataType = AbstractType::Ptr(), const TopDUContext* topContext = 0, SearchFlags flags = NoSearchFlags) const;

  /**
   * Searches for and returns a declaration with a given \a identifier in this context, which
   * is currently active at the given text \a position.
   *
   * \param identifier the identifier of the definition to search for
   * \param topContext the top-context from where a completion is triggered. This is needed so delayed types(templates in C++) can be resolved in the correct context.
   * \param location the text position to search for
   *
   * \returns the requested declaration if one was found, otherwise null.
   *
   * \warning this may return declarations which are not in this tree, you may need to lock them too...
   *
   * \overload
   */
  QList<Declaration*> findDeclarations(const Identifier& identifier, const SimpleCursor& position = SimpleCursor::invalid(), const TopDUContext* topContext = 0, SearchFlags flags = NoSearchFlags) const;

  /**
   * Returns the type of any \a identifier defined in this context, or
   * null if one is not found.
   *
   * Does not search imported parent-contexts(like base-classes).
   */
  QList<Declaration*> findLocalDeclarations(const QualifiedIdentifier& identifier, const SimpleCursor& position = SimpleCursor::invalid(), const TopDUContext* topContext = 0, const AbstractType::Ptr& dataType = AbstractType::Ptr(), bool allowUnqualifiedMatch = false, SearchFlags flags = NoSearchFlags) const;

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
   * Searches for the most specific context for the given cursor \a position in the given \a url.
   *
   * \param location the text position to search for
   * \param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * \returns the requested context if one was found, otherwise null.
   */
  DUContext* findContext(const SimpleCursor& position, DUContext* parent = 0) const;

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
  QList<DUContext*> findContexts(ContextType contextType, const QualifiedIdentifier& identifier, const SimpleCursor& position = SimpleCursor::invalid(), SearchFlags flags = NoSearchFlags) const;

  /**
   * Iterates the tree to see if the provided \a context is a subcontext of this context.
   * \returns true if \a context is a subcontext, otherwise false.
   */
  bool parentContextOf(DUContext* context) const;

  /**
   * Return a list of all reachable declarations for a given cursor \a position in a given \a url.
   *
   * \param location the text position to search for
   * \param topContext the top-context from where a completion is triggered. This is needed so delayed types(templates in C++) can be resolved in the correct context.
   * \param searchInParents should declarations from parent-contexts be listed? If false, only declarations from this and imported contexts will be returned.
   *
   * The returned declarations are paired together with their inheritance-depth, which is the count of steps
   * to into other contexts that were needed to find the declaration. Declarations reached through a namespace- or global-context
   * are offsetted by 1000.
   *
   * \returns the requested declarations, if any were active at that location. Declarations propagated into this context(@see setPropagateDeclarations) are included.
   */
  QList< QPair<Declaration*, int> > allDeclarations(const SimpleCursor& position, const TopDUContext* topContext, bool searchInParents=true) const;

  /**
   * Return all declarations in this context that have the given \a identifier, without any filtering.
   * */
  QList<Declaration*> allLocalDeclarations(const Identifier& identifier) const;

  /**
   * Delete and remove all slaves(uses, declarations, definitions, contexts) that are not in the given set
   */
  void cleanIfNotEncountered(const QSet<DUChainBase*>& encountered, bool firstPass);

  /**
   * Used exclusively by Declaration, do not use this.
   * */
  void changingIdentifier( Declaration* decl, const Identifier& from, const Identifier& to );

   /**
    * Uses:
    * A "Use" represents any position in a document where a Declaration is used literally.
    * For efficiency, since there can be many many uses, they are managed efficiently by
    * TopDUContext and DUContext. In TopDUContext, the used declarations are registered
    * and assigned a "Declaration-Index" while calling TopDUContext::indexForUsedDeclaration.
    * From such a declaration-index, the declaration can be retrieved back by calling TopDUContext::usedDeclarationForIndex.
    *
    * The actual uses are stored within DUContext, where each use consists of a range and the declaration-index of
    * the used declaration.
    *
    * To save memory, smart-ranges are only attached to the uses when the document is actually loaded.
    * Uses need to be ordered by their appearance.
    * */
  
  /**
   * Return a list of all uses which occur in this context.
   * To get the actual declarations, use TopDUContext::usedDeclarationForIndex(..) with the declarationIndex.
   */
  const QVector<Use>& uses() const;

  /**
   * Find the use which encompasses \a position, if one exists.
   * @return The local index of the use, or -1
   */
  int findUseAt(const SimpleCursor& position) const;
  
  /**
   * Returns the SmartRange assigned to the given use, or zero.
   * */
  KTextEditor::SmartRange* useSmartRange(int useIndex);
  
  /**
   * Assigns the given SmartRange to the given use.
   * If one use gets a smart range, all uses need to get a smart range.
   * The ownership of the range is given to this context.
   * */
  void setUseSmartRange(int useIndex, KTextEditor::SmartRange* range);
  
  /**
   * Assigns the declaration represented by @param declarationIndex to the use with index @param useIndex
   * */
  void setUseDeclaration(int useIndex, int declarationIndex);

  /**
   * Creates a new use of the declaration given  through @param declarationIndex.
   * The index must be retrieved through TopDUContext::indexForUsedDeclaration(..).
   * @param range The range of the use
   * @param smartRange The smart range, or zero if the document is not opened.
   * @param insertBefore A hint where in the vector of uses to insert the use.
   *                     Must be correct so the order is preserved(ordered by position),
   *                     or -1 to automatically choose the position.
   *
   * @return Local index of the created use
   * */
  int createUse(int declarationIndex, const SimpleRange& range, KTextEditor::SmartRange* smartRange, int insertBefore = -1);

  /**
   * Deletes the use number @param index . index is the position in the vector of uses, not a used declaration index.
   * */
  void deleteUse(int index);
  
  /**
   * Clear and delete all uses in this context.
   */
  void deleteUses();

  /**
   * Can be specialized by languages to create a navigation/information-widget.
   * Ideally, the widget would be based on KDevelop::QuickOpenEmbeddedWidgetInterface
   * for user-interaction within the quickopen list.
   *
   * The returned widget will be owned by the caller.
   *
   * @param decl A member-declaration of this context the navigation-widget should be created for
   * @param topContext Top-context from where the navigation-widget is triggered. In C++, this is needed to resolve forward-declarations.
   * @param htmlPrefix Html-formatted text that should be prepended before any information shown by this widget
   * @param htmlSuffux Html-formatted text that should be appended to any information shown by this widget
   *
   * Can return zero, which the default-implementation currently always does.
   * */
  virtual QWidget* createNavigationWidget(Declaration* decl = 0, TopDUContext* topContext = 0, const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString()) const;

  ///@todo Should be protected, moved here temporarily until I have figured out why the gcc 4.1.3 fails in cppducontext.h:212, which should work (within kdevelop)
  /// Declaration search implementation
  /**
   * This is a more complex interface to the declaration search engine.
   * Always prefer findDeclarations(..) when possible.
   * Advantage of this interface:
   * - You can search multiple identifiers at one time. However, those should be aliased identifiers for one single item, because
   *   search might stop as soon as one item is found.
   * - You can give an ImportTrace to correctly resolve template-paremeters.
   * @param position A valid position, if in doubt use textRange().end()
   *
   * @warning position Must be valid!
   * @return whether the search was successful. If it is false, it had to be stopped for special reasons(like some flags)
   * */
  virtual bool findDeclarationsInternal(const QList<QualifiedIdentifier>& identifiers, const SimpleCursor& position, const AbstractType::Ptr& dataType, QList<Declaration*>& ret, const ImportTrace& trace, SearchFlags flags ) const;

  protected:

  /**
   * After one scope was searched, this function is asked whether more results should be collected. Override it, for example to collect overloaded functions.
   * The default-implementation returns true as soon as decls is not empty.
   * */
  virtual bool foundEnough( const QList<Declaration*>& decls ) const;
  /**
   * Merges definitions and their inheritance-depth up all branches of the definition-use chain into one hash.
   * @param hadUrls is used to count together all contexts that already were visited, so they are not visited again.
   */
  virtual void mergeDeclarationsInternal(QList< QPair<Declaration*, int> >& definitions, const SimpleCursor& position, QHash<const DUContext*, bool>& hadContexts, const ImportTrace& trace, bool searchInParents = true, int currentDepth = 0) const;

  /// Logic for calculating the fully qualified scope name
  QualifiedIdentifier scopeIdentifierInternal(DUContext* context) const;

  virtual void findLocalDeclarationsInternal( const QualifiedIdentifier& identifier, const SimpleCursor & position, const AbstractType::Ptr& dataType, bool allowUnqualifiedMatch, QList<Declaration*>& ret, const ImportTrace& trace, SearchFlags flags ) const;

  /// Context search implementation
  virtual void findContextsInternal(ContextType contextType, const QList<QualifiedIdentifier>& identifier, const SimpleCursor& position, QList<DUContext*>& ret, SearchFlags flags = NoSearchFlags) const;

  /**Applies namespace-imports and namespace-aliases and returns possible absolute identifiers that need to be searched.
   * @param targetIdentifiers will be filled with all identifiers that should be searched for, instead of identifier.
   * */
  void applyAliases(const QList<QualifiedIdentifier>& identifier, QList<QualifiedIdentifier>& targetIdentifiers, const SimpleCursor& position, bool canBeNamespace) const;

  /**
   * Applies the aliases that need to be applied when moving the search from this context up to the parent-context.
   * The default-implementation adds a set of identifiers with the own local identifier prefixed, if this is a namespace.
   * For C++, this is needed when searching out of a namespace, so the item can be found within that namespace in another place.
   * */
  virtual void applyUpwardsAliases(QList<QualifiedIdentifier>& identifiers) const;
  
  DUContext(DUContextPrivate& dd, const HashedString& url, const SimpleRange& range, DUContext* parent = 0, bool anonymous = false);
  
private:
  virtual void rangePositionChanged(KTextEditor::SmartRange* range);
  virtual void rangeContentsChanged(KTextEditor::SmartRange* range, KTextEditor::SmartRange* range2);
  virtual void rangeContentsChanged(KTextEditor::SmartRange* range);
  virtual void rangeDeleted(KTextEditor::SmartRange* range);

  Q_DECLARE_PRIVATE(DUContext)
};

/**
 * This is the identifier that can be used to search namespace-import declarations, and should be used to store namespace-imports.
 * It is stored statically for performance-reasons, so it doesn't need to be constructed every time it is used.
 *
 * @see NamespaceAliasDeclaration.
 * */
KDEVPLATFORMLANGUAGE_EXPORT extern const Identifier globalImportIdentifier;

/**
  * Collects all uses of the given @param declarationIndex
  * */
QList<SimpleRange> allUses(DUContext* context, int declarationIndex);

/**
  * Collects the smart-ranges of all uses of the given @param declarationIndex
  * */
QList<KTextEditor::SmartRange*> allSmartUses(DUContext* context, int declarationIndex);

}

#endif // DUCONTEXT_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
