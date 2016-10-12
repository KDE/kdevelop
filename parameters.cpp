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

#include "parameters.h"

#include "globalsettings.h"
#include "projectsettings.h"

#include <interfaces/iproject.h>
#include <KShell>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

namespace cppcheck
{

void includesForItem(KDevelop::ProjectBaseItem* parent, QSet<KDevelop::Path>& includes)
{
    foreach (auto child, parent->children()) {
        if (child->type() == KDevelop::ProjectBaseItem::ProjectItemType::File)
            continue;

        else if (child->type() == KDevelop::ProjectBaseItem::ProjectItemType::ExecutableTarget ||
                 child->type() == KDevelop::ProjectBaseItem::ProjectItemType::LibraryTarget ||
                 child->type() == KDevelop::ProjectBaseItem::ProjectItemType::Target) {

            foreach (auto dir, child->project()->buildSystemManager()->includeDirectories(child))
                includes.insert(dir);
        }

        includesForItem(child, includes);
    }
}

QList<KDevelop::Path> includesForProject(KDevelop::IProject* project)
{
    QSet<KDevelop::Path> includesSet;
    includesForItem(project->projectItem(), includesSet);

    return includesSet.toList();
}

Parameters::Parameters(KDevelop::IProject* project)
    : executablePath(defaults::executablePath)
    , hideOutputView(defaults::hideOutputView)
    , showXmlOutput(defaults::showXmlOutput)
    , checkStyle(defaults::checkStyle)
    , checkPerformance(defaults::checkPerformance)
    , checkPortability(defaults::checkPortability)
    , checkInformation(defaults::checkInformation)
    , checkUnusedFunction(defaults::checkUnusedFunction)
    , checkMissingInclude(defaults::checkMissingInclude)
    , inconclusiveAnalysis(defaults::inconclusiveAnalysis)
    , forceCheck(defaults::forceCheck)
    , checkConfig(defaults::checkConfig)
    , useProjectIncludes(defaults::useProjectIncludes)
    , useSystemIncludes(defaults::useSystemIncludes)
    , m_project(project)
{
    executablePath = KDevelop::Path(GlobalSettings::executablePath()).toLocalFile();
    hideOutputView = GlobalSettings::hideOutputView();
    showXmlOutput  = GlobalSettings::showXmlOutput();

    if (!project)
        return;

    ProjectSettings projectSettings;
    projectSettings.setSharedConfig(project->projectConfiguration());
    projectSettings.load();

    checkStyle           = projectSettings.checkStyle();
    checkPerformance     = projectSettings.checkPerformance();
    checkPortability     = projectSettings.checkPortability();
    checkInformation     = projectSettings.checkInformation();
    checkUnusedFunction  = projectSettings.checkUnusedFunction();
    checkMissingInclude  = projectSettings.checkMissingInclude();
    inconclusiveAnalysis = projectSettings.inconclusiveAnalysis();
    forceCheck           = projectSettings.forceCheck();
    checkConfig          = projectSettings.checkConfig();

    extraParameters      = projectSettings.extraParameters();

    m_projectRootPath = m_project->path();
    m_projectBuildPath = m_project->buildSystemManager()->buildDirectory(m_project->projectItem());
    m_includeDirectories = includesForProject(project);
}

QStringList Parameters::commandLine() const
{
    QStringList result;

    result << executablePath;
    result << QStringLiteral("--xml-version=2");

    if (checkStyle)
        result << QStringLiteral("--enable=style");

    if (checkPerformance)
        result << QStringLiteral("--enable=performance");

    if (checkPortability)
        result << QStringLiteral("--enable=portability");

    if (checkInformation)
        result << QStringLiteral("--enable=information");

    if (checkUnusedFunction)
        result << QStringLiteral("--enable=unusedFunction");

    if (checkMissingInclude)
        result << QStringLiteral("--enable=missingInclude");

    if (inconclusiveAnalysis)
        result << QStringLiteral("--inconclusive");

    if (forceCheck)
        result << QStringLiteral("--force");

    if (checkConfig)
        result << QStringLiteral("--check-config");

    if (!extraParameters.isEmpty())
        result << KShell::splitArgs(applyPlaceholders(extraParameters));

    if (m_project && useProjectIncludes) {
        QList<KDevelop::Path> ignored;

        foreach (QString element, applyPlaceholders(ignoredIncludes).split(';')) {
            if (!element.trimmed().isEmpty())
                ignored.append(KDevelop::Path(element));
        }

        foreach (auto dir, m_includeDirectories) {
            if (ignored.contains(dir))
                continue;

            else if (useSystemIncludes ||
                     dir == m_projectRootPath || m_projectRootPath.isParentOf(dir) ||
                     dir == m_projectBuildPath || m_projectBuildPath.isParentOf(dir)) {

                result << QStringLiteral("-I");
                result << dir.toLocalFile();
            }
        }
    }

    result << checkPath;

    return result;
}

QString Parameters::applyPlaceholders(const QString& text) const
{
    QString result(text);

    if (m_project) {
        result.replace("%p", m_projectRootPath.toLocalFile());
        result.replace("%b", m_projectBuildPath.toLocalFile());
    }

    return result;
}

}
