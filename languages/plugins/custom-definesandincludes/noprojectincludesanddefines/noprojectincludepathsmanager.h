/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef NOPROJECTINCLUDEPATHSMANAGER_H
#define NOPROJECTINCLUDEPATHSMANAGER_H

#include <QString>

#include <util/path.h>

using KDevelop::Path;

class NoProjectIncludePathsManager
{
public:
    /// @return list of include directories for @p oath
    Path::List includes( const QString& path );

    /// Opens the configuration page for file with the @p path
    void openConfigurationDialog( const QString& path );
private:
    bool writeIncludePaths( const QString& storageDirectory, const Path::List& includePaths );

private:
    ///Finds the configuration file starting with the directory @p path
    QString findConfigurationFile( const QString& path );
};

#endif // NOPROJECTINCLUDEPATHSMANAGER_H
