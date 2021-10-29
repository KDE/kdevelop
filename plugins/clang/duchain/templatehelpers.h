/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TEMPLATEHELPERS_H
#define TEMPLATEHELPERS_H

#include <QDebug>

#include <type_traits>

enum class Decision
{
    True,
    False,
    Maybe
};

// see also: http://flamingdangerzone.com/cxx11/2012/06/01/almost-static-if.html
namespace detail { enum class enabler {}; }
constexpr detail::enabler dummy = {};
template <bool Condition>
using EnableIf = typename std::enable_if<Condition, detail::enabler>::type;

inline QDebug operator<<(QDebug dbg, Decision decision)
{
    switch (decision) {
    case Decision::True:  dbg << "true"; break;
    case Decision::False: dbg << "false"; break;
    default:              dbg << "maybe"; break;
    }
    return dbg;
}

#endif //TEMPLATEHELPERS_H
