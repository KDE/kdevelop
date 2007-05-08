/* This file is part of KDevelop
 *
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
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

#ifndef IATOMICBROWSABLEVERSIONCONTROL_H
#define IATOMICBROWSABLEVERSIONCONTROL_H

class IAtomicBrowsableVersionControl
{
public:
    /**
     * retrieve the last revision in which the repository location was changed
     */
    virtual VcsJob repositoryRevision( const QString& repoLocation,
                                       VcsRevision::RevisionType ) = 0;

    /**
     * Get the changes made by a particular revision
     */
    virtual VcsJob change( const VcsRevision& ) = 0;

    /**
     * Show the changes made by a particular revision
     */
    virtual VcsJob showChange( const VcsRevision& ) = 0;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
