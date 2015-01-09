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

#include <QStringList>
#include <QVector>

#include <QUrl>

/// Helper class for handling @see IBuddyDocumentFinder features.
class DocumentFinderHelpers
{
public:
    /// @return All supported mime types
    static QStringList mimeTypesList();

    /**
     * Considers the URLs as buddy documents if the base path (without extension)
     * is the same, and one extension starts with h/H and the other one with c/C.
     * For example, foo.hpp and foo.C are buddies.
     */
    static bool areBuddies(const QUrl &url1, const QUrl& url2);

    /// @see KDevelop::IBuddyDocumentFinder
    static bool buddyOrder(const QUrl &url1, const QUrl& url2);

    /// @see KDevelop::IBuddyDocumentFinder
    static QVector< QUrl > getPotentialBuddies(const QUrl &url);
};

#endif // DOCUMENTFINDERHELPERS_H
