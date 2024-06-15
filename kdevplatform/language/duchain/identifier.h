/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_IDENTIFIER_H
#define KDEVPLATFORM_IDENTIFIER_H

#include <QList>
#include <QMetaType>
#include <QStringList>
#include <QStringView>

#include <util/kdevvarlengtharray.h>

#include <language/languageexport.h>
#include <serialization/referencecounting.h>

//We use shared d-pointers, which is even better than a d-pointer, but krazy probably won't get it, so exclude the test.
//krazy:excludeall=dpointer

namespace KDevelop {
class IndexedTypeIdentifier;
class Identifier;
class QualifiedIdentifier;
template <bool>
class QualifiedIdentifierPrivate;
template <bool>
class IdentifierPrivate;
class IndexedString;

/**
 * @note Move constructor and move assignment operator are deliberately not implemented for
 * Indexed[Qualified]Identifier. The move operations are tricky to implement correctly and more
 * efficiently in practice than the copy operations. swap() could be specialized for these two
 * classes, but it would never be called in practice. See a similar note for class IndexedString.
 */

/**
 * A helper-class to store an identifier by index in a type-safe way.
 *
 * The difference to Identifier is that this class only stores the index of an identifier that is in the repository, without any dynamic
 * abilities or access to the contained data.
 *
 * This class does "disk reference counting"
 *
 * @warning Do not use this after QCoreApplication::aboutToQuit() has been emitted, items that are not disk-referenced will be invalid at that point.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedIdentifier
    : public ReferenceCountManager
{
public:
    IndexedIdentifier();
    explicit IndexedIdentifier(const Identifier& id);
    IndexedIdentifier(const IndexedIdentifier& rhs) noexcept;
    IndexedIdentifier& operator=(const Identifier& id);
    IndexedIdentifier& operator=(const IndexedIdentifier& rhs) noexcept;
    ~IndexedIdentifier();
    bool operator==(const IndexedIdentifier& rhs) const;
    bool operator!=(const IndexedIdentifier& rhs) const;
    bool operator==(const Identifier& id) const;

    bool isEmpty() const;

    Identifier identifier() const;
    operator Identifier() const;

    unsigned int index() const
    {
        return m_index;
    }

private:
    explicit IndexedIdentifier(unsigned int index);
    IndexedIdentifier& operator=(unsigned int index);

    unsigned int m_index;
};

/**
 * A helper-class to store an identifier by index in a type-safe way.
 *
 * The difference to QualifiedIdentifier is that this class only stores the index of an identifier that is in the repository, without any dynamic
 * abilities or access to the contained data.
 *
 * This class does "disk reference counting"
 *
 * @warning Do not use this after QCoreApplication::aboutToQuit() has been emitted, items that are not disk-referenced will be invalid at that point.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedQualifiedIdentifier
    : public ReferenceCountManager
{
public:
    IndexedQualifiedIdentifier();
    IndexedQualifiedIdentifier(const QualifiedIdentifier& id);
    IndexedQualifiedIdentifier(const IndexedQualifiedIdentifier& rhs) noexcept;
    IndexedQualifiedIdentifier& operator=(const QualifiedIdentifier& id);
    IndexedQualifiedIdentifier& operator=(const IndexedQualifiedIdentifier& id) noexcept;
    ~IndexedQualifiedIdentifier();
    bool operator==(const IndexedQualifiedIdentifier& rhs) const;
    bool operator==(const QualifiedIdentifier& id) const;

    bool operator<(const IndexedQualifiedIdentifier& rhs) const
    {
        return m_index < rhs.m_index;
    }

    bool isValid() const;
    bool isEmpty() const;

    QualifiedIdentifier identifier() const;
    operator QualifiedIdentifier() const;

    unsigned int index() const
    {
        return m_index;
    }

private:
    explicit IndexedQualifiedIdentifier(unsigned int index);
    IndexedQualifiedIdentifier& operator=(unsigned int index);
    unsigned int m_index;
};

/**
 * Flags to control the string representation of identifiers.
 */
enum IdentifierStringFormattingOption {
    NoOptions = 0x0,

    /// Removes explicit global prefix from the result.
    /// When enabled, global identifiers will be formatted as "globalIdentifierFormattedString"
    /// instead "::globalIdentifierFormattedString".
    RemoveExplicitlyGlobalPrefix = 0x1,

    /// Removes template information from the result.
    /// When enabled, TemplateClass< someDataType > will be formatted as plain "TemplateClass".
    RemoveTemplateInformation = 0x2
};
Q_DECLARE_FLAGS(IdentifierStringFormattingOptions, IdentifierStringFormattingOption)

/**
 * Represents a single unqualified identifier
 */
class KDEVPLATFORMLANGUAGE_EXPORT Identifier
{
    friend class QualifiedIdentifier;

public:
    /**
     * @param start The position in the given string where to start searching for the identifier. (optional)
     * @param takenRange If this is nonzero, it will be filled with the length of the range from the beginning
     *                   of the given string, that was used to construct this identifier. (optional)
     *
     * @warning The identifier is parsed in a C++-similar way, and the result may not be what you expect.
     *          If you want to prevent that parsing, use the constructor that takes IndexedString.
     */
    explicit Identifier(QStringView str, uint start = 0, uint* takenRange = nullptr);
    explicit Identifier(const QString& str, uint start = 0, uint* takenRange = nullptr)
        : Identifier(QStringView{str}, start, takenRange)
    {
    }

    /**
     * Preferred constructor, use this if you already have an IndexedString available. This does not decompose the given string.
     */
    explicit Identifier(const IndexedString& str);
    Identifier(const Identifier& rhs);
    explicit Identifier(uint index);
    Identifier();
    Identifier(Identifier&& rhs) Q_DECL_NOEXCEPT;
    ~Identifier();
    Identifier& operator=(const Identifier& rhs);
    Identifier& operator=(Identifier&& rhs) Q_DECL_NOEXCEPT;

    static Identifier unique(int token);

    bool isUnique() const;
    int uniqueToken() const;
    /**
     * If \a token is non-zero, turns this Identifier into the special per-document unique identifier.
     *
     * This is used e.g. for anonymous namespaces.
     *
     * Pass a token which is specific to the document to allow correct equality comparison.
     */
    void setUnique(int token);

    const IndexedString identifier() const;
    void setIdentifier(QStringView identifier);
    /**
     * Should be preferred over the other version
     */
    void setIdentifier(const IndexedString& identifier);

    uint hash() const;

    /**
     * Comparison ignoring the template-identifiers
     */
    bool nameEquals(const Identifier& rhs) const;

    /**
     * @warning This is expensive.
     */
    IndexedTypeIdentifier templateIdentifier(int num) const;
    uint templateIdentifiersCount() const;
    void appendTemplateIdentifier(const IndexedTypeIdentifier& identifier);
    void clearTemplateIdentifiers();
    void setTemplateIdentifiers(const QList<IndexedTypeIdentifier>& templateIdentifiers);

    QString toString(IdentifierStringFormattingOptions options = NoOptions) const;

    /**
     * Whether this identifier begins with an underscore followed by a capital letter or by another underscore.
     */
    bool isReserved() const;

    bool operator==(const Identifier& rhs) const;
    bool operator!=(const Identifier& rhs) const;

    bool isEmpty() const;

    /**
     * @return a unique index within the global identifier repository for this identifier.
     *
     * If the identifier isn't in the repository yet, it is added to the repository.
     */
    uint index() const;

    bool inRepository() const;

private:
    void makeConstant() const;
    void prepareWrite();

    //Only one of the following pointers is valid at a given time
    mutable uint m_index; //Valid if cd is valid
    union {
        mutable IdentifierPrivate<true>* dd; //Dynamic, owned by this identifier
        mutable const IdentifierPrivate<false>* cd; //Constant, owned by the repository
    };
};

/**
 * Represents a qualified identifier
 *
 * QualifiedIdentifier has it's hash-values stored, so using the hash-values is very efficient.
 */
class KDEVPLATFORMLANGUAGE_EXPORT QualifiedIdentifier
{
public:
    explicit QualifiedIdentifier(QStringView id, bool isExpression = false);
    explicit QualifiedIdentifier(const QString& id, bool isExpression = false)
        : QualifiedIdentifier(QStringView{id}, isExpression)
    {
    }

    explicit QualifiedIdentifier(const Identifier& id);
    QualifiedIdentifier(const QualifiedIdentifier& id);
    explicit QualifiedIdentifier(uint index);
    QualifiedIdentifier();
    QualifiedIdentifier(QualifiedIdentifier&& rhs) Q_DECL_NOEXCEPT;
    ~QualifiedIdentifier();
    QualifiedIdentifier& operator=(const QualifiedIdentifier& rhs);
    QualifiedIdentifier& operator=(QualifiedIdentifier&& rhs) Q_DECL_NOEXCEPT;

    /**
     * Append @p id to this qualified identifier.
     */
    void push(const IndexedIdentifier& id);
    /**
     * Append @p id to this qualified identifier.
     *
     * NOTE: If you have an indexed identifier available, use the above method instead.
     */
    void push(const Identifier& id);
    /**
     * Append all identifiers of @p id to this qualified identifier.
     */
    void push(const QualifiedIdentifier& id);

    /**
     * Pops one identifier from back:
     */
    void pop();
    void clear();
    bool isEmpty() const;
    int count() const;
    Identifier first() const;
    IndexedIdentifier indexedFirst() const;
    Identifier last() const;
    IndexedIdentifier indexedLast() const;
    Identifier top() const;
    Identifier at(int i) const;
    IndexedIdentifier indexedAt(int i) const;
    /**
     * @param pos Position where to start the copy.
     * @param len If this is -1, the whole following part will be returned.
     */
    QualifiedIdentifier mid(int pos, int len = -1) const;

    /**
     * Copy the leftmost \a len number of identifiers.
     *
     * @param len The number of identifiers to copy, or if negative, the number of identifiers to omit from the right
     */
    inline QualifiedIdentifier left(int len) const
    {
        return mid(0, len > 0 ? len : count() + len);
    }

    ///@todo Remove this flag
    bool explicitlyGlobal() const;
    void setExplicitlyGlobal(bool eg);
    bool isQualified() const;

    /**
     * A flag that can be set by setIsExpression
     */
    bool isExpression() const;
    /**
     * Set the expression-flag, that can be retrieved by isExpression().
     * This flag is not respected while creating the hash-value and while operator==() comparison.
     * It is respected while isSame(..) comparison.
     */
    void setIsExpression(bool);

    QString toString(IdentifierStringFormattingOptions options = NoOptions) const;
    QStringList toStringList(IdentifierStringFormattingOptions options = NoOptions) const;

    QualifiedIdentifier operator+(const QualifiedIdentifier& rhs) const;
    QualifiedIdentifier& operator+=(const QualifiedIdentifier& rhs);

    /**
     * Nicer interfaces to merge
     */
    QualifiedIdentifier operator+(const Identifier& rhs) const;
    QualifiedIdentifier& operator+=(const Identifier& rhs);

    QualifiedIdentifier operator+(const IndexedIdentifier& rhs) const;
    QualifiedIdentifier& operator+=(const IndexedIdentifier& rhs);

    /**
     * @return a QualifiedIdentifier with this one appended to the other.
     *
     * It is explicitly global if either this or base is.
     */
    QualifiedIdentifier merge(const QualifiedIdentifier& base) const;

    /**
     * The comparison-operators do not respect explicitlyGlobal and isExpression, they only respect the real scope.
     * This is for convenient use in hash-tables etc.
     */
    bool operator==(const QualifiedIdentifier& rhs) const;
    bool operator!=(const QualifiedIdentifier& rhs) const;

    bool beginsWith(const QualifiedIdentifier& other) const;

    uint index() const;

    /**
     * @return true if this qualified identifier is already in the persistent identifier repository
     */
    bool inRepository() const;

    /**
     * The hash does not respect explicitlyGlobal, only the real scope.
     */
    uint hash() const;

protected:
    bool sameIdentifiers(const QualifiedIdentifier& rhs) const;

    void makeConstant() const;
    void prepareWrite();

    mutable uint m_index;
    union {
        mutable QualifiedIdentifierPrivate<true>* dd;
        mutable const QualifiedIdentifierPrivate<false>* cd;
    };
};

/**
 * Extends IndexedQualifiedIdentifier by:
 * - Arbitrary count of pointer-poperators with cv-qualifiers
 * - Reference operator
 * All the properties set here are respected in the hash value.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedTypeIdentifier
{
public:
    /**
     * Variables like pointerDepth, isReference, etc. are not parsed from the string, so this parsing is quite limited.
     */
    explicit IndexedTypeIdentifier(const IndexedQualifiedIdentifier& identifier = IndexedQualifiedIdentifier());
    explicit IndexedTypeIdentifier(QStringView identifer, bool isExpression = false);

    bool isReference() const;
    void setIsReference(bool);

    bool isRValue() const;
    void setIsRValue(bool);

    bool isConstant() const;
    void setIsConstant(bool);

    bool isVolatile() const;
    void setIsVolatile(bool);

    IndexedQualifiedIdentifier identifier() const;

    void setIdentifier(const IndexedQualifiedIdentifier& id);

    /**
     * @return the pointer depth. Example for C++: "char*" has pointer-depth 1, "char***" has pointer-depth 3
     */
    int pointerDepth() const;
    /**
     * Sets the pointer-depth to the specified count.
     *
     * When the pointer-depth is increased, the "isConstPointer" values for new depths will be initialized with false.
     *
     * For efficiency-reasons the maximum currently is 23.
     */
    void setPointerDepth(int);

    /**
     * Whether the target of pointer 'depthNumber' is constant
     */
    bool isConstPointer(int depthNumber) const;
    void setIsConstPointer(int depthNumber, bool constant);

    QString toString(IdentifierStringFormattingOptions options = NoOptions) const;

    uint hash() const;

    /**
     * The comparison-operators do not respect explicitlyGlobal and isExpression, they only respect the real scope.
     * This is for convenient use in hash-tables etc.
     */
    bool operator==(const IndexedTypeIdentifier& rhs) const;
    bool operator!=(const IndexedTypeIdentifier& rhs) const;

private:
    IndexedQualifiedIdentifier m_identifier;
    // The overall number of bits shared by these bit-fields should not exceed 32,
    // so that we don't waste space. IndexedTypeIdentifer should be as compact as possible.
    bool m_isConstant : 1;
    bool m_isReference : 1;
    bool m_isRValue : 1;
    bool m_isVolatile : 1;
    uint m_pointerDepth : 5;
    uint m_pointerConstMask : 23;
};

KDEVPLATFORMLANGUAGE_EXPORT size_t qHash(const IndexedTypeIdentifier& id);
KDEVPLATFORMLANGUAGE_EXPORT size_t qHash(const QualifiedIdentifier& id);
KDEVPLATFORMLANGUAGE_EXPORT size_t qHash(const Identifier& id);

inline size_t qHash(const IndexedIdentifier& id)
{
    return id.index();
}

inline size_t qHash(const IndexedQualifiedIdentifier& id)
{
    return id.index();
}
}

Q_DECLARE_TYPEINFO(KDevelop::IndexedTypeIdentifier, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::IndexedQualifiedIdentifier, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::IndexedIdentifier, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::IndexedQualifiedIdentifier)
Q_DECLARE_METATYPE(KDevelop::IndexedIdentifier)

Q_DECLARE_TYPEINFO(KDevelop::QualifiedIdentifier, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::Identifier, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::QualifiedIdentifier)
Q_DECLARE_METATYPE(KDevelop::Identifier)

/**
 * {q,k}Debug() stream operator: Writes the Identifier to the debug output.
 */
KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::Identifier& identifier);

/**
 * {q,k}Debug() stream operator: Writes the QualifiedIdentifier to the debug output.
 */
KDEVPLATFORMLANGUAGE_EXPORT QDebug operator<<(QDebug s, const KDevelop::QualifiedIdentifier& identifier);

#endif // KDEVPLATFORM_IDENTIFIER_H
