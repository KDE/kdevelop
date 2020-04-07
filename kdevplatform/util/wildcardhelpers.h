/* WildcardHelpers

    Copyright 2020 Igor Kushnir <igorkuo@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_WILDCARDHELPERS_H
#define KDEVPLATFORM_WILDCARDHELPERS_H

#include <QRegExp>
#include <QString>
#include <QStringList>

#include <algorithm>

namespace WildcardHelpers {
/**
 * @note Unlike the QDir::match() overload with the same signature, this
 * function treats its first argument as a single pattern, not a filter with
 * multiple patterns separated by spaces or semicolons.
 */
inline bool matchSinglePattern(const QString& pattern, const QString& fileName)
{
    QRegExp rx(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
    return rx.exactMatch(fileName);
}

/**
 *  @brief Works exactly as the QDir::match() overload with the same signature
 *  before Qt Base commit a2c85bffbeaa027e98fb6c23b2d7919adc8d28b7.
 *
 *  @note In this commit QDir::match() migrated to QRegularExpression and changed
 *  behavior as described in QTBUG-73797. Unlike QRegExp, QRegularExpression
 *  wildcard support follows closely the definition of wildcard for glob patterns.
 *  Most notably '*' in a wildcard no longer matches '/' (and '\\' on Windows).
 */
inline bool match(const QStringList& filters, const QString& fileName)
{
    return std::any_of(filters.cbegin(), filters.cend(), [&fileName](const QString& pattern) {
        return matchSinglePattern(pattern, fileName);
    });
}
}

#endif // KDEVPLATFORM_WILDCARDHELPERS_H
