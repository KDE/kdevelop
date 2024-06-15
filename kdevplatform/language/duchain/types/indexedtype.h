/*
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_INDEXEDTYPE_H
#define KDEVPLATFORM_INDEXEDTYPE_H

#include "abstracttype.h"

#include <serialization/referencecounting.h>

namespace KDevelop {
/**
 * \short Indexed type pointer.
 *
 * IndexedType is a class which references a type by an index.
 * This way the type can be stored to disk.
 *
 * This class does "disk reference counting"
 * @warning Do not use this after QCoreApplication::aboutToQuit() has been emitted, items that are not disk-referenced will be invalid at that point
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedType
    : public ReferenceCountManager
{
public:
    IndexedType(const IndexedType& rhs);
    /**
     * Construct an indexed type from the @p type
     */
    explicit IndexedType(const AbstractType* type);
    /**
     * Construct an indexed type from the data in the @c TypePtr @p type
     *
     * NOTE: This is required to ensure we don't call `IndexedType(uint)` with a `TypePtr` casted implicitly to bool.
     */
    explicit IndexedType(const AbstractType::Ptr& type)
        : IndexedType(type.data())
    {
    }
    explicit IndexedType(uint index = 0);

    ~IndexedType();

    IndexedType& operator=(const IndexedType& rhs);

    /**
     * Access the type.
     *
     * \returns the type pointer, or null if this index is invalid.
     */
    AbstractType::Ptr abstractType() const;

    /**
     * Access the type, dynamically casted to the type you provide.
     *
     * \returns the type pointer, or null if this index is invalid.
     */
    template<class T>
    TypePtr<T> type() const
    {
        return abstractType().dynamicCast<T>();
    }

    /// Determine if the type is valid. \returns true if valid, otherwise false.
    bool isValid() const
    {
        return ( bool )m_index;
    }

    /// \copydoc isValid
    operator bool() const {
        return ( bool )m_index;
    }

    /// Equivalence operator. \param rhs indexed type to compare. \returns true if equal (or both invalid), otherwise false.
    bool operator==(const IndexedType& rhs) const
    {
        return m_index == rhs.m_index;
    }

    /// Not equal operator. \param rhs indexed type to compare. \returns true if types are not the same, otherwise false.
    bool operator!=(const IndexedType& rhs) const
    {
        return m_index != rhs.m_index;
    }

    /// Less than operator, \param rhs indexed type to compare.
    /// \returns true if integral index value of this type is lower than the index of \p rhs.
    bool operator<(const IndexedType& rhs) const
    {
        return m_index < rhs.m_index;
    }

    /// Access the type's hash value. \returns the hash value.
    size_t hash() const
    {
        return m_index >> 1;
    }

    /// Access the type's index. \returns the index.
    uint index() const
    {
        return m_index;
    }

private:
    ///This class must _never_ hold more than one unsigned integer
    uint m_index;
};

inline size_t qHash(const IndexedType& type)
{
    return type.hash();
}
}

Q_DECLARE_TYPEINFO(KDevelop::IndexedType, Q_MOVABLE_TYPE);

#endif
