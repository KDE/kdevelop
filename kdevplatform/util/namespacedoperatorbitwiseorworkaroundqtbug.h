/*
    SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NAMESPACED_OPERATOR_BITWISE_OR_WORKAROUND_QT_BUG_H
#define NAMESPACED_OPERATOR_BITWISE_OR_WORKAROUND_QT_BUG_H

/// This header works around QTBUG-126820 and can be safely removed once we require a Qt version where the bug
/// is fixed. Until then, each file that declares an operator| with any parameter types, directly or indirectly
/// (e.g. via the macro Q_DECLARE_OPERATORS_FOR_FLAGS()), in a non-global named namespace must include
/// this header to prevent hiding QKeyCombination-returning operators within the said named namespace.

/// This header should be copied to and used in other projects without modifications, including
/// keeping the header guard name unchanged, in order to prevent conflicts if two such headers
/// from different projects end up being included in a translation unit.

#include <QtVersionChecks>

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)

#include <QKeyCombination>
#include <Qt>

#include <type_traits>

template<typename Modifiers, typename Key>
using KeyCombinationIfModifiersAndKeyTypes =
    std::enable_if_t<(std::is_same_v<Modifiers, Qt::Modifier> || std::is_same_v<Modifiers, Qt::KeyboardModifier>
                      || std::is_same_v<Modifiers, Qt::Modifiers> || std::is_same_v<Modifiers, Qt::KeyboardModifiers>)
                         && std::is_same_v<Key, Qt::Key>,
                     QKeyCombination>;

namespace Qt {
// NOTE: Compilers prefer a non-template function over a function template specialization with identical
// signature during overload resolution. Therefore defining function templates rather than non-template
// functions here prevents ambiguous overload compilation errors in the following scenarios:
// 1. operator| is used in global or unnamed namespace;
// 2. `using ::operator|;` is declared before operator| usage;
// 3. QKeyCombination-returning operators defined in QtCore/qnamespace.h are enclosed in namespace Qt.
template<typename Modifiers, typename Key>
constexpr ::KeyCombinationIfModifiersAndKeyTypes<Modifiers, Key> operator|(Modifiers modifiers, Key key) noexcept
{
    return QKeyCombination(modifiers, key);
}
template<typename Modifiers, typename Key>
constexpr ::KeyCombinationIfModifiersAndKeyTypes<Modifiers, Key> operator|(Key key, Modifiers modifiers) noexcept
{
    return QKeyCombination(modifiers, key);
}
} // namespace Qt

#endif // QT_VERSION

#endif // NAMESPACED_OPERATOR_BITWISE_OR_WORKAROUND_QT_BUG_H
