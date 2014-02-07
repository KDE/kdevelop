/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#ifndef CONFIGCONSTANTS_H
#define CONFIGCONSTANTS_H

#include <QString>

namespace ConfigConstants
{
    const QString buildConfigPrefix("BuildConfig");
    const QString buildDirKey("BuildDir");
    const QString currentConfigKey("CurrentConfiguration");
    const QString toolGroupPrefix("Tool");
    const QString toolArguments("Arguments");
    const QString toolExecutable("Executable");
    const QString toolEnvironment("Environment");
    const QString toolType("Type");
    const QString toolEnabled("Enabled");
    const QString projectPathPrefix("ProjectPath");
    const QString projectPathKey("Path");
    const QString customBuildSystemGroup("CustomBuildSystem");
    const QString configTitleKey("Title");
}

#endif
