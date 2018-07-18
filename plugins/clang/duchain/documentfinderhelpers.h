/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef DOCUMENTFINDERHELPERS_H
#define DOCUMENTFINDERHELPERS_H

#include <QVector>

#include <QUrl>

#include "clangprivateexport.h"

class QStringList;
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
KDEVCLANGPRIVATE_EXPORT QVector< QUrl > getPotentialBuddies(const QUrl &url, bool checkDUChain = true);

/**
 * Returns path to the source file for given @p headerPath
 *
 * If no source file exists or @p headerPath is not a header an empty sting is returned
 */
KDEVCLANGPRIVATE_EXPORT QString sourceForHeader(const QString& headerPath);
}

#endif // DOCUMENTFINDERHELPERS_H
