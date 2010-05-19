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
#include <QStringList>

namespace KDevelop
{
    class ProjectBaseItem;
    class IProject;
}

class KUrl;
class ICMakeDocumentation;

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
     * @returns the current builddir for the given project or an empty url if none
     * has been set by the user.
     */
    KDEVCMAKECOMMON_EXPORT KUrl currentBuildDir( KDevelop::IProject* project );

    /**
     * @returns the current build type for the given project or "Release" as default value.
     */
    KDEVCMAKECOMMON_EXPORT QString currentBuildType( KDevelop::IProject* project );

    /**
     * @returns the current cmake binary for the given project or
     * KStandardDirs::findExe("cmake") as default value. 
     */
    KDEVCMAKECOMMON_EXPORT KUrl currentCMakeBinary( KDevelop::IProject* project );

    /**
     * @returns the current install dir for the given project or "/usr/local" as default value.
     */
    KDEVCMAKECOMMON_EXPORT KUrl currentInstallDir( KDevelop::IProject* project );
    
    /**
     * @returns the current extra arguments for the given project or "" as default value.
     */
    KDEVCMAKECOMMON_EXPORT QString currentExtraArguments( KDevelop::IProject* project );

    /**
     * Sets the current build dir for the given project.
     */
    KDEVCMAKECOMMON_EXPORT QString projectRootRelative( KDevelop::IProject* project );
    
    /**
     * Convenience function to get the project root.
     */
    KDEVCMAKECOMMON_EXPORT KUrl projectRoot( KDevelop::IProject* project );

    /**
     * Sets the current install dir for the given project.
     */
    KDEVCMAKECOMMON_EXPORT void setCurrentInstallDir( KDevelop::IProject* project, const KUrl& url );

    /**
     * Sets the current build type for the given project.
     */
    KDEVCMAKECOMMON_EXPORT void setCurrentBuildType( KDevelop::IProject* project, const QString& type );

    /**
     * Sets the current cmake binary for the given project.
     */
    KDEVCMAKECOMMON_EXPORT void setCurrentCMakeBinary( KDevelop::IProject* project, const KUrl& url );

    /**
     * Sets the current build dir for the given project.
     */
    KDEVCMAKECOMMON_EXPORT void setCurrentBuildDir( KDevelop::IProject* project, const KUrl& url );
    
    /**
     * Sets the current build dir for the given project.
     */
    KDEVCMAKECOMMON_EXPORT void setProjectRootRelative( KDevelop::IProject* project, const QString& path);
    
    /**
     * Sets the current extra arguments for the given project.
     */
    KDEVCMAKECOMMON_EXPORT void setCurrentExtraArguments( KDevelop::IProject* project, const QString& args );
    
    /**
     * Obtains a cmake documentation instance if it exists
     */
    KDEVCMAKECOMMON_EXPORT ICMakeDocumentation* cmakeDocumentation();
    
    /**
     * Retrieves the configured build directories for @p project.
     */
    KDEVCMAKECOMMON_EXPORT QStringList allBuildDirs(KDevelop::IProject* project);
}

#endif


