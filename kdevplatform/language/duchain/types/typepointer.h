/*
    SPDX-FileCopyrightText: 2005 Frerich Raabe <raabe@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KDEVPLATFORM_TYPEPOINTER_H
#define KDEVPLATFORM_TYPEPOINTER_H

#include <QExplicitlySharedDataPointer>

namespace KDevelop {
/**
 * @brief QExplicitlySharedDataPointer wrapper with convenience functions attached
 */
template <class T>
class TypePtr
    : public QExplicitlySharedDataPointer<T>
{
    using Base = QExplicitlySharedDataPointer<T>;

public:
    using Base::QExplicitlySharedDataPointer;
    TypePtr<T>() = default;
    TypePtr<T>(const TypePtr<T>&) = default;
    TypePtr<T>(TypePtr<T>&&) = default;

    using Base::operator=;
    TypePtr<T>& operator=(const TypePtr<T>&) = default;
    TypePtr<T>& operator=(TypePtr<T>&&) = default;

    ///Uses dynamic_cast to cast this pointer to the given type
    template<class U>
    TypePtr<U> cast() const
    {
        return TypePtr<U>(dynamic_cast<U*>(Base::data()));
    }

    /**
     * Convert TypePtr<U> to TypePtr<T>, using a static_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * @code
     *   TypePtr<T> tPtr;
     *   TypePtr<U> uPtr = TypePtr<U>::staticCast( tPtr );
     * @endcode
     */
    template <class U>
    static TypePtr<T> staticCast(const TypePtr<U>& o)
    {
        return TypePtr<T>(static_cast<T*>(o.data()));
    }
    /**
     * Convert TypePtr<U> to TypePtr<T>, using a dynamic_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * @code
     *   TypePtr<T> tPtr;
     *   TypePtr<U> uPtr = TypePtr<U>::dynamicCast( tPtr );
     * @endcode
     * Since a dynamic_cast is used, if U derives from T, and tPtr isn't an instance of U, uPtr will be 0.
     */
    template <class U>
    static TypePtr<T> dynamicCast(const TypePtr<U>& o)
    {
        return TypePtr<T>(dynamic_cast<T*>(o.data()));
    }
};
}

#endif
