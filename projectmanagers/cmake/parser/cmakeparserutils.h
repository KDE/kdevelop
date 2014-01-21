/* KDevelop CMake Support
 *
 * Copyright 2008 Matt Rogers <mattr@kde.org>
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
#ifndef CMAKEPARSERUTILS_H
#define CMAKEPARSERUTILS_H

#include "cmakeexport.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QPair>
#include "cmaketypes.h"
#include <language/duchain/topducontext.h>

struct CMakeProjectData;
class VariableMap;

namespace KDevelop {
    class IProject;
    class Path;
}

namespace CMakeParserUtils
{
    /** 
     * Parse a string in the form of "x.y.z" that could possibly
     * be a version number.
     */
    KDEVCMAKECOMMON_EXPORT QList<int> parseVersion(const QString &version, bool *ok);
    /**
     * extracts the value of a variable from the --system-information output from CMake
     */
    KDEVCMAKECOMMON_EXPORT QString valueFromSystemInfo( const QString& variable, const QString& systeminfo );
    
    /**
     * produces a map of initially set variables and scripts to include. 
     * This is based on the CMake output from --system-information
     * and on reading the CMake sources.
     */
    KDEVCMAKECOMMON_EXPORT QPair<VariableMap,QStringList> initialVariables( );

    /** Runs the process specified by @p execName with @p args */
    KDEVCMAKECOMMON_EXPORT QString executeProcess(const QString& execName, const QStringList& args=QStringList());
    
    KDEVCMAKECOMMON_EXPORT KDevelop::ReferencedTopDUContext includeScript( const QString& file, const KDevelop::ReferencedTopDUContext& parent, CMakeProjectData* data, const QString& sourcedir, const QMap< QString, QString >& env);
    
    KDEVCMAKECOMMON_EXPORT CacheValues readCache(const KDevelop::Path& path);

    KDEVCMAKECOMMON_EXPORT QString binaryPath(const QString& sourcedir, const QString& projectSourceDir, const QString projectBinDir);

    KDEVCMAKECOMMON_EXPORT void addDefinitions(const QStringList& definitions, CMakeDefinitions* to, const bool expectDashD = false);
    KDEVCMAKECOMMON_EXPORT void removeDefinitions(const QStringList& definitions, CMakeDefinitions* from, const bool expectDashD = false);
}

#endif

