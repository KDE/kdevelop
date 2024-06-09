/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef NOPROJECTINCLUDEPATHSMANAGER_H
#define NOPROJECTINCLUDEPATHSMANAGER_H

#include <QString>

#include <util/path.h>

using KDevelop::Path;

namespace NoProjectIncludePathsManager {
/// @return list of include directories for @p path
Path::List includes(const QString& path);
/// @return defines for @p path
QHash<QString, QString> defines(const QString& path);

/// Opens the configuration page for file with the @p path
void openConfigurationDialog(const QString& path);
}

#endif // NOPROJECTINCLUDEPATHSMANAGER_H
