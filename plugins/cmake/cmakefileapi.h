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

#pragma once

class QJsonObject;
class QString;

#include <cmakecommonexport.h>

struct CMakeProjectData;

namespace KDevelop
{
class Path;
}

/// see: https://cmake.org/cmake/help/latest/manual/cmake-file-api.7.html
namespace CMake {
namespace FileApi {
/**
 * @returns true when the given @p cmakeExecutable supports the cmake-file-api, false otherwise
 */
KDEVCMAKECOMMON_EXPORT bool supported(const QString &cmakeExecutable);

/**
 * Write the KDevelop-specific query file into the given @p buildDirectory.
 *
 * See also: https://cmake.org/cmake/help/latest/manual/cmake-file-api.7.html#v1-client-stateful-query-files
 */
KDEVCMAKECOMMON_EXPORT void writeClientQueryFile(const QString &buildDirectory);

/**
 * Read and parse latest available reply index file that corresponds to our query in @p buildDirectory.
 *
 * See also: https://cmake.org/cmake/help/latest/manual/cmake-file-api.7.html#v1-reply-index-file
 */
KDEVCMAKECOMMON_EXPORT QJsonObject findReplyIndexFile(const QString &buildDirectory);

/**
 * Read and parse the code model referenced by the given @p replyIndex
 *
 * See also: https://cmake.org/cmake/help/latest/manual/cmake-file-api.7.html#id11
 */
KDEVCMAKECOMMON_EXPORT CMakeProjectData parseReplyIndexFile(const QJsonObject& replyIndex,
                                                            const KDevelop::Path& sourceDirectory,
                                                            const KDevelop::Path& buildDirectory);
}
}
