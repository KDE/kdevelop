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

#ifndef TOPDUCONTEXT_H
#define TOPDUCONTEXT_H

#include <QtCore/QMutex>

#include "ducontext.h"

template< class T >
class KSharedPtr;

namespace KDevelop
{
  class QualifiedIdentifier;
  class DUChain;
  class ParsingEnvironmentFile;
  class TopDUContextData;
  class TopDUContextLocalPrivate;
//   class TopDUContextDynamicData;
  class Problem;
  class DeclarationChecker;
  class TopDUContext;

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
    
    TopDUContext* data() const {
      return m_topContext;
    }
    
    operator TopDUContext*() const {
      return m_topContext;
    }
    
    bool operator==(const ReferencedTopDUContext& rhs) const {
      return m_topContext == rhs.m_topContext;
    }
    
    bool operator!=(const ReferencedTopDUContext& rhs) const {
      return m_topContext != rhs.m_topContext;
    }
    
    TopDUContext* operator->() const {
      return m_topContext;
    }
    
    uint hash() const {
      return (uint)(((quint64)m_topContext) * 37);
    }
    
  private:
  TopDUContext* m_topContext;
};

///Allows simple indirect access to top-contexts with on-demand loading
class KDEVPLATFORMLANGUAGE_EXPORT IndexedTopDUContext {
  public:
    IndexedTopDUContext(uint index) : m_index(index) {
    }
    IndexedTopDUContext(TopDUContext* context = 0);
    
    ///Returns the top-context represented by this indexed top-context. If it wasn't loaded yet, it is loaded.
    ///To prevent automatic unloading, store it in ReferencedTopDUContext
    TopDUContext* data() const;
    
    ///Returns whether the top-context is currently loaded. If not, it will be loaded when you call data().
    bool isLoaded() const;
    
    bool operator==(const IndexedTopDUContext& rhs) const {
      return m_index == rhs.m_index;
    }
    bool operator!=(const IndexedTopDUContext& rhs) const {
      return m_index != rhs.m_index;
    }
    uint index() const {
      return m_index;
    }
  private:
  uint m_index;
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
 * @warning When you delete a top-context, delete it using TopDUContext::deleteSelf(), else you will leak memory
 */
class KDEVPLATFORMLANGUAGE_EXPORT TopDUContext : public DUContext
{
public:
  explicit TopDUContext(const IndexedString& url, const SimpleRange& range, ParsingEnvironmentFile* file = 0);
  explicit TopDUContext(TopDUContextData& data);
  
  ///Call this to destroy a top-context.
  void deleteSelf();
  
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
  bool inDuChain() const;
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
   * */
  void addProblem(const ProblemPointer& problem);

  /// Clear the list of problems
  void clearProblems();

  /**
   * Determine if this chain imports another chain.
   *
   * \note you must be holding a read but not a write chain lock when you access this function.
   */
  virtual bool imports(const DUContext* origin, const SimpleCursor& position) const;

  /**
   * Returns the trace of imports from this context top the given target.
   * The positions in the returned trace may be invalid.
   * This is more efficient then the version below, because the trace doesn't need to be copied
   * */
  void importTrace(const TopDUContext* target, ImportTrace& store) const;

  ///More convenient version of the above
  ImportTrace importTrace(const TopDUContext* target) const;

  enum {
    Identity = 4
  };
  
  enum Flags {
    NoFlags = 0,
    /**
     * A language-specific flag used by C++ to mark one context as a proxy of another.
     * If this flag is set on a context, the first imported context should be used for any computations
     * like searches, listing, etc. instead of using this context.
     *
     * The problems should be stored within the proxy-contexts, and optionally there may be
     * any count of imported proxy-contexts imported behind the content-context(This can be used for tracking problems)
     *
     * Note: This flag does not directly change the behavior of the language-independent du-chain.
     */
    ProxyContextFlag = 1,
    /**
     * Placeholder used in searching-functions, when the flag should simply be ignored.
     * */
    AnyFlag=2,
    /**
     * Can be used by language parts to mark contexts they currently update(for their internal usage)
     * */
    UpdatingContext = 4,
    LastFlag
  };

  bool hasUses() const;
  void setHasUses(bool hasUses);

  /**
   * Retrieves or creates a local index that is to be used for referencing the given @param declaration
   * in local uses. Also registers this context as a user of the declaration.
   * @param create If this is false, only already registered indices will be returned.
   *               If the declaration is not registered, std::limits<int>::max is returned
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
   * Use flags to mark top-contexts for special behavior. Any flags above LastFlag may be used for language-specific stuff.
   * */
  Flags flags() const;
  void setFlags(Flags f);

  ///@param temporary If this is true, importers of this context will not be notified of the new imports. This greatly increases performance while removing the context,
  ///but creates in inconsistent import-structure. Therefore it is only suitable for temporary imports. These imports will not be visible from contexts that import this one.
  ///When this top-context does not own its private data, the import is added locally only to this context, not into the shared data.
  virtual void addImportedParentContext(DUContext* context, const SimpleCursor& position = SimpleCursor(), bool anonymous=false, bool temporary=false);
  ///Use this for mass-adding of imported contexts, it is faster than adding them individually.
  ///@param temporary If this is true, importers of this context will not be notified of the new imports. This greatly increases performance while removing the context,
  ///but creates in inconsistent import-structure. Therefore it is only suitable for temporary imports. These imports will not be visible from contexts that import this one.
  ///When this top-context does not own its private data, the import is added locally only to this context, not into the shared data.
  virtual void addImportedParentContexts(const QList<QPair<TopDUContext*, SimpleCursor> >& contexts, bool temporary=false);

  ///When this top-context does not own its private data, the import is removed locally only from this context, not from the shared data.
  virtual void removeImportedParentContext(DUContext* context);
  ///Use this for mass-removing of imported contexts, it is faster than removing them individually.
  ///When this top-context does not own its private data, the import is removed locally only from this context, not from the shared data.
  virtual void removeImportedParentContexts(const QList<TopDUContext*>& contexts);

  ///When this top-context does not own its private data, only the local imports of this context are removed, not those from the shared data.
  virtual void clearImportedParentContexts();
  
  virtual QVector<Import> importedParentContexts() const;
  
  virtual QVector<DUContext*> importers() const;
  
  virtual SimpleCursor importPosition(const DUContext* target) const;
  
  class CacheData;

  /**The cache allows speeding up repeated searches. When you're planning to do many searches from within the same top-context,
    *this should be done. Just create an item of type Cache, and give it the du-context. The duchain must not be locked when calling this.
    *The cache will automatically be discarded on destruction of the item. It is thread-local, so you don't need to care about multi-threading.
    *@warning The DUChain must not be locked when the constructor is called, and it must not be locked when the destructor is executed.
    *@warning Only use this in phases when you're not changing the Data within this contexts, else the cache will become invalid and search will be fuzzy.
    */
  class KDEVPLATFORMLANGUAGE_EXPORT Cache {
    public:
    Cache(TopDUContextPointer context);
    ~Cache();

    private:
    Q_DISABLE_COPY(Cache)
    CacheData* d;
  };

  virtual bool findDeclarationsInternal(const SearchItem::PtrList& identifiers, const SimpleCursor& position, const AbstractType::Ptr& dataType, DeclarationList& ret, const TopDUContext* source, SearchFlags flags) const;
protected:
  void setParsingEnvironmentFile(ParsingEnvironmentFile*);

  virtual void findContextsInternal(ContextType contextType, const SearchItem::PtrList& identifiers, const SimpleCursor& position, QList<DUContext*>& ret, SearchFlags flags = NoSearchFlags) const;

  /**
   * Does the same as DUContext::updateAliases, except that it uses the symbol-store, and processes the whole identifier.
   * @param canBeNamespace whether the searched identifier may be a namespace.
   * If this is true, namespace-aliasing is applied to the last elements of the identifiers.
   * */
  template<class Acceptor>
  void applyAliases( const SearchItem::PtrList& identifiers, Acceptor& accept, const SimpleCursor& position, bool canBeNamespace ) const;

protected:

  virtual ~TopDUContext();
private:
  ///Returns a set of all recursively imported top-contexts. Each imported top-context is mapped to the distance, and the direct
  ///import through which the top-context is imported.
  RecursiveImports recursiveImports() const;
  
  void rebuildDynamicData(DUContext* parent, uint ownIndex);
  //Must be called after all imported top-contexts were loaded into the du-chain
  void rebuildDynamicImportStructure();
  
  struct AliasChainElement;
  struct FindDeclarationsAcceptor;
  struct FindContextsAcceptor;
  struct DeclarationChecker;
  struct ContextChecker;

  template<class Acceptor>
  void applyAliases( const AliasChainElement* backPointer, const SearchItem::Ptr& identifier, Acceptor& acceptor, const SimpleCursor& position, bool canBeNamespace ) const;
  //Same as imports, without the slow access-check, for internal usage
  bool importsPrivate(const DUContext * origin, const SimpleCursor& position) const;
  DUCHAIN_DECLARE_DATA(TopDUContext)
  
  //Most of these classes need access to m_dynamicData
  friend class DUChain;
  friend class DUChainPrivate;
  friend class TopDUContextLocalPrivate;
  friend class TopDUContextDynamicData;
  friend class Declaration;
  friend class DUContext;
  friend class IndexedDeclaration;
  friend class IndexedDUContext;
  friend class LocalIndexedDeclaration;
  friend class LocalIndexedDUContext;
  
  TopDUContextLocalPrivate* m_local;
  
  class TopDUContextDynamicData* m_dynamicData;
};

/**
  * Returns all uses of the given declaration within this top-context and all sub-contexts
  * */
KDEVPLATFORMLANGUAGE_EXPORT QList<SimpleRange> allUses(TopDUContext* context, Declaration* declaration, bool noEmptyRanges = false);

/**
  * Returns the smart-ranges of all uses
  * */
KDEVPLATFORMLANGUAGE_EXPORT QList<KTextEditor::SmartRange*> allSmartUses(TopDUContext* context, Declaration* declaration);

inline uint qHash(const ReferencedTopDUContext& ctx) {
  return ctx.hash();
}

}

#endif // TOPDUCONTEXT_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
