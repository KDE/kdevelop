/*
    SPDX-FileCopyrightText: 2020 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QJsonObject>
#include <QDateTime>

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

struct ReplyIndex {
    QDateTime queryLastModified;
    QJsonObject data;

    bool isValid() const
    {
        return !data.isEmpty();
    }
    bool isOutdated() const
    {
        return !queryLastModified.isValid();
    }
    void markOutdated()
    {
        queryLastModified = {};
    }
};

/**
 * Read and parse latest available reply index file that corresponds to our query in @p buildDirectory.
 *
 * See also: https://cmake.org/cmake/help/latest/manual/cmake-file-api.7.html#v1-reply-index-file
 */
KDEVCMAKECOMMON_EXPORT ReplyIndex findReplyIndexFile(const QString& buildDirectory);

/**
 * Read and parse the code model referenced by the given @p replyIndex
 *
 * See also: https://cmake.org/cmake/help/latest/manual/cmake-file-api.7.html#id11
 */
KDEVCMAKECOMMON_EXPORT CMakeProjectData parseReplyIndexFile(const ReplyIndex& replyIndex,
                                                            const KDevelop::Path& sourceDirectory,
                                                            const KDevelop::Path& buildDirectory);
}
}
