/* This file is part of KDevelop
    Copyright 2010 Milian Wolff <mail@milianw.de>

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

#ifndef KDEVPLATFORM_PROJECTHELPER_H
#define KDEVPLATFORM_PROJECTHELPER_H

#include "projectexport.h"

class QUrl;

namespace KDevelop {
class IProject;
class Path;

/**
 * Deletes the url at @p url. The user is NOT asked for confirmation.
 * Set @p isFolder properly to get the correct messages.
 * @return whether url got deleted or not
 */
bool KDEVPLATFORMPROJECT_EXPORT removeUrl(const KDevelop::IProject* project, const QUrl& url, const bool isFolder);

/**
 * Deletes the path at @p path. The user is NOT asked for confirmation.
 * Set @p isFolder properly to get the correct messages.
 * @return whether url got deleted or not
 */
bool KDEVPLATFORMPROJECT_EXPORT removePath(const KDevelop::IProject* project, const Path& path, const bool isFolder);

/**
 * Creates a file at @p url. Shows the user an error message on failure.
 * @return whether file got created or not
 */
bool KDEVPLATFORMPROJECT_EXPORT createFile(const QUrl& file);

/**
 * Creates a file at @p path. Shows the user an error message on failure.
 * @return whether file got created or not
 */
bool KDEVPLATFORMPROJECT_EXPORT createFile(const Path& file);

/**
 * Creates a folder at @p url. Shows the user an error message on failure.
 * @return whether folder got created or not
 */
bool KDEVPLATFORMPROJECT_EXPORT createFolder(const QUrl& folder);

/**
 * Creates a folder at @p path. Shows the user an error message on failure.
 * @return whether folder got created or not
 */
bool KDEVPLATFORMPROJECT_EXPORT createFolder(const Path& folder);

/**
 * Renames anything at @p oldname to @p oldname
 * @return whether it got renamed or not
 */
bool KDEVPLATFORMPROJECT_EXPORT renameUrl(const KDevelop::IProject* project, const QUrl& oldname, const QUrl& newname);

/**
 * Renames anything at @p oldName to @p oldName
 * @return whether it got renamed or not
 */
bool KDEVPLATFORMPROJECT_EXPORT renamePath(const KDevelop::IProject* project, const Path& oldName, const Path& newName);

/**
 * Copies anything at @p source to @p target
 * @return whether it got copied or not
 */
bool KDEVPLATFORMPROJECT_EXPORT copyUrl(const KDevelop::IProject* project, const QUrl& source, const QUrl& target);

/**
 * Copies anything at @p source to @p target
 * @return whether it got copied or not
 */
bool KDEVPLATFORMPROJECT_EXPORT copyPath(const KDevelop::IProject* project, const Path& source, const Path& target);
}

#endif // KDEVPLATFORM_PROJECTHELPER_H
