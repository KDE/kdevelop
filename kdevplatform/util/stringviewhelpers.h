/*
    SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_STRING_VIEW_HELPERS_H
#define KDEVPLATFORM_STRING_VIEW_HELPERS_H

#include <QString>
#include <QStringView>

namespace KDevelop {

/**
 * @pre @p offset >= 0
 * @return @c true if the substring of @p view from position @p offset starts with @p str
 */
template<class String>
constexpr bool matchesAtOffset(QStringView view, qsizetype offset, String str)
{
    Q_ASSERT(offset >= 0);
    const auto strSize = str.size();
    return offset + strSize <= view.size() && view.mid(offset, strSize) == str;
}

/**
 * @copydoc matchesAtOffset(QStringView,qsizetype,String)
 */
inline bool matchesAtOffset(QStringView view, qsizetype offset, const QString& str)
{
    return matchesAtOffset(view, offset, QStringView{str});
}

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
