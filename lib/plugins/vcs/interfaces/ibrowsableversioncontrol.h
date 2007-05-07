/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef IBROWSABLEVERSIONCONTROL_H
#define IBROWSABLEVERSIONCONTROL_H

class IBrowsableVersionControl
{
public:
    /**
     * Retrieve a list of entries in the given repository location
     */
    virtual VcsJob ls( const QString& repoLocation, const VcsRevision& rev ) = 0;

    /**
     * Retrieve the history of the repository location
     *
     * @param rev List @p rev and earlier. The default is HEAD.
     * @param limit Restrict to the most recent @p limit entries. Note that the
     * limit is @e advisory and may be ignored.
     */
    virtual VcsJob log( const QString& repoLocation,
                        const VcsRevision& rev,
                        unsigned int limit ) = 0;

    /**
     * Show the history of the repository location
     *
     * @param rev List @p rev and earlier. The default is HEAD.
     */
    virtual VcsJob showLog( const QString& repoLocation,
                            const VcsRevision& rev ) = 0;

    /**
     * Retrieve a file from the repository without checking it out
     */
    virtual VcsJob cat( const QString& repoLocation, const VcsRevision& rev ) = 0;
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
