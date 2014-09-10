/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>
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

#ifndef KDEVPLATFORM_IDISTRIBUTEDVERSIONCONTROL_H
#define KDEVPLATFORM_IDISTRIBUTEDVERSIONCONTROL_H

#include "ibasicversioncontrol.h"

namespace KDevelop
{

class VcsJob;
class VcsLocation;

/**
 * This interface has methods to support distributed version control systems
 * like git or svk.
 */
class IDistributedVersionControl : public KDevelop::IBasicVersionControl
{
public:

    virtual ~IDistributedVersionControl(){}

    /**
     * Create a new repository inside the given local directory.
     */
    virtual VcsJob* init(const QUrl& localRepositoryRoot) = 0;

    /**
     * Export the locally committed revisions to another repository.
     *
     * @param localRepositoryLocation Any location inside the local repository.
     * @param localOrRepoLocationDst The repository which will receive the pushed revisions.
     */
    virtual VcsJob* push(const QUrl& localRepositoryLocation,
                         const VcsLocation& localOrRepoLocationDst) = 0;

    /**
     * Import revisions from another repository the local one, but don't yet
     * merge them into the working copy.
     *
     * @param localOrRepoLocationSrc The repository which contains the revisions
     *                           to be pulled.
     * @param localRepositoryLocation Any location inside the local repository.
     */
    virtual VcsJob* pull(const VcsLocation& localOrRepoLocationSrc,
                         const QUrl& localRepositoryLocation) = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IDistributedVersionControl, "org.kdevelop.IDistributedVersionControl" )

#endif

