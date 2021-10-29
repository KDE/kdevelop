/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "analyzer.h"

// plugin
#include "plugin.h"
#include "job.h"
#include "checksetselectionmanager.h"
#include "globalsettings.h"
#include "projectsettings.h"
#include "checksdb.h"
// KDevPlatform
#include <interfaces/iproject.h>
#include <util/path.h>
// KF
#include <KLocalizedString>
// Qt
#include <QThread>

namespace Clazy
{

Analyzer::Analyzer(Plugin* plugin, CheckSetSelectionManager* checkSetSelectionManager, QObject* parent)
    : KDevelop::CompileAnalyzer(plugin,
                                i18n("Clazy"), QStringLiteral("clazy"),
                                QStringLiteral("clazy_file"), QStringLiteral("clazy_project"),
                                QStringLiteral("clazy"),
                                KDevelop::ProblemModel::CanDoFullUpdate |
                                KDevelop::ProblemModel::ScopeFilter |
                                KDevelop::ProblemModel::SeverityFilter |
                                KDevelop::ProblemModel::Grouping |
                                KDevelop::ProblemModel::CanByPassScopeFilter|
                                KDevelop::ProblemModel::ShowSource,
                                parent)
    , m_plugin(plugin)
    , m_checkSetSelectionManager(checkSetSelectionManager)
{
}

Analyzer::~Analyzer() = default;

bool Analyzer::isOutputToolViewPreferred() const
{
    return !GlobalSettings::hideOutputView();
}

KDevelop::CompileAnalyzeJob * Analyzer::createJob(KDevelop::IProject* project,
                                                  const KDevelop::Path& buildDirectory,
                                                  const QUrl& url, const QStringList& filePaths)
{
    ProjectSettings projectSettings;
    projectSettings.setSharedConfig(project->projectConfiguration());
    projectSettings.load();

    JobParameters params;

    params.executablePath = GlobalSettings::executablePath().toLocalFile();

    params.url = url;
    params.filePaths = filePaths;
    params.buildDir = buildDirectory.toLocalFile();
    QString checkSetSelectionId = projectSettings.checkSetSelection();
    if (checkSetSelectionId == QLatin1String("Default")) {
        checkSetSelectionId = m_checkSetSelectionManager->defaultCheckSetSelectionId();
    }
    const auto checks = checkSetSelectionId.isEmpty() ? projectSettings.checks() : m_checkSetSelectionManager->checkSetSelection(checkSetSelectionId).selectionAsString();
    if (!checks.isEmpty()) {
        params.checks = checks;
    } else {
        params.checks = ChecksDB::defaultChecks();
    }
    params.onlyQt = projectSettings.onlyQt();
    params.qtDeveloper = projectSettings.qtDeveloper();
    params.qt4Compat = projectSettings.qt4Compat();
    params.visitImplicitCode = projectSettings.visitImplicitCode();

    params.ignoreIncludedFiles = projectSettings.ignoreIncludedFiles();
    params.headerFilter = projectSettings.headerFilter();

    params.enableAllFixits = projectSettings.enableAllFixits();
    params.noInplaceFixits = projectSettings.noInplaceFixits();

    params.extraAppend = projectSettings.extraAppend();
    params.extraPrepend = projectSettings.extraPrepend();
    params.extraClazy = projectSettings.extraClazy();

    params.verboseOutput = GlobalSettings::verboseOutput();

    params.parallelJobCount =
        GlobalSettings::parallelJobsEnabled() ?
            (GlobalSettings::parallelJobsAutoCount() ?
                QThread::idealThreadCount() :
                GlobalSettings::parallelJobsFixedCount()) :
        1;

    auto db = m_plugin->loadedChecksDB();

    return new Job(params, db);
}

}
