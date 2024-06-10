/*
    SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_STRING_VIEW_HELPERS_H
#define KDEVPLATFORM_STRING_VIEW_HELPERS_H

#include <QStringView>

namespace KDevelop {

/**
 * Finds the first occurrence of @p needle in @p view and returns
 * the subview before the needle or the entire view if not found.
 */
template<typename Needle>
QStringView leftOfNeedleOrEntireView(QStringView view, Needle needle)
{
    const auto index = view.indexOf(needle);
    return index == -1 ? view : view.left(index);
}

/**
 * Finds the last occurrence of @p needle in @p view and returns
 * the subview before the needle or the entire view if not found.
 */
template<typename Needle>
QStringView leftOfLastNeedleOrEntireView(QStringView view, Needle needle)
{
    const auto index = view.lastIndexOf(needle);
    return index == -1 ? view : view.left(index);
}

} // namespace KDevelop

#endif // KDEVPLATFORM_STRING_VIEW_HELPERS_H
