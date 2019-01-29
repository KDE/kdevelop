/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

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

#include "jobparameters.h"

#include "debug.h"
#include "globalsettings.h"
#include "projectsettings.h"
#include "utils.h"

#include <interfaces/iproject.h>
#include <serialization/indexedstring.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <KShell>
#include <KLocalizedString>

#include <QDir>
#include <QStandardPaths>

namespace Clazy
{

JobGlobalParameters::JobGlobalParameters()
    : JobGlobalParameters(GlobalSettings::executablePath(), GlobalSettings::docsPath())
{
}

JobGlobalParameters::JobGlobalParameters(const QUrl& executablePath, const QUrl& docsPath)
{
    m_executablePath = executablePath.toLocalFile();
    m_docsPath = docsPath.toLocalFile();

    QFileInfo info;

    if (m_executablePath.isEmpty()) {
        if (defaultExecutablePath().toLocalFile().isEmpty()) {
            m_error = i18n(
                "clazy-standalone path cannot be detected. "
                "Set the path manually if Clazy is already installed.");
        } else {
            m_error = i18n("clazy-standalone path is empty.");
        }
        return;
    }

    info.setFile(m_executablePath);

    if (!info.exists()) {
        m_error = i18n("clazy-standalone path '%1' does not exists.", m_executablePath);
        return;
    }

    if (!info.isFile() || !info.isExecutable()) {
        m_error = i18n("clazy-standalone path '%1' is not an executable.", m_executablePath);
        return;
    }

    // =============================================================================================

    if (m_docsPath.isEmpty()) {
        if (defaultDocsPath().toLocalFile().isEmpty()) {
            m_error = i18n(
                "Clazy documentation path cannot be detected. "
                "Set the path manually if Clazy is already installed.");
        } else {
            m_error = i18n("Clazy documentation path is empty.");
        }
        return;
    }

    info.setFile(m_docsPath);

    if (!info.exists()) {
        m_error = i18n("Clazy documentation path '%1' does not exists.", m_docsPath);
        return;
    }

    if (!info.isDir()) {
        m_error = i18n("Clazy documentation path '%1' is not a directory.", m_docsPath);
        return;
    }

    m_error.clear();
}

QUrl JobGlobalParameters::defaultExecutablePath()
{
    return QUrl::fromLocalFile(QStandardPaths::findExecutable(QStringLiteral("clazy-standalone")));
}

QUrl JobGlobalParameters::defaultDocsPath()
{
    const QString subPathsCandidates[2]{
        // since clazy 1.4
        QStringLiteral("doc/clazy"),
        // before
        QStringLiteral("clazy/doc"),
    };
    for (auto subPath : subPathsCandidates) {
        const auto docsPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, subPath, QStandardPaths::LocateDirectory);

        if (!docsPath.isEmpty()) {
            return QUrl::fromLocalFile(QDir(docsPath).canonicalPath());
        }
    }

    return {};
}

bool JobGlobalParameters::isValid() const
{
    return m_error.isEmpty();
}

QString JobGlobalParameters::error() const
{
    return m_error;
}

JobParameters::JobParameters(KDevelop::IProject* project)
    : JobParameters(project, QString())
{
}

JobParameters::JobParameters(KDevelop::IProject* project, const QString& checkPath)
    : m_checkPath(checkPath)
{
    Q_ASSERT(project);

    auto projectRootPath = project->path().toLocalFile();
    auto projectCanonicalRootPath = QFileInfo(projectRootPath).canonicalFilePath();

    auto buildPath = project->buildSystemManager()->buildDirectory(project->projectItem());
    m_projectBuildPath = buildPath.toLocalFile();

    buildPath.addPath(QStringLiteral("compile_commands.json"));

    auto commandsFilePath = buildPath.toLocalFile();
    if (!QFile::exists(commandsFilePath)) {
        m_error = i18n("Compile commands file '%1' does not exist.", commandsFilePath);
        return;
    }
 
    const auto pathInfo = QFileInfo(m_checkPath);
    const bool checkPathIsFile = pathInfo.isFile();
    const auto canonicalPathToCheck = checkPathIsFile ? pathInfo.canonicalFilePath() : QStringLiteral("");

    if (!m_checkPath.isEmpty()) {
        const auto allFiles = compileCommandsFiles(commandsFilePath, m_error);
        if (!m_error.isEmpty()) {
            return;
        }

        if (canonicalPathToCheck == projectCanonicalRootPath) {
            m_sources = allFiles;
        } else {
            for (auto& file : allFiles) {
                if (checkPathIsFile) {
                    if (file == canonicalPathToCheck) {
                        m_sources.clear();
                        m_sources += canonicalPathToCheck;
                        break;
                    }
                } else if (file.startsWith(m_checkPath) || file.startsWith(canonicalPathToCheck)) {
                    m_sources += file;
                }
            }
        }
    }

    ProjectSettings projectSettings;
    projectSettings.setSharedConfig(project->projectConfiguration());
    projectSettings.load();

    {
        // blocks changed() signal from setters
        QSignalBlocker blocker(this);

        setChecks(projectSettings.checks());

        setOnlyQt(projectSettings.onlyQt());
        setQtDeveloper(projectSettings.qtDeveloper());
        setQt4Compat(projectSettings.qt4Compat());
        setVisitImplicitCode(projectSettings.visitImplicitCode());

        setIgnoreIncludedFiles(projectSettings.ignoreIncludedFiles());
        setHeaderFilter(projectSettings.headerFilter());

        setEnableAllFixits(projectSettings.enableAllFixits());
        setNoInplaceFixits(projectSettings.noInplaceFixits());

        setExtraAppend(projectSettings.extraAppend());
        setExtraPrepend(projectSettings.extraPrepend());
        setExtraClazy(projectSettings.extraClazy());
    }

    if (m_sources.isEmpty()) {
        m_error = i18n("Nothing to check: compile commands file '%1' contains no matching items.", commandsFilePath);
    }
}

QString JobParameters::defaultChecks()
{
    return QStringLiteral("level1");
}

QString JobParameters::checkPath() const
{
    return m_checkPath;
}

const QStringList& JobParameters::sources() const
{
    return m_sources;
}

QString JobParameters::projectBuildPath() const
{
    return m_projectBuildPath;
}

QStringList JobParameters::commandLine() const
{
    QStringList arguments;

    arguments << m_executablePath;

    if (!m_checks.isEmpty()) {
        arguments << QStringLiteral("-checks=%1").arg(m_checks);
    }

    if (m_onlyQt) {
        arguments << QStringLiteral("-only-qt");
    }

    if (m_qtDeveloper) {
        arguments << QStringLiteral("-qt-developer");
    }

    if (m_qt4Compat) {
        arguments << QStringLiteral("-qt4-compat");
    }

    if (m_visitImplicitCode) {
        arguments << QStringLiteral("-visit-implicit-code");
    }

    if (m_ignoreIncludedFiles) {
        arguments << QStringLiteral("-ignore-included-files");
    }

    if (!m_headerFilter.isEmpty()) {
        arguments << QStringLiteral("-header-filter=%1").arg(m_headerFilter);
    }

    if (m_enableAllFixits) {
        arguments << QStringLiteral("-enable-all-fixits");
    }

    if (m_noInplaceFixits) {
        arguments << QStringLiteral("-no-inplace-fixits");
    }

    if (!m_extraAppend.isEmpty()) {
        arguments << QStringLiteral("-extra-arg=%1").arg(m_extraAppend);
    }

    if (!m_extraPrepend.isEmpty()) {
        arguments << QStringLiteral("-extra-arg-before=%1").arg(m_extraPrepend);
    }

    if (!m_extraClazy.isEmpty()) {
        arguments << KShell::splitArgs(m_extraClazy);
    }

    arguments << QStringLiteral("-p");
    arguments << m_projectBuildPath;

    return arguments;
}


template<typename T>
void JobParameters::setValue(T& currentValue, const T& newValue)
{
    if (currentValue != newValue) {
        currentValue = newValue;
        emit changed();
    }
}

void JobParameters::setChecks(const QString& checks)
{
    if (checks.isEmpty()) {
        setValue(m_checks, defaultChecks());
    } else {
        setValue(m_checks, checks);
    }
}

void JobParameters::setOnlyQt(bool onlyQt)
{
    setValue(m_onlyQt, onlyQt);
}

void JobParameters::setQtDeveloper(bool qtDeveloper)
{
    setValue(m_qtDeveloper, qtDeveloper);
}

void JobParameters::setQt4Compat(bool qt4Compat)
{
    setValue(m_qt4Compat, qt4Compat);
}

void JobParameters::setVisitImplicitCode(bool visitImplicitCode)
{
    setValue(m_visitImplicitCode, visitImplicitCode);
}

void JobParameters::setIgnoreIncludedFiles(bool ignoreIncludedFiles)
{
    setValue(m_ignoreIncludedFiles, ignoreIncludedFiles);
}

void JobParameters::setHeaderFilter(const QString& headerFilter)
{
    setValue(m_headerFilter, headerFilter);
}

void JobParameters::setEnableAllFixits(bool enableAllFixits)
{
    setValue(m_enableAllFixits, enableAllFixits);
}

void JobParameters::setNoInplaceFixits(bool noInplaceFixits)
{
    setValue(m_noInplaceFixits, noInplaceFixits);
}

void JobParameters::setExtraAppend(const QString& extraAppend)
{
    setValue(m_extraAppend, extraAppend);
}

void JobParameters::setExtraPrepend(const QString& extraPrepend)
{
    setValue(m_extraPrepend, extraPrepend);
}

void JobParameters::setExtraClazy(const QString& extraClazy)
{
    setValue(m_extraClazy, extraClazy);
}

}
