/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CPPCHECK_PARAMETERS_H
#define CPPCHECK_PARAMETERS_H

#include <util/path.h>

namespace KDevelop
{
class IProject;
}

namespace cppcheck
{

namespace defaults
{

// global settings
static const bool hideOutputView = true;
static const bool showXmlOutput = false;

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
    bool showXmlOutput;

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

    KDevelop::Path projectRootPath() const;

private:
    QString applyPlaceholders(const QString& text) const;

    KDevelop::IProject* m_project;

    KDevelop::Path m_projectRootPath;
    KDevelop::Path m_projectBuildPath;

    QList<KDevelop::Path> m_includeDirectories;
};

}
#endif
