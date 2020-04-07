/* FilesystemHelpers

    Copyright 2020 Igor Kushnir <igorkuo@gmail.com>

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

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    constexpr auto mode = QIODevice::NewOnly;
#else
    constexpr auto mode = QIODevice::WriteOnly;
    if (file.exists()) {
        qCritical() << Q_FUNC_INFO << "File already exists";
        return false;
    }
#endif

    if (!file.open(mode)) {
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
