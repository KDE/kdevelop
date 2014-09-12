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

#include "noprojectincludepathsmanager.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>

#include <KDebug>
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <serialization/indexedstring.h>

#include "noprojectcustomincludepaths.h"

namespace
{
const QString includePathsFile = ".kdev_include_paths";


bool removeSettings(const QString& storageDirectory)
{
    auto file = storageDirectory + QDir::separator() + includePathsFile;
    return QFile::remove(file);
}

QStringList pathListToStringList(const Path::List& paths)
{
    QStringList sl;
    for (const auto& p : paths) {
        sl << p.path();
    }
    return sl;
}
}

QString NoProjectIncludePathsManager::findConfigurationFile(const QString& path)
{
    QDir dir(path);
    while (dir.exists()) {
        QFileInfo customIncludePathsFile(dir, includePathsFile);
        if (customIncludePathsFile.exists()) {
            return customIncludePathsFile.absoluteFilePath();
        }

        if (!dir.cdUp()) {
            break;
        }
    }
    return {};
}

Path::List NoProjectIncludePathsManager::includes(const QString& path)
{
    QFileInfo fi(path);

    auto pathToFile = findConfigurationFile(fi.absoluteDir().absolutePath());
    if (pathToFile.isEmpty()) {
        return {};
    }
    Path::List ret;

    QFile f(pathToFile);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        auto lines = QString::fromLocal8Bit(f.readAll()).split('\n', QString::SkipEmptyParts);
        QFileInfo dir(pathToFile);
        for (const auto& line : lines) {
            auto textLine = line.trimmed();
            if (!textLine.isEmpty()) {
                QFileInfo pathInfo(textLine);
                if (pathInfo.isRelative()) {
                    ret << Path(dir.canonicalPath() + QDir::separator() + textLine);
                } else {
                    ret << Path(textLine);
                }
            }
        }
        f.close();
    }
    return ret;
}

bool NoProjectIncludePathsManager::writeIncludePaths(const QString& storageDirectory, const QStringList& includePaths)
{
    QDir dir(storageDirectory);
    QFileInfo customIncludePaths(dir, includePathsFile);
    QFile f(customIncludePaths.filePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&f);
        for (const auto& customPath : includePaths) {
            out << customPath << endl;
        }
        if (includePaths.isEmpty()) {
            removeSettings(storageDirectory);
        }
        return true;
    } else {
        return false;
    }
}

void NoProjectIncludePathsManager::openConfigurationDialog(const QString& path)
{
    NoProjectCustomIncludePaths cip;

    QFileInfo fi(path);
    auto dir = fi.absoluteDir().absolutePath();
    cip.setStorageDirectory(dir);

    auto paths = includes(path);

    cip.setCustomIncludePaths(pathListToStringList(paths));

    if (cip.exec() == QDialog::Accepted) {
        if (!writeIncludePaths(cip.storageDirectory(), cip.customIncludePaths())) {
            kWarning() << i18n("Failed to save custom include paths in directory: %1", cip.storageDirectory());
        }
        KDevelop::ICore::self()->languageController()->backgroundParser()->addDocument(KDevelop::IndexedString(path));
    }
}
