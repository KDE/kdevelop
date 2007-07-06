/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef IDISTRIBUTEDVERSIONCONTROL_H
#define IDISTRIBUTEDVERSIONCONTROL_H

#include <iextension.h>

class QString;

namespace KDevelop
{

/**
 * This interface has methods to support distributed version control systems
 * like git or svk.
 */
class KDEVPLATFORMVCS_EXPORT IDistributedVersionControl
{
public:
    /**
     * Create a new repository inside the given local directory.
     */
    virtual VcsJob* init( const KUrl& localRepositoryRoot );

    /**
     * Create a new repository by cloning another one into a newly created
     * local directory, including all of the other repository's history.
     *
     * @param repositoryLocation The repository that will be cloned.
     * @param localRepositoryRoot The root folder of the newly created repository.
     */
    virtual VcsJob* clone( const QString& repositoryLocationSrc,
                           const KUrl& localRepositoryRoot ) = 0;

    /**
     * Export the locally committed revisions to another repository.
     *
     * @param localRepositoryLocation Any location inside the local repository.
     * @param repositoryLocation The repository which will receive the pushed revisions.
     */
    virtual VcsJob* push( const KUrl& localRepositoryLocation,
                          const QString& repositoryLocation ) = 0;

    /**
     * Import revisions from another repository the local one, but don't yet
     * merge them into the working copy.
     *
     * @param repositoryLocation The repository which contains the revisions
     *                           to be pulled.
     * @param localRepositoryLocation Any location inside the local repository.
     */
    virtual VcsJob* pull( const QString& repositoryLocation,
                          const KUrl& localRepositoryLocation ) = 0;
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IDistributedVersionControl, "org.kdevelop.IDistributedVersionControl" )
Q_DECLARE_INTERFACE( KDevelop::IDistributedVersionControl, "org.kdevelop.IDistributedVersionControl" )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
