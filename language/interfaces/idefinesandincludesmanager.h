/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#ifndef IDEFINESANDINCLUDES_H
#define IDEFINESANDINCLUDES_H

#include <QHash>
#include <QString>
#include <QStringList>

#include <KConfigGroup>

#include <project/path.h>

#include "../languageexport.h"

namespace KDevelop
{
class ProjectBaseItem;

typedef QHash<QString, QVariant> Defines;
struct ConfigEntry
{
    QString path;
    QStringList includes;
    Defines defines;

    ConfigEntry( const QString& path = QString() ) : path( path ) {}
};

/** An interface that provides language plugins with user specified include directories/files and defines.
* Call IDefinesAndIncludesManager::manager() to get the instance of the plugin.
**/
class KDEVPLATFORMLANGUAGE_EXPORT IDefinesAndIncludesManager
{
public:
    ///@return list of all custom defines for @p item
    virtual QHash<QString, QString> defines( const ProjectBaseItem* item ) const = 0;

    ///@return list of all custom includes for @p item
    virtual Path::List includes( const ProjectBaseItem* item ) const = 0;

    ///@return instance of the plugin if one is loaded, nullptr otherwise
    static IDefinesAndIncludesManager* manager();

    virtual ~IDefinesAndIncludesManager();
};
}

Q_DECLARE_INTERFACE( KDevelop::IDefinesAndIncludesManager, "org.kdevelop.IDefinesAndIncludesManager" )

#endif
