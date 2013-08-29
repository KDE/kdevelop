/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FILTER_H
#define FILTER_H

#include <QRegExp>
#include <QVector>
#include <KSharedConfig>

namespace KDevelop {

struct Filter
{
public:
    enum Target {
        Files = 1,
        Folders = 2
    };
    Q_DECLARE_FLAGS(Targets, Target);
    enum MatchOn {
        RelativePath,
        Basename
    };

    enum Type {
        /// Hides matched targets.
        Exclusive,
        /// Reverses the match to be inclusive and negates the previously applied exclusive filters.
        Inclusive
    };

    Filter();
    Filter(const QString& pattern, Targets targets, MatchOn matchOn, Type type = Exclusive);

    bool operator==(const Filter& other) const;

    QRegExp pattern;
    Targets targets;
    MatchOn matchOn;
    Type type;
};

typedef QVector<Filter> Filters;

Filters defaultFilters();

Filters readFilters(const KSharedConfig::Ptr& config);
void writeFilters(const Filters& filters, KSharedConfig::Ptr config);

}

Q_DECLARE_TYPEINFO(KDevelop::Filter, Q_MOVABLE_TYPE);

#endif // FILTER_H
