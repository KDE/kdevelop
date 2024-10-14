/*
    SPDX-FileCopyrightText: 2024 Jarmo Tiitto <jarmo.tiitto@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KDEVPLATFORM_SCOPEDINCREMENTOR_H
#define KDEVPLATFORM_SCOPEDINCREMENTOR_H

#include <QtAssert>
#include <QtClassHelperMacros>

namespace KDevelop {

template<typename T = int>
class NonNegative
{
public:
    NonNegative& operator++()
    {
        ++m_value;
        return *this;
    }

    NonNegative& operator--()
    {
        Q_ASSERT(m_value);
        --m_value;
        return *this;
    }

    operator bool() const
    {
        return m_value;
    }

private:
    T m_value = 0;
};

template<typename T = NonNegative<int>>
class ScopedIncrementor
{
public:
    explicit ScopedIncrementor(T& value)
        : m_value(value)
    {
        ++m_value;
    }

    ~ScopedIncrementor()
    {
        --m_value;
    }

    Q_DISABLE_COPY_MOVE(ScopedIncrementor)

private:
    T& m_value;
};

}

#endif // KDEVPLATFORM_SCOPEDINCREMENTOR_H
