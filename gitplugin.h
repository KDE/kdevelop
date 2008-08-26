/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
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

#ifndef GIT_PLUGIN_H
#define GIT_PLUGIN_H

#include <vcs/interfaces/idistributedversioncontrol.h>
#include <vcs/dvcs/dvcsplugin.h>
#include <qobject.h>

namespace KDevelop
{
    class VcsJob;
    class VcsRevision;
}

class GitExecutor;

/**
 * This is the main class of KDevelop's Git plugin.
 *
 * It implements the DVCS dependent things not implemented in KDevelop::DistributedVersionControlPlugin
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class GitPlugin: public KDevelop::DistributedVersionControlPlugin
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::IDistributedVersionControl)

friend class GitExecutor;

public:
    GitPlugin(QObject *parent, const QVariantList & args = QVariantList() );
    ~GitPlugin();

    //TODO:Things to be moved to DVCSplugin, but not moved because require executor changes in all implemented DVCS
    KDevelop::VcsJob* log(const KUrl& localLocation,
                          const KDevelop::VcsRevision& rev,
                          unsigned long limit);
    KDevelop::VcsJob* log(const KUrl& localLocation,
                          const KDevelop::VcsRevision& rev,
                          const KDevelop::VcsRevision& limit);
    //TODO: use KUrl for location!
    KDevelop::VcsJob* checkout(const KUrl &localLocation,
                               const QString &repo);

};

#endif
