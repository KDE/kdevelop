/*
    SPDX-FileCopyrightText: 2012 Ivan Shapovalov <intelfx100@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SHELLUTILS_H
#define SHELLUTILS_H

#include "utilexport.h"
#include <QList>

class QDialog;
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

/**
 * Restores window geometry of @p dialog from the global @c KConfigGroup [@p configGroupName][@p configSubgroupName]
 * and connects saving of the dialog's geometry in the same config group to the dialog's @c QDialog::finished signal.
 *
 * @param configSubgroupName the name of a config subgroup to save and restore @p dialog's window geometry entry in.
 *                           If empty, the entry is saved in and restored from the @p configGroupName group directly.
 * @return @c true if @p dialog's window geometry was successfully restored; @c false otherwise.
 */
bool KDEVPLATFORMUTIL_EXPORT restoreAndAutoSaveGeometry(QDialog& dialog, const QString& configGroupName,
                                                        const QString& configSubgroupName = QString{});
}

#endif // SHELLUTILS_H
