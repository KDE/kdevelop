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
     * Copy the source location in the repository to the destination
     */
    int copy( const QString& repoSrc, const QString& repoDst,
              const Revision& srcRev );


    /**
     * Move the source location in the repository to the destination
     */
    int move( const QString& repoSrc, const QString& repoDst,
              const Revision& srcRev );
};

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
