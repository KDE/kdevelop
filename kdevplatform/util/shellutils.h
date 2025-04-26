/*
    SPDX-FileCopyrightText: 2012 Ivan Shapovalov <intelfx100@gmail.com>
    SPDX-FileCopyrightText: 2024, 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SHELLUTILS_H
#define SHELLUTILS_H

#include "utilexport.h"

#include <QList>
#include <QStringList>

class QUrl;
class QWidget;

namespace KDevelop {
class ILaunchConfiguration;

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

struct KDEVPLATFORMUTIL_EXPORT LaunchConfigurationEntryName
{
    const char* untranslatable = nullptr; ///< the entry name to be printed via qCDebug/qCWarning
    QString translatable; ///< the entry name to be displayed as part of an error message in the UI
};

/**
 * Split a given text of a launch configuration entry according to system shell word splitting and quoting rules.
 *
 * @param launchConfiguration the launch configuration from whose config the entry text has been read
 * @param entryText the text of the launch configuration entry to be split
 * @param entryName the name of the launch configuration entry to be split
 * @param[out] errorMessage if the splitting fails, an explaining error message text is assigned
 *                          to this parameter; otherwise the parameter remains untouched
 * @return the list, to which the entry text has been split, or an empty list in case of an error
 */
QStringList KDEVPLATFORMUTIL_EXPORT splitLaunchConfigurationEntry(const ILaunchConfiguration& launchConfiguration,
                                                                  const QString& entryText,
                                                                  const LaunchConfigurationEntryName& entryName,
                                                                  QString& errorMessage);

/**
 * Split a given text of a launch configuration entry according to system shell word splitting and quoting rules.
 *
 * Unlike splitLaunchConfigurationEntry(), this function treats and reports
 * as errors an empty entry text and an empty list result of the splitting.
 *
 * @sa splitLaunchConfigurationEntry()
 */
QStringList KDEVPLATFORMUTIL_EXPORT
splitNonemptyLaunchConfigurationEntry(const ILaunchConfiguration& launchConfiguration, const QString& entryText,
                                      const LaunchConfigurationEntryName& entryName, QString& errorMessage);

/**
 * Split the local file path of a given launch configuration URL
 * entry according to system shell word splitting and quoting rules.
 *
 * This function treats and reports as errors an empty URL, an URL that does
 * not point to a local file path, and an empty list result of the splitting.
 *
 * @sa splitLaunchConfigurationEntry()
 */
QStringList KDEVPLATFORMUTIL_EXPORT
splitLocalFileLaunchConfigurationEntry(const ILaunchConfiguration& launchConfiguration, const QUrl& entryUrl,
                                       const LaunchConfigurationEntryName& entryName, QString& errorMessage);

} // namespace KDevelop

#endif // SHELLUTILS_H
