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

#include "configconstants.h"

#include <QString>

namespace ConfigConstants
{
    QString buildConfigPrefix()      { return QStringLiteral("BuildConfig"); }
    QString buildDirKey()            { return QStringLiteral("BuildDir"); }
    QString currentConfigKey()       { return QStringLiteral("CurrentConfiguration"); }
    QString toolGroupPrefix()        { return QStringLiteral("Tool"); }
    QString toolArguments()          { return QStringLiteral("Arguments"); }
    QString toolExecutable()         { return QStringLiteral("Executable"); }
    QString toolEnvironment()        { return QStringLiteral("Environment"); }
    QString toolType()               { return QStringLiteral("Type"); }
    QString toolEnabled()            { return QStringLiteral("Enabled"); }
    QString projectPathPrefix()      { return QStringLiteral("ProjectPath"); }
    QString projectPathKey()         { return QStringLiteral("Path"); }
    QString customBuildSystemGroup() { return QStringLiteral("CustomBuildSystem"); }
    QString configTitleKey()         { return QStringLiteral("Title"); }
}
