/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DECLARATION_H
#define KDEVPLATFORM_DECLARATION_H

#include <QList>
#include <QMap>

#include "types/abstracttype.h"
#include "duchainbase.h"
#include "identifier.h"
#include "indexeddeclaration.h"

class QByteArray;

namespace KDevelop {
class AbstractType;
class DUContext;
class IndexedString;
class DeclarationData;
class DeclarationId;
class Declaration;
class IndexedTopDUContext;
class TopDUContext;
class IndexedInstantiationInformation;

/**
 * \short Represents a single declaration in a definition-use chain.
 *
 * \note A du-context can be freely edited as long as it's parent-context is zero.
 * In the moment the parent-context is set, the context may only be edited when it
 * is allowed to edited it's top-level context (@see TopLevelContext::inDUChain())
 */
class KDEVPLATFORMLANGUAGE_EXPORT Declaration
    : public DUChainBase
{
public:
    /// Access types
    enum AccessPolicy : quint8 {
        Public /**< a public declaration */,
        Protected /**< a protected declaration */,
        Private /**< a private declaration */,
        DefaultAccess /**<a declaration with default access; in java, only package-level access. */
    };
    /// Enumeration of the types of declarations
    enum Kind : quint8 {
        Type /**< A type is declared, like a class-declaration or function-declaration, or a typedef("class MyClass {};") */,
        Instance /**< An instance of a type is declared("MyClass m;") */,
        NamespaceAlias /**< This is a namespace-alias. You can safely cast this object to NamespaceAliasDeclaration. */,
        Alias, /**<This is an alias-declaration. You can safely cast this object to AliasDeclaration. */
        Namespace, /**< Declaration of a namespace. */
        Import, /**< Declaration marks the Import of a file. */
        Macro /**< Declaration of a macro such as "#define FOO 1". */
    };

    /**
     * Constructor.
     *
     * If \a parentContext is in the symbol table, the declaration will automatically
     * be added into the symbol table.
     *
     * \param range range of the alias declaration's identifier
     * \param parentContext context in which this declaration occurred
     * */
    Declaration(const RangeInRevision& range, DUContext* parentContext);
    ///Copy-constructor for cloning
    Declaration(const Declaration& rhs);
    /// Destructor
    ~Declaration() override;
    /// Uses the given data
    explicit Declaration(DeclarationData& dd);

    Declaration& operator=(const Declaration& rhs) = delete;

    TopDUContext* topContext() const override;

    /**
     * Determine whether this declaration is a forward declaration.
     *
     * \returns true if this is a forward declaration, otherwise returns false.
     */
    virtual bool isForwardDeclaration() const;

    /**
     * Determine whether this declaration is a function declaration.
     *
     * \returns true if this is a function declaration, otherwise returns false.
     */
    virtual bool isFunctionDeclaration() const;

    /**
     * Determine whether this declaration is accessible through the du-chain.
     * If it is, it cannot be edited without holding the du-chain write lock.
     *
     * \sa DUChain::lock()
     * \sa DUChainWriteLocker
     *
     * \returns true if the Declaration is already inserted into a duchain.
     */
    virtual bool inDUChain() const;

    /**
     * Determine whether this declaration is also a definition.
     *
     * \returns true if this declaration is also a definition, otherwise false.
     */
    bool isDefinition() const;
    /**
     * Set whether this declaration is also a definition.
     *
     * \param dd set this to true if this declaration is also a definition, otherwise false.
     */
    void setDeclarationIsDefinition(bool dd);

    /**
     * Determine if this declaration is a type-alias (in c++ typedef).
     *
     * \returns true if the declaration is a type alias, otherwise false.
     */
    bool isTypeAlias() const;
    /**
     * Set whether this declaration is a type alias.
     *
     * \param typeAlias true if the declaration is a type alias, otherwise false.
     */
    void setIsTypeAlias(bool typeAlias);

    /**
     * Determine whether the declaration is deprecated.
     */
    bool isDeprecated() const;
    /**
     * Set whether the declaration is deprecated.
     *
     * \param deprecated true if the declaration is deprecated, otherwise false.
     */
    void setDeprecated(bool deprecated);

    /**
     * Changes whether this declaration must be direct in all cases or not.
     *
     * By default this is set to false.
     *
     * \param direct true to force direct, false otherwise.
     */
    void setAlwaysForceDirect(bool direct);
    /**
     * Determine whether this declaration must always be direct.
     */
    bool alwaysForceDirect() const;

    /**
     * Changes whether this declaration is "implicitly created".
     *
     * An implicit declaration is not declared in the class context,
     * but written somewhere else outside.
     *
     * Declarations are by default not implicitly declared.
     *
     * \param _auto true for implicit, false for default behaviour
     */
    void setAutoDeclaration(bool _auto);
    /**
     * Determine whether this declaration is implicitly created or not.
     */
    bool isAutoDeclaration() const;

    /**
     * Changes whether this declaration is "explicitly deleted", i.e. not implicitly declared or accessible.
     *
     * \param deleted true for deleted, false for default behaviour
     * */
    void setExplicitlyDeleted(bool deleted);
    /**
     * Determine whether this declaration is "explicitly deleted" or not.
     */
    bool isExplicitlyDeleted() const;

    /**
     * Changes whether this declaration is explicitly typed.
     *
     * Explicitly typed declaration has the type written as part of the
     * declaration. The opposite, implicitly typed declaration, has the type
     * deduced by the compiler.
     *
     * E.g. in C++ variable declarations are explicitly typed unless the "auto"
     * keyword is used.
     *
     * \param explicitlyTyped true for explicitly typed, false for implicitly typed
     */
    void setExplicitlyTyped(bool explicitlyTyped);
    /**
     * Determine whether this declaration is explicitly typed.
     */
    bool isExplicitlyTyped() const;

    /**
     * Retrieve the declaration which is specialized with the given
     * \a specialization index as seen from \a topContext.
     *
     * \param specialization the specialization index (see DeclarationId)
     * \param topContext the top context representing the perspective from which to specialize.
     *                   if @p topContext is zero, only already existing specializations are returned,
     *                   and if none exists, zero is returned.
     * \param upDistance upwards distance in the context-structure of the
     *                   given specialization-info. This allows specializing children.
     */
    virtual Declaration* specialize(const IndexedInstantiationInformation& specialization,
                                    const TopDUContext* topContext, int upDistance = 0);

    /**
     * Retrieve the context that is opened by this declaration, if one exists.
     *
     * For example, a class will have a declaration which is contained within the context in which
     * it is declared, and a new context will be created to hold class members.  This function returns
     * that context.
     * The declaration has to be part of the same top-context.
     *
     * \returns the internal context for this declaration or, if none exists, nullptr
     */
    DUContext* internalContext() const;

    /**
     * Set the internal \a context for this declaration.
     *
     * \param context the internal context
     */
    void setInternalContext(DUContext* context);

    /**
     * Determine the logical internal context for the resolved form of this declaration.
     *
     * \li If this declaration has a definition, and the definition is resolved,
     *     it returns the internal context of the definition.
     * \li If this declaration is a forward-declaration, the forward-declaration is
     *     resolved, it returns the internal context of the resolved declaration.
     * \li If this is a type-alias, it returns the internal context of the actual type.
     * \li Otherwise, it returns the same as internalContext().
     *
     * \param topContext Needed to resolve forward-declarations.
     * \returns the resolved internal context, as described above
     */
    virtual DUContext* logicalInternalContext(const TopDUContext* topContext) const;

    /**
     * This is a convenience function to determine the resolved declaration, if this is a forward declaration.
     * Otherwise, it just returns this declaration.
     * \param topContext Context within which to search for the resolved declaration.
     * \returns the resolved declaration if one was found, otherwise this declaration.
     * */
    const Declaration* logicalDeclaration(const TopDUContext* topContext) const;

    /// \copydoc logicalDeclaration(const TopDUContext* topContext) const
    Declaration* logicalDeclaration(const TopDUContext* topContext);

    /**
     * Access the parent context of this declaration.
     * \returns the parent context of this declaration.
     */
    DUContext* context() const;

    /**
     * Set the context in which this declaration occurs.
     *
     * When setContext() is called, this declaration is inserted into the given context.
     * You only need to be able to write this declaration. You do not need write-privileges
     * for the context, because addDeclaration(..) works separately to that.
     *
     * If the given context is not in the symbol-table, or if the declaration is inserted anonymously,
     * or if the context is zero, this declaration is removed from the symbol-table.
     * Else it is added to the symbol table with the new scope. See TopDUContext for information
     * about the symbol table.
     *
     * \param context New context which contains this declaration. The context must have a
     *                top-context if it is not zero.
     * \param anonymous If this is set, this declaration will be added anonymously into the parent-context.
     *                  This way it can never be found through any of the context's functions, and will
     *                  not be deleted when the context is deleted, so it must be deleted from elsewhere.
     */
    void setContext(DUContext* context, bool anonymous = false);

    /**
     * Convenience function to return this declaration's type dynamically casted to \a T.
     *
     * \returns this declaration's type as \a T, or null if there is no type or it is not of type \a T.
     */
    template<class T>
    TypePtr<T> type() const
    {
        return abstractType().dynamicCast<T>();
    }

    /**
     * Access this declaration's type.
     *
     * \note You should not compare or permanently store instances of AbstractType::Ptr. Use IndexedType instead.
     * \returns this declaration's type, or null if none has been assigned.
     */
    AbstractType::Ptr abstractType() const;

    /**
     * Set this declaration's type.
     *
     * \param type the type to assign.
     */
    void setType(AbstractType::Ptr type)
    {
        setAbstractType(std::move(type));
    }

    /**
     * Set this declaration's \a type.
     *
     * \param type this declaration's new type.
     */
    virtual void setAbstractType(AbstractType::Ptr type);

    /**
     * Return an indexed form of this declaration's type.
     * Should be preferred, this is the fastest way, and the correct way for doing equality-comparison.
     *
     * \returns the declaration's type.
     */
    IndexedType indexedType() const;

    /**
     * Set this declaration's \a identifier.
     *
     * \param identifier this declaration's new identifier
     */
    void setIdentifier(const Identifier& identifier);

    /**
     * Access this declaration's \a identifier.
     *
     * \returns this declaration's identifier.
     */
    Identifier identifier() const;

    /**
     * Access this declaration's \a identifier.
     *
     * \return this declaration's identifier in indexed form. This is faster than identifier(), because it
     *         equals the internal representation. Use this for example to do equality-comparison.
     */
    const IndexedIdentifier& indexedIdentifier() const;

    /**
     * Determine the global qualified identifier of this declaration.
     *
     * \note This function is expensive, equalQualifiedIdentifier() is preferred if you
     *       just want to compare equality.
     */
    QualifiedIdentifier qualifiedIdentifier() const;

    /**
     * Compares the qualified identifier of this declaration with the other one, without needing to compute it.
     * This is more efficient than comparing the results of qualifiedIdentifier().
     *
     * \param rhs declaration to compare identifiers with
     * \returns true if the identifiers are equal, otherwise false.
     */
    bool equalQualifiedIdentifier(const Declaration* rhs) const;

    /**
     * Returns the kind of this declaration. @see Kind
     */
    Kind kind() const;

    /**
     * Set the kind.
     *
     * \param kind new kind
     */
    void setKind(Kind kind);

    /**
     * Returns the comment associated to this declaration in the source-code,
     * or an invalid string if there is none.
     *
     * Stored in utf-8 encoding.
     */
    QByteArray comment() const;

    /**
     * Sets the comment for this declaration.
     *
     * @note Should be utf-8 encoded.
     */
    void setComment(const QByteArray& str);
    /**
     * Sets the comment for this declaration.
     */
    void setComment(const QString& str);

    /**
     * Access whether this declaration is in the symbol table.
     *
     * \returns true if this declaration is in the symbol table, otherwise false.
     */
    bool inSymbolTable() const;

    /**
     * Adds or removes this declaration to/from the symbol table.
     *
     * \param inSymbolTable true to add this declaration to the symbol table, false to remove it.
     */
    virtual void setInSymbolTable(bool inSymbolTable);

    /**
     * Equivalence operator.
     *
     * \param other Other declaration to compare.
     * \returns true if the declarations are equal, otherwise false.
     */
    bool operator==(const Declaration& other) const;

    /**
     * Determine this declaration as a string. \returns this declaration as a string.
     */
    virtual QString toString() const;

    /**
     * Returns a map of files to use-ranges.
     *
     * The key of the returned map is an url of a file. The value
     * is a list with all use-ranges of this declaration in that file.
     *
     * \note The ranges are in the documents local revision,
     *       use \c DUChainUtils::transformFromRevision or \c usesCurrentRevision()
     *
     * \note The uses are unique, no 2 uses are returned that have the same range within the same file.
     *
     * \note This is a non-trivial operation and hence expensive.
     */
    QMap<IndexedString, QVector<RangeInRevision>> uses() const;

    /**
     * Determines whether the declaration has any uses or not.
     *
     * Cheaper than calling uses().
     */
    bool hasUses() const;

    /**
     * Returns a map of files to use-ranges.
     *
     * The key of the returned map is an url of a file. The value
     * is a list with all use-ranges of this declaration in that file.
     *
     * \note The uses are unique, no 2 uses are returned that have the same range within the same file.
     *
     * \warning This must be called only from within the foreground, or with the foreground lock locked.
     *
     * \note This is a non-trivial operation and hence expensive.
     */
    QMap<IndexedString, QVector<KTextEditor::Range>> usesCurrentRevision() const;

    /**
     * Returns a list of uses in the given top-context.
     */
    QVector<KTextEditor::Range> usesCurrentRevision(TopDUContext* context) const;

    /**
     * This hash-value should differentiate between multiple different
     * declarations that have the same qualifiedIdentifier, but should have a different
     * identity, and thus own Definitions and own Uses assigned.
     *
     * Affected by function-arguments, whether this is a template-declaration, etc..
     */
    virtual uint additionalIdentity() const;

    /**
     * TODO document
     * */
    virtual IndexedInstantiationInformation specialization() const;

    /**
     * \see DeclarationId
     *
     * \param forceDirect When this is true, the DeclarationId is force to be direct,
     *                    and can be resolved without a symbol-table and top-context.
     *                    The same goes for Declarations that have \c alwaysForceDirect()
     *                    set to true.
     */
    virtual DeclarationId id(bool forceDirect = false) const;

    /**
     * Returns an index that uniquely identifies this declaration within its surrounding top-context.
     *
     * That index can be passed to \c TopDUContext::declarationFromIndex(index) to get the declaration.
     * This is only valid when the declaration is not a specialization (\c specialization() returns 0),
     * and if it is not anonymous in its context.
     *
     * \note for this to be valid, allocateOwnIndex() must have been called first.
     * \note the highest big of the index is always zero!
     * \returns the index of the declaration within its TopDUContext.
     */
    uint ownIndex() const;

    /**
     * Whether this declaration has been inserted anonymously into its parent-context
     */
    bool isAnonymous() const;

    /**
     * Clear the index for this declaration in the top context that was allocated with allocateOwnIndex().
     */
    void clearOwnIndex();

    /**
     * Create an index to this declaration from the topContext().  Needed to be able to retrieve ownIndex().
     */
    void allocateOwnIndex();

    /**
     * Returns a clone of this declaration, with the difference that the returned declaration
     * has no context set, i.e. \c context() returns zero.
     *
     * The declaration will not be registered anywhere, so you must care about its deletion.
     *
     * This declaration's text-range will be referenced from the clone, so the clone must not
     * live longer than the original.
     */
    Declaration* clone() const;

    /**
     * Signalized that among multiple possible specializations, this one should be used in the UI from now on.
     *
     * Currently mainly used in C++ for template support. The default-implementation registers the current
     * specialization of this declaration to SpecializationStore if it is nonzero.
     */
    virtual void activateSpecialization();

    enum {
        Identity = 7
    };

protected:

    /**
     * Constructor for copy constructors in subclasses.
     *
     * \param dd data to copy.
     * \param range text range which this object covers.
     */
    Declaration(DeclarationData& dd, const RangeInRevision& range);

    /**
     * Returns true if this declaration is being currently destroyed persistently,
     * which means that it should eventually deregister itself from persistent storage facilities.
     *
     * Only call this from destructors.
     */
    bool persistentlyDestroying() const;

    DUCHAIN_DECLARE_DATA(Declaration)

private:
    /**
     * Sub-classes should implement this and should copy as much information into the clone as possible without breaking the du-chain.
     * Sub-classes should also implement a public copy-constructor that can be used for cloning by sub-classes.
     *
     * \note You do not have to implement this for your language if you are not going to use it(the du-chain itself does not and should not depend on it).
     * */
    virtual Declaration* clonePrivate() const;

    void updateCodeModel();

    void rebuildDynamicData(DUContext* parent, uint ownIndex) override;

    friend class DUContext;
    friend class IndexedDeclaration;
    friend class LocalIndexedDeclaration;
    friend class TopDUContextDynamicData;

    DUContext* m_context = nullptr;
    TopDUContext* m_topContext = nullptr;
    int m_indexInTopContext = 0;
};
}

#endif // KDEVPLATFORM_DECLARATION_H
