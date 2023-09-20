/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILTER_H
#define FILTER_H

#include <QRegularExpression>
#include <QVector>
#include <KSharedConfig>

namespace KDevelop {

struct SerializedFilter;

/**
 * The Filter is a the class which is used for actual matching against items.
 *
 * It "compiles" serialized filters for performance and extracts useful information.
 */
struct Filter
{
public:
    enum Target {
        Files = 1,
        Folders = 2
    };
    Q_DECLARE_FLAGS(Targets, Target)

    enum Type {
        /// Hides matched targets.
        Exclusive,
        /// Reverses the match to be inclusive and negates the previously applied exclusive filters.
        Inclusive
    };

    Filter();
    Filter(const SerializedFilter& filter);

    bool operator==(const Filter& filter) const
    {
        return filter.pattern == pattern
            && filter.targets == targets
            && filter.type == type;
    }

    QRegularExpression pattern;
    Targets targets;
    Type type = Exclusive;
};

using Filters = QVector<Filter>;

/**
 * SerializedFilter is what gets stored on disk in the configuration and represents
 * the interface which the user can interact with.
 */
struct SerializedFilter
{
    SerializedFilter();
    SerializedFilter(const QString& pattern, Filter::Targets targets, Filter::Type type = Filter::Exclusive);
    QString pattern;
    Filter::Targets targets;
    Filter::Type type = Filter::Exclusive;
};

using SerializedFilters = QVector<SerializedFilter>;

SerializedFilters defaultFilters();
SerializedFilters readFilters(const KSharedConfigPtr& config);
void writeFilters(const SerializedFilters& filters, KSharedConfigPtr config);
Filters deserialize(const SerializedFilters& filters);

}

Q_DECLARE_TYPEINFO(KDevelop::Filter, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::SerializedFilter, Q_MOVABLE_TYPE);

#endif // FILTER_H
