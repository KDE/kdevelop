/*
 * This file is part of KDevelop
 *
   Copyright 2018 Anton Anikin <anton@anikin.xyz>
 * Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>
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

Analyzer::Analyzer(Plugin* plugin, QObject* parent)
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
    const auto checks = projectSettings.checks();
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