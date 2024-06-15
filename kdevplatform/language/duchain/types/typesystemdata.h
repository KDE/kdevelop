/*
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_TYPESYSTEMDATA_H
#define KDEVPLATFORM_TYPESYSTEMDATA_H

#include "../appendedlist.h"
#include "indexedtype.h"
#include "delayedtype.h"
#include "identifiedtype.h"
#include "integraltype.h"

namespace KDevelop {
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(FunctionTypeData, m_arguments, IndexedType)

/**
 * Private data structure for AbstractType.
 *
 * Inherit from this for custom type private data.
 *
 * Within your inherited data types, you can use the mechanisms described in appendedlist.h
 *
 * You must explicitly implement the copy constructor, see appendedlist.h for more information on copying dynamic data.
 * When calling initializeAppendedLists() as described there, you should always use m_dynamic as parameter.
 */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractTypeData
{
public:
    /// Constructor.
    AbstractTypeData();
    /**
     * Copy constructor.
     *
     * While cloning, the dynamic/constant attribute alternates(The copy of dynamic data is constant, and the copy of constant data is dynamic)
     * This means that when copying dynamic data, the size of the allocated buffer must be big enough to hold the appended lists.
     * the AbstractType::copyData function cares about that.
     *
     * \param rhs data to copy.
     */
    AbstractTypeData(const AbstractTypeData& rhs);
    /// Destructor.
    ~AbstractTypeData();

    AbstractTypeData& operator=(const AbstractTypeData&) = delete;

    /**
     * Internal setup for the data structure.
     *
     * This must be called from actual class that belongs to this data(not parent classes), and the class must have the
     * "Identity" enumerator with a unique identity. Do NOT call this in copy-constructors!
     */
    template <class T>
    void setTypeClassId()
    {
        static_assert(T::Identity < std::numeric_limits<decltype(typeClassId)>::max(), "TypeClass ID out of bounds");
        typeClassId = T::Identity;
    }

    /// Stores sizeOf in bytes or -1 if unknown.
    int64_t m_sizeOf = -1;

    /**
     * Since alignOf must be integral power of 2, we only need to store the power.
     * The max value (63) represents unknown alignment.
     */
    unsigned m_alignOfExponent : 6;
    static constexpr unsigned MaxAlignOfExponent = 63;

    /// Type modifier flags
    quint32 m_modifiers = AbstractType::NoModifiers;

    /// Reference-count for this type within the repository. Not used for comparison or hashes.
    uint refCount = 0;

    /// Remember which type this data was created for. \sa setTypeClassId()
    quint16 typeClassId;

    /// Remember whether this type is in a TypeRepository. Not used for comparison or hashes.
    bool inRepository : 1;

    APPENDED_LISTS_STUB(AbstractTypeData)

    /// Returns the pure data size of this class(not including anything dynamic).
    uint classSize() const;

    /// Returns the complete size of this item in memory, including derived class data and derived class appended list data
    unsigned int itemSize() const;

    /// Expensive
    size_t hash() const;

    void freeDynamicData()
    {
    }
};

/// Private data structure for IntegralType
class KDEVPLATFORMLANGUAGE_EXPORT IntegralTypeData
    : public AbstractTypeData
{
public:
    /// Constructor
    IntegralTypeData();
    /// Copy constructor. \param rhs data to copy
    IntegralTypeData(const IntegralTypeData& rhs);
    IntegralTypeData& operator=(const IntegralTypeData& rhs) = delete;
    ~IntegralTypeData() = default;
    /// Data type
    uint m_dataType = IntegralType::TypeNone;
};

/// Private data structure for PointerType
class KDEVPLATFORMLANGUAGE_EXPORT PointerTypeData
    : public AbstractTypeData
{
public:
    /// Constructor
    PointerTypeData();
    /// Copy constructor. \param rhs data to copy
    PointerTypeData(const PointerTypeData& rhs);
    ~PointerTypeData() = default;
    PointerTypeData& operator=(const PointerTypeData& rhs) = delete;
    /// Type of data at which the pointer points
    IndexedType m_baseType;
};

/// Private data structure for ReferenceType
class KDEVPLATFORMLANGUAGE_EXPORT ReferenceTypeData
    : public AbstractTypeData
{
public:
    /// Constructor
    ReferenceTypeData();
    /// Copy constructor. \param rhs data to copy
    ReferenceTypeData(const ReferenceTypeData& rhs);
    ~ReferenceTypeData() = default;
    ReferenceTypeData& operator=(const ReferenceTypeData& rhs) = delete;
    /// Type of data which is referenced
    IndexedType m_baseType;
    /// True if this is an rvalue-reference, false for lvalue-references
    bool m_isRValue : 1;
};

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(FunctionTypeData, m_arguments, IndexedType)

/// Private data structure for FunctionType
class KDEVPLATFORMLANGUAGE_EXPORT FunctionTypeData
    : public AbstractTypeData
{
public:
    /// Constructor
    FunctionTypeData();
    /// Copy constructor. \param rhs data to copy
    FunctionTypeData(const FunctionTypeData& rhs);
    /// Destructor
    ~FunctionTypeData();

    /// Function return type
    IndexedType m_returnType;

    START_APPENDED_LISTS_BASE(FunctionTypeData, AbstractTypeData);

    APPENDED_LIST_FIRST(FunctionTypeData, IndexedType, m_arguments);

    END_APPENDED_LISTS(FunctionTypeData, m_arguments);

private:
    void operator=(const FunctionTypeData& rhs);
};

/// Private data structure for ReferenceType
class KDEVPLATFORMLANGUAGE_EXPORT TypeAliasTypeData
    :  public MergeIdentifiedType<AbstractType>::Data
{
public:
    /// Type of data which is typedeffed
    IndexedType m_type;
};

/// Private data structure for StructureType
class KDEVPLATFORMLANGUAGE_EXPORT StructureTypeData
    : public MergeIdentifiedType<AbstractType>::Data
{
public:
    /// Constructor
    StructureTypeData();
    /// Copy constructor. \param rhs data to copy
    StructureTypeData(const StructureTypeData& rhs);
    ~StructureTypeData() = default;
    /// Whether the type is closed yet
    StructureTypeData& operator=(const StructureTypeData& rhs) = delete;
};

/// Private data structure for ArrayType
class KDEVPLATFORMLANGUAGE_EXPORT ArrayTypeData
    : public AbstractTypeData
{
public:
    /// Constructor
    ArrayTypeData();
    /// Copy constructor. \param rhs data to copy
    ArrayTypeData(const ArrayTypeData& rhs);
    ~ArrayTypeData() = default;
    ArrayTypeData& operator=(const ArrayTypeData& rhs) = delete;
    /// Dimension of the array
    int m_dimension = 0;
    /// Element type of the array
    IndexedType m_elementType;
};

/// Private data structure for DelayedType
class KDEVPLATFORMLANGUAGE_EXPORT DelayedTypeData
    : public AbstractTypeData
{
public:
    /// Constructor
    DelayedTypeData();
    /// Copy constructor. \param rhs data to copy
    DelayedTypeData(const DelayedTypeData& rhs);
    ~DelayedTypeData() = default;
    DelayedTypeData& operator=(const DelayedTypeData& rhs) = delete;
    /// Identifier of the delayed type
    IndexedTypeIdentifier m_identifier;
    /// Type of delay in resolving the type
    DelayedType::Kind m_kind = DelayedType::Delayed;
};

/// Private data structure for ConstantIntegralType
struct ConstantIntegralTypeData
    : public IntegralTypeData
{
    /// Constructor
    ConstantIntegralTypeData();
    /// Constant integer value
    qint64 m_value = 0;
};
}
#endif
