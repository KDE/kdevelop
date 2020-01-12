/*
 * This file is part of KDevelop
 *
 * Copyright 2018,2020 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "compileanalyzeutils.h"

// lib
#include <debug.h>
// KDevPlatform
#include <util/path.h>
// KF
#include <KLocalizedString>
// Qt
#include <QStandardPaths>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace KDevelop
{

namespace Utils
{

QString findExecutable(const QString& fallbackExecutablePath)
{
    const QString executablePath = QStandardPaths::findExecutable(fallbackExecutablePath);
    return executablePath.isEmpty() ? fallbackExecutablePath : executablePath;
}

QStringList filesFromCompilationDatabase(const KDevelop::Path& buildPath,
                                         const QUrl& urlToCheck, bool allFiles,
                                         QString& error)
{
    QStringList result;

    const auto commandsFilePath = KDevelop::Path(buildPath, QStringLiteral("compile_commands.json")).toLocalFile();

    if (!QFile::exists(commandsFilePath)) {
        error = i18n("Compilation database file not found: '%1'", commandsFilePath);
        return result;
    }

    const auto pathToCheck = urlToCheck.toLocalFile();
    if (pathToCheck.isEmpty()) {
        error = i18n("Nothing to check: compilation database file '%1' contains no matching items.", commandsFilePath);
        return result;
    }

    QFile commandsFile(commandsFilePath);
    if (!commandsFile.open(QFile::ReadOnly | QFile::Text)) {
        error = i18n("Could not open compilation database file for reading: '%1'", commandsFilePath);
        return result;
    }

    QJsonParseError jsonError;
    const auto commandsDocument = QJsonDocument::fromJson(commandsFile.readAll(), &jsonError);

    if (jsonError.error) {
        error = i18n("JSON error during parsing compilation database file '%1': %2", commandsFilePath, jsonError.errorString());
        return result;
    }

    if (!commandsDocument.isArray()) {
        error = i18n("JSON error during parsing compilation database file '%1': document is not an array.", commandsFilePath);
        return result;
    }

    const auto pathToCheckInfo = QFileInfo(pathToCheck);
    const bool isPathToCheckAFile = pathToCheckInfo.isFile();
    const auto canonicalPathToCheck = pathToCheckInfo.canonicalFilePath();

    const auto fileDataArray = commandsDocument.array();
    for (const auto& value : fileDataArray) {
        if (!value.isObject()) {
            continue;
        }

        const auto entry = value.toObject();
        const auto it = entry.find(QLatin1String("file"));
        if (it != entry.end()) {
            // using the original path from the commands file
            // but matching the canonical ones
            const auto path = it->toString();
            const auto pathInfo = QFileInfo(path);
            if (pathInfo.exists()) {
                if (allFiles) {
                    result += path;
                } else {
                    const auto canonicalPath = pathInfo.canonicalFilePath();
                    if (isPathToCheckAFile) {
                        if (canonicalPath == canonicalPathToCheck) {
                            result = QStringList{path};
                            break;
                        }
                    } else if (canonicalPath.startsWith(canonicalPathToCheck)) {
                        result.append(path);
                    }
                }
            }
        }
    }

    if (result.isEmpty()) {
        error = i18n("Nothing to check: compilation database file '%1' contains no matching items.", commandsFilePath);
    }

    return result;
}

}

}
