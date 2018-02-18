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
    , type(Exclusive)
{

}

Filter::Filter(const SerializedFilter& filter)
    : pattern(QString(), Qt::CaseSensitive, QRegExp::WildcardUnix)
    , targets(filter.targets)
    , type(filter.type)
{
    QString pattern = filter.pattern;
    if (!filter.pattern.startsWith('/') && !filter.pattern.startsWith('*')) {
        // implicitly match against trailing relative path
        pattern.prepend(QLatin1String("*/"));
    }
    if (pattern.endsWith('/') && targets != Filter::Files) {
        // implicitly match against folders
        targets = Filter::Folders;
        pattern.chop(1);
    }
    this->pattern.setPattern(pattern);
}

SerializedFilter::SerializedFilter()
    : targets(Filter::Files | Filter::Folders)
    , type(Filter::Exclusive)
{

}

SerializedFilter::SerializedFilter(const QString& pattern, Filter::Targets targets, Filter::Type type)
    : pattern(pattern)
    , targets(targets)
    , type(type)
{

}

namespace KDevelop {

SerializedFilters defaultFilters()
{
    SerializedFilters ret;

    // filter hidden files
    ret << SerializedFilter(QStringLiteral(".*"), Filter::Targets(Filter::Files | Filter::Folders));
    // but do show some with special meaning
    ret << SerializedFilter(QStringLiteral(".gitignore"), Filter::Files, Filter::Inclusive)
        << SerializedFilter(QStringLiteral(".gitmodules"), Filter::Files, Filter::Inclusive);

    // common vcs folders which we want to hide
    static const QVector<QString> invalidFolders = {
        QStringLiteral(".git"), QStringLiteral("CVS"), QStringLiteral(".svn"), QStringLiteral("_svn"),
        QStringLiteral("SCCS"), QStringLiteral("_darcs"), QStringLiteral(".hg"), QStringLiteral(".bzr"), QStringLiteral("__pycache__")
    };
    foreach(const QString& folder, invalidFolders) {
        ret << SerializedFilter(folder, Filter::Folders);
    }

    // common files which we want to hide
    static const QVector<QString> filePatterns = {
        // binary files (Unix)
        QStringLiteral("*.o"), QStringLiteral("*.a"), QStringLiteral("*.so"), QStringLiteral("*.so.*"),
        // binary files (Windows)
        QStringLiteral("*.obj"), QStringLiteral("*.lib"), QStringLiteral("*.dll"), QStringLiteral("*.exp"), QStringLiteral("*.pdb"),
        // generated files
        QStringLiteral("moc_*.cpp"), QStringLiteral("*.moc"), QStringLiteral("ui_*.h"), QStringLiteral("*.qmlc"), QStringLiteral("qrc_*.cpp"),
        // backup files
        QStringLiteral("*~"), QStringLiteral("*.orig"), QStringLiteral(".*.kate-swp"), QStringLiteral(".*.swp"),
        // python cache and object files
        QStringLiteral("*.pyc"), QStringLiteral("*.pyo")
    };
    foreach(const QString& filePattern, filePatterns) {
        ret << SerializedFilter(filePattern, Filter::Files);
    }

    return ret;
}

SerializedFilters readFilters(const KSharedConfigPtr& config)
{
    if (!config->hasGroup("Filters")) {
        return defaultFilters();
    }
    const KConfigGroup& group = config->group("Filters");
    const int size = group.readEntry("size", -1);
    if (size == -1) {
        // fallback
        return defaultFilters();
    }

    SerializedFilters filters;
    filters.reserve(size);
    for (int i = 0; i < size; ++i) {
        const QByteArray subGroup = QByteArray::number(i);
        if (!group.hasGroup(subGroup)) {
            continue;
        }
        const KConfigGroup& subConfig = group.group(subGroup);
        const QString pattern = subConfig.readEntry("pattern", QString());
        Filter::Targets targets(subConfig.readEntry("targets", 0));
        Filter::Type type = static_cast<Filter::Type>(subConfig.readEntry("inclusive", 0));
        filters << SerializedFilter(pattern, targets, type);
    }

    return filters;
}

void writeFilters(const SerializedFilters& filters, KSharedConfigPtr config)
{
    // clear existing
    config->deleteGroup("Filters");

    // write new
    KConfigGroup group = config->group("Filters");
    group.writeEntry("size", filters.size());
    int i = 0;
    foreach(const SerializedFilter& filter, filters) {
        KConfigGroup subGroup = group.group(QByteArray::number(i++));
        subGroup.writeEntry("pattern", filter.pattern);
        subGroup.writeEntry("targets", static_cast<int>(filter.targets));
        subGroup.writeEntry("inclusive", static_cast<int>(filter.type));
    }
    config->sync();
}

Filters deserialize(const SerializedFilters& filters)
{
    Filters ret;
    ret.reserve(filters.size());
    foreach(const SerializedFilter& filter, filters) {
        ret << Filter(filter);
    }
    return ret;
}

}
