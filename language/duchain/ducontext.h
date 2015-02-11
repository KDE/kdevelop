/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_DUCONTEXT_H
#define KDEVPLATFORM_DUCONTEXT_H

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QVector>

#include <util/kdevvarlengtharray.h>

#include "identifier.h"
#include "duchainbase.h"
#include "types/abstracttype.h"
#include "duchainpointer.h"
#include "declarationid.h"
#include "indexedducontext.h"

class QWidget;

namespace KDevelop
{

class Declaration;
class DUChain;
class Use;
class TopDUContext;
class DUContext;
class DUContextData;

class KDEVPLATFORMLANGUAGE_EXPORT DUChainVisitor {
public:
  virtual void visit(DUContext* context) = 0;
  virtual void visit(Declaration* declaration) = 0;
  virtual ~DUChainVisitor();
};

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
 * @todo change child relationships to a linked list within the context?
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUContext : public DUChainBase
{
  friend class Use;
  friend class Declaration;
  friend class DeclarationData;
  friend class DUContextData;
  friend class DUContextDynamicData;
  friend class Definition;
  friend class VisibleDeclarationIterator;

public:
  /**
   * Constructor. No convenience methods, as the initialisation order is important,
   *
   * @param anonymous Whether the context should be added as an anonymous context to the parent. That way the context can never be found through any of the parent's member-functions.
   *
   * If the parent is in the symbol table and the context is not anonymous, it will also be added to the symbol table. You nead a write-lock to the DUChain then
   */
  explicit DUContext(const RangeInRevision& range, DUContext* parent = 0, bool anonymous = false);
  explicit DUContext(DUContextData&);

  /**
   * Destructor. Will delete all child contexts which are defined within
   * the same file as this context.
   */
  virtual ~DUContext();

  enum ContextType {
    Global    /**< A context that declares functions, namespaces or classes */,
    Namespace /**< A context that declares namespace members */,
    Class     /**< A context that declares class members */,
    Function  /**< A context that declares function-arguments */,
    Template  /**< A context that declares template-parameters */,
    Enum      /**< A context that contains a list of enumerators */,
    Helper    /**< A helper context. This context is treated specially during search:
               *   when searching within the imports of a context, and that context's parent
               *   is a context of type DUContext::Helper, then the upwards search is continued
               *   into that helper(The decision happens in shouldSearchInParent)  */,
    Other     /**< Represents executable code, like for example within a compound-statement */
  };

  enum SearchFlag {
    NoSearchFlags = 0             /**< Searching for everything */,
    InImportedParentContext = 1   /**< Internal, do not use from outside */,
    OnlyContainerTypes = 2        /**< Not implemented yet */,
    DontSearchInParent = 4        /**< IF  this flag is set, findDeclarations(..) will not search for the identifier in parent-contexts(which does not include imported parent-contexts) */,
    NoUndefinedTemplateParams = 8 /**< For languages that support templates(like C++). If this is set, the search should fail as soon as undefined template-parameters are involved. */,
    DirectQualifiedLookup = 16    /**< When this flag is used, the searched qualified identifier should NOT be split up into it's components and looked up one by one. Currently only plays a role in C++ specific parts. */,
    NoFiltering = 32              /**< Should be set when no filtering at all is wished, not even filtering that is natural for the underlying language(For example in C++, constructors are filtered out be default) */,
    OnlyFunctions = 64            /**< When this is given, only function-declarations are returned. In case of C++, this also means that constructors can be retrieved, while normally they are filtered out. */,
    NoImportsCheck = 128          /**< With this parameter, a global search will return all matching items, from all contexts, not only from imported ones. */,
    NoSelfLookUp = 256            /**< With this parameter, the special-treatment during search that allows finding the context-class by its name is disabled. */,
    DontResolveAliases = 512      /**< Disables the resolution of alias declarations in the returned list*/,
    LastSearchFlag = 1024
  };

  Q_DECLARE_FLAGS(SearchFlags, SearchFlag)


  ContextType type() const;
  void setType(ContextType type);

  /**
   * If this context was opened by a declaration or definition, this returns that item.
   *
   * The returned declaration/definition will have this context set as @c internalContext()
   */
  Declaration* owner() const;
  /**
   * Sets the declaration/definition, and also updates it's internal context (they are strictly paired together).
   *
   * The declaration has to be part of the same top-context.
   */
  void setOwner(Declaration* decl);

  /**
   * Calculate the depth of this context, from the top level context in the file.
   */
  int depth() const;

  /**
   * Find the top context.
   */
  virtual TopDUContext* topContext() const override;

  /**
   * Visits all duchain objects in the whole duchain.
   *
   * Classes that hold a unique link to duchain objects like instantiations
   * have to pass the visitor over to those classes.
   * */
  virtual void visit(DUChainVisitor& visitor);

  /**
   * Find the context which most specifically covers @a position.
   *
   * The search is recursive, so the most specific context is found.
   *
   * @param includeRightBorder When this is true, contexts will also be found that
   *                           have the position on their right border.
   *
   * @warning This uses the ranges in the local revision of the document (at last parsing time).
   *          Use DUChainBase::transformToLocalRevision to transform the cursor into that revision first.
   */
  DUContext* findContextAt(const CursorInRevision& position, bool includeBorders = false) const;

  /**
   * Find a child declaration that has a rang that covers the given @a position.
   *
   * The search is local, not recursive.
   *
   * @warning This uses the ranges in the local revision of the document (at last parsing time).
   *          Use DUChainBase::transformToLocalRevision to transform the cursor into that revision first.
   */
  Declaration* findDeclarationAt(const CursorInRevision& position) const;

  /**
   * Find the context which most specifically covers @a range.
   *
   * @warning This uses the ranges in the local revision of the document (at last parsing time).
   *          Use DUChainBase::transformToLocalRevision to transform the cursor into that revision first.
   */
  DUContext* findContextIncluding(const RangeInRevision& range) const;

  /**
   * Calculate the fully qualified scope identifier.
   */
  QualifiedIdentifier scopeIdentifier(bool includeClasses = false) const;

  /**
   * Returns true if this context has the same scope identifier as the given one.
   *
   * @note This is much more efficient than computing the identifiers through @c scopeIdentifier(..)
   * and comparing them
   */
  bool equalScopeIdentifier(const DUContext* rhs) const;

  /**
   * Scope identifier, used to qualify the identifiers occurring in each context.
   *
   * This is the part relative to the parent context.
   */
  QualifiedIdentifier localScopeIdentifier() const;

  /**
   * Same as @c localScopeIdentifier(), but faster.
   */
  IndexedQualifiedIdentifier indexedLocalScopeIdentifier() const;

  /**
   * Scope identifier, used to qualify the identifiers occurring in each context
   * This must not be called once this context has children.
   */
  void setLocalScopeIdentifier(const QualifiedIdentifier& identifier);

  /**
   * Returns whether this context is listed in the symbol table (Namespaces and classes)
   */
  bool inSymbolTable() const;

  /**
   * Move this object into/out of the symbol table.
   *
   * @note You need to have a duchain write lock, unless this is a TopDUContext.
   */
  void setInSymbolTable(bool inSymbolTable);

  /**
   * Returns the immediate parent context of this context.
   */
  DUContext* parentContext() const;

  /**
   * Represents an imported parent context.
   */
  struct KDEVPLATFORMLANGUAGE_EXPORT Import {
    /**
     * @note DUChain must be read-locked when this is called
     */
    Import(DUContext* context, const DUContext* importer,
           const CursorInRevision& position = CursorInRevision::invalid());
    Import() : position(CursorInRevision::invalid()) { }
    Import(const DeclarationId& id, const CursorInRevision& position = CursorInRevision::invalid());

    bool operator==(const Import& rhs) const {
      return m_context == rhs.m_context && m_declaration == rhs.m_declaration;
    }

    /**
     * @param topContext The top-context from where to start searching.
     *                   This is important to find the correct imports
     *                   in the case of templates or similar structures.
     */
    DUContext* context(const TopDUContext* topContext, bool instantiateIfRequired = true) const;

    /**
     * Returns the top-context index, if this import is not a specialization import.
     */
    uint topContextIndex() const {
      return m_context.topContextIndex();
    }

    IndexedDUContext indexedContext() const {
      return m_context;
    }

    /**
     * Returns true if this import is direct.
     *
     * That is, the import is not referred to by its identifier,
     * but rather directly by its index.
     */
    bool isDirect() const;

    /**
     * If this import is indirect, returns the imported declaration-id
     */
    DeclarationId indirectDeclarationId() const {
      return m_declaration;
    }

      CursorInRevision position;

    private:
      //Either we store m_declaration, or m_context. That way we can resolve specialized contexts.
      ///@todo Compress using union
      DeclarationId m_declaration;
      IndexedDUContext m_context;
  };

  /**
   * Returns the list of imported parent contexts for this context.
   *
   * @warning The list may contain objects that are not valid any more,
   *          i.e. data() returns zero, @see addImportedParentContext)
   * @warning The import structure may contain loops if this is a TopDUContext,
   *          so be careful when traversing the tree.
   * @note This is expensive.
   */
  virtual QVector<Import> importedParentContexts() const;

  /**
   * If the given context is directly imported into this one, and
   * @c addImportedParentContext(..) was called with a valid cursor,
   * this will return that position. Otherwise an invalid cursor is returned.
   */
  virtual CursorInRevision importPosition(const DUContext* target) const;

  /**
   * Returns true if this context imports @param origin at any depth, else false.
   */
  virtual bool imports(const DUContext* origin,
                       const CursorInRevision& position = CursorInRevision::invalid()) const;

  /**
   * Adds an imported context.
   *
   * @param anonymous If this is true, the import will not be registered at the imported context.
   *                  This allows du-chain contexts importing without having a write-lock.
   * @param position Position where the context is imported. This is mainly important in C++ with included files.
   *
   * If the context is already imported, only the position is updated.
   *
   * @note Be sure to have set the text location first, so that the chain is sorted correctly.
   */
  virtual void addImportedParentContext(DUContext* context,
                                        const CursorInRevision& position = CursorInRevision::invalid(),
                                        bool anonymous = false, bool temporary = false);

  /**
   * Adds an imported context, which may be indirect.
   *
   * @warning This is only allowed if this context is _NOT_ a top-context.
   * @warning When using this mechanism, this context will not be registered as importer to the other one.
   * @warning The given import _must_ be indirect.
   *
   * @return true if the import was already imported before, else false.
   */
  bool addIndirectImport(const DUContext::Import& import);

  /**
   * Removes a child context.
   */
  virtual void removeImportedParentContext(DUContext* context);

  /**
   * Clear all imported parent contexts.
   */
  virtual void clearImportedParentContexts();

  /**
   * If this is set to true, all declarations that are added to this context
   * will also be visible in the parent-context.
   *
   * They will be visible in the parent using @c findDeclarations(...) and
   * @c findLocalDeclarations(...), but will not be in the list of @c localDeclarations(...).
   */
  void setPropagateDeclarations(bool propagate);

  bool isPropagateDeclarations() const;

  /**
   * Returns the list of contexts importing this context.
   *
   * @note Very expensive, since the importers top-contexts need to be loaded.
   */
  virtual QVector<DUContext*> importers() const;

  /**
   * Returns the list of indexed importers.
   *
   * Cheap, because nothing needs to be loaded.
   */
  KDevVarLengthArray<IndexedDUContext> indexedImporters() const;

  /**
   * Returns the list of immediate child contexts for this context.
   *
   * @note This is expensive.
   */
  QVector<DUContext*> childContexts() const;

  /**
   * Clears and deletes all child contexts recursively.
   *
   * This will not cross file boundaries.
   */
  void deleteChildContextsRecursively();

  /**
   * Returns true if this declaration is accessible through the du-chain,
   * and thus cannot be edited without a du-chain write lock
   */
  virtual bool inDUChain() const;

  /**
   * Retrieve the context which is specialized with the given
   * @a specialization as seen from the given @a topContext.
   *
   * @param specialization the specialization index (see DeclarationId)
   * @param topContext the top context representing the perspective from which to specialize.
   *                   if @p topContext is zero, only already existing specializations are returned,
   *                   and if none exists, zero is returned.
   * @param upDistance upwards distance in the context-structure of the
   *                   given specialization-info. This allows specializing children.
   */
  virtual DUContext* specialize(const IndexedInstantiationInformation& specialization,
                                const TopDUContext* topContext, int upDistance = 0);

  /**
   * Searches for and returns a declaration with a given @a identifier in this context, which
   * is currently active at the given text @a position, with the given type @a dataType.
   * In fact, only items are returned that are declared BEFORE that position.
   *
   * @param identifier the identifier of the definition to search for
   * @param location the text position to search for
   * @param topContext the top-context from where a completion is triggered.
   *                   This is needed so delayed types (templates in C++) can be resolved in the correct context.
   * @param type the type to match, or null for no type matching.
   *
   * @returns the requested declaration if one was found, otherwise null.
   *
   * @warning this may return declarations which are not in this tree, you may need to lock them too...
   */
  QList<Declaration*> findDeclarations(const QualifiedIdentifier& identifier,
                                       const CursorInRevision& position = CursorInRevision::invalid(),
                                       const AbstractType::Ptr& dataType = AbstractType::Ptr(),
                                       const TopDUContext* topContext = 0,
                                       SearchFlags flags = NoSearchFlags) const;

  /**
   * Searches for and returns a declaration with a given @a identifier in this context, which
   * is currently active at the given text @a position.
   *
   * @param identifier the identifier of the definition to search for
   * @param topContext the top-context from where a completion is triggered.
   *                   This is needed so delayed types(templates in C++) can be resolved in the correct context.
   * @param location the text position to search for
   *
   * @returns the requested declaration if one was found, otherwise null.
   *
   * @warning this may return declarations which are not in this tree, you may need to lock them too...
   *
   * @overload
   */
  QList<Declaration*> findDeclarations(const IndexedIdentifier& identifier,
                                       const CursorInRevision& position = CursorInRevision::invalid(),
                                       const TopDUContext* topContext = 0,
                                       SearchFlags flags = NoSearchFlags) const;

  /**
   * Prefer the version above for speed reasons.
   */
  QList<Declaration*> findDeclarations(const Identifier& identifier,
                                       const CursorInRevision& position = CursorInRevision::invalid(),
                                       const TopDUContext* topContext = 0,
                                       SearchFlags flags = NoSearchFlags) const;

  /**
   * Returns the type of any @a identifier defined in this context, or
   * null if one is not found.
   *
   * Does not search imported parent-contexts(like base-classes).
   */
  QList<Declaration*> findLocalDeclarations(const IndexedIdentifier& identifier,
                                            const CursorInRevision& position = CursorInRevision::invalid(),
                                            const TopDUContext* topContext = 0,
                                            const AbstractType::Ptr& dataType = AbstractType::Ptr(),
                                            SearchFlags flags = NoSearchFlags) const;

  /**
   * Prefer the version above for speed reasons.
   */
  QList<Declaration*> findLocalDeclarations(const Identifier& identifier,
                                            const CursorInRevision& position = CursorInRevision::invalid(),
                                            const TopDUContext* topContext = 0,
                                            const AbstractType::Ptr& dataType = AbstractType::Ptr(),
                                            SearchFlags flags = NoSearchFlags) const;

  /**
   * Clears all local declarations.
   *
   * Does not delete the declaration; the caller assumes ownership.
   */
  QVector<Declaration*> clearLocalDeclarations();

  /**
   * Clears all local declarations.
   *
   * Deletes these declarations, as the context has ownership.
   */
  void deleteLocalDeclarations();

  /**
   * Returns all local declarations
   *
   * @param source A source-context that is needed to instantiate template-declarations in some cases.
   *               If it is zero, that signalizes that missing members should not be instantiated.
   */
  virtual QVector<Declaration*> localDeclarations(const TopDUContext* source = 0) const;

  /**
   * Searches for the most specific context for the given cursor @a position in the given @a url.
   *
   * @param location the text position to search for
   * @param parent the parent context to search from (this is mostly an internal detail, but if you only
   *               want to search in a subbranch of the chain, you may specify the parent here)
   *
   * @returns the requested context if one was found, otherwise null.
   */
  DUContext* findContext(const CursorInRevision& position, DUContext* parent = 0) const;

  /**
   * Iterates the tree to see if the provided @a context is a subcontext of this context.
   *
   * @returns true if @a context is a subcontext, otherwise false.
   */
  bool parentContextOf(DUContext* context) const;

  /**
   * Return a list of all reachable declarations for a given cursor @a position in a given @a url.
   *
   * @param location the text position to search for
   * @param topContext the top-context from where a completion is triggered.
   *                   This is needed so delayed types(templates in C++) can be resolved
   *                   in the correct context.
   * @param searchInParents should declarations from parent-contexts be listed?
   *                        If false, only declarations from this and imported contexts will be returned.
   *
   * The returned declarations are paired together with their inheritance-depth,
   * which is the count of steps to into other contexts that were needed to find the declaration.
   * Declarations reached through a namespace- or global-context are offsetted by 1000.
   *
   * This also includes Declarations from sub-contexts that were propagated upwards
   * using @c setPropagateDeclarations(true).
   *
   * @returns the requested declarations, if any were active at that location.
   *          Declarations propagated into this context(@c setPropagateDeclarations) are included.
   */
  QList< QPair<Declaration*, int> > allDeclarations(const CursorInRevision& position,
                                                    const TopDUContext* topContext,
                                                    bool searchInParents = true) const;

  /**
   * Delete and remove all slaves (uses, declarations, definitions, contexts) that are not in the given set.
   */
  void cleanIfNotEncountered(const QSet<DUChainBase*>& encountered);

   /**
    * Uses:
    * A "Use" represents any position in a document where a Declaration is used literally.
    * For efficiency, since there can be many many uses, they are managed efficiently by
    * TopDUContext and DUContext. In TopDUContext, the used declarations are registered
    * and assigned a "Declaration-Index" while calling TopDUContext::indexForUsedDeclaration.
    * From such a declaration-index, the declaration can be retrieved back by calling
    * @c TopDUContext::usedDeclarationForIndex.
    *
    * The actual uses are stored within DUContext, where each use consists of a range and
    * the declaration-index of the used declaration.
    * */

  /**
   * Return a vector of all uses which occur in this context.
   *
   * To get the actual declarations, use @c TopDUContext::usedDeclarationForIndex(..)
   * with the declarationIndex.
   */
  const Use* uses() const;

  /**
   * Returns the count of uses that can be accessed through @c uses()
   */
  int usesCount() const;

  /**
   * Determines whether the given declaration has uses or not
   */
  static bool declarationHasUses(Declaration* decl);

  /**
   * Find the use which encompasses @a position, if one exists.
   * @return The local index of the use, or -1
   */
  int findUseAt(const CursorInRevision& position) const;

  /**
   * @note The change must not break the ordering
   */
  void changeUseRange(int useIndex, const RangeInRevision& range);

  /**
   * Assigns the declaration represented by @param declarationIndex
   * to the use with index @param useIndex.
   */
  void setUseDeclaration(int useIndex, int declarationIndex);

  /**
   * Creates a new use of the declaration given  through @param declarationIndex.
   * The index must be retrieved through @c TopDUContext::indexForUsedDeclaration(..).
   *
   * @param range The range of the use
   * @param insertBefore A hint where in the vector of uses to insert the use.
   *                     Must be correct so the order is preserved(ordered by position),
   *                     or -1 to automatically choose the position.
   *
   * @return Local index of the created use
   */
  int createUse(int declarationIndex, const RangeInRevision& range, int insertBefore = -1);

  /**
   * Deletes the use number @param index.
   *
   * @param index is the position in the vector of uses, not a used declaration index.
   */
  void deleteUse(int index);

  /**
   * Clear and delete all uses in this context.
   */
  virtual void deleteUses();

  /**
   * Recursively delete all uses in this context and all its child-contexts
   */
  virtual void deleteUsesRecursively();

  /**
   * Can be specialized by languages to create a navigation/information-widget.
   *
   * Ideally, the widget would be based on @c KDevelop::QuickOpenEmbeddedWidgetInterface
   * for user-interaction within the quickopen list.
   *
   * The returned widget will be owned by the caller.
   *
   * @param decl A member-declaration of this context the navigation-widget should be created for.
   *            Zero to create a widget for this context.
   * @param topContext Top-context from where the navigation-widget is triggered.
   *                   In C++, this is needed to resolve forward-declarations.
   * @param htmlPrefix Html-formatted text that should be prepended before any information shown by this widget
   * @param htmlSuffix Html-formatted text that should be appended to any information shown by this widget
   *
   * Can return zero which disables the navigation widget.
   *
   * If you setProperty("DoNotCloseOnCursorMove", true) on the widget returned,
   * then the widget will not close when the cursor moves in the document, which
   * enables you to change the document contents from the widget without immediately closing the widget.
   */
  virtual QWidget* createNavigationWidget(Declaration* decl = 0, TopDUContext* topContext = 0,
                                          const QString& htmlPrefix = QString(),
                                          const QString& htmlSuffix = QString()) const;

  enum {
    Identity = 2
  };

  /**
   * Represents multiple qualified identifiers in a way that is better
   * to manipulate and allows applying namespace-aliases or -imports easily.
   *
   * A SearchItem generally represents a tree of identifiers, and represents
   * all the qualified identifiers that can be constructed by walking
   * along the tree starting at an arbitrary root-node into the depth using the "next" pointers.
   *
   * The insertion order in the hierarchy determines the order of the represented list.
   */
  struct KDEVPLATFORMLANGUAGE_EXPORT SearchItem : public QSharedData
  {
    typedef QExplicitlySharedDataPointer<SearchItem> Ptr;
    typedef KDevVarLengthArray<Ptr, 256> PtrList;

    /**
     * Constructs a representation of the given @param id qualified identifier,
     * starting at its index @param start.
     *
     * @param nextItem is set as next item to the last item in the chain
     */
    SearchItem(const QualifiedIdentifier& id, const Ptr& nextItem = Ptr(), int start = 0);

    /**
     * Constructs a representation of the given @param id qualified identifier,
     * starting at its index @param start.
     *
     * @param nextItem is set as next item to the last item in the chain
     */
    SearchItem(const QualifiedIdentifier& id, const PtrList& nextItems, int start = 0);

    SearchItem(bool explicitlyGlobal, const IndexedIdentifier& id, const PtrList& nextItems);
    SearchItem(bool explicitlyGlobal, const IndexedIdentifier& id, const Ptr& nextItem);

    bool isEmpty() const;
    bool hasNext() const;

    /**
     * Appends the given item to every item that can be reached from this item,
     * and not only to the end items.
     *
     * The effect to search is that the given item is searched with all prefixes
     * contained in this earch-item prepended.
     *
     * @warning This changes all contained sub-nodes, but they can be shared with
     *          other SearchItem trees. You should not use this on SearchItem trees
     *          that have shared nodes with other trees.
     *
     * @note These functions ignore explicitly global items.
     */
    void addToEachNode(const Ptr& item);
    void addToEachNode(const PtrList& items);

    /**
     * Returns true if the given identifier matches one of the identifiers
     * represented by this SearchItem. Does not respect the explicitlyGlobal flag
     */
    bool match(const QualifiedIdentifier& id, int offset = 0) const;

    /**
     * @note expensive
     */
    QList<QualifiedIdentifier> toList(const QualifiedIdentifier& prefix = QualifiedIdentifier()) const;

    void addNext(const Ptr& other);

    bool isExplicitlyGlobal;
    IndexedIdentifier identifier;
    PtrList next;
  };

  ///@todo Should be protected, moved here temporarily until I have figured
  ///out why the gcc 4.1.3 fails in cppducontext.h:212, which should work (within kdevelop)

  /// Declaration search implementation

  /**
   * This is a more complex interface to the declaration search engine.
   *
   * Always prefer @c findDeclarations(..) when possible.
   *
   * Advantage of this interface:
   * - You can search multiple identifiers at one time.
   *   However, those should be aliased identifiers for one single item, because
   *   search might stop as soon as one item is found.
   * - The source top-context is needed to correctly resolve template-parameters
   *
   * @param position A valid position, if in doubt use textRange().end()
   *
   * @warning @p position must be valid!
   *
   * @param depth Depth of the search in parents. This is used to prevent endless
   *              recursions in endless import loops.
   *
   *
   * @return whether the search was successful. If it is false, it had to be stopped
   *         for special reasons (like some flags)
   */
  typedef QList<Declaration*> DeclarationList;

  virtual bool findDeclarationsInternal(const SearchItem::PtrList& identifiers,
                                        const CursorInRevision& position, const AbstractType::Ptr& dataType,
                                        DeclarationList& ret, const TopDUContext* source, SearchFlags flags,
                                        uint depth ) const;

  /**
   * Returns the qualified identifier @p id with all aliases (for example namespace imports) applied
   *
   * @example: If the namespace 'Foo' is imported, and id is 'Bar',
   *           then the returned list is 'Bar' and 'Foo::Bar'
   */
  QList<QualifiedIdentifier> fullyApplyAliases(const QualifiedIdentifier& id,
                                               const TopDUContext* source) const;

protected:

  /**
   * After one scope was searched, this function is asked whether more
   * results should be collected. Override it, for example to collect overloaded functions.
   *
   * The default-implementation returns true as soon as decls is not empty.
   */
  virtual bool foundEnough( const DeclarationList& decls , SearchFlags flags ) const;

  /**
   * Merges definitions and their inheritance-depth up all branches of the
   * definition-use chain into one hash.
   *
   * This includes declarations propagated from sub-contexts.
   *
   * @param hadUrls is used to count together all contexts that already were
   *                visited, so they are not visited again.
   */
  virtual void mergeDeclarationsInternal(QList< QPair<Declaration*, int> >& definitions,
                                         const CursorInRevision& position,
                                         QHash<const DUContext*, bool>& hadContexts,
                                         const TopDUContext* source,
                                         bool searchInParents = true, int currentDepth = 0) const;

  void findLocalDeclarationsInternal(const Identifier& identifier,
                                     const CursorInRevision & position,
                                     const AbstractType::Ptr& dataType,
                                     DeclarationList& ret,
                                     const TopDUContext* source,
                                     SearchFlags flags ) const;

  virtual void findLocalDeclarationsInternal(const IndexedIdentifier& identifier,
                                             const CursorInRevision & position,
                                             const AbstractType::Ptr& dataType,
                                             DeclarationList& ret,
                                             const TopDUContext* source,
                                             SearchFlags flags ) const;

  /**
   * Applies namespace-imports and namespace-aliases and returns
   * possible absolute identifiers that need to be searched.
   *
   * @param targetIdentifiers will be filled with all identifiers that should
   *                          be searched for, instead of identifier.
   * @param onlyImports if this is true, namespace-aliases will not be respected,
   *                    but only imports. This is faster.
   */
  void applyAliases(const SearchItem::PtrList& identifiers, SearchItem::PtrList& targetIdentifiers,
                    const CursorInRevision& position, bool canBeNamespace, bool onlyImports = false) const;
  /**
   * Applies the aliases that need to be applied when moving the search
   * from this context up to the parent-context.
   *
   * The default-implementation adds a set of identifiers with the own local
   * identifier prefixed, if this is a namespace.
   *
   * For C++, this is needed when searching out of a namespace, so the item
   * can be found within that namespace in another place.
   */
  virtual void applyUpwardsAliases(SearchItem::PtrList& identifiers, const TopDUContext* source) const;

  DUContext(DUContextData& dd, const RangeInRevision& range, DUContext* parent = 0, bool anonymous = false);

  /**
   * Just uses the data from the given context. Doesn't copy or change anything,
   * and the data will not be deleted on this contexts destruction.
   */
  DUContext(DUContext& useDataFrom);

  /**
   * Whether this context, or any of its parent contexts, has been inserte
   * anonymously into the du-chain
   *
   * @see DUContext::DUContext
   */
  bool isAnonymous() const;

  /**
   * This is called whenever the search needs to do the decision whether it
   * should be continued in the parent context.
   *
   * It is not called when the DontSearchInParent flag is set. Else this should
   * be overridden to do language-specific logic.
   *
   * The default implementation returns false if the flag InImportedParentContext is set.
   */
  virtual bool shouldSearchInParent(SearchFlags flags) const;

private:
  void rebuildDynamicData(DUContext* parent, uint ownIndex) override;

  friend class TopDUContext;
  friend class IndexedDUContext;
  friend class LocalIndexedDUContext;
  friend class TopDUContextDynamicData;

  DUCHAIN_DECLARE_DATA(DUContext)
  class DUContextDynamicData* m_dynamicData;
};

/**
 * This is the identifier that can be used to search namespace-import declarations,
 * and should be used to store namespace-imports.
 *
 * It is stored statically for performance-reasons, so it doesn't need to be
 * constructed every time it is used.
 *
 * @see NamespaceAliasDeclaration.
 */
KDEVPLATFORMLANGUAGE_EXPORT const Identifier& globalImportIdentifier();

/**
 * This is the identifier that can be used to search namespace-alias declarations.
 *
 * It is stored statically for performance-reasons, so it doesn't need to be
 * constructed every time it is used.
 *
 * @see NamespaceAliasDeclaration.
 */
KDEVPLATFORMLANGUAGE_EXPORT const Identifier& globalAliasIdentifier();

/**
 * This is the identifier that can be used to search namespace-import declarations,
 * and should be used to store namespace-imports.
 *
 * It is stored statically for performance-reasons, so it doesn't need to be
 * constructed every time it is used.
 *
 * @see NamespaceAliasDeclaration.
 */
KDEVPLATFORMLANGUAGE_EXPORT const IndexedIdentifier& globalIndexedImportIdentifier();

/**
 * This is the identifier that can be used to search namespace-alias declarations.
 *
 * It is stored statically for performance-reasons, so it doesn't need to be
 * constructed every time it is used.
 *
 * @see NamespaceAliasDeclaration.
 */
KDEVPLATFORMLANGUAGE_EXPORT const IndexedIdentifier& globalIndexedAliasIdentifier();

/**
 * Collects all uses of the given @param declarationIndex
 */
KDEVPLATFORMLANGUAGE_EXPORT QList<RangeInRevision> allUses(DUContext* context,
                                                           int declarationIndex,
                                                           bool noEmptyRanges = false);
}

Q_DECLARE_TYPEINFO(KDevelop::DUContext::Import, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_DUCONTEXT_H
