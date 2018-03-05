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

#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <KShell>
#include <KLocalizedString>

#include <QFile>
#include <QRegularExpression>

namespace cppcheck
{

void includesForItem(KDevelop::ProjectBaseItem* parent, QSet<KDevelop::Path>& includes)
{
    foreach (auto child, parent->children()) {
        if (child->type() == KDevelop::ProjectBaseItem::ProjectItemType::File) {
            continue;
        }

        else if (child->type() == KDevelop::ProjectBaseItem::ProjectItemType::ExecutableTarget ||
                 child->type() == KDevelop::ProjectBaseItem::ProjectItemType::LibraryTarget ||
                 child->type() == KDevelop::ProjectBaseItem::ProjectItemType::Target) {

            if (auto buildSystemManager = child->project()->buildSystemManager()) {
                foreach (auto dir, buildSystemManager->includeDirectories(child)) {
                    includes.insert(dir);
                }
            }
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
    : m_project(project)
{
    executablePath = KDevelop::Path(GlobalSettings::executablePath()).toLocalFile();
    hideOutputView = GlobalSettings::hideOutputView();
    showXmlOutput  = GlobalSettings::showXmlOutput();

    if (!project) {
        checkStyle           = defaults::checkStyle;
        checkPerformance     = defaults::checkPerformance;
        checkPortability     = defaults::checkPortability;
        checkInformation     = defaults::checkInformation;
        checkUnusedFunction  = defaults::checkUnusedFunction;
        checkMissingInclude  = defaults::checkMissingInclude;
        inconclusiveAnalysis = defaults::inconclusiveAnalysis;
        forceCheck           = defaults::forceCheck;
        checkConfig          = defaults::checkConfig;

        useProjectIncludes   = defaults::useProjectIncludes;
        useSystemIncludes    = defaults::useSystemIncludes;

        return;
    }

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

    useProjectIncludes   = projectSettings.useProjectIncludes();
    useSystemIncludes    = projectSettings.useSystemIncludes();
    ignoredIncludes      = projectSettings.ignoredIncludes();

    extraParameters      = projectSettings.extraParameters();

    m_projectRootPath    = m_project->path();

    if (auto buildSystemManager = m_project->buildSystemManager()) {
        m_projectBuildPath   = buildSystemManager->buildDirectory(m_project->projectItem());
    }
    m_includeDirectories = includesForProject(project);
}

QStringList Parameters::commandLine() const
{
    QString temp;
    return commandLine(temp);
}

QStringList Parameters::commandLine(QString& infoMessage) const
{
    static const auto mocHeaderRegex = QRegularExpression(QStringLiteral("#define\\s+Q_MOC_OUTPUT_REVISION\\s+(.+)"));
    static const auto mocParametersRegex = QRegularExpression(QStringLiteral("-DQ_MOC_OUTPUT_REVISION=\\d{2,}"));

    const QString mocMessage = i18n(
        "It seems that this project uses Qt library. For correctly work of cppcheck "
        "the value for define Q_MOC_OUTPUT_REVISION must be set. Unfortunately, the plugin is unable "
        "to find this value automatically - you should set it manually by adding "
        "'-DQ_MOC_OUTPUT_REVISION=XX' to extra parameters. The 'XX' value can be found in any project's "
        "moc-generated file or in the <QtCore/qobjectdefs.h> header file.");

    QStringList result;

    result << executablePath;
    result << QStringLiteral("--xml-version=2");

    if (checkStyle) {
        result << QStringLiteral("--enable=style");
    }

    if (checkPerformance) {
        result << QStringLiteral("--enable=performance");
    }

    if (checkPortability) {
        result << QStringLiteral("--enable=portability");
    }

    if (checkInformation) {
        result << QStringLiteral("--enable=information");
    }

    if (checkUnusedFunction) {
        result << QStringLiteral("--enable=unusedFunction");
    }

    if (checkMissingInclude) {
        result << QStringLiteral("--enable=missingInclude");
    }

    if (inconclusiveAnalysis) {
        result << QStringLiteral("--inconclusive");
    }

    if (forceCheck) {
        result << QStringLiteral("--force");
    }

    if (checkConfig) {
        result << QStringLiteral("--check-config");
    }

    // Try to automatically get value of Q_MOC_OUTPUT_REVISION for Qt-projects.
    // If such define is not correctly set, cppcheck 'fails' on files with moc-includes
    // and not return any errors, even if the file contains them.
    if (!mocParametersRegex.match(extraParameters).hasMatch()) {
        bool qtUsed = false;
        bool mocDefineFound = false;
        foreach (auto dir, m_includeDirectories) {
            if (dir.path().endsWith(QLatin1String("QtCore"))) {
                qtUsed = true;

                QFile qtHeader(dir.path() + QStringLiteral("/qobjectdefs.h"));
                if (!qtHeader.open(QIODevice::ReadOnly)) {
                    break;
                }

                while(!qtHeader.atEnd()) {
                    auto match = mocHeaderRegex.match(qtHeader.readLine());
                    if (match.hasMatch()) {
                        mocDefineFound = true;
                        result << QStringLiteral("-DQ_MOC_OUTPUT_REVISION=") + match.captured(1);
                        break;
                    }
                }
                break;
            }
        }

        if (qtUsed && !mocDefineFound) {
            infoMessage = mocMessage;
        }
    }

    if (!extraParameters.isEmpty()) {
        result << KShell::splitArgs(applyPlaceholders(extraParameters));
    }

    if (m_project && useProjectIncludes) {
        QList<KDevelop::Path> ignored;

        foreach (const QString& element, applyPlaceholders(ignoredIncludes).split(';')) {
            if (!element.trimmed().isEmpty()) {
                ignored.append(KDevelop::Path(element));
            }
        }

        foreach (const auto& dir, m_includeDirectories) {
            if (ignored.contains(dir)) {
                continue;
            }

            else if (useSystemIncludes ||
                     dir == m_projectRootPath || m_projectRootPath.isParentOf(dir) ||
                     dir == m_projectBuildPath || m_projectBuildPath.isParentOf(dir)) {

                result << QStringLiteral("-I");
                result << dir.toLocalFile();
            }
        }
    }

    if (m_project && m_project->managerPlugin()) {
        if (m_project->managerPlugin()->componentName() == QStringLiteral("kdevcmakemanager")) {
            result << QStringLiteral("-i %1/CMakeFiles").arg(m_projectBuildPath.toLocalFile());
        }
    }

    result << checkPath;

    return result;
}

KDevelop::Path Parameters::projectRootPath() const
{
    return m_projectRootPath;
}

QString Parameters::applyPlaceholders(const QString& text) const
{
    QString result(text);

    if (m_project) {
        result.replace(QLatin1String("%p"), m_projectRootPath.toLocalFile());
        result.replace(QLatin1String("%b"), m_projectBuildPath.toLocalFile());
    }

    return result;
}

}
