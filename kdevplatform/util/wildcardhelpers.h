/*
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_WILDCARDHELPERS_H
#define KDEVPLATFORM_WILDCARDHELPERS_H

#include <QRegularExpression>
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
    QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(pattern, QRegularExpression::UnanchoredWildcardConversion), QRegularExpression::CaseInsensitiveOption);
    return rx.match(fileName).hasMatch();
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
