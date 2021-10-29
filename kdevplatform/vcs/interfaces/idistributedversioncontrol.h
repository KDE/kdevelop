/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
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

    ~IDistributedVersionControl() override = default;

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

