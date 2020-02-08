/* This file is part of KDevelop

    Copyright 2020 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "cmakefileapi.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVersionNumber>

#include "cmakeutils.h"

#include <debug.h>

namespace {
QJsonObject parseFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(CMAKE) << "failed to read json file" << path << file.errorString();
        return {};
    }
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error) {
        qCWarning(CMAKE) << "failed to parse json file" << path << error.errorString() << error.offset;
        return {};
    }
    return document.object();
}

bool isKDevelopClientResponse(const QJsonObject& indexObject)
{
    return indexObject.value(QLatin1String("reply")).toObject().contains(QLatin1String("client-kdevelop"));
}
}

namespace CMake {
namespace FileApi {
bool supported(const QString& cmakeExecutable)
{
    return QVersionNumber::fromString(cmakeExecutableVersion(cmakeExecutable)) >= QVersionNumber(3, 14);
}

void writeClientQueryFile(const QString& buildDirectory)
{
    const QDir queryDir(buildDirectory + QLatin1String("/.cmake/api/v1/query/client-kdevelop/"));
    if (!queryDir.exists() && !queryDir.mkpath(QStringLiteral("."))) {
        qCWarning(CMAKE) << "failed to create file API query dir:" << queryDir.absolutePath();
        return;
    }

    QFile queryFile(queryDir.absoluteFilePath(QStringLiteral("query.json")));
    if (!queryFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(CMAKE) << "failed to open query file for writing:" << queryFile.fileName() << queryFile.errorString();
        return;
    }

    queryFile.write(R"({"requests": [{"kind": "codemodel", "version": 2}]})");
}

QJsonObject findReplyIndexFile(const QString& buildDirectory)
{
    const QDir replyDir(buildDirectory + QLatin1String("/.cmake/api/v1/reply/"));
    if (!replyDir.exists()) {
        qCWarning(CMAKE) << "cmake-file-api reply directory does not exist:" << replyDir.path();
    }
    for (const auto& entry : replyDir.entryInfoList({QStringLiteral("index-*.json")}, QDir::Files, QDir::Name | QDir::Reversed)) {
        const auto object = parseFile(entry.absoluteFilePath());
        if (isKDevelopClientResponse(object)) {
            return object;
        }
    }
    qCWarning(CMAKE) << "no cmake-file-api reply index file found in" << replyDir.absolutePath();
    return {};
}
}
}
