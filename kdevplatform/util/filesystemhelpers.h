/*
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_FILESYSTEMHELPERS_H
#define KDEVPLATFORM_FILESYSTEMHELPERS_H

#include "utilexport.h"

#include <QByteArray>
#include <QByteArrayList>

class QString;
#include <QStringList>

namespace FilesystemHelpers {
/**
 * @brief Creates a new file at a specified path.
 * @param filePath The path where the file will be created.
 * @param fileContents Data to be written into the new file.
 * @return true on success; false if the file already existed or another error occurred.
 */
KDEVPLATFORMUTIL_EXPORT bool createNewFileAndWrite(const QString& filePath,
                                                   const QByteArray& fileContents = QByteArray{});

/**
 * @brief Creates all necessary parent directories and a new file at a specified path.
 * @param dirPath An absolute path to a directory, the beginning of the path to the new file.
 * @param[in,out] filePath A relative path from @p dirPath to the location of the new file.
 *                         On return this path is made absolute by prepending @p dirPath to it.
 * @param fileContents Data to be written into the new file.
 * @return An empty string on success or the path to the directory or file where an error occurred.
 */
KDEVPLATFORMUTIL_EXPORT QString makeAbsoluteCreateAndWrite(const QString& dirPath, QString& filePath,
                                                           const QByteArray& fileContents = QByteArray{});

/**
 * @brief Calls the single-file overload for each element of @p filePaths.
 * @param fileContents The list of desired file contents. Must be of the same size as @p filePaths.
 * @return An empty string on success or the path to the directory or file where an error occurred.
 */
KDEVPLATFORMUTIL_EXPORT QString makeAbsoluteCreateAndWrite(const QString& dirPath, QStringList& filePaths,
                                                           const QByteArrayList& fileContents);

/**
 * @brief The difference from QByteArrayList fileContents overload is that the same data -
 *        @p commonFileContents - is written into each new file.
 */
KDEVPLATFORMUTIL_EXPORT QString makeAbsoluteCreateAndWrite(const QString& dirPath, QStringList& filePaths,
                                                           const QByteArray& commonFileContents = QByteArray{});
}

#endif // KDEVPLATFORM_FILESYSTEMHELPERS_H
