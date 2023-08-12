/*
    SPDX-FileCopyrightText: 2012 Ivan Shapovalov <intelfx100@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SHELLUTILS_H
#define SHELLUTILS_H

#include "utilexport.h"
#include <QList>
#include <QString>

class QUrl;
class QWidget;

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
 * Sets up automatic saving of @p widget's window geometry in the specified application config group
 * and attempts to restore @p widget's window geometry from the same config group.
 *
 * @param widget a top-level widget (in other words, a window).
 * @param configGroupName the name of a config group.
 * @param configSubgroupName the name of a config subgroup of the @p configGroupName group
 *        where @p widget's window geometry entry is to be restored from and saved in.
 *        If empty, the entry is saved in and restored from the @p configGroupName group directly.
 * @return @c true if @p widget's window geometry was successfully restored; @c false otherwise.
 */
bool KDEVPLATFORMUTIL_EXPORT restoreAndAutoSaveGeometry(QWidget& widget, const QString& configGroupName,
                                                        const QString& configSubgroupName = QString{});
}

#endif // SHELLUTILS_H
