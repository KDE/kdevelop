/*
    SPDX-FileCopyrightText: 2024 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "wildcardhelpers.h"

#include <QRegularExpression>
#include <QSortFilterProxyModel>

void WildcardHelpers::setFilterNonPathWildcard(QSortFilterProxyModel& model, const QString& pattern)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    // This code would have been equivalent to `model.setFilterWildcard(pattern);` had the latter passed the
    // NonPathWildcardConversion flag to wildcardToRegularExpression(). This implementation is based on comparison of
    // the definitions of QSortFilterProxyModel's setFilterRegularExpression() and setFilterWildcard() member functions.
    model.setFilterRegularExpression(QRegularExpression::wildcardToRegularExpression(
        pattern, QRegularExpression::UnanchoredWildcardConversion | QRegularExpression::NonPathWildcardConversion));
#else
    // Fall back to the filepath-globbing wildcard when NonPathWildcardConversion is unavailable.
    model.setFilterWildcard(pattern);
#endif
}
