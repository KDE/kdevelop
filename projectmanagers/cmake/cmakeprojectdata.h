/* KDevelop CMake Support
 *
 * Copyright 2013 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEPROJECTDATA_H
#define CMAKEPROJECTDATA_H

#include <QStringList>
#include <QFileSystemWatcher>
#include "cmaketypes.h"
#include <util/path.h>

/**
 * Represents any file in a cmake project that has been added
 * to the project.
 *
 * Contains the required information to compile it properly
 */
struct CMakeFile
{
    KDevelop::Path::List includes;
    QHash<QString, QString> defines;
};
inline QDebug &operator<<(QDebug debug, const CMakeFile& file)
{
    debug << "CMakeFile(-I" << file.includes << ", -D" << file.defines << ")";
    return debug.maybeSpace();
}

struct CMakeProjectData
{
    CMakeProjectData() : watcher(new QFileSystemWatcher) {}
    ~CMakeProjectData() {}

    CMakeProperties properties;
    CacheValues cache;
    QHash<KDevelop::Path, CMakeFile> files;
    QSharedPointer<QFileSystemWatcher> watcher;
};

#endif
