/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Ivan Shapovalov <intelfx100@gmail.com>
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

#ifndef SHELLUTILS_H
#define SHELLUTILS_H

#include "utilexport.h"
#include <QList>

class QString;
class QUrl;

namespace KDevelop {

/**
* Asks user of an arbitrary question by using either a \ref KMessageBox or stdin/stderr.
*
* @return @c true if user chose "Yes" and @c false otherwise.
*/
bool KDEVPLATFORMUTIL_EXPORT askUser(const QString& mainText,
                                     const QString& ttyPrompt,
                                     const QString& mboxTitle,
                                     const QString& mboxAdditionalText,
                                     const QString& confirmText,
                                     const QString& rejectText,
                                     bool ttyDefaultToYes = true);

/**
 * Ensures that the given list of files is writable. If some files are not writable,
 * asks the user whether they should be made writable. If the user disagrees,
 * or if the operation failed, returns false.
 * */
bool KDEVPLATFORMUTIL_EXPORT ensureWritable(const QList<QUrl>& urls);

}

#endif // SHELLUTILS_H
