/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DOCUMENTFINDERHELPERS_H
#define DOCUMENTFINDERHELPERS_H

#include <QVector>

#include <QUrl>

#include "clangprivateexport.h"

#include <QStringList>
class QString;

/// Helper class for handling @see IBuddyDocumentFinder features.
namespace DocumentFinderHelpers
{
/// @return All supported mime types
KDEVCLANGPRIVATE_EXPORT QStringList mimeTypesList();

/**
 * Considers the URLs as buddy documents if the base path (without extension)
 * is the same, and one extension starts with h/H and the other one with c/C.
 * For example, foo.hpp and foo.C are buddies.
 */
KDEVCLANGPRIVATE_EXPORT bool areBuddies(const QUrl &url1, const QUrl& url2);

/// @see KDevelop::IBuddyDocumentFinder
KDEVCLANGPRIVATE_EXPORT bool buddyOrder(const QUrl &url1, const QUrl& url2);

/// @see KDevelop::IBuddyDocumentFinder
KDEVCLANGPRIVATE_EXPORT QVector<QUrl> potentialBuddies(const QUrl& url, bool checkDUChain = true);

/**
 * Returns path to the source file for given @p headerPath
 *
 * If no source file exists or @p headerPath is not a header an empty string is returned
 */
KDEVCLANGPRIVATE_EXPORT QString sourceForHeader(const QString& headerPath);
}

#endif // DOCUMENTFINDERHELPERS_H
