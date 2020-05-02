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

#ifndef KDEVPLATFORM_FILESYSTEMHELPERS_H
#define KDEVPLATFORM_FILESYSTEMHELPERS_H

#include "utilexport.h"

#include <QByteArray>
#include <QByteArrayList>

class QString;
class QStringList;

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
