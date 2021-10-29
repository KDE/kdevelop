/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INCLUDEPATHSCONVERTER_H
#define INCLUDEPATHSCONVERTER_H

#include <QStringList>

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
};

#endif // INCLUDEPATHSCONVERTER_H
