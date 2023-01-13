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
    TypePtr<T>() = default;
    explicit TypePtr(T* data) noexcept
        : Base(data)
    {
    }
    TypePtr<T>& operator=(T* data) noexcept
    {
        Base::operator=(data);
        return *this;
    }
    TypePtr<T>(const TypePtr<T>&) = default;
    TypePtr<T>& operator=(const TypePtr<T>&) = default;
    TypePtr<T>(TypePtr<T>&&) = default;
    TypePtr<T>& operator=(TypePtr<T>&&) = default;

    template<class X>
    TypePtr(const TypePtr<X>& o) noexcept
        : Base(o)
    {
    }

    template<class X>
    TypePtr(TypePtr<X>&& o) noexcept
        : Base(std::move(o))
    {
    }

    /**
     * Convert TypePtr<T> to TypePtr<U>, using a @c dynamic_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * @code
     *   TypePtr<T> tPtr;
     *   TypePtr<U> uPtr = tPtr.dynamicCast<U>();
     * @endcode
     * Since a dynamic_cast is used, if U derives from T, and tPtr isn't an instance of U, uPtr will be 0.
     */
    template<class U>
    TypePtr<U> dynamicCast() const
    {
        return TypePtr<U>(dynamic_cast<U*>(Base::data()));
    }

    /**
     * Convert TypePtr<T> to TypePtr<U>, using a @c static_cast.
     * This will compile whenever T* and U* are compatible, i.e.
     * T is a subclass of U or vice-versa.
     * Example syntax:
     * @code
     *   TypePtr<T> tPtr;
     *   TypePtr<U> uPtr = tPtr.staticCast<U>();
     * @endcode
     */
    template<class U>
    TypePtr<U> staticCast() const
    {
        return TypePtr<U>(static_cast<U*>(Base::data()));
    }
};
}

#endif
