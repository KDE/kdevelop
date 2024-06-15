/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ABSTRACTTYPE_H
#define KDEVPLATFORM_ABSTRACTTYPE_H

#include "typepointer.h"
#include <language/languageexport.h>

class QString;

namespace KDevelop {
class AbstractTypeData;

class IndexedType;

class TypeVisitor;
class TypeExchanger;

/// This macro is used to declare type-specific data-access functions within subclasses of AbstractType
#define TYPE_DECLARE_DATA(Class) \
    inline Class ## Data * d_func_dynamic() { makeDynamic(); return reinterpret_cast<Class ## Data*>(d_ptr); } \
    inline const Class ## Data* d_func() const { return reinterpret_cast<const Class ## Data*>(d_ptr); }

/// This function creates a local variable named 'd' pointing to the data type (as shortcut)
#define TYPE_D(Class) const Class ## Data * const d = d_func()
#define TYPE_D_DYNAMIC(Class) Class ## Data * const d = d_func_dynamic()

/**
 * \brief Base class for all types.
 *
 * The AbstractType class is a base class from which all types derive.  It features:
 * - mechanisms for visiting types
 * - toString() feature
 * - equivalence feature
 * - cloning of types, and
 * - hashing and indexing
 * - efficient, persistent, and reference-counted storage of types using IndexedType
 *
 *  Type classes are created in a way that allows storing them in memory or on disk
 *  efficiently.  They are classes which can store arbitrary lists immediately after their
 *  private data structures in memory (thus enabling them to be mmapped or memcopied),
 *  or being "dynamic" where you use exactly the same class and same access functions,
 *  but the list data is stored in a temporary KDevVarLengthArray from a central repository,
 *  until we save it back to the static memory-region again.
 *
 * When creating an own (sub-) type, you must:
 * - Override equals(..), hash().
 *   - The functions should _fully_ distinguish all types,
 *     in regard to all stored information, and regarding their identity.
 *   - This can be skipped if you're overriding a base-type which already incorporates
 *     all of your own types status within its equals/hash functions (eg. you don't add own data).
 * - Implement a copy-constructor in which you copy the data from the source using copyData<YourType>()
 * - Override the clone() function in which you use the copy-constructor to clone the type
 * - Add an enumerator "Identity" that contains an arbitrary unique identity value of the type
 * - Register your type in a source-file using REGISTER_TYPE(..), @see typeregister.h
 * - Add a typedef "Data", that contains the actual data of the type using the mechanisms described in appendedlist.h.
 *   That data type must follow the same inheritance chain as the type itself, so it must be based on the Data object
 *   of the base class. See AbstractTypeData.
 * - Use createData<YourType>() to create the data-object in a constructor (which you then reach to the parent constructor)
 * - Use TYPE_DECLARE_DATA(YourType) to declare the data access functions d_func and d_func_dynamic,
 *   and  then use d_func()->.. and d_func_dynamic()->.. to access your type data
 * - Create a constructor that only takes a reference to the type data, and passes it to the parent type
 *
 *   Every type can have only one other type as base-type,
 *   but it can have additional base-classes that are not a direct part of the type-system(@see IdentifiedType).
 *
 *  \sa appendedlist.h
 */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractType
    : public QSharedData
{
public:
    using Ptr = TypePtr<AbstractType>;

    /**
     * An enumeration of common modifiers for data types.
     * If you have any language-specific modifiers that don't belong here,
     * you can add them at/after LanguageSpecificModifier
     * @warning Think twice what information you store into the type-system.
     *          The type-system should store information that is shared among many declarations,
     *          and attributes of specific Declarations like public/private should be stored in
     *          the Declarations themselves, not in the type-system.
     */
    enum CommonModifiers : quint32 {
        NoModifiers                 = 0,

        ConstModifier               = 1 << 0,
        VolatileModifier            = 1 << 1,
        TransientModifier           = 1 << 2,
        NewModifier                 = 1 << 3,
        SealedModifier              = 1 << 4,
        UnsafeModifier              = 1 << 5,
        FixedModifier               = 1 << 6,
        ShortModifier               = 1 << 7,
        LongModifier                = 1 << 8,
        LongLongModifier            = 1 << 9,
        SignedModifier              = 1 << 10,
        UnsignedModifier            = 1 << 11,
        AtomicModifier              = 1 << 12,

        LanguageSpecificModifier    = 1 << 13
    };

    /// Constructor.
    AbstractType();
    /// Constructor from data.
    explicit AbstractType(AbstractTypeData& dd);
    /// Destructor.
    virtual ~AbstractType ();

    AbstractType(const AbstractType& rhs) = delete;
    AbstractType& operator=(const AbstractType& rhs) = delete;

    /**
     * Access the type modifiers
     *
     * \returns the type's modifiers.
     */
    quint32 modifiers() const;

    /**
     * Set the type's modifiers.
     *
     * \param modifiers modifiers of this type.
     */
    void setModifiers(quint32 modifiers);

    /**
     * \returns The size in bytes or -1 if unknown.
     */
    int64_t sizeOf() const;

    /**
     * Set the size to given number of bytes. Use -1 to represent unknown size.
     */
    void setSizeOf(int64_t sizeOf);

    /**
     * \returns The alignment in bytes or -1 if unknown.
     */
    int64_t alignOf() const;

    /**
     * Set the alignment to given number of bytes.
     *
     * The value must be non-negative power of 2 or -1 to represent unknown alignment.
     */
    void setAlignOf(int64_t alignedTo);

    /**
     * Visitor method.  Called by TypeVisitor to visit the type hierarchy.
     * Do not reimplement this, reimplement accept0 instead.
     *
     * \param v visitor which is calling this function.
     */
    void accept(TypeVisitor* v) const;

    /**
     * Convenience visitor method which can be called with a null type.
     *
     * \param type type to visit, may be null.
     * \param v visitor which is visiting the given \a type
     */
    static void acceptType(AbstractType::Ptr type, TypeVisitor* v);

    /**
     * Returns this type as a string, preferably the same as it is expressed in the code.
     *
     * \return this type as a string
     */
    virtual QString toString() const;

    ///Must always be called before anything in the data pointer is changed!
    ///If it's not called beforehand, the type-repository gets corrupted
    void makeDynamic();

    ///Should return whether this type's content equals the given one
    ///Since this is used by the type-repository, it must compare ALL members of the data type.
    virtual bool equals(const AbstractType* rhs) const;

    ///Should return whether this type contains the given one
    ///For types holding only a single type, this is identical to equals()
    virtual bool contains(const AbstractType* type) const;

    /**
     * Should create a clone of the source-type, with as much data copied as possible without breaking the du-chain.
     * */
    virtual AbstractType* clone() const = 0;

    /**
     * A hash-value that should have the following properties:
     * - When two types match on equals(), it should be same.
     * - When two types don't match on equals(), it should be different with a high probability.
     * */
    virtual size_t hash() const;

    IndexedType indexed() const;

    /// Enumeration of major data types.
    enum WhichType : quint8 {
        TypeAbstract /**< an abstract type */,
        TypeIntegral /**< an integral */,
        TypePointer /**< a pointer*/,
        TypeReference /**< a reference */,
        TypeFunction /**< a function */,
        TypeStructure /**< a structure */,
        TypeArray /**< an array */,
        TypeDelayed /**< a delayed type */,
        TypeEnumeration /**< an enumeration type */,
        TypeEnumerator /**< an enumerator type */,
        TypeAlias /**< a type-alias type */,
        TypeUnsure /**< may represent multiple different types */
    };

    /**
     * Determine which data type this abstract type represents.
     *
     * \returns the data type represented by this type.
     */
    virtual WhichType whichType() const;

    enum {
        Identity = 1
    };

    /**
     * Should, like accept0, be implemented by all types that hold references to other types.
     *
     * If this is called on one type, that type should call exchangeTypes(..) with all its referenced sub-types.
     * The type itself does not recurse into the sub-types, that can be done by the exchanger itself if desired.
     * */
    virtual void exchangeTypes(TypeExchanger* exchanger);

    /**
     * Method to create copies of internal type data. You must use this to create the internal
     * data instances in copy constructors. It is needed, because it may need to allocate more memory
     * for appended lists.
     *
     * \param rhs data to copy
     * \returns copy of the data
     */

    template <class Type>
    static typename Type::Data& copyData(const typename Type::Data& rhs)
    {
        uint size;
        if (!rhs.m_dynamic)
            size = sizeof(typename Type::Data); //Create a dynamic data instance
        else
            size = rhs.dynamicSize(); //Create a constant data instance, that holds all the data embedded.

        typename Type::Data& ret(*new (new char[size]) typename Type::Data(rhs));
        ret.template setTypeClassId<Type>();
        return ret;
    }

    /**
     * As above, but does not support copying data into a lower class(Should not be used while cloning)
     */
    template <class DataType>
    static DataType& copyDataDirectly(const DataType& rhs)
    {
        uint size;
        if (!rhs.m_dynamic)
            size = sizeof(DataType); //Create a dynamic data instance
        else
            size = rhs.dynamicSize(); //Create a constant data instance, that holds all the data embedded.

        return *new (new char[size]) DataType(rhs);
    }

    /**
     * Method to create internal data structures. Use this in normal constructors.
     *
     * \returns the internal data structure
     */
    template <class Type>
    static typename Type::Data& createData()
    {
        typename Type::Data& ret(*new (new char[sizeof(typename Type::Data)]) typename Type::Data());
        ret.template setTypeClassId<Type>();
        return ret;
    }

    using Data = AbstractTypeData;

protected:
    /**
     * Visitor method, reimplement to allow visiting of types.
     *
     * \param v visitor which is visiting.
     */
    virtual void accept0 (TypeVisitor* v) const = 0;

    /// toString() function which can provide \a spaceOnLeft rather than on right if desired.
    QString toString(bool spaceOnLeft) const;

    AbstractTypeData* d_ptr;

    TYPE_DECLARE_DATA(AbstractType)

    friend class AbstractTypeDataRequest;
};
}

#endif
