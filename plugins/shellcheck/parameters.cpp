/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "parameters.h"

#include "globalsettings.h"


namespace shellcheck 
{
    
Parameters::Parameters()
{
    executablePath = GlobalSettings::executablePath();
    hideOutputView = GlobalSettings::hideOutputView();
    includeChecks = GlobalSettings::includeChecks();
    excludeChecks = GlobalSettings::excludeChecks();
    includeOptionalChecks = GlobalSettings::includeOptionalChecks();
    outputFormat = defaults::outputFormat;
    shellDialect = GlobalSettings::shellDialect();
    severityLevel = GlobalSettings::severityLevel();
}

QString Parameters::intToSeverityLevel(int in) const
{
    switch(in)
    {
        case 0:
            return QString(QStringLiteral("style"));
        case 1:
            return QString(QStringLiteral("info"));
        case 2:
            return QString(QStringLiteral("warning"));
        case 3:
            return QString(QStringLiteral("error"));
        default:
            return QString(QStringLiteral("style"));
    }
}

QString Parameters::intToShellDialect(int in) const
{
    switch(in)
    {
        case 0:
            return QString(QStringLiteral("")); // No string is auto
        case 1:
            return QString(QStringLiteral("sh"));
        case 2:
            return QString(QStringLiteral("bash"));
        case 3:
            return QString(QStringLiteral("dash"));
        case 4:
            return QString(QStringLiteral("ksh"));
        default:
            return QString(QStringLiteral(""));
    }
}


QStringList Parameters::commandLine() const
{
    QStringList result;
    QUrl executable(executablePath);
    result << executable.toLocalFile();

    if(!includeChecks.isEmpty())
    {
        result << QStringLiteral("--include=") + includeChecks;
    }
    if(!excludeChecks.isEmpty())
    {
        result << QStringLiteral("--exclude=") + excludeChecks;
    }
    if(!includeOptionalChecks.isEmpty())
    {
        result << QStringLiteral("--enable=") + includeOptionalChecks;
    }
    if(!outputFormat.isEmpty())
    {
        result << QStringLiteral("--format=") + outputFormat;
    }
    if(!shellDialect.isEmpty())
    {
        bool ok;
        int dialectInteger = shellDialect.toInt(&ok, 10);
        if(ok) {
            QString tmp  = intToShellDialect(dialectInteger);
            if(!tmp.isEmpty()) {
                result << QStringLiteral("--shell=") + tmp;
            }
        } else {
            result << QStringLiteral("--shell=") + shellDialect;
        }
    }
    if(!severityLevel.isEmpty())
    {
        bool ok;
        int severityInteger = severityLevel.toInt(&ok, 10);
        if(ok) {
            QString tmp  = intToSeverityLevel(severityInteger);
            result << QStringLiteral("--severity=") + tmp;
        } else {
            result << QStringLiteral("--severity=") + severityLevel;
        }
    }
    result << shellFileToCheck;
    return result;
}

} 
