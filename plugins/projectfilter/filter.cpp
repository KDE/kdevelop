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

#include "filter.h"

#include <KConfigGroup>

using namespace KDevelop;

Filter::Filter()
    : targets(Files | Folders)
    , matchOn(RelativePath)
    , inclusive(false)
{

}

Filter::Filter(const QString& pattern, Filter::Targets targets, Filter::MatchOn matchOn, bool inclusive)
    : pattern(pattern, Qt::CaseSensitive, QRegExp::Wildcard)
    , targets(targets)
    , matchOn(matchOn)
    , inclusive(inclusive)
{

}

bool Filter::operator==(const Filter& other) const
{
    return pattern == other.pattern
        && targets == other.targets
        && inclusive == other.inclusive;
}

namespace KDevelop {

Filters defaultFilters()
{
    Filters ret;

    Filter filter;
    filter.pattern = QRegExp(".*", Qt::CaseSensitive, QRegExp::Wildcard);
    filter.matchOn = Filter::Basename;
    filter.targets = Filter::Targets(Filter::Files | Filter::Folders);
    ret << filter;

    return ret;
}

Filters readFilters(const KSharedConfig::Ptr& config)
{
    Filters filters;
    if (!config->hasGroup("Filters")) {
        return defaultFilters();
    }
    const KConfigGroup& group = config->group("Filters");

    foreach(const QString& subGroup, group.groupList()) {
        const KConfigGroup& subConfig = group.group(subGroup);
        const QString pattern = subConfig.readEntry("pattern", QString());
        Filter::Targets targets(subConfig.readEntry("targets", 0));
        Filter::MatchOn matchOn = static_cast<Filter::MatchOn>(subConfig.readEntry("matchOn", 0));
        bool inclusive = subConfig.readEntry("inclusive", false);
        filters << Filter(pattern, targets, matchOn, inclusive);
    }

    return filters;
}

void writeFilters(const Filters& filters, KSharedConfig::Ptr config)
{
    // clear existing
    config->deleteGroup("Filters");

    // write new
    KConfigGroup group = config->group("Filters");
    int i = 0;
    foreach(const Filter& filter, filters) {
        KConfigGroup subGroup = group.group(QString::number(i++));
        subGroup.writeEntry("pattern", filter.pattern.pattern());
        subGroup.writeEntry("matchOn", static_cast<int>(filter.matchOn));
        subGroup.writeEntry("targets", static_cast<int>(filter.targets));
        subGroup.writeEntry("inclusive", filter.inclusive);
    }
    if (i == 0) {
        // to make sure we don't think the filters where not configured yet
        group.writeEntry("dummy", false);
    }
}

}
