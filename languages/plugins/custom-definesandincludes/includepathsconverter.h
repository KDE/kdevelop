/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#ifndef INCLUDEPATHSCONVERTER_H
#define INCLUDEPATHSCONVERTER_H

#include <QStringList>
#include <QScopedPointer>

class SettingsManager;

class IncludePathsConverter
{
public:
    IncludePathsConverter();

    /**
     * Adds @p includeDirectories into @p projectConfigFile
     */
    bool addIncludePaths(const QStringList& includeDirectories, const QString& projectConfigFile, const QString& subdirectory = QString());

    /**
     * Removes @p includeDirectories from @p projectConfigFile
     */
    bool removeIncludePaths(const QStringList& includeDirectories, const QString& projectConfigFile, const QString& subdirectory = QString());

    /**
     * @return include directories retrieved from @p projectConfigFile
     */
    QStringList readIncludePaths(const QString& projectConfigFile, const QString& subdirectory = QString()) const;

private:
    QScopedPointer<SettingsManager> m_settings;
};

#endif // INCLUDEPATHSCONVERTER_H
