/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IBROWSABLEVERSIONCONTROL_H
#define KDEVPLATFORM_IBROWSABLEVERSIONCONTROL_H

#include <QObject>
#include "../vcsrevision.h"

class QString;

namespace KDevelop
{

class VcsJob;

class IBrowsableVersionControl
{
public:
    virtual ~IBrowsableVersionControl() {}

    /**
     * retrieve the last revision in which the repository location was changed
     */
    virtual VcsJob* repositoryRevision( const QString& repoLocation,
                                        VcsRevision::RevisionType ) = 0;

    /**
     * Retrieve the history of the repository location
     *
     * @param rev List @p rev and earlier. The default is HEAD.
     * @param limit Restrict to the most recent @p limit entries. Note that the
     * limit is @e advisory and may be ignored.
     */
    virtual VcsJob* log( const QString& repoLocation,
                         const VcsRevision& rev,
                         unsigned int limit ) = 0;

    /**
     * Retrieve the history of a given local url
     *
     * @param rev List @p rev and earlier. The default is HEAD.
     * @param limit Do not show entries earlier than @p limit. Note that the
     * limit is @e advisory and may be ignored.
     */
    virtual VcsJob* log( const QUrl& localLocation,
                         const VcsRevision& rev,
                         const VcsRevision& limit ) = 0;

    /**
     * Get the changes made by a particular revision
     *
     * @param rev Show information about the revision @p rev.
     * @param repoLocation Any repository path that specifies what VCS server
     * is to be queried. For VCS's that support global versioning, the actual
     * path is unimportant (and ignored), as long as it contains the repository
     * root. Otherwise look up the change associated with the requested path.
     *
     * @note VcsRevision objects with type VcsRevision::FileNumber may store the
     * associated path internally, in which case @p repoLocation may be ignored.
     */
    virtual VcsJob* change( const VcsRevision& rev,
                            const QString& repoLocation ) = 0;

    /**
     * Retrieve a list of entries in the given repository location
     */
    virtual VcsJob* ls( const QString& repoLocation, const VcsRevision& rev ) = 0;

    /**
     * Retrieve a file from the repository without checking it out
     */
    virtual VcsJob* cat( const QString& repoLocation, const VcsRevision& rev ) = 0;


};

}

Q_DECLARE_INTERFACE( KDevelop::IBrowsableVersionControl, "org.kdevelop.IBrowsableVersionControl" )

#endif

