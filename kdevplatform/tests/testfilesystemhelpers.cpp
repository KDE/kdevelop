/* TestFilesystemHelpers

    Copyright 2020, 2021 Igor Kushnir <igorkuo@gmail.com>

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

#include "testfilesystemhelpers.h"

#include <QByteArray>
#include <QByteArrayList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTest>

namespace {
QString formatErrorMessage(const QString& filePath, const QFile& file)
{
    return QStringLiteral("path: %1; error: %2 (%3)")
                .arg(filePath).arg(file.errorString()).arg(file.error());
}
}

void TestFilesystemHelpers::createNewFileAndWrite(const QString& filePath,
                                                  const QByteArray& fileContents)
{
    QFile file(filePath);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    constexpr auto mode = QIODevice::NewOnly;
#else
    constexpr auto mode = QIODevice::WriteOnly;
    QVERIFY2(!file.exists(), qPrintable(filePath));
#endif
    QVERIFY2(file.open(mode), qPrintable(formatErrorMessage(filePath, file)));
    if (!fileContents.isEmpty()) {
        const auto bytesWritten = file.write(fileContents);
        QVERIFY2(bytesWritten == fileContents.size(),
                 qPrintable(QStringLiteral("%1 != %2; ").arg(bytesWritten).arg(fileContents.size())
                 + formatErrorMessage(filePath, file)));
    }
}

void TestFilesystemHelpers::makeAbsoluteCreateAndWrite(const QString& dirPath, QString& filePath,
                                                       const QByteArray& fileContents)
{
    QVERIFY2(!filePath.isEmpty(), qPrintable(dirPath));
    const QFileInfo info{QDir{dirPath}, filePath};
    QVERIFY2(QDir{}.mkpath(info.absolutePath()), qPrintable(info.absolutePath()));
    filePath = info.absoluteFilePath();
    CREATE_NEW_FILE_AND_WRITE(filePath, fileContents);
}

void TestFilesystemHelpers::makeAbsoluteCreateAndWrite(const QString& dirPath, QStringList& filePaths,
                                                       const QByteArrayList& fileContents)
{
    QCOMPARE(fileContents.size(), filePaths.size());
    for (int i = 0; i < filePaths.size(); ++i) {
        MAKE_ABSOLUTE_CREATE_AND_WRITE(dirPath, filePaths[i], fileContents[i]);
    }
}

void TestFilesystemHelpers::makeAbsoluteCreateAndWrite(const QString& dirPath, QStringList& filePaths,
                                                       const QByteArray& commonFileContents)
{
    for (auto& fPath : filePaths) {
        MAKE_ABSOLUTE_CREATE_AND_WRITE(dirPath, fPath, commonFileContents);
    }
}
