/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 * Copyright 2018, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "analyzer.h"

// plugin
#include "plugin.h"
#include "job.h"
#include "checksetselectionmanager.h"
#include <clangtidyconfig.h>
#include <clangtidyprojectconfig.h>
// KDevPlatform
#include <interfaces/iproject.h>
#include <util/path.h>
// KF
#include <KLocalizedString>
// Qt
#include <QThread>

namespace ClangTidy
{

Analyzer::Analyzer(Plugin* plugin, CheckSetSelectionManager* checkSetSelectionManager, QObject* parent)
    : KDevelop::CompileAnalyzer(plugin,
                                i18n("Clang-Tidy"), QStringLiteral("dialog-ok"),
                                QStringLiteral("clangtidy_file"), QStringLiteral("clangtidy_project"),
                                QStringLiteral("ClangTidy"),
                                KDevelop::ProblemModel::CanDoFullUpdate |
                                KDevelop::ProblemModel::ScopeFilter |
                                KDevelop::ProblemModel::SeverityFilter |
                                KDevelop::ProblemModel::Grouping |
                                KDevelop::ProblemModel::CanByPassScopeFilter,
                                parent)
    , m_plugin(plugin)
    , m_checkSetSelectionManager(checkSetSelectionManager)
{
}

Analyzer::~Analyzer() = default;

KDevelop::CompileAnalyzeJob * Analyzer::createJob(KDevelop::IProject* project,
                                                  const KDevelop::Path& buildDirectory,
                                                  const QUrl& url, const QStringList& filePaths)
{
    Q_UNUSED(url);

    ClangTidyProjectSettings projectSettings;
    projectSettings.setSharedConfig(project->projectConfiguration());
    projectSettings.load();

    Job::Parameters params;

    params.projectRootDir = project->path().toLocalFile();

    auto clangTidyPath = KDevelop::Path(ClangTidySettings::clangtidyPath()).toLocalFile();
    params.executablePath = clangTidyPath;

    params.filePaths = filePaths;
    params.buildDir = buildDirectory.toLocalFile();

    params.additionalParameters = projectSettings.additionalParameters();

    QString checkSetSelectionId = projectSettings.checkSetSelection();
    if (checkSetSelectionId == QLatin1String("Default")) {
        checkSetSelectionId = m_checkSetSelectionManager->defaultCheckSetSelectionId();
    }
    const auto enabledChecks = checkSetSelectionId.isEmpty() ? projectSettings.enabledChecks() : m_checkSetSelectionManager->checkSetSelection(checkSetSelectionId).selectionAsString();
    if (!enabledChecks.isEmpty()) {
        params.enabledChecks = enabledChecks;
    } else {
        auto& checkSet = m_plugin->checkSet();
        // make sure the defaults are up-to-date TODO: make async
        checkSet.setClangTidyPath(clangTidyPath);
        params.enabledChecks = checkSet.defaults().join(QLatin1Char(','));
    }
    params.useConfigFile = projectSettings.useConfigFile();
    params.headerFilter = projectSettings.headerFilter();
    params.checkSystemHeaders = projectSettings.checkSystemHeaders();

    params.parallelJobCount =
        ClangTidySettings::parallelJobsEnabled() ?
            (ClangTidySettings::parallelJobsAutoCount() ?
                QThread::idealThreadCount() :
                ClangTidySettings::parallelJobsFixedCount()) :
        1;

    return new Job(params, this);
}

}
