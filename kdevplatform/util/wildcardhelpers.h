/*
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_WILDCARDHELPERS_H
#define KDEVPLATFORM_WILDCARDHELPERS_H

#include "utilexport.h"

#include <QRegExp>
#include <QString>
#include <QStringList>

#include <algorithm>

class QSortFilterProxyModel;

namespace WildcardHelpers {
/**
 * Equivalent to @p model.setFilterWildcard(@p pattern), except
 * that the pattern is not interpreted as filepath globbing.
 */
void KDEVPLATFORMUTIL_EXPORT setFilterNonPathWildcard(QSortFilterProxyModel& model, const QString& pattern);

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
