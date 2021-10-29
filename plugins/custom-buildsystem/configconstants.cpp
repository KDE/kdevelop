/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

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
