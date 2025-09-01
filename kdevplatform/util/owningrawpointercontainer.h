/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_OWNING_RAW_POINTER_CONTAINER_H
#define KDEVPLATFORM_OWNING_RAW_POINTER_CONTAINER_H

#include <utility>

namespace KDevelop {

/**
 * A RAII wrapper for a container of owning raw pointers.
 *
 * Storing an OwningRawPointerContainer<C> in a class is equivalent to storing
 * C @a c itself and calling qDeleteAll(@a c) in the destructor. The only usage
 * difference is dereferencing OwningRawPointerContainer via operator* or operator->.
 */
template<typename C>
class OwningRawPointerContainer
{
public:
    OwningRawPointerContainer() = default;

    explicit OwningRawPointerContainer(const C& c)
        : m_c{c}
    {
    }

    explicit OwningRawPointerContainer(C&& c)
        : m_c{std::move(c)}
    {
    }

    OwningRawPointerContainer(OwningRawPointerContainer&& other)
        : m_c{std::move(other.m_c)}
    {
        other.m_c = {};
    }

    OwningRawPointerContainer(const OwningRawPointerContainer&) = delete;

    ~OwningRawPointerContainer()
    {
        for (const auto* ptr : std::as_const(m_c)) {
            delete ptr;
        }
    }

    OwningRawPointerContainer& operator=(OwningRawPointerContainer&& other)
    {
        m_c = std::move(other.m_c);
        other.m_c = {};
    }

    OwningRawPointerContainer& operator=(const OwningRawPointerContainer&) = delete;

    const C& operator*() const
    {
        return m_c;
    }
    C& operator*()
    {
        return m_c;
    }

    const C* operator->() const
    {
        return &m_c;
    }
    C* operator->()
    {
        return &m_c;
    }

private:
    C m_c{};
};
}

#endif // KDEVPLATFORM_OWNING_RAW_POINTER_CONTAINER_H
