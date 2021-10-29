/*
    SPDX-FileCopyrightText: 2012 Ivan Shapovalov <intelfx100@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
