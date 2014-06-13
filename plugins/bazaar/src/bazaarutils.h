/***************************************************************************
 *   Copyright 2013-2014 Maciej Poleski                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef BAZAAR_BAZAARUTILS_H
#define BAZAAR_BAZAARUTILS_H

#include <QtCore/QDir>

#include <KUrl>

#include <vcs/vcsevent.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

namespace KDevelop
{
class VcsRevision;
class VcsStatusInfo;
class VcsEvent;
}

class KUrl;

namespace BazaarUtils
{

/**
 * Converts @p url to \code QDir \endcode instance assuming file is local
 */
QDir toQDir(const KUrl& url);

/**
 * @return working copy location of working copy which contains @p path.
 */
QDir workingCopy(const KUrl& path);

/**
 * Translate VcsRevision into Revision Identifier accepted by Bazaar. This
 * function is designed for translating signle revision ids.
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
bool isValidDirectory(const KUrl& dirPath);

/**
 * Parses signle status info line to \code KDevelop::VcsStatusInfo \endcode.
 */
KDevelop::VcsStatusInfo parseVcsStatusInfoLine(const QString& line);

/**
 * Concatenates path of working copy location and relative file in working copy
 * location (@p pathInWorkingCopy) and returns absolute path of this file.
 * @return Absolute path of file designated by @p pathInWorkingCopy relative to
 * @p workingCopy.
 */
QString concatenatePath(const QDir& workingCopy, const KUrl& pathInWorkingCopy);

/**
 * Parses informations about single commit from @p action (which is signle part
 * of \code bzr log \endcode output).
 * @return Informations about single commit in instance of
 * \code KDevelop::VcsEvent \endcode class.
 */
KDevelop::VcsEvent parseBzrLogPart(const QString& output);

/**
 * Parses information about single action on single file in some
 * \code bzr log \endcode output.
 */
KDevelop::VcsItemEvent::Action parseActionDescription(const QString& action);

/**
 * Some methods in interface provides @p recursion parameter. In general
 * Bazaar don't support this (only part of interface has native recursion
 * handling support). This function removes directiories from list if
 * we are in NonRecursive mode (as directory for self is not versioned).
 */
KUrl::List handleRecursion(const KUrl::List& listOfUrls, KDevelop::IBasicVersionControl::RecursionMode recursion);

}


#endif // BAZAAR_BAZAARUTILS_H
