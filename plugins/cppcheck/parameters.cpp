/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    // FIXME: only one build system manager - QMakeManager - ever returns a nonempty include directory list
    // for a target item. With any other project build system manager, cppcheck::Parameters::m_includeDirectories
    // is always empty. For example, during the loop below CMakeManager prints the following debug message:
    //     kdevelop.plugins.cmake: no information found for ""
    // because a target item's path() is always empty.
    // The algorithm here attempts to collect include directories for all project targets and use them to analyze any
    // file. This algorithm can be patched up by passing the first child of a target item instead of the target item
    // itself to IBuildSystemManager::includeDirectories(). However, the include directories would needlessly contain
    // include paths for all unit tests then. Also each file in a target can potentially have different include paths.
    // So perhaps the current algorithm should be replaced with something else altogether:
    // 1. When KDevelop runs cppcheck on a single file, that file's project item should be passed to
    // IBuildSystemManager::includeDirectories().
    // 2. When KDevelop runs cppcheck on an entire project, it passes the project's root directory to cppcheck
    // on the command line. cppcheck checks all source files in the given directory recursively.
    // db0d8027749ba8c94702981ccb3062fa6c6006eb even implemented a workaround to skip cppcheck-ing files in
    // <current build dir>/CMakeFiles/. That workaround is not perfect: what if there are multiple build
    // subdirectories (e.g. debug and release) within the project's directory? Instead of running a single
    // cppcheck command, we could run a separate cppcheck command for each not-filtered-out source file in
    // the project. This way we could pass each file's project item to IBuildSystemManager::includeDirectories()
    // and use a separate include directory list to analyze every file. But this would require determining which
    // files should and which shouldn't be analyzed (C and C++ MIME types?), and might be slower.
    // Apparently fixing this issue properly requires substantial refactoring and testing effort.

    const auto children = parent->children();
    for (auto* child : children) {
        if (child->type() == KDevelop::ProjectBaseItem::ProjectItemType::File) {
            continue;
        }

        else if (child->type() == KDevelop::ProjectBaseItem::ProjectItemType::ExecutableTarget ||
                 child->type() == KDevelop::ProjectBaseItem::ProjectItemType::LibraryTarget ||
                 child->type() == KDevelop::ProjectBaseItem::ProjectItemType::Target) {

            if (auto buildSystemManager = child->project()->buildSystemManager()) {
                const auto includeDirectories = buildSystemManager->includeDirectories(child);
                for (auto& dir : includeDirectories) {
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

    return includesSet.values();
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
        for (const auto& dir : m_includeDirectories) {
            if (dir.path().endsWith(QLatin1String("QtCore"))) {
                qtUsed = true;

                QFile qtHeader(dir.path() + QStringLiteral("/qobjectdefs.h"));
                if (!qtHeader.open(QIODevice::ReadOnly)) {
                    break;
                }

                while(!qtHeader.atEnd()) {
                    auto match = mocHeaderRegex.match(QString::fromUtf8(qtHeader.readLine()));
                    if (match.hasMatch()) {
                        mocDefineFound = true;
                        result << QLatin1String("-DQ_MOC_OUTPUT_REVISION=") + match.capturedView(1);
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

        const auto elements = applyPlaceholders(ignoredIncludes).split(QLatin1Char(';'));
        for (const QString& element : elements) {
            if (!element.trimmed().isEmpty()) {
                ignored.append(KDevelop::Path(element));
            }
        }

        for (const auto& dir : m_includeDirectories) {
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
        if (m_project->managerPlugin()->componentName() == QLatin1String("kdevcmakemanager")) {
            result << QStringLiteral("-i")
                   << m_projectBuildPath.toLocalFile() + QLatin1String("/CMakeFiles");
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
