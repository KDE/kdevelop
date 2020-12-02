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

#ifndef SHELLCHECK_PARAMETERS_H
#define SHELLCHECK_PARAMETERS_H

#include <QString>
#include <QStringList>

namespace shellcheck
{

namespace defaults
{

/// global settings
static const bool hideOutputView = true;
/// Include checks for sourced files
static const bool checkSourced = false;
// Default all (Non-optional) checks are excluded
static const QString includeChecks;
// Default no checks are excluded
static const QString excludeChecks;
// Default no optional checks are included ('all' will include all)
static const QString includeOptionalChecks;
// The standard for shellcheck is tty, but we use json because we can
// already parse this output without having to write a new parser.
static const QString outputFormat = QStringLiteral("json1");
// Left empty it means shellcheck will try and dermine Bourne shell dialect from the
// shebang line in the script. Bash is used if cannot be determined.
static const QString shellDialect = QStringLiteral("");
/// Default severity level is style (i.e. all levels are output)
static const QString severityLevel = QStringLiteral("style");
}

class Parameters
{
public:
    explicit Parameters();

    QStringList commandLine() const;

    QString intToShellDialect(int in) const;

    QString intToSeverityLevel(int in) const;

    // global settings
    QString executablePath;
    bool hideOutputView;
    bool checkSourced;
    QString includeChecks;
    QString excludeChecks;
    QString includeOptionalChecks;
    QString outputFormat;
    QString shellDialect;
    QString severityLevel;

    // file to check
    QString shellFileToCheck;

};

}
#endif
