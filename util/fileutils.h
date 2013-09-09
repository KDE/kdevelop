/*
 * This file is part of KDevelop
 *
 * Copyright 2011 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_FILEUTILS_H
#define KDEVPLATFORM_FILEUTILS_H

#include "utilexport.h"

class QString;

namespace KDevelop {

/**
 * Removes the local directory @p dir with all its contents.
 *
 * This is especially useful in cases where we want to remove
 * a directory synchronously. The alternative @c KIO::NetAccess::del
 * uses a nested eventloop *and* requires a UI. The function below
 * requires neither but is of course limited to local files.
 *
 * @warning The user is not asked for permission.
 *
 * @param dir The path to the local directory that should be removed.
 *
 * @return @c true if the directory could be removed or did not
 *         exist in the first place and @c false otherwise.
 */
bool KDEVPLATFORMUTIL_EXPORT removeDirectory(const QString& dir);

}

#endif // KDEVPLATFORM_FILEUTILS_H
