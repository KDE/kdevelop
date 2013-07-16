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

#ifndef KDEVPLATFORM_TOPDUCONTEXT_H
#define KDEVPLATFORM_TOPDUCONTEXT_H

#include <QtCore/QMutex>

#include "ducontext.h"
#include <language/util/setrepository.h>
#include <QMetaType>

template< class T >
class KSharedPtr;

namespace KDevelop
{
  class IAstContainer;
  class QualifiedIdentifier;
  class DUChain;
  class ParsingEnvironmentFile;
  class TopDUContextData;
  class TopDUContextLocalPrivate;
  class IndexedTopDUContext;
//   class TopDUContextDynamicData;
  class Problem;
  class DeclarationChecker;
  class TopDUContext;

  struct KDEVPLATFORMLANGUAGE_EXPORT RecursiveImportRepository {
    static Utils::BasicSetRepository* repository();
  };
  
  ///Maps an imported top-context to a pair:
  ///1. The distance to the top-context, and 2. The next step towards the top-context
  ///in the chain.
  typedef QHash<const TopDUContext*, QPair<int, const TopDUContext*> > RecursiveImports;
  
  typedef DUChainPointer<TopDUContext> TopDUContextPointer;

  typedef KSharedPtr<Problem> ProblemPointer;

///KDevelop can unload unused top-context at any time. To prevent unloading,
///keep a ReferencedTopDUContext.
class KDEVPLATFORMLANGUAGE_EXPORT ReferencedTopDUContext {
  public:
    ReferencedTopDUContext(TopDUContext* context = 0);
    ReferencedTopDUContext(const ReferencedTopDUContext& rhs);
    ~ReferencedTopDUContext();
    
    ReferencedTopDUContext& operator=(const ReferencedTopDUContext& rhs);
    
    inline TopDUContext* data() const {
      return m_topContext;
    }
    
    inline operator TopDUContext*() const {
      return m_topContext;
    }
    
    inline bool operator==(const ReferencedTopDUContext& rhs) const {
      return m_topContext == rhs.m_topContext;
    }
    
    inline bool operator!=(const ReferencedTopDUContext& rhs) const {
      return m_topContext != rhs.m_topContext;
    }
    
    inline TopDUContext* operator->() const {
      return m_topContext;
    }
    
    inline uint hash() const {
      return (uint)(((quint64)m_topContext) * 37);
    }
    
  private:
  TopDUContext* m_topContext;
};

/**
 * The top context in a definition-use chain for one source file.
 *
 * Implements SymbolTable lookups and locking for the chain.
 *
 * Contexts and Classes can only be found through TopDUContext if they are in the symbol table.
 * @see DUContext::setInSymbolTable, Declaration::setInSymbolTable
 *
 * \todo move the registration with DUChain here
 *
 * @warning Do not delete top-contexts directly, use DUChain::removeDocumentChain instead.
 */
class KDEVPLATFORMLANGUAGE_EXPORT TopDUContext : public DUContext
{
public:
  explicit TopDUContext(const IndexedString& url, const RangeInRevision& range, ParsingEnvironmentFile* file = 0);
  explicit TopDUContext(TopDUContextData& data);
  
  /**This creates a top-context that shares most of its data with @param sharedDataFrom. The given context must be the owner of the data
   * (it must not have been created with this constructor).
   * 
   * When creating a context like this, all the data is shared among the context, except:
   * parsingEnvironmentFile, ownIndex, problems, imports and importers.
   * 
   * When you change any other attributes(including duchain data etc.), that data is changed within all contexts that also share the data of the
   * given one.
   * 
   * Special thing to consider: The imported contexts will be merged with the imported contexts of the data owner, and the problems will be merged
   * with the problems of the data owner. When one of these things changes in the data owner, it will automatically also change in this context.
   * 
   * @warning When creating context with shared data, the shared ones always have to be deleted before the owner is deleted.
   * */
  explicit TopDUContext(TopDUContext* shareDataFrom, ParsingEnvironmentFile* file = 0);

  ///If this top-context uses the data from another top-context, this returns that one.
  TopDUContext* sharedDataOwner() const;
  
  TopDUContext* topContext() const;

  ///Returns an indexed representation of this top-context. Indexed representations stay valid even if the top-context is unloaded.
  IndexedTopDUContext indexed() const;
  
  uint ownIndex() const;
  
  IndexedString url() const;
  
  /**
   * @see ParsingEnvironmentFile
   * May return zero if no file was set.
   * */
  KSharedPtr<ParsingEnvironmentFile> parsingEnvironmentFile() const;

  /// Returns true if this object is being deleted, otherwise false.
  bool deleting() const;

  /// Returns true if this object is registered in the du-chain. If it is not, all sub-objects(context, declarations, etc.) can be changed
  virtual bool inDUChain() const;
  /// This flag is only used by DUChain, never change it from outside.
  void setInDuChain(bool);

  /// Whether this top-context has a stored version on disk
  bool isOnDisk() const;
  
  /**
   * Returns a list of all problems encountered while parsing this top-context.
   * Does not include the problems of imported contexts.
   * */
  QList<ProblemPointer> problems() const;

  /**
   * Add a parsing-problem to this context.
   *
   * \note you must be holding a write lock when you access this function.
   * */
  void addProblem(const ProblemPointer& problem);

  /**
   * Remove a parsing-problem from this context.
   *
   * \note you must be holding a write lock when you access this function.
   * */
  void removeProblem(const ProblemPointer& problem);

  /**
   * Clear the list of problems
   *
   * \note you must be holding a write lock when you access this function.
   */
  void clearProblems();

  /**
   * Set the list of problems, replacing all existing ones.
   *
   * \note you must be holding a write lock when you access this function.
   */
  void setProblems(const QList<ProblemPointer>& pointers);

  /**
   * Determine if this chain imports another chain recursively.
   * 
   * This uses the imports-cache for speedup if it is available, thus it is not necessarily 100% correct
   * if the cache is not up-to-date.
   * 
   * \note you must be holding a read but not a write chain lock when you access this function.
   */
  virtual bool imports(const DUContext* origin, const CursorInRevision& position) const;

  enum {
    Identity = 4
  };
  
  enum Flags {
    NoFlags = 0,
    ///Can be used by language parts to mark contexts they currently update(for their internal usage)
    UpdatingContext = 1,
    ///You can define own language-dependent flags behind this flag
    LastFlag = 2
  };
  
  enum Features {
    ///Top-context features standard that can be requested from the duchain, and that are stored in the features() member.
    Empty = 0, //Only the top-context structure (imports etc.) is built, but no declarations and no contexts
    SimplifiedVisibleDeclarationsAndContexts = 2, //The top-context should only contain publically simplified accessible declarations and contexts, without doing type look-up,
                                                                               //without extended information like function-argument declarations, etc., imported contexts can be parsed with 'Empty' features
                                                                               //This flag essentially leads to a ctags-like processing level.
    VisibleDeclarationsAndContexts = SimplifiedVisibleDeclarationsAndContexts + 4, //Default: The top-context should only contain publically accessible declarations and contexts
    AllDeclarationsAndContexts = VisibleDeclarationsAndContexts + 8, //The top-context should also contain non-public declarations and contexts, but no uses
    AllDeclarationsContextsAndUses = 16 + AllDeclarationsAndContexts, //The top-context should contain uses and all declarations + contexts
    AST = 32,             //Signalizes that the ast() should be filled
    AllDeclarationsContextsUsesAndAST = AST | AllDeclarationsContextsAndUses, //Convenience flag, combining AST and AllDeclarationsContextsAndUses

    ///Additional update-flags that have a special meaning during updating, but are not set stored into a top-context
    Recursive = 64,  //Request the given features on all recursively imported contexts. Only the features are applied recursively (including AST)
    ForceUpdate = 128, //Enforce updating the top-context
    ForceUpdateRecursive = ForceUpdate | 256, //Enforce updating the top-context and all its imports

    ///You can define own language-dependent features behind this flag
    LastFeature = 512
  };

  ///Returns the currently active features of this top-context. The features will include AST if ast() is valid.
  Features features() const;
  ///Set the features of this top-context. These features are ignored: AST, ForceUpdate, and ForceUpdateRecursive.
  void setFeatures(Features);

  /**
   * Retrieves or creates a local index that is to be used for referencing the given @param declaration
   * in local uses. Also registers this context as a user of the declaration.
   * @param create If this is false, only already registered indices will be returned.
   *               If the declaration is not registered, std::numeric_limits<int>::max() is returned
   *
   * The duchain must be write-locked if create is true, else it must at least be read-locked.
   * */
  int indexForUsedDeclaration(Declaration* declaration, bool create = true);

  /**
   * Tries to retrieve the used declaration @param declarationIndex
   * @param context must be the context where the use happened
   * */
  Declaration* usedDeclarationForIndex(unsigned int declarationIndex) const;

  /**
   * You can use this before you rebuild all uses. This does not affect any uses directly,
   * it only invalidates the mapping of declarationIndices to Declarations.
   *
   * usedDeclarationForIndex(..) must not be called until the use has gotten a new index through
   * indexForUsedDeclaration(..).
   * */
  void clearUsedDeclarationIndices();


  /**
   * Recursively deletes all contained uses, declaration-indices, etc.
   */
  virtual void deleteUsesRecursively();
  
  /**
   * Use flags to mark top-contexts for special behavior. Any flags above LastFlag may be used for language-specific stuff.
   * */
  Flags flags() const;
  void setFlags(Flags f);
  
  /**
   * Returns the AST Container, that contains the AST created during parsing.
   * This is only created if you request the AST feature for parsing.
   * It may be discarded at any time. Every update without the AST feature will discard it.
   * The actual contents is language-specific.
   *
   * @todo Figure out logic to get rid of AST when it is not needed/useful
   */
  KSharedPtr<IAstContainer> ast() const;
  
  /**
   * Sets the AST Container.
  */
  void setAst(KSharedPtr<IAstContainer> ast);
  
  /**
   * Utility function to clear the AST Container
   */
  void clearAst();

  ///@param temporary If this is true, importers of this context will not be notified of the new imports. This greatly increases performance while removing the context,
  ///but creates in inconsistent import-structure. Therefore it is only suitable for temporary imports. These imports will not be visible from contexts that import this one.
  ///When this top-context does not own its private data, the import is added locally only to this context, not into the shared data.
  virtual void addImportedParentContext(DUContext* context, const CursorInRevision& position = CursorInRevision(), bool anonymous=false, bool temporary=false);
  ///Use this for mass-adding of imported contexts, it is faster than adding them individually.
  ///@param temporary If this is true, importers of this context will not be notified of the new imports. This greatly increases performance while removing the context,
  ///but creates in inconsistent import-structure. Therefore it is only suitable for temporary imports. These imports will not be visible from contexts that import this one.
  ///When this top-context does not own its private data, the import is added locally only to this context, not into the shared data.
  virtual void addImportedParentContexts(const QList<QPair<TopDUContext*, CursorInRevision> >& contexts, bool temporary=false);

  ///When this top-context does not own its private data, the import is removed locally only from this context, not from the shared data.
  virtual void removeImportedParentContext(DUContext* context);
  ///Use this for mass-removing of imported contexts, it is faster than removing them individually.
  ///When this top-context does not own its private data, the import is removed locally only from this context, not from the shared data.
  virtual void removeImportedParentContexts(const QList<TopDUContext*>& contexts);

  ///When this top-context does not own its private data, only the local imports of this context are removed, not those from the shared data.
  virtual void clearImportedParentContexts();
  
  typedef Utils::StorableSet<IndexedTopDUContext, IndexedTopDUContextIndexConversion, RecursiveImportRepository, true> IndexedRecursiveImports;
  
  virtual QVector<Import> importedParentContexts() const;
  
  virtual QVector<DUContext*> importers() const;

  ///Returns all currently loade importers
  virtual QList<DUContext*> loadedImporters() const;
  
  virtual CursorInRevision importPosition(const DUContext* target) const;
  
  ///Returns the set of all recursively imported top-contexts. If import-caching is used, this returns the cached set.
  ///The list also contains this context itself. This set is used to determine declaration-visibility from within this top-context.
  const IndexedRecursiveImports& recursiveImportIndices() const;

  /**
   * Updates the cache of recursive imports. When you call this, from that moment on the set returned by recursiveImportIndices() is fixed, until
   * you call it again to update them. If your language has a very complex often-changing import-structure, 
   * like for example in the case of C++, it is recommended to call this during while parsing, instead of using
   * the expensive builtin implicit mechanism.
   * Note that if you use caching, you _must_ call this before you see any visibility-effect after adding imports.
   *
   * Using import-caching has another big advantage: A top-context can be loaded without loading all its imports.
   * 
   * Note: This is relatively expensive since it requires loading all imported contexts.
   * 
   * When this is called, the top-context must already be registered in the duchain.
   */
  void updateImportsCache();
  
  bool usingImportsCache() const;

  virtual bool findDeclarationsInternal(const SearchItem::PtrList& identifiers, const CursorInRevision& position, const AbstractType::Ptr& dataType, DeclarationList& ret, const TopDUContext* source, SearchFlags flags, uint depth) const;
protected:
  void setParsingEnvironmentFile(ParsingEnvironmentFile*);

  /**
   * Does the same as DUContext::updateAliases, except that it uses the symbol-store, and processes the whole identifier.
   * @param canBeNamespace whether the searched identifier may be a namespace.
   * If this is true, namespace-aliasing is applied to the last elements of the identifiers.
   * */
  template<class Acceptor>
  void applyAliases( const SearchItem::PtrList& identifiers, Acceptor& accept, const CursorInRevision& position, bool canBeNamespace ) const;

protected:
  ///Call this to destroy a top-context.
  void deleteSelf();

  virtual ~TopDUContext();
  
  void clearFeaturesSatisfied();
  void rebuildDynamicData(DUContext* parent, uint ownIndex);
  //Must be called after all imported top-contexts were loaded into the du-chain
  void rebuildDynamicImportStructure();
  
  struct AliasChainElement;
  struct FindDeclarationsAcceptor;
  struct DeclarationChecker;
  struct ApplyAliasesBuddyInfo;

  template<class Acceptor>
  bool applyAliases( const QualifiedIdentifier& previous, const SearchItem::Ptr& identifier, Acceptor& acceptor, const CursorInRevision& position, bool canBeNamespace, ApplyAliasesBuddyInfo* buddy, uint recursionDepth ) const;
  //Same as imports, without the slow access-check, for internal usage
  bool importsPrivate(const DUContext * origin, const CursorInRevision& position) const;
  DUCHAIN_DECLARE_DATA(TopDUContext)
  
  //Most of these classes need access to m_dynamicData
  friend class DUChain;
  friend class DUChainPrivate;
  friend class TopDUContextData;
  friend class TopDUContextLocalPrivate;
  friend class TopDUContextDynamicData;
  friend class Declaration;
  friend class DUContext;
  friend class IndexedDeclaration;
  friend class IndexedDUContext;
  friend class LocalIndexedDeclaration;
  friend class LocalIndexedDUContext;
  friend class DeclarationId;
  friend class ParsingEnvironmentFile;
  
  TopDUContextLocalPrivate* m_local;
  
  class TopDUContextDynamicData* m_dynamicData;
};

/**
  * Returns all uses of the given declaration within this top-context and all sub-contexts
  * */
KDEVPLATFORMLANGUAGE_EXPORT QList<RangeInRevision> allUses(TopDUContext* context, Declaration* declaration, bool noEmptyRanges = false);

inline uint qHash(const ReferencedTopDUContext& ctx) {
  return ctx.hash();
}

inline uint qHash(const IndexedTopDUContext& ctx) {
  return ctx.index();
}

}
Q_DECLARE_METATYPE(KDevelop::ReferencedTopDUContext);

#endif // KDEVPLATFORM_TOPDUCONTEXT_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
