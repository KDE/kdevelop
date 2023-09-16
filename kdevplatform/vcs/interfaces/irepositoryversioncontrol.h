/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IREPOSITORYVERSIONCONTROL_H
#define KDEVPLATFORM_IREPOSITORYVERSIONCONTROL_H

#include <QObject>

class QString;
#include <QStringList>

namespace KDevelop
{

class VcsRevision;
class VcsJob;

class IRepositoryVersionControl
{
public:
    virtual ~IRepositoryVersionControl() {}

    /**
     * Copies the source location in the repository to the destination
     */
    virtual VcsJob* copy( const QString& commitMessage,
                          const QString& repoSrc,
                          const QString& repoDst,
                          const VcsRevision& srcRev ) = 0;

    /**
     * Moves the source location in the repository to the destination
     */
    virtual VcsJob* move( const QString& commitMessage,
                          const QString& repoSrc,
                          const QString& repoDst,
                          const VcsRevision& srcRev ) = 0;


    /**
     * Removes the source locations in the repository
     */
    virtual VcsJob* remove( const QString& commitMessage,
                            const QStringList& repoLocations ) = 0;
};

}

Q_DECLARE_INTERFACE( KDevelop::IRepositoryVersionControl, "org.kdevelop.IRepositoryVersionControl" )

#endif

