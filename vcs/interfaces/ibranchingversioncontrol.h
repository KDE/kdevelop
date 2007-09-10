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

#ifndef IBRANCHINGVERSIONCONTROL_H
#define IBRANCHINGVERSIONCONTROL_H

#include <iextension.h>

class QString;
class VcsMapping;
class VcsRevision;

namespace KDevelop
{

class IBranchingVersionControl
{
public:
    /**
     * Creates a branch from the given mapping information
     *
     * @param mapping List of source/destination pairs of repository paths
     * specifying what should end up where. Some VCS's may not support
     * destination paths and will ignore them. The source paths always specify
     * what is to be branched.
     * @param rev What revision of the requested items should be branched.
     * @param branchName Short, descriptive name for the branch used for VCS's
     * that take branch names instead of destination paths, or store metadata
     * about the branch. Not all VCS's will use this parameter.
     */
    virtual VcsJob* branch( const QString& commitMessage,
                            const VcsMapping& mapping,
                            const VcsRevision& rev,
                            const QString& branchName ) = 0;


    /**
     * Creates a new tag from the given mapping information
     *
     * @param mapping List of source/destination pairs of repository paths
     * specifying what should end up where. Some VCS's may not support
     * destination paths and will ignore them. The source paths always specify
     * what is to be tagged.
     * @param rev What revision of the requested items should be tagged.
     * @param tagName Short, descriptive name for the tag used for VCS's that
     * take tag names instead of destination paths, or store metadata about the
     * tag. Not all VCS's will use this parameter.
     */
    virtual VcsJob* tag( const QString& commitMessage,
                         const VcsMapping& mapping,
                         const VcsRevision& rev,
                         const QString& tagName ) = 0;

};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IBranchingVersionControl, "org.kdevelop.IBranchingVersionControl" )
Q_DECLARE_INTERFACE( KDevelop::IBranchingVersionControl, "org.kdevelop.IBranchingVersionControl" )

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
