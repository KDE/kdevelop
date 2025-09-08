/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_TOPDUCONTEXT_H
#define KDEVPLATFORM_TOPDUCONTEXT_H

#include "ducontext.h"

#include <language/util/setrepository.h>
#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <QMetaType>

template <class T>
class QExplicitlySharedDataPointer;

namespace KDevelop {
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

struct KDEVPLATFORMLANGUAGE_EXPORT RecursiveImportRepository
{
    static Utils::BasicSetRepository* repository();
};

///Maps an imported top-context to a pair:
///1. The distance to the top-context, and 2. The next step towards the top-context
///in the chain.
using RecursiveImports = QHash<const TopDUContext*, QPair<int, const TopDUContext*>>;

using TopDUContextPointer = DUChainPointer<TopDUContext>;

using ProblemPointer = QExplicitlySharedDataPointer<Problem>;

///KDevelop can unload unused top-context at any time. To prevent unloading,
///keep a ReferencedTopDUContext.
class KDEVPLATFORMLANGUAGE_EXPORT ReferencedTopDUContext
{
public:
    ReferencedTopDUContext(TopDUContext* context = nullptr);
    ReferencedTopDUContext(const ReferencedTopDUContext& rhs);
    ~ReferencedTopDUContext();

    ReferencedTopDUContext& operator=(const ReferencedTopDUContext& rhs);

    inline TopDUContext* data() const
    {
        return m_topContext;
    }

    inline operator TopDUContext*() const {
        return m_topContext;
    }

    inline bool operator==(const ReferencedTopDUContext& rhs) const
    {
        return m_topContext == rhs.m_topContext;
    }

    inline bool operator!=(const ReferencedTopDUContext& rhs) const
    {
        return m_topContext != rhs.m_topContext;
    }

    inline TopDUContext* operator->() const
    {
        return m_topContext;
    }

    inline uint hash() const
    {
        return ( uint )((( quint64 )m_topContext) * 37);
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
class KDEVPLATFORMLANGUAGE_EXPORT TopDUContext
    : public DUContext
{
public:
    explicit TopDUContext(const IndexedString& url, const RangeInRevision& range,
                          ParsingEnvironmentFile* file = nullptr);
    explicit TopDUContext(TopDUContextData& data);

    TopDUContext& operator=(const TopDUContext& rhs) = delete;

    TopDUContext* topContext() const override;

    ///Returns an indexed representation of this top-context. Indexed representations stay valid even if the top-context is unloaded.
    IndexedTopDUContext indexed() const;

    uint ownIndex() const;

    IndexedString url() const override;

    /**
     * @see ParsingEnvironmentFile
     * May return zero if no file was set.
     * */
    QExplicitlySharedDataPointer<ParsingEnvironmentFile> parsingEnvironmentFile() const;

    /// Returns true if this object is being deleted, otherwise false.
    bool deleting() const;

    /// Returns true if this object is registered in the du-chain. If it is not, all sub-objects(context, declarations, etc.) can be changed
    bool inDUChain() const override;
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
    bool imports(const DUContext* origin, const CursorInRevision& position) const override;

    enum {
        Identity = 4
    };

    enum Feature : quint16 {
        ///Top-context features standard that can be requested from the duchain, and that are stored in the features() member.
        Empty = 0, //Only the top-context structure (imports etc.) is built, but no declarations and no contexts
        SimplifiedVisibleDeclarationsAndContexts = 2, //The top-context should only contain publicly simplified accessible declarations and contexts, without doing type look-up,
        //without extended information like function-argument declarations, etc., imported contexts can be parsed with 'Empty' features
        //This flag essentially leads to a ctags-like processing level.
        VisibleDeclarationsAndContexts = SimplifiedVisibleDeclarationsAndContexts + 4, //Default: The top-context should only contain publicly accessible declarations and contexts
        AllDeclarationsAndContexts = VisibleDeclarationsAndContexts + 8, //The top-context should also contain non-public declarations and contexts, but no uses
        AllDeclarationsContextsAndUses = 16 + AllDeclarationsAndContexts, //The top-context should contain uses and all declarations + contexts
        AST = 32,         //Signalizes that the ast() should be filled
        AllDeclarationsContextsUsesAndAST = AST | AllDeclarationsContextsAndUses, //Convenience flag, combining AST and AllDeclarationsContextsAndUses

        ///Additional update-flags that have a special meaning during updating, but are not set stored into a top-context
        Recursive = 64, //Request the given features on all recursively imported contexts. Only the features are applied recursively (including AST)
        ForceUpdate = 128, //Enforce updating the top-context
        ForceUpdateRecursive = ForceUpdate | 256, //Enforce updating the top-context and all its imports

        ///You can define own language-dependent features behind this flag
        LastFeature = 512
    };
    Q_DECLARE_FLAGS(Features, Feature)

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
     * Tries to retrieve the used declaration
     * @param declarationIndex The index of the declaration which have to be retrieved
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
    void deleteUsesRecursively() override;

    /**
     * Returns the AST Container, that contains the AST created during parsing.
     * This is only created if you request the AST feature for parsing.
     * It may be discarded at any time. Every update without the AST feature will discard it.
     * The actual contents is language-specific.
     *
     * @todo Figure out logic to get rid of AST when it is not needed/useful
     */
    QExplicitlySharedDataPointer<IAstContainer> ast() const;

    /**
     * Sets the AST Container.
     */
    void setAst(const QExplicitlySharedDataPointer<IAstContainer>& ast);

    /**
     * Utility function to clear the AST Container
     */
    void clearAst();

    ///@param temporary If this is true, importers of this context will not be notified of the new imports. This greatly increases performance while removing the context,
    ///but creates in inconsistent import-structure. Therefore it is only suitable for temporary imports. These imports will not be visible from contexts that import this one.
    ///When this top-context does not own its private data, the import is added locally only to this context, not into the shared data.
    void addImportedParentContext(DUContext* context,
                                  const CursorInRevision& position = CursorInRevision(), bool anonymous = false,
                                  bool temporary = false) override;
    ///Use this for mass-adding of imported contexts, it is faster than adding them individually.
    ///@param temporary If this is true, importers of this context will not be notified of the new imports. This greatly increases performance while removing the context,
    ///but creates in inconsistent import-structure. Therefore it is only suitable for temporary imports. These imports will not be visible from contexts that import this one.
    ///When this top-context does not own its private data, the import is added locally only to this context, not into the shared data.
    virtual void addImportedParentContexts(const QVector<QPair<TopDUContext*, CursorInRevision>>& contexts,
                                           bool temporary = false);

    ///When this top-context does not own its private data, the import is removed locally only from this context, not from the shared data.
    void removeImportedParentContext(DUContext* context) override;
    ///Use this for mass-removing of imported contexts, it is faster than removing them individually.
    ///When this top-context does not own its private data, the import is removed locally only from this context, not from the shared data.
    virtual void removeImportedParentContexts(const QList<TopDUContext*>& contexts);

    ///When this top-context does not own its private data, only the local imports of this context are removed, not those from the shared data.
    void clearImportedParentContexts() override;

    using IndexedRecursiveImports = Utils::StorableSet<IndexedTopDUContext, IndexedTopDUContextIndexConversion, RecursiveImportRepository,
        true>;

    QVector<Import> importedParentContexts() const override;

    QVector<DUContext*> importers() const override;

    ///Returns all currently loaded importers
    virtual QList<DUContext*> loadedImporters() const;

    CursorInRevision importPosition(const DUContext* target) const override;

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

    bool findDeclarationsInternal(const SearchItem::PtrList& identifiers, const CursorInRevision& position,
                                  const AbstractType::Ptr& dataType, DeclarationList& ret, const TopDUContext* source,
                                  SearchFlags flags, uint depth) const override;

protected:
    void setParsingEnvironmentFile(ParsingEnvironmentFile*);

    /**
     * Does the same as DUContext::updateAliases, except that it uses the symbol-store, and processes the whole identifier.
     * @param canBeNamespace whether the searched identifier may be a namespace.
     * If this is true, namespace-aliasing is applied to the last elements of the identifiers.
     * */
    template <class Acceptor>
    void applyAliases(const SearchItem::PtrList& identifiers, Acceptor& accept, const CursorInRevision& position,
                      bool canBeNamespace) const;

protected:
    ~TopDUContext() override;

    void clearFeaturesSatisfied();
    void rebuildDynamicData(DUContext* parent, uint ownIndex) override;
    //Must be called after all imported top-contexts were loaded into the du-chain
    void rebuildDynamicImportStructure();

    struct AliasChainElement;
    struct FindDeclarationsAcceptor;
    struct DeclarationChecker;
    struct ApplyAliasesBuddyInfo;

    template <class Acceptor>
    bool applyAliases(const QualifiedIdentifier& previous, const SearchItem::Ptr& identifier, Acceptor& acceptor,
                      const CursorInRevision& position, bool canBeNamespace, ApplyAliasesBuddyInfo* buddy,
                      uint recursionDepth) const;
    //Same as imports, without the slow access-check, for internal usage
    bool importsPrivate(const DUContext* origin, const CursorInRevision& position) const;
    DUCHAIN_DECLARE_DATA(TopDUContext)

    ///Called by DUChain::removeDocumentChain to destroy this top-context.
    void deleteSelf();

    //Most of these classes need access to m_dynamicData
    friend class DUChain;
    friend class DUChainPrivate;
    friend class TopDUContextData;
    friend class TopDUContextLocalPrivate;
    friend class TopDUContextDynamicData;
    friend class Declaration;
    friend class DUContext;
    friend class Problem;
    friend class IndexedDeclaration;
    friend class IndexedDUContext;
    friend class LocalIndexedDeclaration;
    friend class LocalIndexedDUContext;
    friend class LocalIndexedProblem;
    friend class DeclarationId;
    friend class ParsingEnvironmentFile;

    TopDUContextLocalPrivate* m_local;

    class TopDUContextDynamicData* m_dynamicData;
};

/**
 * Returns all uses of the given declaration within this top-context and all sub-contexts
 * */
KDEVPLATFORMLANGUAGE_EXPORT QVector<RangeInRevision> allUses(TopDUContext* context, Declaration* declaration,
                                                             bool noEmptyRanges = false);

inline size_t qHash(const ReferencedTopDUContext& ctx)
{
    return ctx.hash();
}

Q_DECLARE_OPERATORS_FOR_FLAGS(TopDUContext::Features)
}

Q_DECLARE_TYPEINFO(KDevelop::ReferencedTopDUContext, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::ReferencedTopDUContext)

#endif // KDEVPLATFORM_TOPDUCONTEXT_H
