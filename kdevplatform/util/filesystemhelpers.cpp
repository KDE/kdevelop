/*
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filesystemhelpers.h"

#include <QByteArray>
#include <QByteArrayList>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>

bool FilesystemHelpers::createNewFileAndWrite(const QString& filePath,
                                              const QByteArray& fileContents)
{
    QFile file(filePath);

    if (!file.open(QIODevice::NewOnly)) {
        qCritical() << Q_FUNC_INFO << file.error() << file.errorString();
        return false;
    }

    if (!fileContents.isEmpty() && file.write(fileContents) == -1) {
        qCritical() << Q_FUNC_INFO << file.error() << file.errorString();
        return false;
    }
    return true;
}

QString FilesystemHelpers::makeAbsoluteCreateAndWrite(const QString& dirPath, QString& filePath,
                                                      const QByteArray& fileContents)
{
    const QFileInfo info{QDir{dirPath}, filePath};
    QString pathToFile = info.absolutePath();
    if (!QDir{}.mkpath(pathToFile)) {
        return pathToFile;
    }
    filePath = info.absoluteFilePath();
    if (!createNewFileAndWrite(filePath, fileContents)) {
        return filePath;
    }
    return QString{};
}

QString FilesystemHelpers::makeAbsoluteCreateAndWrite(const QString& dirPath, QStringList& filePaths,
                                                      const QByteArrayList& fileContents)
{
    Q_ASSERT(fileContents.size() == filePaths.size());
    for (int i = 0; i < filePaths.size(); ++i) {
        QString errorPath = makeAbsoluteCreateAndWrite(dirPath, filePaths[i], fileContents[i]);
        if (!errorPath.isEmpty()) {
            return errorPath;
        }
    }
    return QString{};
}

QString FilesystemHelpers::makeAbsoluteCreateAndWrite(const QString& dirPath, QStringList& filePaths,
                                                      const QByteArray& commonFileContents)
{
    for (auto& fPath : filePaths) {
        QString errorPath = makeAbsoluteCreateAndWrite(dirPath, fPath, commonFileContents);
        if (!errorPath.isEmpty()) {
            return errorPath;
        }
    }
    return QString{};
}
