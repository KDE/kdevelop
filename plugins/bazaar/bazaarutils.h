/*
    SPDX-FileCopyrightText: 2013-2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BAZAAR_BAZAARUTILS_H
#define BAZAAR_BAZAARUTILS_H

#include <QDir>
#include <QUrl>

#include <vcs/vcsevent.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

namespace KDevelop
{
class VcsRevision;
class VcsStatusInfo;
class VcsEvent;
}

namespace BazaarUtils
{

/**
 * Converts @p url to QDir instance assuming file is local
 */
QDir toQDir(const QUrl& url);

/**
 * @return working copy location of working copy which contains @p path.
 */
QDir workingCopy(const QUrl& path);

/**
 * Translate VcsRevision into Revision Identifier accepted by Bazaar. This
 * function is designed for translating single revision ids.
 */
QString getRevisionSpec(const KDevelop::VcsRevision& revision);

/**
 * Translate VcsRevision into revision range option accepted by Bazaar. This
 * function is designed for translating end point into range from begin to
 * given end point.
 */
QString getRevisionSpecRange(const KDevelop::VcsRevision& end);

/**
 * Translate VcsRevision range into revision range option accepted by Bazaar.
 * This function translates VcsRevision range into best possible approximation
 * of revision range accepted by Bazaar.
 */
QString getRevisionSpecRange(const KDevelop::VcsRevision& begin,
                             const KDevelop::VcsRevision& end);

/**
 * Checks if @p dirPath is valid working directory location.
 * @return true if @p dirPath is valid working directory location, false
 * otherwise.
 */
bool isValidDirectory(const QUrl& dirPath);

/**
 * Parses single status info line to KDevelop::VcsStatusInfo.
 */
KDevelop::VcsStatusInfo parseVcsStatusInfoLine(const QString& line);

/**
 * Concatenates path of working copy location and relative file in working copy
 * location (@p pathInWorkingCopy) and returns absolute path of this file.
 * @return Absolute path of file designated by @p pathInWorkingCopy relative to
 * @p workingCopy.
 */
QString concatenatePath(const QDir& workingCopy, const QUrl& pathInWorkingCopy);

/**
 * Parses information about single commit from @p output (which is single part
 * of @c bzr @c log output).
 * @return Information about single commit in instance of
 * KDevelop::VcsEvent class.
 */
KDevelop::VcsEvent parseBzrLogPart(const QString& output);

/**
 * Parses information about single action on single file in some
 * @c bzr @c log output.
 */
KDevelop::VcsItemEvent::Action parseActionDescription(const QString& action);

/**
 * Some methods in interface provides @p recursion parameter. In general
 * Bazaar don't support this (only part of interface has native recursion
 * handling support). This function removes directories from list if
 * we are in NonRecursive mode (as directory for self is not versioned).
 */
QList<QUrl> handleRecursion(const QList<QUrl>& listOfUrls, KDevelop::IBasicVersionControl::RecursionMode recursion);

}


#endif // BAZAAR_BAZAARUTILS_H
