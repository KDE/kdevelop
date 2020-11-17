/* This file is part of KDevelop

   Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SHELLCHECK_PARAMETERS_H
#define SHELLCHECK_PARAMETERS_H

//#include <util/path.h>

namespace KDevelop
{
class IProject;
}

namespace shellcheck
{

namespace defaults
{

// global settings
static const bool hideOutputView = true;
//static const bool showXmlOutput = false;

// project settings
static const bool checkStyle = false;
static const bool checkPerformance = false;
static const bool checkPortability = false;
static const bool checkInformation = false;
static const bool checkUnusedFunction = false;
static const bool checkMissingInclude = false;
static const bool inconclusiveAnalysis = false;
static const bool forceCheck = false;
static const bool checkConfig = false;

static const bool useProjectIncludes = true;
static const bool useSystemIncludes = false;

}

class Parameters
{
public:
    explicit Parameters(KDevelop::IProject* project = nullptr);

    QStringList commandLine() const;
    QStringList commandLine(QString& infoMessage) const;

    // global settings
    QString executablePath;
    bool hideOutputView;
    //bool showXmlOutput;

    // project settings
    bool checkStyle;
    bool checkPerformance;
    bool checkPortability;
    bool checkInformation;
    bool checkUnusedFunction;
    bool checkMissingInclude;
    bool inconclusiveAnalysis;
    bool forceCheck;
    bool checkConfig;

    bool useProjectIncludes;
    bool useSystemIncludes;
    QString ignoredIncludes;

    QString extraParameters;

    // runtime settings
    QString checkPath;

//    KDevelop::Path projectRootPath() const;

private:
    QString applyPlaceholders(const QString& text) const;

    KDevelop::IProject* m_project;

//    KDevelop::Path m_projectRootPath;
//    KDevelop::Path m_projectBuildPath;

//    QList<KDevelop::Path> m_includeDirectories;
};

}
#endif
