/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2013 Sven Brauch <svenbrauch@googlemail.com>
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
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
 *
 */
#ifndef __CACHE_H__
#define __CACHE_H__

#include <QHash>

namespace QmlJS
{

/**
 * Cache for values that may be slow to compute (search paths, things
 * involving KStandardDirs, etc)
 */
class Cache
{
private:
    Cache();

public:
    static Cache& instance();

    /**
     * Path corresponding to a module name
     *
     * The path returned can either be the one of a module file shipped with
     * kdev-qmljs, or the path of a directory containing the module components
     * (one .qml file per component, and possibly .so files if the module has
     * native components)
     *
     * @param version If not null, the file being looked for is "uri_version.qml".
     *                If null, then the untouched uri is used for searching.
     */
    QString modulePath(const QString& uri, const QString& version = QString());

private:
    QHash<QString, QString> m_modulePaths;
};

}

#endif