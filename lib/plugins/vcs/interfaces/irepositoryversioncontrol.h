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

#ifndef IREPOSITORYVERSIONCONTROL_H
#define IREPOSITORYVERSIONCONTROL_H

class IRepositoryVersionControl
{
public:
    /**
     * Copies the source location in the repository to the destination
     */
    virtual VcsJob copy( const QString& commitMessage,
                         const QString& repoSrc,
                         const QString& repoDst,
                         const VcsRevision& srcRev ) = 0;

    /**
     * Shows a dialog asking for a commit message that will copy the source
     * location in the repository to the destination
     */
    virtual VcsJob showCopy( const QString& commitMessage,
                             const QString& repoSrc,
                             const QString& repoDst,
                             const VcsRevision& srcRev ) = 0;

    /**
     * Moves the source location in the repository to the destination
     */
    virtual VcsJob move( const QString& commitMessage,
                         const QString& repoSrc,
                         const QString& repoDst,
                         const VcsRevision& srcRev ) = 0;

    /**
     * Shows a dialog asking for a commit message that will move the source
     * location in the repository to the destination
     */
    virtual VcsJob showMove( const QString& commitMessage,
                             const QString& repoSrc,
                             const QString& repoDst,
                             const VcsRevision& srcRev ) = 0;

    /**
     * Removes the source locations in the repository
     */
    virtual VcsJob remove( const QString& commitMessage,
                           const QStringList& repoLocations ) = 0;

    /**
     * Shows a dialog asking for a commit message that will remove the source
     * locations in the repository
     */
    virtual VcsJob showRemove( const QString& commitMessage,
                               const QStringList& repoLocations ) = 0;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
