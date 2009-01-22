/* KDevelop CMake Support
 *
 * Copyright 2009 Andreas Pakulat <apaku@gmx.de>
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

#ifndef CMAKEUTILS_H
#define CMAKEUTILS_H

#include "cmakeexport.h"

namespace KDevelop
{
    class ProjectBaseItem;
    class IProject;
}

class KUrl;

namespace CMake
{
    /**
     * Checks wether there's a need to run cmake for the given project item
     * This is the case if no builddir has been specified, in which case
     * it asks for one.
     *
     * @returns true if configure should be run, false otherwise
     */
    KDEVCMAKECOMMON_EXPORT bool checkForNeedingConfigure( KDevelop::ProjectBaseItem* item );

    /**
     * @returns the builddir for the given project or an empty url if none
     * has been set by the user.
     */
    KDEVCMAKECOMMON_EXPORT KUrl buildDirForProject( KDevelop::IProject* project );
}

#endif


