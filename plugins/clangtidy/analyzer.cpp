/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
    SPDX-FileCopyrightText: 2018, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
