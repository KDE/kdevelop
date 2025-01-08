/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TOGGLE_ONLY_BOOL_H
#define KDEVPLATFORM_TOGGLE_ONLY_BOOL_H

#include <QScopedValueRollback>
#include <QtAssert>

namespace KDevelop {

/**
 * This class stores a bool value and asserts that the value is toggled each time the assignment operator is called.
 *
 * ToggleOnlyBool can be used to verify that a simple bool state variable is never "locked" recursively
 * (like a mutex). Use QScopedValueRollback to limit the "locking" of a ToggleOnlyBool to a scope.
 * Do not call the single-argument constructor or commit() of QScopedValueRollback<ToggleOnlyBool>,
 * because ~QScopedValueRollback() would likely trigger an assertion failure in ToggleOnlyBool::operator=() then.
 * Use ToggleOnlyBool::makeGuard() to call the safe two-argument constructor of QScopedValueRollback<ToggleOnlyBool>.
 *
 * If recursive "locking" needs to be supported, use NonNegative and ScopedIncrementor instead of this class.
 */
class ToggleOnlyBool
{
public:
    explicit constexpr ToggleOnlyBool(bool value) noexcept
        : m_value(value)
    {
    }

    [[nodiscard]] constexpr operator bool() const noexcept
    {
        return m_value;
    }

    bool operator=(bool value)
    {
        Q_ASSERT(m_value != value);
        m_value = value;
        return value;
    }

    [[nodiscard]] QScopedValueRollback<ToggleOnlyBool> makeGuard(bool temporaryValue)
    {
        return QScopedValueRollback(*this, ToggleOnlyBool{temporaryValue});
    }

private:
    bool m_value;
};
}

#endif // KDEVPLATFORM_TOGGLE_ONLY_BOOL_H
