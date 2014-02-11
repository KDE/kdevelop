/*
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef CUSTOMDEFINESANDINCLUDESMANAGER_H
#define CUSTOMDEFINESANDINCLUDESMANAGER_H

#include <QScopedPointer>
#include <QHash>
#include <QString>
#include <QList>
#include <KUrl>

#include <kdemacros.h>

#include <project/path.h>

namespace KDevelop
{
class ProjectBaseItem;
}

using KDevelop::ProjectBaseItem;
using KDevelop::Path;

/// @brief: Class for retrieving custom defines and includes.
class KDE_EXPORT CustomDefinesAndIncludesManager
{
private:
    CustomDefinesAndIncludesManager();
public:
    ~CustomDefinesAndIncludesManager();
    static CustomDefinesAndIncludesManager* self();

    ///@return list of all custom defines for @p item
    QHash< QString, QString > defines(const ProjectBaseItem* item) const;

    ///@return list of all custom includes for @p item
    Path::List includes(const ProjectBaseItem* item) const;
private:
    class ManagerPrivate;
    const QScopedPointer<ManagerPrivate> d;
};

#endif // CUSTOMDEFINESANDINCLUDESMANAGER_H
